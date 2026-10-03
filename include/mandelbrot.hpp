#pragma once

#include <string>
#include <cstddef>

#include "image.hpp"


struct render_data {
    double a_centre;
    double b_centre;
    double zoom;
    std::size_t iterations;
};

enum render_type {
    MANDELBROT,
    JULIA,
};

class mandel_image : public image {
    public:
    void render(const std::string &file_name, const render_data &render, render_type type);

    private:
    void mandelbrot_render(const render_data &render);
    void julia_render(const render_data &render);

    std::size_t mandelbrot_check(double ca, double cb, std::size_t iter);
    std::size_t julia_check(double za, double zb, double ca, double cb, std::size_t iter);
};
