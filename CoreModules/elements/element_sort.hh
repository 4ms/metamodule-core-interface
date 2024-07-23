#pragma once
#include "CoreModules/elements/element_counter.hh"
#include "CoreModules/elements/elements.hh"
#include <vector>

namespace MetaModule
{

inline void
populate_sorted_elements_indices(std::span<std::pair<MetaModule::Element, ElementCount::Indices>> const elem_idx,
								 std::vector<MetaModule::Element> &elements,
								 std::vector<ElementCount::Indices> &indices) {

	std::sort(elem_idx.begin(), elem_idx.end(), [](auto const &a, auto const &b) {
		if (a.second.param_idx < b.second.param_idx)
			return true;
		else if (a.second.param_idx > b.second.param_idx)
			return false;

		else if (a.second.input_idx < b.second.input_idx)
			return true;
		else if (a.second.input_idx > b.second.input_idx)
			return false;

		else if (a.second.output_idx < b.second.output_idx)
			return true;
		else if (a.second.output_idx > b.second.output_idx)
			return false;

		else if (a.second.light_idx < b.second.light_idx)
			return true;

		else
			return false;
	});

	elements.clear();
	indices.clear();

	auto num_elems = elem_idx.size();
	elements.reserve(num_elems);
	indices.reserve(num_elems);

	for (auto &elemidx : elem_idx) {
		elements.push_back(elemidx.first);
		indices.push_back(elemidx.second);
	}
}

} // namespace MetaModule
