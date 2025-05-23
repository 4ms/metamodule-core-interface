#pragma once
#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/elements/units.hh"
#include "thorvg/thorvg/inc/thorvg.h"

namespace MetaModule
{

class StreamingWaveformDisplay {
public:
	StreamingWaveformDisplay(float display_width_mm)
		: display_width_mm{display_width_mm} {
	}

	void set_waveform_length() {
	}

	void set_current_position(float pos) {
		this->pos = pos;
	}

	void show_graphic_display(std::span<uint32_t> pix_buffer, unsigned width, lv_obj_t *lvgl_canvas) {
		auto scene = tvg::Scene::gen();
		canvas = tvg::SwCanvas::gen();

		auto height = pix_buffer.size() / width;
		display_width_px = width;
		canvas->target(pix_buffer.data(), width, width, height, tvg::ColorSpace::ARGB8888);
		scaling = float(width) / mm_to_px(display_width_mm, 240);
		scene->scale(scaling);

		bar = tvg::Shape::gen();
		bar->appendRect(0, 0, width, 10);
		bar->fill(0xF0, 0x80, 0x00, 0xFF);
		scene->push(bar);

		bar_cursor = tvg::Shape::gen();
		bar_cursor->appendRect(0, 0, bar_width, 10);
		bar_cursor->fill(0xFF, 0xFF, 0xFF, 0xFF);
		scene->push(bar_cursor);

		canvas->push(scene);

		printf("Show display: scaling %f (px: %u)\n", scaling, width);
	}

	bool draw_graphic_display() {
		if (!canvas)
			return false;

		bar_cursor->translate(pos * (display_width_px - bar_width), 0);
		canvas->update(bar_cursor);
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
	// tvg::Shape *wave = nullptr;

	float scaling = 1.f;

	float pos = 0;
	float bar_width = 10;
	std::span<uint32_t> buffer;
	float display_width_mm = 0;
	unsigned display_width_px = 0;
};
} // namespace MetaModule
