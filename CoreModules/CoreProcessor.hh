#pragma once
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

class CoreProcessor {
public:
	CoreProcessor() = default;

	virtual void update() = 0;

	virtual void set_samplerate(float sr) = 0;
	virtual void set_param(int param_id, float val) = 0;
	virtual void set_input(int input_id, float val) = 0;

	virtual float get_output(int output_id) const = 0;
	virtual float get_led_brightness(int led_id) const {
		return 0;
	}
	virtual size_t get_display_text(int display_id, std::span<char> text) {
		return 0;
	}
	virtual float get_param(int param_id) const {
		return 0;
	}

	virtual void mark_all_inputs_unpatched() {
	}
	virtual void mark_input_unpatched(int input_id) {
	}
	virtual void mark_input_patched(int input_id) {
	}
	virtual void mark_all_outputs_unpatched() {
	}
	virtual void mark_output_unpatched(int output_id) {
	}
	virtual void mark_output_patched(int output_id) {
	}

	virtual void load_state(std::string_view state_data) {
	}
	virtual std::string save_state() {
		return "";
	}

	virtual ~CoreProcessor() = default;

	bool bypassed = false;

	// common default values, OK to override or ignore
	static constexpr float CvRangeVolts = 5.0f;
	static constexpr float MaxOutputVolts = 8.0f;
	static constexpr unsigned NameChars = 15;
	static constexpr unsigned LongNameChars = 39;
};
