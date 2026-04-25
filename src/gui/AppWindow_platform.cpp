// Platform-specific resize-redraw hook used by caerwyn::gui::AppWindow.
//
// On Windows, raylib's main loop is blocked by the modal resize/move loop
// while the user drags a window edge, so the window stops repainting until
// the mouse is released. This file subclasses the GLFW window proc and
// invokes the supplied render callback on WM_SIZE / WM_TIMER / WM_PAINT
// while the user is sizing.
//
// On other platforms the hook is a no-op.

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace
{
    void (*g_onRender)(void*) = nullptr;
    void* g_userData = nullptr;
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
                    g_onRender(g_userData);
                }
                break;
            default:
                break;
        }
        return result;
    }
} // namespace

extern "C" auto caerwyn_installResizeRedrawHook(void* nativeWindowHandle, void (*onRender)(void*), void* userData) -> void
{
    if (!nativeWindowHandle || !onRender)
    {
        return;
    }
    g_onRender = onRender;
    g_userData = userData;
    const auto hwnd = static_cast<HWND>(nativeWindowHandle);
    g_prevWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&resizeWndProc)));
}

extern "C" auto caerwyn_uninstallResizeRedrawHook(void* nativeWindowHandle) -> void
{
    if (!nativeWindowHandle || !g_prevWndProc)
    {
        return;
    }
    const auto hwnd = static_cast<HWND>(nativeWindowHandle);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_prevWndProc));
    g_prevWndProc = nullptr;
    g_onRender = nullptr;
    g_userData = nullptr;
}

#else

extern "C" auto caerwyn_installResizeRedrawHook(void*, void (*)(void*), void*) -> void
{
}

extern "C" auto caerwyn_uninstallResizeRedrawHook(void*) -> void
{
}

#endif
