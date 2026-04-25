#include "resize_hook.hpp"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace
{
    std::function<void()> g_onRender;
    WNDPROC g_prevWndProc = nullptr;

    auto CALLBACK resizeWndProc(HWND h, UINT msg, WPARAM wp, LPARAM lp) -> LRESULT
    {
        const auto result = CallWindowProcW(g_prevWndProc, h, msg, wp, lp);
        switch (msg)
        {
            case WM_ENTERSIZEMOVE:
                SetTimer(h, 1, USER_TIMER_MINIMUM, nullptr);
                break;
            case WM_EXITSIZEMOVE:
                KillTimer(h, 1);
                break;
            case WM_SIZE:
            case WM_TIMER:
            case WM_PAINT:
                if (g_onRender)
                {
                    g_onRender();
                }
                break;
            default:
                break;
        }
        return result;
    }
} // namespace

namespace caerwyn
{
    auto installResizeRedrawHook(void* nativeWindowHandle, std::function<void()> onRender) -> void
    {
        if (!nativeWindowHandle || !onRender)
        {
            return;
        }
        g_onRender = std::move(onRender);
        const auto hwnd = static_cast<HWND>(nativeWindowHandle);
        g_prevWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&resizeWndProc)));
    }
} // namespace caerwyn

#else

namespace caerwyn
{
    auto installResizeRedrawHook(void*, std::function<void()>) -> void
    {
    }
} // namespace caerwyn

#endif
