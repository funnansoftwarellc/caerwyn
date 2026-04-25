#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include <chrono>
#include <format>
#include <print>

struct Message
{
    std::string username;
    std::string message;
    std::chrono::nanoseconds timestamp;
};

auto main() -> int
{
    constexpr auto width = 720;
    constexpr auto height = 1440;

    using namespace std::chrono_literals;

    auto dummy = std::vector{
        Message{"Alice", "Hello, world!", 19h + 5min},
        Message{"Bob", "Hi, Alice!", 19h + 6min},
        Message{"Charlie", "Good morning!", 19h + 7min},
    };

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, "Caerwyn");

    const auto fontRegular = LoadFont("D:\\dev\\caerwyn\\assets\\fonts\\Roboto\\static\\Roboto-Regular.ttf");
    const auto fontBold = LoadFont("D:\\dev\\caerwyn\\assets\\fonts\\Roboto\\static\\Roboto-Bold.ttf");

    constexpr auto fontSize = 40;
    constexpr auto spacing = 5;
    constexpr auto lineHeight = fontSize + 8; // fixed row pitch
    Rectangle panelRec = {0, 0, width, height};
    Rectangle panelContentRec = {10, 10, width - 20, height - 20};
    Rectangle panelView = {0};
    Vector2 panelScroll = {0, 0};

    GuiSetStyle(DEFAULT, TEXT_SIZE, fontSize);
    GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_NONE);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(BLACK));

    while (!WindowShouldClose())
    {
        PollInputEvents();
        BeginDrawing();
        ClearBackground(BLACK);

        GuiScrollPanel(panelRec, NULL, panelContentRec, &panelScroll, &panelView);

        BeginScissorMode(static_cast<int>(panelContentRec.x), static_cast<int>(panelContentRec.y), static_cast<int>(panelContentRec.width),
                         static_cast<int>(panelContentRec.height));

        const auto startPosX = panelContentRec.x + panelScroll.x;
        auto posY = panelContentRec.y + panelScroll.y;

        for (const auto& message : dummy)
        {
            const auto timestamp = std::format("{:%H:%M}", std::chrono::hh_mm_ss{message.timestamp});
            const auto username = std::format("{}:", message.username);

            auto posX = startPosX;

            GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(GRAY));
            GuiSetFont(fontRegular);
            auto lblSize = MeasureTextEx(fontRegular, timestamp.c_str(), fontSize, spacing);
            GuiLabel({posX, posY, lblSize.x, lineHeight}, timestamp.c_str());
            posX += lblSize.x;

            GuiSetFont(fontBold);
            GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(WHITE));
            lblSize = MeasureTextEx(fontBold, username.c_str(), fontSize, spacing);
            GuiLabel({posX, posY, lblSize.x, lineHeight}, username.c_str());
            posX += lblSize.x;

            GuiSetFont(fontRegular);
            GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(WHITE));
            lblSize = MeasureTextEx(fontRegular, message.message.c_str(), fontSize, spacing);
            GuiLabel({posX, posY, lblSize.x, lineHeight}, message.message.c_str());
            posY += lineHeight;
        }

        EndScissorMode();

        EndDrawing();
    }

    CloseWindow();

    return EXIT_SUCCESS;
}