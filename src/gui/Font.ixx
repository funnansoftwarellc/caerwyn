module;

#include <raylib.h>

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <utility>

export module caerwyn.gui.font;

extern "C" const unsigned char caerwyn_RobotoRegular_data[];
extern "C" const unsigned int caerwyn_RobotoRegular_size;

export namespace caerwyn::gui
{

    // RAII wrapper around a raylib `::Font`. Move-only.
    // `handle()` returns the underlying raylib value as a non-owning view that
    // can be stored in `TextRun` etc.
    class Font
    {
    public:
        Font() = default;

        // Load from an in-memory TTF buffer.
        Font(std::span<const std::byte> ttf, int pixelSize, std::span<const int> codepoints = {})
        {
            handle_ = LoadFontFromMemory(".ttf", reinterpret_cast<const unsigned char*>(ttf.data()), static_cast<int>(ttf.size()), pixelSize,
                                         const_cast<int*>(codepoints.empty() ? nullptr : codepoints.data()), static_cast<int>(codepoints.size()));
        }

        // Load from a TTF file on disk.
        Font(const std::filesystem::path& path, int pixelSize, std::span<const int> codepoints = {})
        {
            const auto p = path.string();
            handle_ = LoadFontEx(p.c_str(), pixelSize, const_cast<int*>(codepoints.empty() ? nullptr : codepoints.data()),
                                 static_cast<int>(codepoints.size()));
        }

        Font(const Font&) = delete;
        auto operator=(const Font&) -> Font& = delete;

        Font(Font&& other) noexcept : handle_{std::exchange(other.handle_, ::Font{})}
        {
        }

        auto operator=(Font&& other) noexcept -> Font&
        {
            if (this != &other)
            {
                unload();
                handle_ = std::exchange(other.handle_, ::Font{});
            }
            return *this;
        }

        ~Font()
        {
            unload();
        }

        auto setTextureFilter(int filter) const -> void
        {
            if (handle_.texture.id != 0)
            {
                SetTextureFilter(handle_.texture, filter);
            }
        }

        [[nodiscard]] auto handle() const -> ::Font
        {
            return handle_;
        }

        [[nodiscard]] auto valid() const -> bool
        {
            return handle_.texture.id != 0;
        }

    private:
        auto unload() -> void
        {
            if (handle_.texture.id != 0)
            {
                UnloadFont(handle_);
                handle_ = ::Font{};
            }
        }

        ::Font handle_{};
    };

    // Embedded default font (Roboto-Regular).
    [[nodiscard]] inline auto robotoRegularBytes() -> std::span<const std::byte>
    {
        return {reinterpret_cast<const std::byte*>(caerwyn_RobotoRegular_data), caerwyn_RobotoRegular_size};
    }

    [[nodiscard]] inline auto defaultRobotoRegular(int pixelSize) -> Font
    {
        return Font{robotoRegularBytes(), pixelSize};
    }

} // namespace caerwyn::gui
