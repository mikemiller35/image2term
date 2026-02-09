#ifndef IMAGE2TERM_H
#define IMAGE2TERM_H

#include <string>
#include <string_view>
#include <stdexcept>
#include <cstdint>

// Configuration for the image2term application
struct Config {
    std::string input_file;
    int width = 0;
    bool invert = false;
    bool help_requested = false;
};

// Argument parser class for command-line argument handling
class ArgumentParser {
public:
    explicit ArgumentParser(std::string_view program_name);
    [[nodiscard]] Config parse(int argc, char* argv[]);

    void print_usage() const;

private:
    std::string program_name_;
};

// Wrapper for image data loaded via stb_image
class Image {
public:
    explicit Image(std::string_view filepath);

    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    ~Image();

    // Accessors
    [[nodiscard]] const std::uint8_t* data() const noexcept { return data_; }
    [[nodiscard]] int width() const noexcept { return width_; }
    [[nodiscard]] int height() const noexcept { return height_; }
    [[nodiscard]] int channels() const noexcept { return channels_; }

private:
    std::uint8_t* data_ = nullptr;
    int width_ = 0;
    int height_ = 0;
    int channels_ = 0;
};

// ASCII art renderer
class AsciiRenderer {
public:
    // Default ASCII ramp from dark to light
    static constexpr std::string_view DEFAULT_RAMP = "@%#*+=-:. ";

    explicit AsciiRenderer(bool invert = false);
    explicit AsciiRenderer(std::string_view custom_ramp, bool invert = false);

    // Render an image to ASCII art
    [[nodiscard]] std::string render(const Image& image, int output_width) const;

    // Render raw image data to ASCII art
    [[nodiscard]] std::string render(const std::uint8_t* img_data, int img_width, int img_height,
                                     int channels, int output_width) const;

    void set_invert(bool invert) noexcept { invert_ = invert; }
    void set_ramp(std::string_view ramp) { ramp_ = ramp; }

    // Convert RGB to grayscale brightness (0-255)
    [[nodiscard]] static constexpr std::uint8_t rgb_to_brightness(
        std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept {
        return static_cast<std::uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);
    }

private:
    // Map brightness (0-255) to ASCII character
    [[nodiscard]] char brightness_to_char(std::uint8_t brightness) const noexcept;

    std::string ramp_;
    bool invert_;
};

// Utility function to get terminal width
[[nodiscard]] int get_terminal_width() noexcept;

// Legacy free functions for backward compatibility
[[nodiscard]] std::uint8_t rgb_to_brightness(std::uint8_t r, std::uint8_t g, std::uint8_t b);
[[nodiscard]] char brightness_to_char(std::uint8_t brightness, bool invert);
[[nodiscard]] std::string render_ascii(const std::uint8_t* img_data, int img_width, int img_height,
                                       int channels, int output_width, bool invert);

#endif // IMAGE2TERM_H

