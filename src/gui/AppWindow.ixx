module;

#include <raylib.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>

export module caerwyn.gui.app_window;

import caerwyn.gui.widget;
import caerwyn.gui.font;

// Implemented per platform in AppWindow_platform.cpp. Use C linkage so the
// declaration here doesn't get module-attached by MSVC, which would prevent
// it from resolving against the non-module .cpp definition.
extern "C"
{
    auto caerwyn_installResizeRedrawHook(void* nativeWindowHandle, void (*onRender)(void*), void* userData) -> void;
    auto caerwyn_uninstallResizeRedrawHook(void* nativeWindowHandle) -> void;
}

export namespace caerwyn::gui
{

    class AppWindow
    {
    public:
        struct Config
        {
            int width{800};
            int height{600};
            std::string title{"Caerwyn"};
            unsigned configFlags{FLAG_WINDOW_RESIZABLE};
            int defaultFontSize{40};
            Color clearColor{0, 0, 0, 255};
        };

        explicit AppWindow(Config cfg) : config_{std::move(cfg)}
        {
            SetConfigFlags(config_.configFlags);
            InitWindow(config_.width, config_.height, config_.title.c_str());
            caerwyn_installResizeRedrawHook(GetWindowHandle(), &AppWindow::renderTrampoline, this);
        }

        AppWindow(const AppWindow&) = delete;
        AppWindow(AppWindow&&) = delete;
        auto operator=(const AppWindow&) -> AppWindow& = delete;
        auto operator=(AppWindow&&) -> AppWindow& = delete;

        ~AppWindow()
        {
            caerwyn_uninstallResizeRedrawHook(GetWindowHandle());
            defaultFont_.reset(); // unload before CloseWindow destroys GL context
            CloseWindow();
        }

        auto setRoot(std::unique_ptr<Widget> root) -> void
        {
            root_ = std::move(root);
        }

        [[nodiscard]] auto root() -> Widget*
        {
            return root_.get();
        }

        // Lazily-built default font (embedded Roboto-Regular).
        [[nodiscard]] auto defaultFont() -> Font&
        {
            if (!defaultFont_)
            {
                defaultFont_.emplace(defaultRobotoRegular(config_.defaultFontSize));
                defaultFont_->setTextureFilter(TEXTURE_FILTER_POINT);
            }
            return *defaultFont_;
        }

        auto frame() -> void
        {
            BeginDrawing();
            ClearBackground(config_.clearColor);
            if (root_)
            {
                const auto screen = Size{static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
                (void)root_->measure(screen);
                root_->arrange(Rect{0.0F, 0.0F, screen.width, screen.height});
                root_->update();
                root_->draw();
            }
            EndDrawing();
        }

        auto run() -> void
        {
            while (!WindowShouldClose())
            {
                frame();
            }
        }

    private:
        static auto renderTrampoline(void* self) -> void
        {
            static_cast<AppWindow*>(self)->frame();
        }

        Config config_;
        std::unique_ptr<Widget> root_;
        std::optional<Font> defaultFont_;
    };

} // namespace caerwyn::gui
