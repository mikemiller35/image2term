#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "image2term.h"

#include <print>

int main(int argc, char* argv[]) {
    try {
        ArgumentParser parser(argv[0]);
        Config config = parser.parse(argc, argv);

        if (config.help_requested) {
            parser.print_usage();
            return 0;
        }

        if (config.input_file.empty()) {
            std::println(stderr, "Error: --input is required");
            parser.print_usage();
            return 1;
        }

        if (config.width == 0) {
            config.width = get_terminal_width();
        }

        Image image(config.input_file);

        AsciiRenderer renderer(config.invert);
        std::string ascii = renderer.render(image, config.width);

        std::print("{}", ascii);

        return 0;
    } catch (const std::exception& e) {
        std::println(stderr, "Error: {}", e.what());
        return 1;
    }
}

