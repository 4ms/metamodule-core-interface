#pragma once
#include "util/fixed_vector.hh"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <span>

namespace MetaModule
{

class AudioResampler {
public:
	AudioResampler(uint32_t num_channels) {
		num_chans = std::clamp<size_t>(num_channels, 1u, MAX_RESAMPLER_CHANNELS);

		chans.resize(num_chans, Channel{});

		input_stride = static_cast<uint8_t>(num_chans);
		output_stride = static_cast<uint8_t>(num_chans);
	}

	~AudioResampler() = default;

	int process(uint32_t channel_index, std::span<const float> &in, std::span<float> &out) {
		if (channel_index >= num_chans)
			return -1;

		Channel &chan = chans[channel_index];

		uint32_t in_size = in.size() * input_stride;
		uint32_t out_size = out.size() * output_stride;

		if (chan.ratio == 1.f) {
			//copy
			uint32_t len = std::min(in.size(), out.size());
			in = in.subspan(0, len);
			out = out.subspan(0, len);
			std::copy(in.begin(), in.end(), out.begin());
			return 0;
		}

		uint32_t inpos = 0;

		auto get_next_in = [&](float &next) {
			if (inpos < in_size) {
				next = in[inpos];
				inpos += input_stride;
			}
		};

		if (chan.flush && in_size >= 3) {
			chan.flush = false;
			chan.xm1 = 0;
			get_next_in(chan.x0);
			get_next_in(chan.x1);
			get_next_in(chan.x2);
			chan.fractional_pos = 0.0;
		}

		uint32_t outpos = 0;

		while (outpos < out_size && inpos < in_size) {

			// Optimize for resample rates >= 2
			if (chan.fractional_pos >= 2.f && (inpos + 2) <= in_size) {
				chan.fractional_pos = chan.fractional_pos - 2.f;

				// shift samples back two
				// and read a new sample
				chan.xm1 = chan.x1;
				chan.x0 = chan.x2;
				get_next_in(chan.x1);
				get_next_in(chan.x2);
			}

			// Optimize for resample rates >= 1
			if (chan.fractional_pos >= 1.f && (inpos + 1) <= in_size) {
				chan.fractional_pos = chan.fractional_pos - 1.f;

				// shift samples back one
				// and read a new sample
				chan.xm1 = chan.x0;
				chan.x0 = chan.x1;
				chan.x1 = chan.x2;
				get_next_in(chan.x2);
			}

			// calculate coefficients
			float a = (3 * (chan.x0 - chan.x1) - chan.xm1 + chan.x2) / 2;
			float b = 2 * chan.x1 + chan.xm1 - (5 * chan.x0 + chan.x2) / 2;
			float c = (chan.x1 - chan.xm1) / 2;

			// calculate as many fractionally placed output points as we need
			while (chan.fractional_pos < 1.f && outpos < out_size) {
				out[outpos] =
					(((a * chan.fractional_pos) + b) * chan.fractional_pos + c) * chan.fractional_pos + chan.x0;
				outpos += output_stride;

				chan.fractional_pos += chan.ratio;
			}
		}

		out = out.subspan(0, outpos / output_stride);
		in = in.subspan(0, inpos / input_stride);

		return 0;
	}

	void set_sample_rate_out(uint32_t input_rate, uint32_t output_rate) {
		for (auto i = 0u; i < num_chans; i++) {
			chans[i].ratio = (float)input_rate / (float)output_rate;
		}
	}

	void set_input_stride(uint32_t stride) {
		input_stride = stride;
	}

	void set_output_stride(uint32_t stride) {
		output_stride = stride;
	}

private:
	struct Channel {
		float ratio = 1;
		bool flush{true};

		float fractional_pos{};
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
