#include "reframework/API.hpp"
#include <chrono>
#include <filesystem>
#include <shlobj.h>
#include <sol/sol.hpp>
#include <windows.h>

using API = reframework::API;
namespace fs = std::filesystem;
lua_State *g_lua{nullptr};

const auto mod_name = "HudController";
const auto data_path = fs::current_path() / "reframework/data";

bool is_path_valid(const fs::path &relative_path) {
    try {
        auto canonical_path = fs::weakly_canonical(data_path / relative_path);
        auto canonical_data = fs::weakly_canonical(data_path);

        if (canonical_path.string().find(canonical_data.string()) != 0)
            return false;

        return canonical_path.string().find(mod_name) != std::string::npos;
    } catch (const fs::filesystem_error &) {
        return false;
    }
}

bool remove_file(const char *path_c) {
    if (!path_c)
        return false;

    fs::path path = path_c;
    if (!is_path_valid(path))
        return false;

    auto full_path = fs::absolute(data_path / path).wstring();

    full_path.push_back(L'\0');
    full_path.push_back(L'\0');

    SHFILEOPSTRUCTW fileOp = {0};
    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = full_path.c_str();
    fileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;

    return SHFileOperationW(&fileOp) == 0;
}

double now_us() {
    auto now = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(
                  now.time_since_epoch())
                  .count();
    return static_cast<double>(us);
}

bool rename_file(const char *old_path_c, const char *new_path_c) {
    if (!old_path_c || !new_path_c)
        return false;

    fs::path old_path = old_path_c;
    fs::path new_path = new_path_c;

    if (!is_path_valid(old_path) || !is_path_valid(new_path))
        return false;

    try {
        fs::rename(data_path / old_path, data_path / new_path);
    } catch (const fs::filesystem_error &) {
        return false;
    }

    return true;
}

void on_lua_state_created(lua_State *l) {
    API::LuaLock _{};
    g_lua = l;
    sol::state_view lua{g_lua};

    auto res = lua.create_table();
    res["rename"] = rename_file;
    res["remove"] = remove_file;
    res["now_us"] = now_us;
    lua["hudcontroller_util"] = res;
}

void on_lua_state_destroyed(lua_State *l) {
    API::LuaLock _{};
    g_lua = nullptr;
}

extern "C" __declspec(dllexport) bool
reframework_plugin_initialize(const REFrameworkPluginInitializeParam *param) {
    API::initialize(param);

    const auto functions = param->functions;
    functions->on_lua_state_created(on_lua_state_created);
    functions->on_lua_state_destroyed(on_lua_state_destroyed);

    return true;
}
