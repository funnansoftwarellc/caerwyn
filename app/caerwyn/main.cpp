#include <raylib.h>

#include <chrono>
#include <cstdlib>
#include <format>
#include <memory>
#include <string>
#include <utility>
#include <vector>

import caerwyn.gui;

namespace
{

    struct Message
    {
        std::string username;
        std::string message;
        std::chrono::nanoseconds timestamp;
    };

    constexpr auto InitialWidth = 720;
    constexpr auto InitialHeight = 1440;
    constexpr auto FontSize = 40;
    constexpr auto FontSpacing = 0.0F;
    constexpr auto LineSpacing = 8.0F;
    constexpr auto PanelMargin = 10.0F;
    constexpr auto RowSpacing = 6.0F;

    auto buildMessageLabel(const Message& msg, const caerwyn::gui::Font& regular, const caerwyn::gui::Font& bold)
        -> std::unique_ptr<caerwyn::gui::Widget>
    {
        using namespace caerwyn::gui;

        auto label = std::make_unique<RichTextLabel>();
        label->setWrapMode(WrapMode::Word);
        label->setLineSpacing(LineSpacing);

        constexpr auto white = Color{255, 255, 255, 255};
        constexpr auto grey = Color{160, 160, 160, 255};
        label->addRun(TextRun{.text = std::format("{:%H:%M} ", std::chrono::hh_mm_ss{msg.timestamp}),
                              .font = regular.handle(),
                              .fontSize = static_cast<float>(FontSize),
                              .spacing = FontSpacing,
                              .color = grey});
        label->addRun(TextRun{.text = std::format("{}: ", msg.username),
                              .font = bold.handle(),
                              .fontSize = static_cast<float>(FontSize),
                              .spacing = FontSpacing,
                              .color = white});
        label->addRun(
            TextRun{.text = msg.message, .font = regular.handle(), .fontSize = static_cast<float>(FontSize), .spacing = FontSpacing, .color = white});
        return label;
    }

    auto buildRoot(const std::vector<Message>& messages, const caerwyn::gui::Font& regular, const caerwyn::gui::Font& bold)
        -> std::unique_ptr<caerwyn::gui::Widget>
    {
        using namespace caerwyn::gui;

        auto column = std::make_unique<ColumnLayout>();
        column->setSpacing(RowSpacing);
        column->setPadding(Insets::all(PanelMargin));
        for (const auto& msg : messages)
        {
            column->addChild(buildMessageLabel(msg, regular, bold));
        }

        auto scroll = std::make_unique<ScrollView>();
        scroll->setChild(std::move(column));
        return scroll;
    }

} // namespace

auto main() -> int
{
    using namespace std::chrono_literals;

    const auto messages = std::vector<Message>{
        Message{"Alice", "Hello twitch chatters, my name is Funnan, I am a C++ developer of 15 years.", 19h + 5min},
        Message{"Bob",
                "Hello youtube chatters, I am actively developing a video game called Briarthorn about aircombat "
                "situations.",
                19h + 6min},
        Message{"Charlie", "Hello Mom, Dad, and Brothers. Thank you for subscribing to my youtube channel.", 19h + 7min},
    };

    caerwyn::gui::AppWindow window{{.width = InitialWidth, .height = InitialHeight, .title = "Caerwyn", .defaultFontSize = FontSize}};

    const auto& fontRegular = window.defaultFont();
    caerwyn::gui::Font fontBold{"D:\\dev\\caerwyn\\assets\\fonts\\Roboto\\static\\Roboto-Bold.ttf", FontSize};
    fontBold.setTextureFilter(TEXTURE_FILTER_POINT);

    window.setRoot(buildRoot(messages, fontRegular, fontBold));
    window.run();

    return EXIT_SUCCESS;
}
