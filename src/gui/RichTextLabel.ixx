module;

#include <raylib.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module caerwyn.gui.rich_text_label;

import caerwyn.gui.widget;

export namespace caerwyn::gui
{

    enum class WrapMode : std::uint8_t
    {
        None,
        Word,
    };

    struct TextRun
    {
        std::string text;
        Font font{};
        float fontSize{16.0F};
        float spacing{1.0F};
        Color color{255, 255, 255, 255};
    };

    class RichTextLabel final : public Widget
    {
    public:
        RichTextLabel() = default;

        explicit RichTextLabel(std::vector<TextRun> runs) : runs_(std::move(runs))
        {
        }

        auto setRuns(std::vector<TextRun> runs) -> void
        {
            runs_ = std::move(runs);
            layoutValid_ = false;
        }

        [[nodiscard]] auto runs() const -> const std::vector<TextRun>&
        {
            return runs_;
        }

        auto addRun(TextRun run) -> void
        {
            runs_.push_back(std::move(run));
            layoutValid_ = false;
        }

        auto setWrapMode(WrapMode m) -> void
        {
            if (wrapMode_ != m)
            {
                wrapMode_ = m;
                layoutValid_ = false;
            }
        }

        auto setLineSpacing(float ls) -> void
        {
            if (lineSpacing_ != ls)
            {
                lineSpacing_ = ls;
                layoutValid_ = false;
            }
        }

        [[nodiscard]] auto measure(Size available) -> Size override
        {
            relayoutIfNeeded(available.width);
            return Size{measuredWidth_, measuredHeight_};
        }

        auto arrange(Rect localBounds) -> void override
        {
            Widget::arrange(localBounds);
            relayoutIfNeeded(localBounds.width);
        }

    protected:
        auto drawSelf() -> void override
        {
            const auto b = bounds();
            for (const auto& frag : fragments_)
            {
                const auto pos = Vector2{b.x + frag.x, b.y + frag.y};
                DrawTextEx(frag.font, frag.text.c_str(), pos, frag.fontSize, frag.spacing, frag.color);
            }
        }

    private:
        struct Fragment
        {
            std::string text;
            Font font{};
            float fontSize{0.0F};
            float spacing{0.0F};
            Color color{};
            float x{0.0F};
            float y{0.0F};
        };

        struct Token
        {
            std::string_view text;
            std::size_t runIndex{0};
            float width{0.0F};
            bool isWhitespace{false};
            bool isNewline{false};
        };

        auto relayoutIfNeeded(float width) -> void
        {
            if (layoutValid_ && lastLayoutWidth_ == width)
            {
                return;
            }
            layout(width);
            lastLayoutWidth_ = width;
            layoutValid_ = true;
        }

        auto layout(float maxWidth) -> void
        {
            fragments_.clear();
            measuredWidth_ = 0.0F;
            measuredHeight_ = 0.0F;
            if (runs_.empty())
            {
                return;
            }

            const auto tokens = tokenize();
            const auto wrap = wrapMode_ == WrapMode::Word && maxWidth > 0.0F;

            float lineX = 0.0F;
            std::vector<Token> lineTokens;
            const auto flushLine = [&](bool /*finalLine*/)
            {
                buildLineFragments(lineTokens);
                lineTokens.clear();
                lineX = 0.0F;
            };

            for (const auto& tok : tokens)
            {
                if (tok.isNewline)
                {
                    flushLine(false);
                    continue;
                }
                const auto next = lineX + tok.width;
                if (wrap && !tok.isWhitespace && next > maxWidth && !lineTokens.empty())
                {
                    // Strip trailing whitespace from current line, then wrap.
                    while (!lineTokens.empty() && lineTokens.back().isWhitespace)
                    {
                        lineX -= lineTokens.back().width;
                        lineTokens.pop_back();
                    }
                    flushLine(false);
                }
                // Skip leading whitespace on a fresh line.
                if (tok.isWhitespace && lineTokens.empty())
                {
                    continue;
                }
                lineTokens.push_back(tok);
                lineX += tok.width;
            }
            flushLine(true);
        }

        [[nodiscard]] auto tokenize() const -> std::vector<Token>
        {
            std::vector<Token> tokens;
            for (std::size_t i = 0; i < runs_.size(); ++i)
            {
                const auto& run = runs_[i];
                const std::string_view sv{run.text};
                std::size_t pos = 0;
                while (pos < sv.size())
                {
                    if (sv[pos] == '\n')
                    {
                        Token t{};
                        t.runIndex = i;
                        t.isNewline = true;
                        tokens.push_back(t);
                        ++pos;
                        continue;
                    }
                    if (sv[pos] == ' ' || sv[pos] == '\t')
                    {
                        auto end = pos;
                        while (end < sv.size() && (sv[end] == ' ' || sv[end] == '\t'))
                        {
                            ++end;
                        }
                        Token t{};
                        t.text = sv.substr(pos, end - pos);
                        t.runIndex = i;
                        t.isWhitespace = true;
                        t.width = measureToken(run, t.text);
                        tokens.push_back(t);
                        pos = end;
                        continue;
                    }
                    auto end = pos;
                    while (end < sv.size() && sv[end] != ' ' && sv[end] != '\t' && sv[end] != '\n')
                    {
                        ++end;
                    }
                    Token t{};
                    t.text = sv.substr(pos, end - pos);
                    t.runIndex = i;
                    t.width = measureToken(run, t.text);
                    tokens.push_back(t);
                    pos = end;
                }
            }
            return tokens;
        }

        static auto measureToken(const TextRun& run, std::string_view text) -> float
        {
            if (text.empty())
            {
                return 0.0F;
            }
            const std::string copy{text};
            return MeasureTextEx(run.font, copy.c_str(), run.fontSize, run.spacing).x;
        }

        auto buildLineFragments(const std::vector<Token>& lineTokens) -> void
        {
            // Compute line height = max fontSize on this line.
            float lineHeight = 0.0F;
            for (const auto& tok : lineTokens)
            {
                lineHeight = std::max(lineHeight, runs_[tok.runIndex].fontSize);
            }
            if (lineHeight <= 0.0F && !runs_.empty())
            {
                lineHeight = runs_.front().fontSize;
            }

            float x = 0.0F;
            float lineWidth = 0.0F;
            for (const auto& tok : lineTokens)
            {
                if (tok.isWhitespace)
                {
                    x += tok.width;
                    continue;
                }
                const auto& run = runs_[tok.runIndex];
                Fragment frag{};
                frag.text = std::string{tok.text};
                frag.font = run.font;
                frag.fontSize = run.fontSize;
                frag.spacing = run.spacing;
                frag.color = run.color;
                frag.x = x;
                frag.y = measuredHeight_;
                fragments_.push_back(std::move(frag));
                x += tok.width;
                lineWidth = x;
            }
            measuredWidth_ = std::max(measuredWidth_, lineWidth);
            measuredHeight_ += lineHeight + lineSpacing_;
        }

        std::vector<TextRun> runs_;
        std::vector<Fragment> fragments_;
        WrapMode wrapMode_{WrapMode::None};
        float lineSpacing_{4.0F};
        float measuredWidth_{0.0F};
        float measuredHeight_{0.0F};
        float lastLayoutWidth_{-1.0F};
        bool layoutValid_{false};
    };

} // namespace caerwyn::gui
