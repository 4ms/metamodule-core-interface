#pragma once
#include "osdialog.h"
#include <functional>
#include <string>
#include <string_view>

void async_open_file(std::string_view initial_path,
					 std::string_view filter_extension_list,
					 std::string_view title,
					 std::function<void(char *path)> &&action);

void async_open_dir(std::string_view initial_path,
					std::string_view filter_extension_list,
					std::string_view title,
					std::function<void(char *path)> &&action);

void async_save_file(std::string_view initial_path,
					 std::string_view filename,
					 std::string_view title,
					 std::function<void(char *path)> &&action);

// Interface is close to rack's osdialog_file()

void async_osdialog_file(osdialog_file_action action,
						 const char *path,
						 const char *filename,
						 osdialog_filters *filters,
						 std::function<void(char *path)> &&action_function);

// saving: true if we are saving a file (not supported), false to open a file or dir.
// nameOrExtensions:
//      - If saving, this is the initial filename to save.
//      - If opening a dir, put "*/" here. This will only show directories and let you choose a dir.
//      - If opening a file, this is a comma-separated list of file extensions to show.
//        Example ".wav, .WAV, .raw"
//        If it's "" or nullptr, or it contains "*.*" then all files will be shown.
// startDir: What directory to initially show. If "" or nullptr, will show the list of mounted drives.
// title: text to display at the top
// action: a function with a char* parameter that will be called when the browser is closed.
//     If the user cancels, then the parameter will be nullptr
//
// Note: Interface matches Cardinal for ease of interoperability
void async_dialog_filebrowser(const bool saving,
							  const char *const nameOrExtensions,
							  const char *const startDir,
							  const char *const title,
							  const std::function<void(char *path)> action_function);

std::string stringify_osdialog_filters(osdialog_filters *filters);
