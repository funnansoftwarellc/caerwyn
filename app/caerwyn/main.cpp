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
    constexpr auto FontSize = 40.0F;
    constexpr auto FontSpacing = 0.0F;
    constexpr auto LineSpacing = 8.0F;
    constexpr auto PanelMargin = 10.0F;
    constexpr auto RowSpacing = 6.0F;

    auto buildMessageLabel(const Message& msg, Font fontRegular, Font fontBold) -> std::unique_ptr<caerwyn::gui::Widget>
    {
        using namespace caerwyn::gui;

        auto label = std::make_unique<RichTextLabel>();
        label->setWrapMode(WrapMode::Word);
        label->setLineSpacing(LineSpacing);

        label->addRun(TextRun{.text = std::format("{:%H:%M} ", std::chrono::hh_mm_ss{msg.timestamp}),
                              .font = fontRegular,
                              .fontSize = FontSize,
                              .spacing = FontSpacing,
                              .color = Color{160, 160, 160, 255}});
        label->addRun(TextRun{.text = std::format("{}: ", msg.username),
                              .font = fontBold,
                              .fontSize = FontSize,
                              .spacing = FontSpacing,
                              .color = Color{255, 255, 255, 255}});
        label->addRun(
            TextRun{.text = msg.message, .font = fontRegular, .fontSize = FontSize, .spacing = FontSpacing, .color = Color{255, 255, 255, 255}});
        return label;
    }

    auto buildRoot(const std::vector<Message>& messages, Font fontRegular, Font fontBold) -> std::unique_ptr<caerwyn::gui::Widget>
    {
        using namespace caerwyn::gui;

        auto column = std::make_unique<ColumnLayout>();
        column->setSpacing(RowSpacing);
        column->setPadding(Insets::all(PanelMargin));
        for (const auto& msg : messages)
        {
            column->addChild(buildMessageLabel(msg, fontRegular, fontBold));
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

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(InitialWidth, InitialHeight, "Caerwyn");

    const auto fontRegular = LoadFont("D:\\dev\\caerwyn\\assets\\fonts\\Roboto\\static\\Roboto-Regular.ttf");
    const auto fontBold = LoadFont("D:\\dev\\caerwyn\\assets\\fonts\\Roboto\\static\\Roboto-Bold.ttf");

    caerwyn::gui::App app{buildRoot(messages, fontRegular, fontBold)};

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        const auto screen = caerwyn::gui::Size{static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
        app.frame(screen);
        EndDrawing();
    }

    UnloadFont(fontRegular);
    UnloadFont(fontBold);
    CloseWindow();
    return EXIT_SUCCESS;
}
