#pragma once
#include "CoreProcessor.hh"
#include "elements/element_info_view.hh"
#include <functional>
#include <memory>

namespace MetaModule {

using CreateModuleFunc = std::function<std::unique_ptr<CoreProcessor>()>;

bool register_module(std::string_view brand_slug, std::string_view module_slug,
                     CreateModuleFunc funcCreate, ModuleInfoView const &info,
                     std::string_view faceplate_filename);

template <typename ModuleT>
bool register_module(std::string_view brand_name, std::string_view module_slug,
                     ModuleInfoView const &info,
                     std::string_view faceplate_filename) {
  return register_module(
      brand_name, module_slug, []() { return std::make_unique<ModuleT>(); },
      info, faceplate_filename);
}

template <typename ModuleT, typename ModuleInfoT>
bool register_module(std::string_view brand_name, std::string_view module_slug,
                     std::string_view faceplate_filename) {
  return register_module(
      brand_name, module_slug, []() { return std::make_unique<ModuleT>(); },
      ModuleInfoView::makeView<ModuleInfoT>(), faceplate_filename);
}

template <typename ModuleT, typename ModuleInfoT>
bool register_module(std::string_view brand_name) {
  return register_module(
      brand_name, ModuleInfoT::slug,
      []() { return std::make_unique<ModuleT>(); },
      ModuleInfoView::makeView<ModuleInfoT>(), ModuleInfoT::png_filename);
}

} // namespace MetaModule
