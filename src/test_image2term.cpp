#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "image2term.h"

#include <cassert>
#include <cstdint>
#include <print>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::print("Running {}... ", #name); \
    try { \
        test_##name(); \
        std::println("PASSED"); \
        tests_passed++; \
    } catch (...) { \
        std::println("FAILED (exception)"); \
        tests_failed++; \
    } \
} while(0)

#define ASSERT_EQ(expected, actual) do { \
    if ((expected) != (actual)) { \
        std::println("FAILED\n  Expected: {}, Got: {}", static_cast<int>(expected), static_cast<int>(actual)); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_STR_EQ(expected, actual) do { \
    if ((expected) != (actual)) { \
        std::println("FAILED\n  Expected: \"{}\"\n  Got: \"{}\"", expected, actual); \
        tests_failed++; \
        return; \
    } \
} while(0)

// ============================================================================
// rgb_to_brightness tests
// ============================================================================

TEST(rgb_to_brightness_black) {
    ASSERT_EQ(0, rgb_to_brightness(0, 0, 0));
}

TEST(rgb_to_brightness_white) {
    ASSERT_EQ(255, rgb_to_brightness(255, 255, 255));
}

TEST(rgb_to_brightness_red) {
    // 0.299 * 255 = 76.245
    auto result = rgb_to_brightness(255, 0, 0);
    assert(result >= 75 && result <= 77);
}

TEST(rgb_to_brightness_green) {
    // 0.587 * 255 = 149.685
    auto result = rgb_to_brightness(0, 255, 0);
    assert(result >= 149 && result <= 151);
}

TEST(rgb_to_brightness_blue) {
    // 0.114 * 255 = 29.07
    auto result = rgb_to_brightness(0, 0, 255);
    assert(result >= 28 && result <= 30);
}

TEST(rgb_to_brightness_gray) {
    // Gray should return approximately same value (may differ by 1 due to rounding)
    auto result = rgb_to_brightness(128, 128, 128);
    assert(result >= 127 && result <= 128);
}

TEST(rgb_to_brightness_mixed) {
    // 0.299*100 + 0.587*150 + 0.114*200 = 29.9 + 88.05 + 22.8 = 140.75
    auto result = rgb_to_brightness(100, 150, 200);
    assert(result >= 140 && result <= 142);
}

// ============================================================================
// brightness_to_char tests
// ============================================================================

TEST(brightness_to_char_darkest) {
    // Brightness 0 should map to '@' (first char in ramp)
    ASSERT_EQ('@', brightness_to_char(0, false));
}

TEST(brightness_to_char_brightest) {
    // Brightness 255 should map to ' ' (last char in ramp)
    ASSERT_EQ(' ', brightness_to_char(255, false));
}

TEST(brightness_to_char_middle) {
    // Middle brightness should map to middle character
    char c = brightness_to_char(128, false);
    // Should be somewhere in the middle of the ramp
    assert(c == '+' || c == '=' || c == '-');
}

TEST(brightness_to_char_invert_darkest) {
    // With invert, brightness 0 should map to ' ' (brightest char)
    ASSERT_EQ(' ', brightness_to_char(0, true));
}

TEST(brightness_to_char_invert_brightest) {
    // With invert, brightness 255 should map to '@' (darkest char)
    ASSERT_EQ('@', brightness_to_char(255, true));
}

// ============================================================================
// render_ascii tests
// ============================================================================

TEST(render_ascii_single_black_pixel) {
    std::uint8_t img[] = {0, 0, 0};  // Single black pixel
    std::string result = render_ascii(img, 1, 1, 3, 1, false);
    ASSERT_STR_EQ(std::string("@\n"), result);
}

TEST(render_ascii_single_white_pixel) {
    std::uint8_t img[] = {255, 255, 255};  // Single white pixel
    std::string result = render_ascii(img, 1, 1, 3, 1, false);
    ASSERT_STR_EQ(std::string(" \n"), result);
}

TEST(render_ascii_2x2_checkerboard) {
    // 2x2 checkerboard: black, white, white, black
    std::uint8_t img[] = {
        0, 0, 0,       255, 255, 255,
        255, 255, 255, 0, 0, 0
    };
    std::string result = render_ascii(img, 2, 2, 3, 2, false);
    // Due to aspect ratio correction (height/2), 2x2 becomes 2x1
    ASSERT_EQ(2 + 1, static_cast<int>(result.length()));  // 2 chars + newline
}

TEST(render_ascii_with_alpha_channel) {
    // RGBA pixel (alpha should be ignored)
    std::uint8_t img[] = {255, 255, 255, 128};  // White with 50% alpha
    std::string result = render_ascii(img, 1, 1, 4, 1, false);
    ASSERT_STR_EQ(std::string(" \n"), result);
}

TEST(render_ascii_invert) {
    std::uint8_t img[] = {0, 0, 0};  // Black pixel
    std::string result = render_ascii(img, 1, 1, 3, 1, true);
    // Inverted: black becomes white char
    ASSERT_STR_EQ(std::string(" \n"), result);
}

TEST(render_ascii_width_scaling) {
    // 1x1 image scaled to width 5
    // With aspect ratio correction, height = 1 * (5/1) / 2 = 2.5 -> 2 rows
    std::uint8_t img[] = {128, 128, 128};
    std::string result = render_ascii(img, 1, 1, 3, 5, false);
    // Should have 2 rows of 5 chars each + 2 newlines = 12 chars
    ASSERT_EQ(12, static_cast<int>(result.length()));
    // Verify structure: 5 chars + newline, repeated twice
    assert(result[5] == '\n');
    assert(result[11] == '\n');
}

int main() {
    std::println("=== image2term unit tests ===\n");

    // rgb_to_brightness tests
    RUN_TEST(rgb_to_brightness_black);
    RUN_TEST(rgb_to_brightness_white);
    RUN_TEST(rgb_to_brightness_red);
    RUN_TEST(rgb_to_brightness_green);
    RUN_TEST(rgb_to_brightness_blue);
    RUN_TEST(rgb_to_brightness_gray);
    RUN_TEST(rgb_to_brightness_mixed);

    // brightness_to_char tests
    RUN_TEST(brightness_to_char_darkest);
    RUN_TEST(brightness_to_char_brightest);
    RUN_TEST(brightness_to_char_middle);
    RUN_TEST(brightness_to_char_invert_darkest);
    RUN_TEST(brightness_to_char_invert_brightest);

    // render_ascii tests
    RUN_TEST(render_ascii_single_black_pixel);
    RUN_TEST(render_ascii_single_white_pixel);
    RUN_TEST(render_ascii_2x2_checkerboard);
    RUN_TEST(render_ascii_with_alpha_channel);
    RUN_TEST(render_ascii_invert);
    RUN_TEST(render_ascii_width_scaling);

    std::println("\n=== Results: {} passed, {} failed ===", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}

