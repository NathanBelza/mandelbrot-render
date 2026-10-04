#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>

constexpr std::uint32_t print_res = 2835;
constexpr std::uint8_t contrast_threshold = 16;

struct edge {
    bool is_horizontal;
    bool is_positive;
};

struct pixel {
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
    std::uint8_t fxaa_red;
    std::uint8_t fxaa_green;
    std::uint8_t fxaa_blue;
    std::uint8_t lum;
    std::uint8_t contr;
};

class image {
    protected:
    std::vector<pixel> pixel_data;

    private:
    std::uint32_t width;
    std::uint32_t height;

    public:
    image(std::uint32_t p_width, std::uint32_t p_height):
        width(p_width),
        height(p_height)
    {
        pixel_data.resize(width * height);
    }
    
    std::int8_t save_bmp(std::string file_name);
    void apply_fxaa();

    // image height and width
    void set_size(std::uint32_t p_width, std::uint32_t p_height) {
        width = p_width;
        height = p_height;
        pixel_data.resize(width * height);
    }
    std::uint32_t get_width() const {
        return width;
    }
    std::uint32_t get_height() const {
        return height;
    }

    protected:
    // indexing
    std::size_t coord_to_index(std::int64_t x, std::int64_t y) const {
        x = std::clamp<std::int64_t> (x, 0, static_cast<std::int64_t>(width) - 1); // Ensure casting unsigned to signed does not overflow
        y = std::clamp<std::int64_t> (y, 0, static_cast<std::int64_t>(height) - 1);
        return x + width * y;
    }

    void get_contr();
    float get_pix_blend_factor(std::uint32_t x, std::uint32_t y);
    edge get_edge_dir(std::uint32_t x, std::uint32_t y);

    private:
    float smoothstep(float a) {
        if(a < 0.0) {
            return 0;
        } else if(a > 1.0) {
            return 1.0;
        } else {
            return 3 * a * a - 2 * a * a * a;
        }
    }
};