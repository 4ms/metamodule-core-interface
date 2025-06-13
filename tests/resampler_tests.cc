#include "doctest.h"
#include "dsp/resampler_one.hh"

TEST_CASE("resample sawtooth with 8 samples") {
	std::array<float, 8> in{0, 1, 2, 3, 4, 5, 6, 7};

	unsigned pos = 0;
	auto get_input = [&] {
		auto next = in[pos];
		pos++;
		if (pos >= in.size())
			pos = 0;
		return next;
	};

	SUBCASE("resampler at 1") {
		MetaModule::ResamplerMono res{};

		CHECK(res.process(get_input) == 0);
		CHECK(res.process(get_input) == 1);
		CHECK(res.process(get_input) == 2);
		CHECK(res.process(get_input) == 3);
		CHECK(res.process(get_input) == 4);
		CHECK(res.process(get_input) == 5);
		CHECK(res.process(get_input) == 6);
		CHECK(res.process(get_input) == 7);
		CHECK(res.process(get_input) == doctest::Approx(0));
	}

	SUBCASE("upsample by 2") {
		MetaModule::ResamplerMono res;
		res.set_sample_rate_in_out(24000, 48000);

		CHECK(res.process(get_input) == doctest::Approx(0));
		CHECK(res.process(get_input) == doctest::Approx(0.4375)); // why?
		CHECK(res.process(get_input) == doctest::Approx(1.0));
		CHECK(res.process(get_input) == doctest::Approx(1.5));
		CHECK(res.process(get_input) == doctest::Approx(2.0));
		CHECK(res.process(get_input) == doctest::Approx(2.5));
		CHECK(res.process(get_input) == doctest::Approx(3.0));
		CHECK(res.process(get_input) == doctest::Approx(3.5));
		CHECK(res.process(get_input) == doctest::Approx(4.0));
		CHECK(res.process(get_input) == doctest::Approx(4.5));
		CHECK(res.process(get_input) == doctest::Approx(5.0));
		CHECK(res.process(get_input) == doctest::Approx(5.5));
		CHECK(res.process(get_input) == doctest::Approx(6.0));
		CHECK(res.process(get_input) == doctest::Approx(7.0));
		CHECK(res.process(get_input) == doctest::Approx(7.0));
		CHECK(res.process(get_input) == doctest::Approx(3.5));
		CHECK(res.process(get_input) == doctest::Approx(0.0));
		CHECK(res.process(get_input) == doctest::Approx(0.0));
		CHECK(res.process(get_input) == doctest::Approx(1.0));
		CHECK(res.process(get_input) == doctest::Approx(1.5));
		CHECK(res.process(get_input) == doctest::Approx(2.0));
	}

	SUBCASE("upsample by 1.33") {
		MetaModule::ResamplerMono res;
		res.set_sample_rate_in_out(36000, 48000);

		CHECK(res.process(get_input) == doctest::Approx(0.0));
		CHECK(res.process(get_input) == doctest::Approx(0.726562).epsilon(0.01));
		CHECK(res.process(get_input) == doctest::Approx(1.5));
		CHECK(res.process(get_input) == doctest::Approx(2.25));
		CHECK(res.process(get_input) == doctest::Approx(3.00));
		CHECK(res.process(get_input) == doctest::Approx(3.75));
		CHECK(res.process(get_input) == doctest::Approx(4.50));
		CHECK(res.process(get_input) == doctest::Approx(5.25));
		CHECK(res.process(get_input) == doctest::Approx(6.00));
		CHECK(res.process(get_input) == doctest::Approx(7.3125));
		CHECK(res.process(get_input) == doctest::Approx(3.50));
		CHECK(res.process(get_input) == doctest::Approx(-0.31250));
		CHECK(res.process(get_input) == doctest::Approx(1.00));
		CHECK(res.process(get_input) == doctest::Approx(1.75));
		CHECK(res.process(get_input) == doctest::Approx(2.50));
		CHECK(res.process(get_input) == doctest::Approx(3.25));
	}

	SUBCASE("downsample by 2") {
		MetaModule::ResamplerMono res;
		res.set_sample_rate_in_out(48000, 24000);

		CHECK(res.process(get_input) == doctest::Approx(0));
		CHECK(res.process(get_input) == doctest::Approx(2));
		CHECK(res.process(get_input) == doctest::Approx(4));
		CHECK(res.process(get_input) == doctest::Approx(6));

		CHECK(res.process(get_input) == doctest::Approx(0));
		CHECK(res.process(get_input) == doctest::Approx(2));
		CHECK(res.process(get_input) == doctest::Approx(4));
		CHECK(res.process(get_input) == doctest::Approx(6));

		CHECK(res.process(get_input) == doctest::Approx(0));
		CHECK(res.process(get_input) == doctest::Approx(2));
		CHECK(res.process(get_input) == doctest::Approx(4));
		CHECK(res.process(get_input) == doctest::Approx(6));
	}

	SUBCASE("downsample by 1.333") {
		MetaModule::ResamplerMono res;
		res.set_sample_rate_in_out(48000, 36000);

		CHECK(res.process(get_input) == doctest::Approx(0));
		CHECK(res.process(get_input) == doctest::Approx(1.33333));
		CHECK(res.process(get_input) == doctest::Approx(2.66666));
		CHECK(res.process(get_input) == doctest::Approx(4.0));
		CHECK(res.process(get_input) == doctest::Approx(5.33333));
		CHECK(res.process(get_input) == doctest::Approx(7.25926).epsilon(0.01));

		CHECK(res.process(get_input) == doctest::Approx(0).epsilon(0.001));
		CHECK(res.process(get_input) == doctest::Approx(1.33333));
		CHECK(res.process(get_input) == doctest::Approx(2.66666));
		CHECK(res.process(get_input) == doctest::Approx(4.0));
		CHECK(res.process(get_input) == doctest::Approx(5.33333));
		CHECK(res.process(get_input) == doctest::Approx(7.25926).epsilon(0.01));
	}
}
