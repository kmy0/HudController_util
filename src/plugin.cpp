#include "reframework/API.hpp"

#include "text_input.hpp"
#include "util.hpp"

#include <sol/sol.hpp>

using API = reframework::API;

static lua_State *g_lua{nullptr};
static TextInput g_text_input{};

void on_lua_state_created(lua_State *l) {
    API::LuaLock _{};

    g_lua = l;

    sol::state_view lua{g_lua};

    auto res = lua.create_table();

    res["rename"] = Util::rename_file;
    res["remove"] = Util::remove_file;
    res["now_us"] = Util::now_us;
    res["is_game_focused"] = Util::is_game_focused;

    res["get_input_chars"] = []() -> std::string {
        return g_text_input.take();
    };
    res["reset_input"] = []() { g_text_input.reset(); };

    lua["hudcontroller_util"] = res;
}

void on_lua_state_destroyed(lua_State *) {
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