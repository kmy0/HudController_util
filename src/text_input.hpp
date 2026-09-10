#pragma once

#include <array>
#include <chrono>
#include <string>
#include <windows.h>

class TextInput {
  public:
    std::string take();
    void reset();

  private:
    using Clock = std::chrono::steady_clock;

    struct KeyState {
        bool down{};
        Clock::time_point next_repeat{};
    };

    std::array<KeyState, 256> m_keys{};
    std::string m_text{};

    static constexpr auto repeat_delay = std::chrono::milliseconds(400);

    static constexpr auto repeat_interval = std::chrono::milliseconds(35);

    void poll();
    void process_key(UINT vk);
    void append_utf8(const wchar_t *chars, int count);
    void snapshot_keys();

    static bool is_ignored_key(UINT vk);
    static bool is_down(UINT vk);
};