#include <iostream>
#include <stdint.h>
#include <cmath>
#include <string>
#include <format>

#include "render_frame.hpp"

int main(void) {
    mandel_image image;
    image.set_width(1920);
    image.set_height(1080);
    render_data render = {0};
    render.iterations = 1000; // TODO: add iterations control
    std::string file_name;
    uint32_t anim, frames;
    
    std::cout << "What are the coordinates of the point to centre on?\n";
    while(!(std::cin >> render.a_centre >> render.b_centre).good()) {
        std::cout << "Incorrect formatting, retry\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
        image.render_frame("img.bmp", render);
        std::cout << "Image done\n";
        break;
    case 1:
        std::cout << "How many frames?\n";
        while(!(std::cin >> frames).good()) {
            std::cout << "Incorrect formatting, retry\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        for (int i = 0; i < frames; i++) {
            render.zoom = pow(2, (double)i / 30);
            file_name.clear();
            file_name = std::format("{}.bmp", i+1);

            image.render_frame(file_name, render);
            std::cout << file_name << '\n';
        }
        break;
    }

    return 0;
}