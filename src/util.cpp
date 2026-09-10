#include "util.hpp"

#include <chrono>
#include <shlobj.h>
#include <string>
#include <windows.h>

namespace fs = std::filesystem;

namespace {
const auto mod_name = "HudController";
const auto data_path = fs::current_path() / "reframework/data";
} // namespace

namespace Util {

bool is_game_focused() {
    const HWND hwnd = GetForegroundWindow();

    if (hwnd == nullptr)
        return false;

    DWORD process_id{};
    GetWindowThreadProcessId(hwnd, &process_id);

    return process_id == GetCurrentProcessId();
}

bool is_path_valid(const fs::path &relative_path) {
    try {
        const auto canonical_path =
            fs::weakly_canonical(data_path / relative_path);

        const auto canonical_data = fs::weakly_canonical(data_path);

        if (canonical_path.string().find(canonical_data.string()) != 0)
            return false;

        return canonical_path.string().find(mod_name) != std::string::npos;
    } catch (const fs::filesystem_error &) {
        return false;
    }
}

bool remove_file(const char **path) {
    if (!path || !*path)
        return false;

    const fs::path relative_path = *path;

    if (!is_path_valid(relative_path))
        return false;

    auto full_path = fs::absolute(data_path / relative_path).wstring();

    full_path.push_back(L'\0');
    full_path.push_back(L'\0');

    SHFILEOPSTRUCTW file_op{};

    file_op.wFunc = FO_DELETE;
    file_op.pFrom = full_path.c_str();
    file_op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;

    return SHFileOperationW(&file_op) == 0;
}

bool rename_file(const char **old_path, const char **new_path) {
    if (!old_path || !new_path || !*old_path || !*new_path)
        return false;

    const fs::path old_relative_path = *old_path;
    const fs::path new_relative_path = *new_path;

    if (!is_path_valid(old_relative_path) || !is_path_valid(new_relative_path))
        return false;

    try {
        fs::rename(data_path / old_relative_path,
                   data_path / new_relative_path);
    } catch (const fs::filesystem_error &) {
        return false;
    }

    return true;
}

double now_us() {
    const auto now = std::chrono::high_resolution_clock::now();

    const auto us = std::chrono::duration_cast<std::chrono::microseconds>(
                        now.time_since_epoch())
                        .count();

    return static_cast<double>(us);
}

} // namespace Util