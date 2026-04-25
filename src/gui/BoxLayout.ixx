module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

export module caerwyn.gui.layout.box;

import caerwyn.gui.widget;

export namespace caerwyn::gui
{

    enum class Axis : std::uint8_t
    {
        Horizontal,
        Vertical,
    };

    class BoxLayout : public Container
    {
    public:
        explicit BoxLayout(Axis axis) : axis_(axis)
        {
        }

        [[nodiscard]] auto axis() const -> Axis
        {
            return axis_;
        }

        auto setSpacing(float s) -> void
        {
            spacing_ = s;
        }

        [[nodiscard]] auto spacing() const -> float
        {
            return spacing_;
        }

        auto setMainAlign(Align a) -> void
        {
            mainAlign_ = a;
        }

        auto setCrossAxisAlign(Align a) -> void
        {
            crossAxisAlign_ = a;
        }

        [[nodiscard]] auto measure(Size available) -> Size override
        {
            const auto pad = padding();
            const auto innerAvail = Size{available.width - pad.horizontal(), available.height - pad.vertical()};

            const auto& kids = children();
            const auto count = kids.size();
            const auto totalSpacing = count > 1 ? spacing_ * static_cast<float>(count - 1) : 0.0F;

            // Pass 1: measure non-flex children to determine consumed main-axis space.
            float mainConsumed = 0.0F;
            float crossMax = 0.0F;
            for (const auto& child : kids)
            {
                if (child->flex() > 0.0F)
                {
                    continue;
                }
                const auto sz = child->measure(innerAvail);
                mainConsumed += mainOf(sz);
                crossMax = std::max(crossMax, crossOf(sz));
            }

            // Distribute remaining main-axis space to flex children proportionally.
            const auto mainAvail = mainOf(innerAvail) - totalSpacing;
            const auto flexTotal = totalFlex();
            const auto flexSpace = std::max(0.0F, mainAvail - mainConsumed);
            for (const auto& child : kids)
            {
                if (child->flex() <= 0.0F)
                {
                    continue;
                }
                const auto share = flexTotal > 0.0F ? flexSpace * (child->flex() / flexTotal) : 0.0F;
                const auto sz = makeSize(share, crossOf(innerAvail));
                const auto measured = child->measure(sz);
                crossMax = std::max(crossMax, crossOf(measured));
            }

            const auto mainTotal = (axis_ == Axis::Horizontal ? mainConsumed : mainConsumed) + totalSpacing;
            return makeSize(mainTotal + padOnAxis(pad, true), crossMax + padOnAxis(pad, false));
        }

        auto arrange(Rect localBounds) -> void override
        {
            Container::arrange(localBounds);
            const auto pad = padding();
            const auto innerOriginMain = padStart(pad, true);
            const auto innerOriginCross = padStart(pad, false);
            const auto innerMain = mainOf(Size{localBounds.width, localBounds.height}) - padOnAxis(pad, true);
            const auto innerCross = crossOf(Size{localBounds.width, localBounds.height}) - padOnAxis(pad, false);

            const auto& kids = children();
            const auto count = kids.size();
            const auto totalSpacing = count > 1 ? spacing_ * static_cast<float>(count - 1) : 0.0F;

            // Re-measure to compute fixed sizes; cache per child.
            std::vector<float> mainSizes(count, 0.0F);
            std::vector<float> crossSizes(count, 0.0F);
            float fixedMain = 0.0F;
            for (std::size_t i = 0; i < count; ++i)
            {
                if (kids[i]->flex() > 0.0F)
                {
                    continue;
                }
                const auto sz = kids[i]->measure(makeSize(innerMain, innerCross));
                mainSizes[i] = mainOf(sz);
                crossSizes[i] = crossOf(sz);
                fixedMain += mainSizes[i];
            }
            const auto flexTotal = totalFlex();
            const auto flexSpace = std::max(0.0F, innerMain - fixedMain - totalSpacing);
            for (std::size_t i = 0; i < count; ++i)
            {
                if (kids[i]->flex() <= 0.0F)
                {
                    continue;
                }
                const auto share = flexTotal > 0.0F ? flexSpace * (kids[i]->flex() / flexTotal) : 0.0F;
                const auto sz = kids[i]->measure(makeSize(share, innerCross));
                mainSizes[i] = share;
                crossSizes[i] = crossOf(sz);
            }

            // Determine main-axis starting offset for alignment of the whole row.
            const auto usedMain = std::max(0.0F, fixedMain + (flexTotal > 0.0F ? flexSpace : 0.0F) + totalSpacing);
            float mainCursor = innerOriginMain + alignOffset(mainAlign_, innerMain, usedMain);

            for (std::size_t i = 0; i < count; ++i)
            {
                auto& child = kids[i];
                const auto childMain = mainSizes[i];
                const auto effectiveCrossAlign =
                    child->crossAlign() == Align::Stretch && crossAxisAlign_ != Align::Stretch ? crossAxisAlign_ : child->crossAlign();
                const auto childCross = effectiveCrossAlign == Align::Stretch ? innerCross : std::min(crossSizes[i], innerCross);
                const auto crossOffset = innerOriginCross + alignOffset(effectiveCrossAlign, innerCross, childCross);

                Rect childRect{};
                if (axis_ == Axis::Horizontal)
                {
                    childRect = Rect{localBounds.x + mainCursor, localBounds.y + crossOffset, childMain, childCross};
                }
                else
                {
                    childRect = Rect{localBounds.x + crossOffset, localBounds.y + mainCursor, childCross, childMain};
                }
                child->arrange(childRect);
                mainCursor += childMain + (i + 1 < count ? spacing_ : 0.0F);
            }
        }

    private:
        [[nodiscard]] auto mainOf(Size s) const -> float
        {
            return axis_ == Axis::Horizontal ? s.width : s.height;
        }

        [[nodiscard]] auto crossOf(Size s) const -> float
        {
            return axis_ == Axis::Horizontal ? s.height : s.width;
        }

        [[nodiscard]] auto makeSize(float main, float cross) const -> Size
        {
            return axis_ == Axis::Horizontal ? Size{main, cross} : Size{cross, main};
        }

        [[nodiscard]] auto padOnAxis(const Insets& pad, bool main) const -> float
        {
            if (main)
            {
                return axis_ == Axis::Horizontal ? pad.horizontal() : pad.vertical();
            }
            return axis_ == Axis::Horizontal ? pad.vertical() : pad.horizontal();
        }

        [[nodiscard]] auto padStart(const Insets& pad, bool main) const -> float
        {
            if (main)
            {
                return axis_ == Axis::Horizontal ? pad.left : pad.top;
            }
            return axis_ == Axis::Horizontal ? pad.top : pad.left;
        }

        [[nodiscard]] auto totalFlex() const -> float
        {
            float t = 0.0F;
            for (const auto& c : children())
            {
                t += c->flex();
            }
            return t;
        }

        static auto alignOffset(Align a, float container, float content) -> float
        {
            switch (a)
            {
                case Align::Start:
                case Align::Stretch:
                    return 0.0F;
                case Align::Center:
                    return (container - content) * 0.5F;
                case Align::End:
                    return container - content;
            }
            return 0.0F;
        }

        Axis axis_;
        float spacing_{0.0F};
        Align mainAlign_{Align::Start};
        Align crossAxisAlign_{Align::Stretch};
    };

} // namespace caerwyn::gui
