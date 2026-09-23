#pragma once

#include <vector>
#include <string>
#include <cstdint>

#define print_res 2835
#define contrast_threshold 16

struct render_data {
    double a_centre;
    double b_centre;
    double zoom;
    uint32_t iterations;
};

struct pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t fxaa_red;
    uint8_t fxaa_green;
    uint8_t fxaa_blue;
    uint8_t lum;
    uint8_t contr;
};

class mandel_image {
    private:
    std::vector<pixel> pixel_data;
    uint32_t width;
    uint32_t height;
    bool edge_dir, edge_sign;

    public:
    int8_t render_frame(std::string file_name, render_data render);

    void set_width(uint32_t new_width) {
        width = new_width;
    }
    void set_height(uint32_t new_height) {
        height = new_height;
    }
    uint32_t get_width() {
        return width;
    }
    uint32_t get_height() {
        return height;
    }

    void index_to_coord(uint32_t index, uint32_t& x, uint32_t& y) {
        x = index % width;
        y = (index - x) / width;
    }
    uint32_t coord_to_index(uint32_t x, uint32_t y) {
        return x + width * y;
    }

    private:
    void populate_image(render_data render);
    uint32_t mandelbrot_check(double ca, double cb, uint32_t iter);
    
    void apply_fxaa();

    void get_contr();
    float get_pix_blend_factor(uint32_t x, uint32_t y);
    void get_edge_dir(uint32_t x, uint32_t y);

    float smoothstep(float a) {
        if(a < 0) return 0;
        else if(a > 1.0) return 1.0;
        else return 3 * a * a - 2 * a * a * a;
    }
};
