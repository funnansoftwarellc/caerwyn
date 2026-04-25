module;

#include <raylib.h>
#include <rlgl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

export module caerwyn.gui.widget;

export namespace caerwyn::gui
{

    struct Size
    {
        float width{0.0F};
        float height{0.0F};
    };

    struct Point
    {
        float x{0.0F};
        float y{0.0F};
    };

    struct Rect
    {
        float x{0.0F};
        float y{0.0F};
        float width{0.0F};
        float height{0.0F};

        [[nodiscard]] auto toRaylib() const -> Rectangle
        {
            return Rectangle{x, y, width, height};
        }
    };

    struct Insets
    {
        float left{0.0F};
        float top{0.0F};
        float right{0.0F};
        float bottom{0.0F};

        [[nodiscard]] auto horizontal() const -> float
        {
            return left + right;
        }

        [[nodiscard]] auto vertical() const -> float
        {
            return top + bottom;
        }

        static auto all(float v) -> Insets
        {
            return Insets{v, v, v, v};
        }
    };

    enum class Align : std::uint8_t
    {
        Start,
        Center,
        End,
        Stretch,
    };

    struct Transform
    {
        glm::vec2 translation{0.0F, 0.0F};
        glm::vec2 scale{1.0F, 1.0F};
        glm::vec2 pivot{0.0F, 0.0F};
        float rotationDegrees{0.0F};

        [[nodiscard]] auto isIdentity() const -> bool
        {
            return translation == glm::vec2{0.0F, 0.0F} && scale == glm::vec2{1.0F, 1.0F} && rotationDegrees == 0.0F;
        }

        [[nodiscard]] auto toMatrix(glm::vec2 origin) const -> glm::mat4
        {
            const auto p = origin + pivot;
            auto m = glm::mat4(1.0F);
            m = glm::translate(m, glm::vec3(p.x + translation.x, p.y + translation.y, 0.0F));
            if (rotationDegrees != 0.0F)
            {
                m = glm::rotate(m, glm::radians(rotationDegrees), glm::vec3(0.0F, 0.0F, 1.0F));
            }
            if (scale != glm::vec2{1.0F, 1.0F})
            {
                m = glm::scale(m, glm::vec3(scale.x, scale.y, 1.0F));
            }
            m = glm::translate(m, glm::vec3(-p.x, -p.y, 0.0F));
            return m;
        }
    };

    class Widget
    {
    public:
        Widget() = default;
        Widget(const Widget&) = delete;
        Widget(Widget&&) = delete;
        auto operator=(const Widget&) -> Widget& = delete;
        auto operator=(Widget&&) -> Widget& = delete;
        virtual ~Widget() = default;

        [[nodiscard]] virtual auto measure(Size available) -> Size = 0;

        virtual auto arrange(Rect localBounds) -> void
        {
            bounds_ = localBounds;
        }

        auto draw() -> void
        {
            const bool hasTransform = !transform_.isIdentity();
            if (hasTransform)
            {
                rlPushMatrix();
                applyTransform();
            }
            drawSelf();
            drawChildren();
            if (hasTransform)
            {
                rlPopMatrix();
            }
        }

        virtual auto update() -> void
        {
            for (const auto& child : children_)
            {
                child->update();
            }
        }

        [[nodiscard]] auto bounds() const -> Rect
        {
            return bounds_;
        }

        [[nodiscard]] auto transform() -> Transform&
        {
            return transform_;
        }

        [[nodiscard]] auto transform() const -> const Transform&
        {
            return transform_;
        }

        [[nodiscard]] auto flex() const -> float
        {
            return flex_;
        }

        auto setFlex(float f) -> void
        {
            flex_ = f;
        }

        [[nodiscard]] auto crossAlign() const -> Align
        {
            return crossAlign_;
        }

        auto setCrossAlign(Align a) -> void
        {
            crossAlign_ = a;
        }

    protected:
        virtual auto drawSelf() -> void
        {
        }

        virtual auto drawChildren() -> void
        {
            for (const auto& child : children_)
            {
                child->draw();
            }
        }

        auto addChildInternal(std::unique_ptr<Widget> child) -> Widget*
        {
            auto* raw = child.get();
            children_.push_back(std::move(child));
            return raw;
        }

        [[nodiscard]] auto children() -> std::vector<std::unique_ptr<Widget>>&
        {
            return children_;
        }

        [[nodiscard]] auto children() const -> const std::vector<std::unique_ptr<Widget>>&
        {
            return children_;
        }

        auto setBounds(Rect r) -> void
        {
            bounds_ = r;
        }

    private:
        auto applyTransform() -> void
        {
            const auto m = transform_.toMatrix(glm::vec2(bounds_.x, bounds_.y));
            rlMultMatrixf(glm::value_ptr(m));
        }

        std::vector<std::unique_ptr<Widget>> children_;
        Rect bounds_{};
        Transform transform_{};
        float flex_{0.0F};
        Align crossAlign_{Align::Stretch};
    };

    // Container: a widget whose layout is delegated to subclasses; exposes addChild publicly.
    class Container : public Widget
    {
    public:
        template <typename T, typename... Args>
        auto emplaceChild(Args&&... args) -> T*
        {
            auto child = std::make_unique<T>(std::forward<Args>(args)...);
            auto* raw = child.get();
            addChildInternal(std::move(child));
            return raw;
        }

        auto addChild(std::unique_ptr<Widget> child) -> Widget*
        {
            return addChildInternal(std::move(child));
        }

        [[nodiscard]] auto padding() const -> Insets
        {
            return padding_;
        }

        auto setPadding(Insets p) -> void
        {
            padding_ = p;
        }

    private:
        Insets padding_{};

    protected:
        [[nodiscard]] auto paddingRef() const -> const Insets&
        {
            return padding_;
        }
    };

    // Top-level driver. Owns one root widget and runs the per-frame pipeline.
    class App
    {
    public:
        explicit App(std::unique_ptr<Widget> root) : root_(std::move(root))
        {
        }

        auto frame(Size screen) -> void
        {
            if (!root_)
            {
                return;
            }
            (void)root_->measure(screen);
            root_->arrange(Rect{0.0F, 0.0F, screen.width, screen.height});
            root_->update();
            root_->draw();
        }

        [[nodiscard]] auto root() -> Widget*
        {
            return root_.get();
        }

    private:
        std::unique_ptr<Widget> root_;
    };

} // namespace caerwyn::gui
