#pragma once
#include "util/fixed_vector.hh"
#include <algorithm>
#include <cstdint>

namespace MetaModule
{

class ResamplerMono {
public:
	// get_input() returns float, and is only called when needed
	// todo: before calling this, we need to know minimum samples required
	float process(auto get_input) {
		if (chan.ratio == 1.f) {
			return get_input();
		}

		if (chan.flush) {
			chan.flush = false;
			chan.xm1 = 0;
			chan.x0 = get_input();
			chan.x1 = get_input();
			chan.x2 = get_input();
			chan.frac_pos = 0.0;
		}

		while (chan.frac_pos >= 1.f) {
			// Optimize for resample rates >= 3
			if (chan.frac_pos >= 3.f) {
				chan.frac_pos = chan.frac_pos - 3.f;
				chan.xm1 = chan.x1;
				chan.x0 = get_input();
				chan.x1 = get_input();
				chan.x2 = get_input();
			}

			// Optimize for resample rates >= 2
			if (chan.frac_pos >= 2.f) {
				chan.frac_pos = chan.frac_pos - 2.f;
				chan.xm1 = chan.x1;
				chan.x0 = chan.x2;
				chan.x1 = get_input();
				chan.x2 = get_input();
			}

			// Optimize for resample rates >= 1
			if (chan.frac_pos >= 1.f) {
				chan.frac_pos = chan.frac_pos - 1.f;

				// shift samples back one
				// and read a new sample
				chan.xm1 = chan.x0;
				chan.x0 = chan.x1;
				chan.x1 = chan.x2;
				chan.x2 = get_input();
			}
		}

		// calculate coefficients
		float a = (3 * (chan.x0 - chan.x1) - chan.xm1 + chan.x2) / 2;
		float b = 2 * chan.x1 + chan.xm1 - (5 * chan.x0 + chan.x2) / 2;
		float c = (chan.x1 - chan.xm1) / 2;

		auto out = (((a * chan.frac_pos) + b) * chan.frac_pos + c) * chan.frac_pos + chan.x0;
		chan.frac_pos += chan.ratio;
		return out;
	}

	void set_sample_rate_in_out(uint32_t input_rate, uint32_t output_rate) {
		auto cur_ratio = (float)input_rate / (float)output_rate;

		if (chan.ratio != cur_ratio) {
			chan.ratio = cur_ratio;
			chan.flush = true;
		}
	}

	float ratio() {
		return chan.ratio;
	}

	void flush() {
		chan.flush = true;
	}

private:
	struct Channel {
		float ratio = 1;
		bool flush{true};

		float frac_pos{};
		float xm1{};
		float x0{};
		float x1{};
		float x2{};
	};

	Channel chan{};
};

class Resampler {
public:
	Resampler(uint32_t num_channels = 2) {
		num_chans = std::clamp<size_t>(num_channels, 1u, MAX_RESAMPLER_CHANNELS);

		chans.resize(num_chans, Channel{});

		input_stride = static_cast<uint8_t>(num_chans);
		output_stride = static_cast<uint8_t>(num_chans);
	}

	~Resampler() = default;

	// get_input() returns float, and is only called when needed
	// todo: before calling this, we need to know minimum samples required
	float process(uint32_t channel_index, auto get_input) {
		if (channel_index >= num_chans)
			return 0;

		Channel &chan = chans[channel_index];

		if (chan.ratio == 1.f) {
			return get_input();
		}

		if (chan.flush) {
			chan.flush = false;
			chan.xm1 = 0;
			chan.x0 = get_input();
			chan.x1 = get_input();
			chan.x2 = get_input();
			chan.frac_pos = 0.0;
		}

		while (chan.frac_pos >= 1.f) {
			// Optimize for resample rates >= 3
			if (chan.frac_pos >= 3.f) {
				chan.frac_pos = chan.frac_pos - 3.f;
				chan.xm1 = chan.x1;
				chan.x0 = get_input();
				chan.x1 = get_input();
				chan.x2 = get_input();
			}

			// Optimize for resample rates >= 2
			if (chan.frac_pos >= 2.f) {
				chan.frac_pos = chan.frac_pos - 2.f;
				chan.xm1 = chan.x1;
				chan.x0 = chan.x2;
				chan.x1 = get_input();
				chan.x2 = get_input();
			}

			// Optimize for resample rates >= 1
			if (chan.frac_pos >= 1.f) {
				chan.frac_pos = chan.frac_pos - 1.f;

				// shift samples back one
				// and read a new sample
				chan.xm1 = chan.x0;
				chan.x0 = chan.x1;
				chan.x1 = chan.x2;
				chan.x2 = get_input();
			}
		}

		// calculate coefficients
		float a = (3 * (chan.x0 - chan.x1) - chan.xm1 + chan.x2) / 2;
		float b = 2 * chan.x1 + chan.xm1 - (5 * chan.x0 + chan.x2) / 2;
		float c = (chan.x1 - chan.xm1) / 2;

		auto out = (((a * chan.frac_pos) + b) * chan.frac_pos + c) * chan.frac_pos + chan.x0;
		chan.frac_pos += chan.ratio;
		return out;
	}

	void set_sample_rate_in_out(uint32_t input_rate, uint32_t output_rate) {
		for (auto &chan : chans) {
			auto cur_ratio = (float)input_rate / (float)output_rate;

			if (chan.ratio != cur_ratio) {
				chan.ratio = cur_ratio;
				chan.flush = true;
			}
		}
	}

	void set_input_stride(uint32_t stride) {
		input_stride = stride;
	}

	void set_output_stride(uint32_t stride) {
		output_stride = stride;
	}

	float ratio(unsigned chan) {
		return chans[chan].ratio;
	}

	void flush() {
		for (auto &chan : chans) {
			chan.flush = true;
		}
	}

private:
	struct Channel {
		float ratio = 1;
		bool flush{true};

		float frac_pos{};
		float xm1{};
		float x0{};
		float x1{};
		float x2{};
	};

	static constexpr size_t MAX_RESAMPLER_CHANNELS = 16;
	FixedVector<Channel, MAX_RESAMPLER_CHANNELS> chans;

	unsigned input_stride = 1;
	unsigned output_stride = 1;
	size_t num_chans = 0;
};

} // namespace MetaModule
