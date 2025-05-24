#pragma once
#include "util/circular_buffer.hh"
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
	AudioResampler(uint32_t num_channels = 2) {
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

		if (chan.ratio == 1.f) {
			// copy with strides
			auto j = 0;
			for (auto i = 0u, o = 0u; i < in.size() && o < out.size(); i += input_stride, o += output_stride) {
				out[o] = in[i];
				j++;
			}
			return 0;
		}

		uint32_t inpos = 0;

		auto get_next_in = [&](float &next) {
			if (inpos < in.size()) {
				next = in[inpos];
				inpos += input_stride;
			}
		};

		if (chan.flush && in.size() >= 3) {
			chan.flush = false;
			chan.xm1 = 0;
			get_next_in(chan.x0);
			get_next_in(chan.x1);
			get_next_in(chan.x2);
			chan.fractional_pos = 0.0;
		}

		uint32_t outpos = 0;

		while (outpos < out.size() && inpos < in.size()) {

			// Optimize for resample rates >= 2
			if (chan.fractional_pos >= 2.f && (inpos + 2) <= in.size()) {
				chan.fractional_pos = chan.fractional_pos - 2.f;

				// shift samples back two
				// and read a new sample
				chan.xm1 = chan.x1;
				chan.x0 = chan.x2;
				get_next_in(chan.x1);
				get_next_in(chan.x2);
			}

			// Optimize for resample rates >= 1
			if (chan.fractional_pos >= 1.f && (inpos + 1) <= in.size()) {
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
			while (chan.fractional_pos < 1.f && outpos < out.size()) {
				out[outpos] =
					(((a * chan.fractional_pos) + b) * chan.fractional_pos + c) * chan.fractional_pos + chan.x0;
				outpos += output_stride;

				chan.fractional_pos += chan.ratio;
			}
		}

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

// block size * maximum conversion ratio
template<size_t NumChans, size_t MaxBlockSize>
class ResamplingRingBuffer {
public:
	ResamplingRingBuffer() = default;

	void push(unsigned chan, std::span<const float> input) {
		printf("rs: %f, [%u] push %zu\n", core.ratio(chan), chan, input.size());

		// out_buff[chan].clear();
		// bool ok = out_buff[chan].resize_for_overwrite(input.size() / core.ratio(chan));
		// printf("req %zu ", out_buff[chan].size());
		// if (!ok)
		// 	printf("Error: resampling output buffer too small: need %f\n", input.size() / core.ratio(chan));

		// auto output = out_buff[chan].span();

		read_size = input.size() / core.ratio(chan);
		auto output = std::span{&out_buff[chan], read_size};

		auto err = core.process(chan, input, output);

		if (err < 0)
			printf("Error: resampling failed\n");

		for (auto i = 0u; i < read_size; i += NumChans) {
			if (output[i] != input[i])
				printf("%f != %f\n", output[i], input[i]);
		}

		read_size = output.size() * NumChans;
		read_idx = 0;
		printf("got %zu\n", output.size());
	}

	float pop() {
		if (read_idx <= read_size)
			return out_buff[read_idx++];
		else {
			printf("%zu >= %zu\n", read_idx, read_size);
			return 0;
		}
	}

	void set_sample_rate_in_out(uint32_t input_rate, uint32_t output_rate) {
		core.set_sample_rate_in_out(input_rate, output_rate);
	}

private:
	AudioResampler core{NumChans};
	// std::array<FixedVector<float, MaxBlockSize>, NumChans> out_buff;
	std::array<float, MaxBlockSize * NumChans> out_buff;

	size_t read_idx{};
	size_t read_size{};
};

// Usage:
// if (resampler.needs_input()) {
// 	   left = resampler.process(0, stream.pop_sample());
// 	   right = stream.is_stereo() ? resampler.process(1, stream.pop_sample()) : left;
// } else {
//     left = resampler.get_next(0);
//     right = stream.is_stere() ? resampler.get_next(1) : left;
// }
// template<size_t NumChans, size_t MaxBlockSize>
// class ResamplingBuffer {

// 	ResamplingBuffer() = default;

// 	float process(unsigned chan, float input) {
// 		if (core.ratio(chan) == 1.f)
// 			return input;
// 		else {
// 			// TODO:

// 			return 0;
// 		}
// 	}

// 	float get_next(unsigned chan) {
// 		return out_buff[chan].pop_front();
// 	}

// 	void set_sample_rate_in_out(uint32_t input_rate, uint32_t output_rate) {
// 		core.set_sample_rate_in_out(input_rate, output_rate);
// 	}

// 	AudioResampler core{NumChans};
// 	std::array<FixedVector<float, MaxBlockSize>, NumChans> in_buff;
// 	std::array<FixedVector<float, MaxBlockSize>, NumChans> out_buff;
// };

} // namespace MetaModule
