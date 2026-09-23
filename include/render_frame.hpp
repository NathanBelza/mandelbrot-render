#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>

#define print_res 2835
#define contrast_threshold 16

struct render_data {
    double a_centre;
    double b_centre;
    double zoom;
    std::size_t iterations;
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

class mandel_image {
    private:
    std::vector<pixel> pixel_data;
    std::uint32_t width;
    std::uint32_t height;
    bool edge_dir, edge_sign;

    public:
    int8_t render_frame(std::string file_name, render_data render);

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

    void index_to_coord(std::size_t index, std::size_t& x, std::size_t& y) {
        x = index % width;
        y = (index - x) / width;
    }
    std::size_t coord_to_index(std::size_t x, std::size_t y) {
        return x + width * y;
    }

    private:
    void populate_image(render_data render);
    std::size_t mandelbrot_check(double ca, double cb, std::size_t iter);
    
    void apply_fxaa();

    void get_contr();
    float get_pix_blend_factor(std::size_t x, std::size_t y);
    void get_edge_dir(std::size_t x, std::size_t y);

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
