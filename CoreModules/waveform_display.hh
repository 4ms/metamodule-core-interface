#pragma once
#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/elements/units.hh"
#include "thorvg/thorvg/inc/thorvg.h"
#include <algorithm>
#include <atomic>
#include <cmath>

namespace MetaModule
{

class StreamingWaveformDisplay {
public:
	StreamingWaveformDisplay(float display_width_mm, float display_height_mm)
		: display_width{mm_to_px(display_width_mm, 240)}
		, display_height{mm_to_px(display_height_mm, 240)}
		, wave_height{(display_height - bar_height) / 2.f} {

		samples.resize(display_width);
	}

	void set_cursor_position(float pos) {
		cursor_pos = pos;
	}

	// sample should range from -1 to +1
	float max = 0;
	void draw_sample(float sample) {
		max = std::max(std::abs(max), std::abs(sample)) * ((sample < 0) ? -1 : 1);
		if (++x_zoom_ctr > x_zoom) {
			x_zoom_ctr = 0;
			auto t = newest_sample.load();

			if (++t >= (int)samples.size())
				t = 0;
			samples[t] = max;
			max = 0;

			newest_sample.store(t);
		}
	}

	void sync() {
		std::ranges::fill(samples, 0);
		newest_sample = 0;
	}

	void set_x_zoom(unsigned zoom) {
		x_zoom = zoom;
	}

	void set_wave_color(uint8_t r, uint8_t g, uint8_t b) {
		wave_r = r;
		wave_g = g;
		wave_b = b;
	}
	void set_bar_color(uint8_t r, uint8_t g, uint8_t b) {
		bar_r = r;
		bar_g = g;
		bar_b = b;
	}

	// Functions below here run in the GUI thread and may get interrupted by the functions above

	void show_graphic_display(std::span<uint32_t> pix_buffer, unsigned width, lv_obj_t *lvgl_canvas) {
		canvas = tvg::SwCanvas::gen();
		auto scene = tvg::Scene::gen();

		auto scaled_height = pix_buffer.size() / width;
		canvas->target(pix_buffer.data(), width, width, scaled_height, tvg::ColorSpace::ARGB8888);
		scaling = float(width) / display_width;
		scene->scale(scaling);

		// Bar to represent entire sample
		auto bar = tvg::Shape::gen();
		bar->appendRect(0, display_height - bar_height, display_width, bar_height);
		bar->fill(bar_r, bar_g, bar_b, 0xFF);
		scene->push(bar);

		// Trolley to indicate position
		bar_cursor = tvg::Shape::gen();
		bar_cursor->appendRect(0, display_height - bar_height, cursor_width, bar_height);
		bar_cursor->fill(0xFF, 0xFF, 0xFF, 0xFF);
		scene->push(bar_cursor);

		// Black background
		auto wave_bg = tvg::Shape::gen();
		wave_bg->appendRect(0, 0, display_width, display_height - bar_height);
		wave_bg->fill(0x00, 0x00, 0x00, 0xFF);
		scene->push(wave_bg);

		// Waveform
		wave = tvg::Shape::gen();
		wave->moveTo(display_width, 0);
		wave->lineTo(0, 0);
		wave->strokeFill(wave_r, wave_g, wave_b, 0xFF);
		wave->strokeWidth(1.0f);
		wave->translate(0, scaling * (display_height - bar_height) / 2);
		scene->push(wave);

		canvas->push(scene);

		std::ranges::fill(samples, 0);
		newest_sample = 0;
	}

	bool draw_graphic_display() {
		if (!canvas)
			return false;

		bar_cursor->translate(cursor_pos * (display_width - cursor_width) * scaling, scaling > 0.9f ? 0 : -10);
		canvas->update(bar_cursor);

		wave->reset();

		//start with the oldest sample
		int i = newest_sample.load() + 1;

		for (auto x = 0u; x < samples.size(); x++) {
			if (x == 0)
				wave->moveTo(x, samples[i] * wave_height);
			else {
				wave->lineTo(x, samples[i] * wave_height);
				// Fill the waveform with vertical lines:
				wave->lineTo(x, 0);
				wave->lineTo(x, samples[i] * wave_height);
			}
			i = (i + 1) % samples.size();
		}
		wave->strokeFill(wave_r, wave_g, wave_b, 0xFF);
		canvas->update(wave);

		canvas->draw();
		canvas->sync();

		return true;
	}

	void hide_graphic_display() {
		if (canvas) {
			delete canvas;
			canvas = nullptr;
		}
	}

private:
	tvg::SwCanvas *canvas = nullptr;
	tvg::Shape *bar_cursor = nullptr;
	tvg::Shape *wave = nullptr;

	std::vector<float> samples;
	std::atomic<int> newest_sample = 0;

	float cursor_pos = 0;
	float cursor_width = 2;
	float bar_height = 5;

	unsigned x_zoom = 1;
	unsigned x_zoom_ctr = 0;

	uint8_t wave_r = 0, wave_g = 0xFF, wave_b = 0xFF;
	uint8_t bar_r = 0xF0, bar_g = 0x88, bar_b = 0x00;

	float scaling = 1;

	std::span<uint32_t> buffer;

	const float display_width;
	const float display_height;
	const float wave_height;
};
} // namespace MetaModule
