module;

#include <raylib.h>

#include <algorithm>
#include <memory>
#include <utility>

export module caerwyn.gui.scroll_view;

import caerwyn.gui.widget;
import caerwyn.gui.layout.box;
import caerwyn.gui.viewport;
import caerwyn.gui.scrollbar;

export namespace caerwyn::gui
{

    class ScrollView final : public Container
    {
    public:
        ScrollView()
        {
            auto viewport = std::make_unique<Viewport>();
            viewport_ = viewport.get();
            addChild(std::move(viewport));

            auto bar = std::make_unique<Scrollbar>(Axis::Vertical);
            scrollbar_ = bar.get();
            addChild(std::move(bar));

            scrollbar_->setOffsetPtr(&viewport_->offsetRef().y);
        }

        auto setChild(std::unique_ptr<Widget> w) -> Widget*
        {
            return viewport_->setChild(std::move(w));
        }

        [[nodiscard]] auto viewport() -> Viewport*
        {
            return viewport_;
        }

        [[nodiscard]] auto scrollbar() -> Scrollbar*
        {
            return scrollbar_;
        }

        auto setScrollSpeed(float pixelsPerWheelTick) -> void
        {
            scrollSpeed_ = pixelsPerWheelTick;
        }

        [[nodiscard]] auto measure(Size available) -> Size override
        {
            return available;
        }

        auto arrange(Rect localBounds) -> void override
        {
            Container::arrange(localBounds);
            const auto barThickness = scrollbar_->thickness();
            // Always reserve bar thickness so wrap-driven content height never feeds back into bar visibility.
            const Rect viewportRect{localBounds.x, localBounds.y, std::max(0.0F, localBounds.width - barThickness), localBounds.height};
            const Rect barRect{localBounds.x + viewportRect.width, localBounds.y, barThickness, localBounds.height};
            viewport_->arrange(viewportRect);
            scrollbar_->setContentSize(viewport_->contentSize().height);
            scrollbar_->setViewportSize(viewport_->viewportSize().height);
            scrollbar_->arrange(barRect);
        }

        auto update() -> void override
        {
            // Mouse wheel: scroll if pointer is over the viewport region.
            const auto mouse = GetMousePosition();
            if (CheckCollisionPointRec(mouse, viewport_->bounds().toRaylib()))
            {
                const auto wheel = GetMouseWheelMove();
                if (wheel != 0.0F)
                {
                    viewport_->offsetRef().y -= wheel * scrollSpeed_;
                    const auto maxY = std::max(0.0F, viewport_->contentSize().height - viewport_->viewportSize().height);
                    viewport_->offsetRef().y = std::clamp(viewport_->offsetRef().y, 0.0F, maxY);
                }
            }
            Container::update();
        }

    private:
        Viewport* viewport_{nullptr};
        Scrollbar* scrollbar_{nullptr};
        float scrollSpeed_{40.0F};
    };

} // namespace caerwyn::gui
