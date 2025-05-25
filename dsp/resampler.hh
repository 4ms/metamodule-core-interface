#pragma once
#include "util/fixed_vector.hh"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <span>

namespace MetaModule
{

class Resampler {
public:
	Resampler(uint32_t num_channels = 2) {
		num_chans = std::clamp<size_t>(num_channels, 1u, MAX_RESAMPLER_CHANNELS);

		chans.resize(num_chans, Channel{});

		input_stride = static_cast<uint8_t>(num_chans);
		output_stride = static_cast<uint8_t>(num_chans);
	}

	~Resampler() = default;

	int process(uint32_t channel_index, std::span<const float> &in, std::span<float> &out) {
		if (channel_index >= num_chans)
			return -1;

		Channel &chan = chans[channel_index];

		auto in_size = in.size() + channel_index;
		auto out_size = out.size() + channel_index;

		if (chan.ratio == 1.f) {
			// copy with strides
			unsigned i = 0;
			unsigned o = 0;
			for (; i < in_size && o < out_size; i += input_stride, o += output_stride) {
				out[o] = in[i];
			}
			out = out.subspan(0, o);
			in = in.subspan(0, i);
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

		if (outpos >= out_size)
			outpos = out_size - output_stride;
		if (inpos >= in_size)
			inpos = in_size - input_stride;
		out = out.subspan(0, outpos); // / output_stride);
		in = in.subspan(0, inpos);	  // / input_stride);

		return 0;
	}

	void set_sample_rate_in_out(uint32_t input_rate, uint32_t output_rate) {
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

	float ratio(unsigned chan) {
		return chans[chan].ratio;
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

// Helper class for using AudioResampler
template<size_t MaxChans, size_t MaxBlockSize, size_t MaxResampleRatio>
class ResamplingInterleavedBuffer {
public:
	ResamplingInterleavedBuffer() = default;

	std::span<float> process_block(unsigned num_chans, std::span<const float> input) {
		core.set_input_stride(num_chans);
		core.set_output_stride(num_chans);

		size_t output_size = 0;

		for (auto chan = 0u; chan < num_chans; chan++) {
			auto output = std::span<float>{out_buff};
			auto input_chan = input.subspan(chan);
			auto output_chan = output.subspan(chan);
			core.process(chan, input_chan, output_chan);

			output_size = output_chan.size();
		}

		return std::span<float>{out_buff.data(), output_size};
	}

	void set_sample_ate_in_out(uint32_t input_rate, uint32_t output_rate) {
		core.set_sample_rate_in_out(input_rate, output_rate);
	}

private:
	Resampler core{MaxChans};
	std::array<float, MaxBlockSize * MaxChans> out_buff;
};

} // namespace MetaModule
