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
		this->cursor_pos = pos;
	}

	// sample should range from -1 to +1
	void draw_sample(float sample) {
		if (++x_zoom_ctr > x_zoom) {
			x_zoom_ctr = 0;
			auto t = newest_sample.load();

			if (++t >= (int)samples.size())
				t = 0;
			samples[t] = sample;

			newest_sample.store(t);
		}
	}

	void set_x_zoom(unsigned zoom) {
		x_zoom = zoom;
	}

	// Function below here run in the GUI thread and may get interrupted by the functions above

	void show_graphic_display(std::span<uint32_t> pix_buffer, unsigned width, lv_obj_t *lvgl_canvas) {
		canvas = tvg::SwCanvas::gen();
		auto scene = tvg::Scene::gen();

		auto scaled_height = pix_buffer.size() / width;
		canvas->target(pix_buffer.data(), width, width, scaled_height, tvg::ColorSpace::ARGB8888);
		scaling = float(width) / display_width;
		scene->scale(scaling);

		bar = tvg::Shape::gen();
		bar->appendRect(0, display_height - bar_height, display_width, bar_height);
		bar->fill(0xF0, 0x80, 0x00, 0xFF);
		scene->push(bar);

		bar_cursor = tvg::Shape::gen();
		bar_cursor->appendRect(0, display_height - bar_height, cursor_width, bar_height);
		bar_cursor->fill(0xFF, 0xFF, 0xFF, 0xFF);
		scene->push(bar_cursor);

		wave_bg = tvg::Shape::gen();
		wave_bg->appendRect(0, 0, display_width, display_height - bar_height);
		wave_bg->fill(0x00, 0x00, 0x00, 0xFF);
		scene->push(wave_bg);

		wave = tvg::Shape::gen();
		// wave->appendRect(0, -display_height / 2, display_width, display_height / 2);
		// wave->fill(0x00, 0x00, 0x00, 0xFF);
		wave->moveTo(display_width, 0);
		wave->lineTo(0, 0);
		wave->strokeFill(0xFF, 0xFF, 0, 0xFF);
		wave->strokeWidth(0.5f);
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
		// wave->appendRect(0, -wave_height, display_width, wave_height * 2);
		// wave->fill(0x00, 0x00, 0x00, 0xFF);

		float start = newest_sample.load();

		int i = start;
		// for (int x = samples.size() - 1; x >= 0; x--) {
		for (auto x = 0u; x < samples.size(); x++) {
			if (x == 0)
				wave->moveTo(x, samples[i] * wave_height);
			else
				wave->lineTo(x, samples[i] * wave_height);
			i = (i + 1) % samples.size();
		}
		wave->strokeFill(0xFF, 0xFF, 0, 0xFF);
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
	tvg::Shape *bar = nullptr;
	tvg::Shape *bar_cursor = nullptr;
	tvg::Shape *wave = nullptr;
	tvg::Shape *wave_bg = nullptr;

	std::vector<float> samples;
	std::atomic<int> newest_sample = 0;

	float cursor_pos = 0;
	float cursor_width = 10;
	float bar_height = 5;

	unsigned x_zoom = 1;
	unsigned x_zoom_ctr = 0;

	float scaling = 1;

	std::span<uint32_t> buffer;

	const float display_width;
	const float display_height;
	const float wave_height;
};
} // namespace MetaModule
