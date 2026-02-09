#include "image2term.h"
#include "stb_image.h"

#include <algorithm>
#include <charconv>
#include <format>
#include <print>
#include <string_view>
#include <utility>
#include <sys/ioctl.h>
#include <unistd.h>

// ============================================================================
// ArgumentParser Implementation
// ============================================================================

ArgumentParser::ArgumentParser(std::string_view program_name)
    : program_name_(program_name) {}

Config ArgumentParser::parse(int argc, char* argv[]) {
    Config config;

    for (int i = 1; i < argc; i++) {
        std::string_view arg{argv[i]};

        if (arg == "--help" || arg == "-h") {
            config.help_requested = true;
            return config;
        } else if (arg == "--invert") {
            config.invert = true;
        } else if (arg == "--input" && i + 1 < argc) {
            config.input_file = argv[++i];
        } else if (arg == "--width" && i + 1 < argc) {
            std::string_view width_str{argv[++i]};
            int value{};
            auto [ptr, ec] = std::from_chars(width_str.data(), width_str.data() + width_str.size(), value);
            if (ec != std::errc{} || value <= 0) {
                throw std::runtime_error("--width must be a positive integer");
            }
            config.width = value;
        } else {
            throw std::runtime_error(std::format("Unknown or incomplete option '{}'", arg));
        }
    }

    return config;
}

void ArgumentParser::print_usage() const {
    std::println("Usage: {} --input <file> [options]\n", program_name_);
    std::println("Convert images to ASCII art.\n");
    std::println("Options:");
    std::println("  --input <file>   Path to image file (required)");
    std::println("  --width <cols>   Output width in characters (default: terminal width or 80)");
    std::println("  --invert         Invert brightness mapping");
    std::println("  --help           Display this help message");
}

// ============================================================================
// Image Implementation
// ============================================================================

Image::Image(std::string_view filepath) {
    // stbi_load needs a null-terminated string
    std::string filepath_str{filepath};
    data_ = stbi_load(filepath_str.c_str(), &width_, &height_, &channels_, 0);
    if (!data_) {
        throw std::runtime_error(std::format("Failed to load image '{}': {}",
                                             filepath, stbi_failure_reason()));
    }
}

Image::Image(Image&& other) noexcept
    : data_(std::exchange(other.data_, nullptr)),
      width_(std::exchange(other.width_, 0)),
      height_(std::exchange(other.height_, 0)),
      channels_(std::exchange(other.channels_, 0)) {}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        if (data_) {
            stbi_image_free(data_);
        }
        data_ = std::exchange(other.data_, nullptr);
        width_ = std::exchange(other.width_, 0);
        height_ = std::exchange(other.height_, 0);
        channels_ = std::exchange(other.channels_, 0);
    }
    return *this;
}

Image::~Image() {
    if (data_) {
        stbi_image_free(data_);
    }
}

// ============================================================================
// AsciiRenderer Implementation
// ============================================================================

AsciiRenderer::AsciiRenderer(bool invert)
    : ramp_(DEFAULT_RAMP), invert_(invert) {}

AsciiRenderer::AsciiRenderer(std::string_view custom_ramp, bool invert)
    : ramp_(custom_ramp), invert_(invert) {}

char AsciiRenderer::brightness_to_char(std::uint8_t brightness) const noexcept {
    if (invert_) {
        brightness = 255 - brightness;
    }
    auto ramp_length = static_cast<int>(ramp_.length());
    int index = (brightness * (ramp_length - 1)) / 255;
    return ramp_[static_cast<std::size_t>(index)];
}

std::string AsciiRenderer::render(const Image& image, int output_width) const {
    return render(image.data(), image.width(), image.height(), image.channels(), output_width);
}

std::string AsciiRenderer::render(const std::uint8_t* img_data, int img_width, int img_height,
                                  int channels, int output_width) const {
    std::string result;

    double scale = static_cast<double>(output_width) / img_width;
    int output_height = std::max(1, static_cast<int>(img_height * scale / 2.0));

    result.reserve(static_cast<std::size_t>((output_width + 1) * output_height));

    for (int y = 0; y < output_height; ++y) {
        for (int x = 0; x < output_width; ++x) {
            int img_x = std::min(static_cast<int>(x / scale), img_width - 1);
            int img_y = std::min(static_cast<int>(y * 2.0 / scale), img_height - 1);

            auto pixel_index = static_cast<std::size_t>((img_y * img_width + img_x) * channels);
            std::uint8_t r = img_data[pixel_index];
            std::uint8_t g = img_data[pixel_index + 1];
            std::uint8_t b = img_data[pixel_index + 2];

            std::uint8_t brightness = rgb_to_brightness(r, g, b);
            result += brightness_to_char(brightness);
        }
        result += '\n';
    }

    return result;
}

// ============================================================================
// Utility Functions
// ============================================================================

int get_terminal_width() noexcept {
    struct winsize w{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        return w.ws_col;
    }
    return 80;
}

// ============================================================================
// Legacy Free Functions (for backward compatibility)
// ============================================================================

constexpr std::string_view ASCII_RAMP = "@%#*+=-:. ";

std::uint8_t rgb_to_brightness(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    return AsciiRenderer::rgb_to_brightness(r, g, b);
}

char brightness_to_char(std::uint8_t brightness, bool invert) {
    if (invert) {
        brightness = 255 - brightness;
    }
    auto index = static_cast<std::size_t>((brightness * (ASCII_RAMP.length() - 1)) / 255);
    return ASCII_RAMP[index];
}

std::string render_ascii(const std::uint8_t* img_data, int img_width, int img_height,
                         int channels, int output_width, bool invert) {
    AsciiRenderer renderer(invert);
    return renderer.render(img_data, img_width, img_height, channels, output_width);
}

