#include "text_input.hpp"
#include "util.hpp"

std::string TextInput::take() {
    poll();

    std::string result = std::move(m_text);
    m_text.clear();

    return result;
}

void TextInput::reset() {
    m_text.clear();
    snapshot_keys();
}

bool TextInput::is_down(UINT vk) {
    return (GetAsyncKeyState(static_cast<int>(vk)) & 0x8000) != 0;
}

void TextInput::snapshot_keys() {
    const auto now = Clock::now();

    for (UINT vk = 0; vk < 256; ++vk) {
        m_keys[vk].down = is_down(vk);
        m_keys[vk].next_repeat = now + repeat_delay;
    }
}

bool TextInput::is_ignored_key(UINT vk) {
    switch (vk) {
    case VK_BACK:
    case VK_TAB:
    case VK_RETURN:
    case VK_ESCAPE:
    case VK_DELETE:
    case VK_INSERT:
    case VK_HOME:
    case VK_END:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
    case VK_DOWN:
    case VK_SHIFT:
    case VK_LSHIFT:
    case VK_RSHIFT:
    case VK_CONTROL:
    case VK_LCONTROL:
    case VK_RCONTROL:
    case VK_MENU:
    case VK_LMENU:
    case VK_RMENU:
    case VK_LWIN:
    case VK_RWIN:
    case VK_CAPITAL:
    case VK_NUMLOCK:
    case VK_SCROLL:
    case VK_F1:
    case VK_F2:
    case VK_F3:
    case VK_F4:
    case VK_F5:
    case VK_F6:
    case VK_F7:
    case VK_F8:
    case VK_F9:
    case VK_F10:
    case VK_F11:
    case VK_F12:
    case VK_F13:
    case VK_F14:
    case VK_F15:
    case VK_F16:
    case VK_F17:
    case VK_F18:
    case VK_F19:
    case VK_F20:
    case VK_F21:
    case VK_F22:
    case VK_F23:
    case VK_F24:
        return true;

    default:
        return false;
    }
}

void TextInput::poll() {
    if (!Util::is_game_focused()) {
        snapshot_keys();
        return;
    }

    const auto now = Clock::now();

    const bool ctrl_down =
        is_down(VK_CONTROL) || is_down(VK_LCONTROL) || is_down(VK_RCONTROL);
    const bool alt_down =
        is_down(VK_MENU) || is_down(VK_LMENU) || is_down(VK_RMENU);
    const bool win_down = is_down(VK_LWIN) || is_down(VK_RWIN);

    // Ignore shortcut chords such as Ctrl+V/C/X/A, Alt shortcuts,
    // and Windows-key shortcuts. Shift remains allowed for normal text.
    const bool shortcut_modifier_down = ctrl_down || alt_down || win_down;

    for (UINT vk = 0; vk < 256; ++vk) {
        const bool down = is_down(vk);
        auto &key = m_keys[vk];

        if (down) {
            if (!key.down) {
                if (!is_ignored_key(vk) && !shortcut_modifier_down)
                    process_key(vk);

                key.next_repeat = now + repeat_delay;
            } else if (!is_ignored_key(vk) && !shortcut_modifier_down &&
                       now >= key.next_repeat) {
                process_key(vk);

                do {
                    key.next_repeat += repeat_interval;
                } while (key.next_repeat <= now);
            }
        }

        key.down = down;
    }
}

void TextInput::process_key(UINT vk) {
    BYTE state[256]{};

    GetKeyboardState(state);

    for (UINT i = 0; i < 256; ++i) {
        if (is_down(i))
            state[i] |= 0x80;
        else
            state[i] &= ~0x80;
    }

    if (GetKeyState(VK_CAPITAL) & 1)
        state[VK_CAPITAL] |= 1;
    else
        state[VK_CAPITAL] &= ~1;

    if (GetKeyState(VK_NUMLOCK) & 1)
        state[VK_NUMLOCK] |= 1;
    else
        state[VK_NUMLOCK] &= ~1;

    if (GetKeyState(VK_SCROLL) & 1)
        state[VK_SCROLL] |= 1;
    else
        state[VK_SCROLL] &= ~1;

    const HKL layout = GetKeyboardLayout(0);

    const UINT scan_code = MapVirtualKeyExW(vk, MAPVK_VK_TO_VSC, layout);

    wchar_t chars[16]{};

    const int count = ToUnicodeEx(
        vk, scan_code, state, chars,
        static_cast<int>(sizeof(chars) / sizeof(chars[0])), 0, layout);

    if (count > 0)
        append_utf8(chars, count);
}

void TextInput::append_utf8(const wchar_t *chars, int count) {
    if (!chars || count <= 0)
        return;

    std::wstring filtered;
    filtered.reserve(static_cast<size_t>(count));

    for (int i = 0; i < count; ++i) {
        const wchar_t c = chars[i];

        if (c >= 0x20 && c != 0x7F)
            filtered.push_back(c);
    }

    if (filtered.empty())
        return;

    const int size = WideCharToMultiByte(CP_UTF8, 0, filtered.data(),
                                         static_cast<int>(filtered.size()),
                                         nullptr, 0, nullptr, nullptr);

    if (size <= 0)
        return;

    const size_t offset = m_text.size();

    m_text.resize(offset + static_cast<size_t>(size));

    WideCharToMultiByte(CP_UTF8, 0, filtered.data(),
                        static_cast<int>(filtered.size()),
                        m_text.data() + offset, size, nullptr, nullptr);
}