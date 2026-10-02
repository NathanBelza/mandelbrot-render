#pragma once

#include <cstdint>
#include <vector>
#include <string>

#define print_res 2835
#define contrast_threshold 16

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
    bool edge_dir, edge_sign;


    public:
    std::int8_t save_bmp(std::string file_name);

    // image height and width
    void set_width(std::uint32_t new_width) {
        width = new_width;
    }
    void set_height(std::uint32_t new_height) {
        height = new_height;
    }
    std::uint32_t get_width() {
        return width;
    }
    std::uint32_t get_height() {
        return height;
    }

    // indexing
    void index_to_coord(std::size_t index, std::size_t& x, std::size_t& y) {
        x = index % width;
        y = (index - x) / width;
    }
    std::size_t coord_to_index(std::size_t x, std::size_t y) {
        return x + width * y;
    }

    void apply_fxaa();

    void get_contr();
    float get_pix_blend_factor(std::size_t x, std::size_t y);
    void get_edge_dir(std::size_t x, std::size_t y);

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