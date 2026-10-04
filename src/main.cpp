#include <iostream>
#include <stdint.h>
#include <cmath>
#include <string>
#include <format>

#include "mandelbrot.hpp"

std::int32_t main(int argc, char* argv[]) {
    mandel::mandel_image image(1920, 1080);

    mandel::render_data render = {0};
    mandel::render_type type = mandel::render_type::MANDELBROT;
    render.iterations = 1000;
    std::string file_name = "img";
    std::uint32_t anim, frames;
    std::uint32_t type_temp;

    
    std::cout << "What are the coordinates of the point to centre on?\n";
    while(!(std::cin >> render.a_centre >> render.b_centre).good()) {
        std::cout << "Incorrect formatting, retry\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << "0 for mandelbrot, 1 for julia \n";
    while(!(std::cin >> type_temp).good()) {
        std::cout << "Incorrect formatting, retry\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    if (type_temp == 0) {
        type = mandel::render_type::MANDELBROT;
    } else {
        type = mandel::render_type::JULIA;
    }

    std::cout << "What zoom level?\n";
    while(!(std::cin >> render.zoom).good()) {
        std::cout << "Incorrect formatting, retry\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << "0 for single image, 1 for zoom animation\n";
    while(!(std::cin >> anim).good() && (anim == 0 || anim == 1)) {
        std::cout << "Incorrect formatting, retry\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    switch (anim) {
    case 0:
        std::cout << "Generating image\n";
        image.render("img.bmp", render, type);
        std::cout << "Image done\n";
        break;
    case 1:
        std::cout << "How many frames?\n";
        while(!(std::cin >> frames).good()) {
            std::cout << "Incorrect formatting, retry\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        for (std::size_t i = 0; i < frames; i++) {
            render.zoom = pow(2, i/30.0);
            file_name.clear();
            file_name = std::format("{}.bmp", i+1);

            image.render(file_name, render, type);
            std::cout << file_name << '\n';
        }
        break;
    }
        

    return 0;
}