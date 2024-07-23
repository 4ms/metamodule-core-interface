#pragma once
#include "CoreProcessor.hh"
#include "elements/element_info_view.hh"
#include <functional>
#include <memory>

namespace MetaModule
{

using CreateModuleFunc = std::function<std::unique_ptr<CoreProcessor>()>;

bool register_module(std::string_view brand_name,
					 std::string_view typeslug,
					 CreateModuleFunc funcCreate,
					 ModuleInfoView const &info,
					 std::string_view faceplate_filename);

} // namespace MetaModule
