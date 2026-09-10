---@meta

---@class (exact) HudControllerUtil
---@field remove fun(path: string): boolean
---@field rename fun(old_path: string, new_path: string): boolean
---@field now_us fun(): number
---@field reset_input fun()
---@field get_input_chars fun(): string
---@field is_game_focused fun(): boolean

---@class HudControllerUtil
hudcontroller_util = {}
