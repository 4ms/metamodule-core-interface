#pragma once
#include <functional>

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
							  const std::function<void(char *path)> action);
