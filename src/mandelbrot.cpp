#include <cstdint>
#include <cstddef>

#include "mandelbrot.hpp"

namespace mandel {

// check if a point is in the mandelbrot set, returns number of iterations until divergence
std::size_t mandel_image::mandelbrot_check(double ca, double cb, std::size_t iter) {
    double za = 0, zb = 0, za2 = 0, zb2 = 0;

    std::size_t i = 0;
    for (i = 0; i < iter; i++) {
        if (za2 + zb2 > 4) { // Point is guarunteed not in set when magnitude greater than 2
            break;
        }

        zb = (2 * za * zb) + cb;
        za = (za2 - zb2) + ca; // zb was updated, but za2 and zb2 are cached from previous iteration

        za2 = za * za;
        zb2 = zb * zb;
    }
    return i;
}

// Check if a point is in the julia set, returns number of iterations until divergence
std::size_t mandel_image::julia_check(double za, double zb, double ca, double cb, std::size_t iter) {
    double za2 = za * za;
    double zb2 = zb * zb;

    std::size_t i = 0;
    for (i = 0; i < iter; i++) {
        if (za2 + zb2 > 4) { // Point is guarunteed not in set when magnitude greater than 2
            break;
        }

        zb = (2 * za * zb) + cb;
        za = (za2 - zb2) + ca; // zb was updated, but za2 and zb2 are cached from previous iteration

        za2 = za * za;
        zb2 = zb * zb;
    }
    return i;
}

static void colour_iterations(std::size_t iterations, std::size_t max_iter, images::pixel& p) {

    if(iterations == max_iter) {
        p.red = 0;
        p.green = 0;
        p.blue = 0;
    } else {
        // Colour based on iterations, repeat using modulo, blend between C1 and C2
        constexpr std::uint8_t C1_RED = 11;
        constexpr std::uint8_t C1_GREEN = 41;
        constexpr std::uint8_t C1_BLUE = 150;

        constexpr std::uint8_t C2_RED = 242;
        constexpr std::uint8_t C2_GREEN = 154;
        constexpr std::uint8_t C2_BLUE = 99;

        constexpr std::size_t repeat_i = 20;
        static_assert(repeat_i > 0);

        p.red = (std::uint8_t)(((double)(C2_RED - C1_RED) / (repeat_i - 1)) * (iterations % repeat_i) + C1_RED);
        p.green = (std::uint8_t)(((double)(C2_GREEN - C1_GREEN) / (repeat_i - 1)) * (iterations % repeat_i) + C1_GREEN);
        p.blue = (std::uint8_t)(((double)(C2_BLUE - C1_BLUE) / (repeat_i - 1)) * (iterations % repeat_i) + C1_BLUE);
    }
}

void mandel_image::mandelbrot_render(const render_data &render) {
    const auto width = get_width();
    const auto height = get_height();
    pixel_data.resize(width * height);

    const double div = width / 4.0; // So that zoom of 1 corresponds to x range of -2 to 2
    const double width_sub = width / 2.0; // Put centre of screen at half of width and height
    const double height_sub = height / 2.0;

    for(std::size_t y = 0; y < height; y++) {
        for(std::size_t x = 0; x < width; x++) {
            // Convert pixel coordinates onto complex plane (in a + bi)
            double ca = (static_cast<double>(x) - width_sub) / (div * render.zoom) + render.a_centre;
            double cb = (static_cast<double>(y) - height_sub) / (div * render.zoom) + render.b_centre;
            std::size_t i = mandelbrot_check(ca, cb, render.iterations); // Check how many iterations until value blows up to infinity

            images::pixel& p = pixel_data[coord_to_index(x,y)];
            colour_iterations(i, render.iterations, p);
        }
    }
    
    apply_fxaa();
}


void mandel_image::julia_render(const render_data &render) {
    const auto width = get_width();
    const auto height = get_height();
    pixel_data.resize(width * height);

    const double div = width / 4.0; // So that zoom of 1 corresponds to x range of -2 to 2
    const double width_sub = width / 2.0; // Put centre of screen at half of width and height
    const double height_sub = height / 2.0;

    for(std::size_t y = 0; y < height; y++) {
        for(std::size_t x = 0; x < width; x++) {
            // Convert pixel coordinates onto complex plane (in a + bi)
            double za = (static_cast<double>(x) - width_sub) / (div * render.zoom);
            double zb = (static_cast<double>(y) - height_sub) / (div * render.zoom);
            std::size_t i = julia_check(za, zb, render.a_centre, render.b_centre, render.iterations); // Check how many iterations until value blows up to infinity

            images::pixel& p = pixel_data[coord_to_index(x,y)];
            colour_iterations(i, render.iterations, p);
        }
    }
    
    apply_fxaa();
}


void mandel_image::render(const std::string &file_name, const render_data &render, render_type type) {
    switch (type) {
    case render_type::MANDELBROT:
        mandelbrot_render(render);
        break;
    case render_type::JULIA:
        julia_render(render);
        break;
    }
    save_bmp(file_name);
}

}