module;

#include <raylib.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <utility>

export module caerwyn.gui.viewport;

import caerwyn.gui.widget;

export namespace caerwyn::gui
{

    class Viewport final : public Container
    {
    public:
        Viewport() = default;

        auto setChild(std::unique_ptr<Widget> newChild) -> Widget*
        {
            children().clear();
            return addChild(std::move(newChild));
        }

        [[nodiscard]] auto child() const -> Widget*
        {
            const auto& kids = children();
            return kids.empty() ? nullptr : kids.front().get();
        }

        auto setScrollAxes(bool x, bool y) -> void
        {
            scrollX_ = x;
            scrollY_ = y;
        }

        auto setOffset(Point o) -> void
        {
            offset_ = o;
        }

        [[nodiscard]] auto offset() const -> Point
        {
            return offset_;
        }

        [[nodiscard]] auto offsetRef() -> Point&
        {
            return offset_;
        }

        [[nodiscard]] auto contentSize() const -> Size
        {
            return contentSize_;
        }

        [[nodiscard]] auto viewportSize() const -> Size
        {
            return viewportSize_;
        }

        [[nodiscard]] auto measure(Size available) -> Size override
        {
            viewportSize_ = available;
            return available;
        }

        auto arrange(Rect localBounds) -> void override
        {
            Container::arrange(localBounds);
            viewportSize_ = Size{localBounds.width, localBounds.height};
            if (auto* c = child())
            {
                const auto inf = std::numeric_limits<float>::infinity();
                const auto avail = Size{scrollX_ ? inf : localBounds.width, scrollY_ ? inf : localBounds.height};
                const auto measured = c->measure(avail);
                contentSize_ = Size{scrollX_ ? measured.width : localBounds.width, scrollY_ ? measured.height : localBounds.height};
                clampOffset();
                const Rect childRect{localBounds.x - offset_.x, localBounds.y - offset_.y, contentSize_.width, contentSize_.height};
                c->arrange(childRect);
            }
        }

    protected:
        auto drawChildren() -> void override
        {
            const auto b = bounds();
            BeginScissorMode(static_cast<int>(b.x), static_cast<int>(b.y), static_cast<int>(b.width), static_cast<int>(b.height));
            Container::drawChildren();
            EndScissorMode();
        }

    private:
        auto clampOffset() -> void
        {
            const auto maxX = std::max(0.0F, contentSize_.width - viewportSize_.width);
            const auto maxY = std::max(0.0F, contentSize_.height - viewportSize_.height);
            offset_.x = std::clamp(offset_.x, 0.0F, maxX);
            offset_.y = std::clamp(offset_.y, 0.0F, maxY);
        }

        Point offset_{0.0F, 0.0F};
        Size contentSize_{0.0F, 0.0F};
        Size viewportSize_{0.0F, 0.0F};
        bool scrollX_{false};
        bool scrollY_{true};
    };

} // namespace caerwyn::gui
