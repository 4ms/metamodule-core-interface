#pragma once
#include <functional>

// Interface matches Cardinal for ease of interoperability

void async_dialog_filebrowser(const bool saving,
							  const char *const nameOrExtensions,
							  const char *const startDir,
							  const char *const title,
							  const std::function<void(char *path)> action);
