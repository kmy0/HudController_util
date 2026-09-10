#pragma once

#include <filesystem>

namespace Util {
bool is_game_focused();
bool is_path_valid(const std::filesystem::path &relative_path);
bool remove_file(const char **path);
bool rename_file(const char **old_path, const char **new_path);
double now_us();
} // namespace Util