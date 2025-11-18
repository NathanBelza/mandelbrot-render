#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "renderFrame.hpp"

int main(void) {
    double aCentre, bCentre, zoom;
    aCentre = -0.5; //Real part at image centre
    bCentre = 0; //Imaginary part at image centre
    zoom = 1;
    char buffer[99];
    char fileName[99];
    uint32_t anim, frames;

    printf("What are the coordinates of the point to centre on?\n");
    while (1) {
        fgets(buffer, 98, stdin);
        if (sscanf(buffer, "%lf %lf", &aCentre, &bCentre) == 2) break;
        printf("Incorrect formatting, retry\n");
    }
    printf("What zoom level?\n");
    while (1) {
        fgets(buffer, 98, stdin);
        if (sscanf(buffer, "%lf", &zoom) == 1) break;
        printf("Incorrect formatting, retry\n");
    }
    printf("0 for single image, 1 for zoom animation\n");
    while (1) {
        fgets(buffer, 98, stdin);
        if (sscanf(buffer, "%d", &anim) == 1 && (anim == 0 || anim == 1)) break;
        printf("Incorrect formatting, retry\n");
    }

    switch (anim) {
    case 0:
        printf("Generating image\n");
        renderFrame("img.bmp", aCentre, bCentre, zoom);
        printf("Image done\n");
        break;
    case 1:
        printf("How many frames?\n");
        while (1) {
            fgets(buffer, 98, stdin);
            if (sscanf(buffer, "%d", &frames) == 1) break;
            printf("Incorrect formatting, retry\n");
        }
        for (int i = 0; i < frames; i++) {
            zoom = pow(2, (double)i / 30);
            fileName[0] = '\0';
            sprintf(fileName, "%d.bmp", i + 1);
            renderFrame(fileName, aCentre, bCentre, zoom);
            printf("%s\n", fileName);
        }
        break;
    }

    return 0;
}