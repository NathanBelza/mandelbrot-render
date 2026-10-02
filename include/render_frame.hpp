#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>

#include "image.hpp"


struct render_data {
    double a_centre;
    double b_centre;
    double zoom;
    std::size_t iterations;
};

class mandel_image : public image {
    public:
    void render_frame(std::string file_name, render_data render);

    private:
    void populate_image(render_data render);
    std::size_t mandelbrot_check(double ca, double cb, std::size_t iter);

};
