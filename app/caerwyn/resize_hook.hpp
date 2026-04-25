#pragma once

#include <functional>

namespace caerwyn
{
    // Installs a Win32 window-proc subclass that invokes `onRender` during the
    // modal resize loop (WM_SIZE / WM_PAINT / timer ticks while sizing) so the
    // window keeps redrawing live while the user drags a window edge. No-op on
    // non-Windows platforms.
    auto installResizeRedrawHook(void* nativeWindowHandle, std::function<void()> onRender) -> void;
} // namespace caerwyn
