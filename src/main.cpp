#include <iostream>
#include <stdint.h>
#include <cmath>
#include <string>
#include <format>
#include "renderFrame.hpp"

int main(void) {
    mandelImage image;
    image.setWidth(1920);
    image.setHeight(1080);
    renderData render = {0};
    render.iterations = 1000;
    std::string fileName;
    uint32_t anim, frames; //anim is small
    
    std::cout << "What are the coordinates of the point to centre on?\n";
    while (1) {
        if((std::cin >> render.aCentre >> render.bCentre).good()) break;
        std::cout << "Incorrect formatting, retry\n";
    }
    std::cout << "What zoom level?\n";
    while (1) {
        if((std::cin >> render.zoom).good()) break;
        std::cout << "Incorrect formatting, retry\n";
    }
    std::cout << "0 for single image, 1 for zoom animation\n";
    while (1) {
        if((std::cin >> anim).good() && (anim == 0 || anim == 1)) break;
        std::cout << "Incorrect formatting, retry\n";
    }

    switch (anim) {
    case 0:
        std::cout << "Generating image\n";
        image.renderFrame("img.bmp", render);
        std::cout << "Image done\n";
        break;
    case 1:
        std::cout << "How many frames?\n";
        while (1) {
            if((std::cin >> frames).good()) break;
            std::cout << "Incorrect formatting, retry\n";
        }
        for (int i = 0; i < frames; i++) {
            render.zoom = pow(2, (double)i / 30);
            fileName.clear();
            fileName = std::format("{}.bmp", i+1);

            image.renderFrame(fileName, render);
            std::cout << fileName << '\n';
        }
        break;
    }

    return 0;
}