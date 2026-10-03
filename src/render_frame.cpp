#include <cstdint>
#include <cstddef>

#include "render_frame.hpp"


// check if a point is in the mandelbrot set, returns number of iterations until divergence
std::size_t mandel_image::mandelbrot_check(double ca, double cb, std::size_t iter) {
    double za = 0, zb = 0, za2 = 0, zb2 = 0;

    std::size_t i;
    for (i = 0; i < iter; i++) {
        zb = (2 * za * zb) + cb;
        za = (za2 - zb2) + ca;

        za2 = za * za;
        zb2 = zb * zb;

        if (za2 + zb2 > 4) {
            break;
        }
    }
    return i;
}

void mandel_image::mandelbrot_render(render_data render) {
    pixel_data.resize(get_width() * get_height());

    const double div = get_width() / 4.0; // So that zoom of 1 corresponds to x range of -2 to 2
    const double width_sub = get_width() / 2.0;
    const double height_sub = get_height() / 2.0;

    double ca = 0, cb = 0;
    for(std::size_t y = 0; y < get_height(); y++) {
        for(std::size_t x = 0; x < get_width(); x++) {
            // Convert pixel coordinates onto complex plane (in a + bi)
            ca = static_cast<double> (x - width_sub) / (div * render.zoom) + render.a_centre;
            cb = static_cast<double> (y - height_sub) / (div * render.zoom) + render.b_centre;
            std::size_t i = mandelbrot_check(ca, cb, render.iterations); // Check how many iterations until value blows up to infinity

            // Pixel colouring logic
            if(i == render.iterations) {
                pixel_data[coord_to_index(x,y)].red = 0;
                pixel_data[coord_to_index(x,y)].green = 0;
                pixel_data[coord_to_index(x,y)].blue = 0;
            }
            else {
                pixel_data[coord_to_index(x,y)].red = (std::uint8_t)(((double)(242 - 11) / 19) * (i % 20) + 11); // Red - add colour based on iterations
                pixel_data[coord_to_index(x,y)].green = (std::uint8_t)(((double)(154 - 41) / 19) * (i % 20) + 41); // Green - add colour based on iterations
                pixel_data[coord_to_index(x,y)].blue = (std::uint8_t)(((double)(99 - 150) / 19) * (i % 20) + 150); // Blue - add colour based on iterations
            }
        }
    }
    
    apply_fxaa();
}


void mandel_image::render_frame(std::string file_name, render_data render) {
    mandelbrot_render(render);
    save_bmp(file_name);
}