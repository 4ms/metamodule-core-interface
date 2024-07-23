#pragma once
#include "CoreModules/elements/element_counter.hh"
#include "CoreModules/elements/elements.hh"
#include <cstdint>

namespace MetaModule::ElementIndex
{

inline uint8_t get_index(const BaseElement &, ElementCount::Indices) {
	return 0; //error
}

inline uint8_t get_index(const ParamElement &, ElementCount::Indices indices) {
	return indices.param_idx;
}

inline uint8_t get_index(const LightElement &, ElementCount::Indices indices) {
	return indices.light_idx;
}

inline uint8_t get_index(const JackInput &, ElementCount::Indices indices) {
	return indices.input_idx;
}

inline uint8_t get_index(const JackOutput &, ElementCount::Indices indices) {
	return indices.output_idx;
}

struct SetIndex {
	uint8_t index;

	ElementCount::Indices operator()(const ParamElement &) {
		ElementCount::Indices indices = ElementCount::NoElementIndices;
		indices.param_idx = index;
		return indices;
	}

	inline ElementCount::Indices operator()(const LightElement &) {
		ElementCount::Indices indices = ElementCount::NoElementIndices;
		indices.light_idx = index;
		return indices;
	}

	inline ElementCount::Indices operator()(const JackInput &) {
		ElementCount::Indices indices = ElementCount::NoElementIndices;
		indices.input_idx = index;
		return indices;
	}

	inline ElementCount::Indices operator()(const JackOutput &) {
		ElementCount::Indices indices = ElementCount::NoElementIndices;
		indices.output_idx = index;
		return indices;
	}

	inline ElementCount::Indices operator()(const BaseElement &) {
		return ElementCount::NoElementIndices;
	}
};

inline ElementCount::Indices set_index(const Element &element, uint8_t idx) {
	return std::visit(MetaModule::ElementIndex::SetIndex{idx}, element);
}

}; // namespace MetaModule::ElementIndex
