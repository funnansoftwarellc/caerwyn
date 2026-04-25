module;

#include <raylib.h>

#include <algorithm>
#include <cstdint>

export module caerwyn.gui.scrollbar;

import caerwyn.gui.widget;
import caerwyn.gui.layout.box;

export namespace caerwyn::gui
{

    struct ScrollbarColors
    {
        Color track{40, 40, 40, 200};
        Color thumb{120, 120, 120, 255};
        Color thumbHover{160, 160, 160, 255};
        Color thumbActive{200, 200, 200, 255};
    };

    class Scrollbar final : public Widget
    {
    public:
        Scrollbar() = default;

        explicit Scrollbar(Axis axis) : axis_(axis)
        {
        }

        auto setAxis(Axis a) -> void
        {
            axis_ = a;
        }

        [[nodiscard]] auto axis() const -> Axis
        {
            return axis_;
        }

        auto setThickness(float t) -> void
        {
            thickness_ = t;
        }

        [[nodiscard]] auto thickness() const -> float
        {
            return thickness_;
        }

        auto setMinThumb(float m) -> void
        {
            minThumb_ = m;
        }

        auto setContentSize(float v) -> void
        {
            contentSize_ = v;
        }

        auto setViewportSize(float v) -> void
        {
            viewportSize_ = v;
        }

        auto setOffsetPtr(float* p) -> void
        {
            offsetPtr_ = p;
        }

        auto setColors(ScrollbarColors c) -> void
        {
            colors_ = c;
        }

        [[nodiscard]] auto isVisible() const -> bool
        {
            return contentSize_ > viewportSize_ + 0.5F;
        }

        [[nodiscard]] auto measure(Size available) -> Size override
        {
            if (axis_ == Axis::Vertical)
            {
                return Size{thickness_, available.height};
            }
            return Size{available.width, thickness_};
        }

        auto update() -> void override
        {
            if (!offsetPtr_ || !isVisible())
            {
                dragging_ = false;
                return;
            }
            const auto mouse = GetMousePosition();
            const auto thumb = computeThumbRect();
            const bool mouseOverThumb = CheckCollisionPointRec(mouse, thumb);
            const bool mouseOverTrack = CheckCollisionPointRec(mouse, bounds().toRaylib());
            thumbHover_ = mouseOverThumb;

            const auto mousePoint = Point{mouse.x, mouse.y};
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                if (mouseOverThumb)
                {
                    dragging_ = true;
                    dragOffsetMain_ = mainCoord(mousePoint) - mainCoord(Point{thumb.x, thumb.y});
                }
                else if (mouseOverTrack)
                {
                    // Page click: jump thumb so mouse is at thumb center.
                    jumpThumbCenterTo(mainCoord(mousePoint));
                }
            }
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                dragging_ = false;
            }
            if (dragging_)
            {
                const auto trackLen = mainSizeOf(bounds());
                const auto thumbLen = thumbLength(trackLen);
                const auto trackStart = mainStartOf(bounds());
                const auto pos = mainCoord(mousePoint) - dragOffsetMain_ - trackStart;
                const auto maxPos = std::max(0.0F, trackLen - thumbLen);
                const auto frac = maxPos > 0.0F ? std::clamp(pos / maxPos, 0.0F, 1.0F) : 0.0F;
                *offsetPtr_ = frac * std::max(0.0F, contentSize_ - viewportSize_);
            }
        }

    protected:
        auto drawSelf() -> void override
        {
            const auto b = bounds();
            DrawRectangleRec(b.toRaylib(), colors_.track);
            if (!isVisible())
            {
                return;
            }
            const auto thumb = computeThumbRect();
            Color c = colors_.thumb;
            if (dragging_)
            {
                c = colors_.thumbActive;
            }
            else if (thumbHover_)
            {
                c = colors_.thumbHover;
            }
            DrawRectangleRec(thumb, c);
        }

    private:
        [[nodiscard]] auto mainCoord(Point p) const -> float
        {
            return axis_ == Axis::Vertical ? p.y : p.x;
        }

        [[nodiscard]] auto mainSizeOf(Rect r) const -> float
        {
            return axis_ == Axis::Vertical ? r.height : r.width;
        }

        [[nodiscard]] auto mainStartOf(Rect r) const -> float
        {
            return axis_ == Axis::Vertical ? r.y : r.x;
        }

        [[nodiscard]] auto thumbLength(float trackLen) const -> float
        {
            if (contentSize_ <= 0.0F)
            {
                return trackLen;
            }
            const auto frac = std::clamp(viewportSize_ / contentSize_, 0.0F, 1.0F);
            return std::max(minThumb_, trackLen * frac);
        }

        [[nodiscard]] auto computeThumbRect() const -> Rectangle
        {
            const auto b = bounds();
            const auto trackLen = mainSizeOf(b);
            const auto thumbLen = thumbLength(trackLen);
            const auto maxOff = std::max(0.0F, contentSize_ - viewportSize_);
            const auto off = offsetPtr_ ? *offsetPtr_ : 0.0F;
            const auto frac = maxOff > 0.0F ? std::clamp(off / maxOff, 0.0F, 1.0F) : 0.0F;
            const auto pos = (trackLen - thumbLen) * frac;
            if (axis_ == Axis::Vertical)
            {
                return Rectangle{b.x, b.y + pos, b.width, thumbLen};
            }
            return Rectangle{b.x + pos, b.y, thumbLen, b.height};
        }

        auto jumpThumbCenterTo(float mouseMain) -> void
        {
            const auto trackLen = mainSizeOf(bounds());
            const auto thumbLen = thumbLength(trackLen);
            const auto trackStart = mainStartOf(bounds());
            const auto desired = (mouseMain - trackStart) - (thumbLen * 0.5F);
            const auto maxPos = std::max(0.0F, trackLen - thumbLen);
            const auto frac = maxPos > 0.0F ? std::clamp(desired / maxPos, 0.0F, 1.0F) : 0.0F;
            *offsetPtr_ = frac * std::max(0.0F, contentSize_ - viewportSize_);
            dragging_ = true;
            dragOffsetMain_ = thumbLen * 0.5F;
        }

        Axis axis_{Axis::Vertical};
        float thickness_{14.0F};
        float minThumb_{20.0F};
        float contentSize_{0.0F};
        float viewportSize_{0.0F};
        float* offsetPtr_{nullptr};
        ScrollbarColors colors_{};
        bool dragging_{false};
        bool thumbHover_{false};
        float dragOffsetMain_{0.0F};
    };

} // namespace caerwyn::gui
