#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "renderFrame.hpp"
#include "mandelbrotCheck.hpp"

uint8_t max(uint8_t a,uint8_t b) {
    if(a > b) return a;
    return b;
}

uint8_t min(uint8_t a,uint8_t b) {
    if(a < b) return a;
    return b;
}

uint32_t pixIdx(uint16_t x,uint16_t y) {return x + width * y;}

int8_t renderFrame(const char fileName[], double aCentre, double bCentre, double zoom) {
    FILE* fp;
    fp = fopen(fileName, "wb");
    if(!fp) return -1; //Maybe change later to not quit whole program
    uint32_t rowCount = ((3 * width + 3) / 4) * 4; //3 bytes per pixel + ensure multiple of 4 bytes
    uint64_t byteCount = 14 + 40 + rowCount * height;
    uint8_t* bytes = (uint8_t*)calloc(byteCount, 1); //Create array for bytes in bitmap file
    if(!bytes) return -2;

    //Generate .BMP header data
    bytes[0x0] = 'B';
    bytes[0x1] = 'M';
    bytes[0x2] = byteCount & 0xFF; //File size
    bytes[0x3] = byteCount >> 8 & 0xFF;
    bytes[0x4] = byteCount >> 16 & 0xFF;
    bytes[0x5] = byteCount >> 24 & 0xFF;
    bytes[0xA] = 54; //Header bytes
    //DIB Header
    bytes[0xE] = 40; //DIB Header Bytes
    bytes[0x12] = width & 0xFF; //Pixel width
    bytes[0x13] = width >> 8 & 0xFF;
    bytes[0x14] = width >> 16 & 0xFF;
    bytes[0x15] = width >> 24 & 0xFF;
    bytes[0x16] = height & 0xFF; //Pixel height
    bytes[0x17] = height >> 8 & 0xFF;
    bytes[0x18] = height >> 16 & 0xFF;
    bytes[0x19] = height >> 24 & 0xFF;
    bytes[0x1A] = 1; //Colour planes
    bytes[0x1C] = 24; //Bits per pixel
    bytes[0x22] = (byteCount - 54) & 0xFF; //Pixel array size
    bytes[0x23] = (byteCount - 54) >> 8 & 0xFF;
    bytes[0x24] = (byteCount - 54) >> 16 & 0xFF;
    bytes[0x25] = (byteCount - 54) >> 24 & 0xFF;
    bytes[0x26] = printRes & 0xFF; //Horizontal printRes
    bytes[0x27] = printRes >> 8 & 0xFF;
    bytes[0x28] = printRes >> 16 & 0xFF;
    bytes[0x29] = printRes >> 24 & 0xFF;
    bytes[0x2A] = printRes & 0xFF; //Vertical printRes
    bytes[0x2B] = printRes >> 8 & 0xFF;
    bytes[0x2C] = printRes >> 16 & 0xFF;
    bytes[0x2D] = printRes >> 24 & 0xFF;

    pixData* pixArr = (pixData*)malloc(width * height * sizeof(pixData)); //Create an array for pixel values
    if(!pixArr) return -3;
    uint16_t x, y;
    int i;
    double widthSub, heightSub, div;
    div = width / 4;
    widthSub = width / 2;
    heightSub = height / 2;
    double ca, cb;
    
    for(y = 0; y < height; y++) {
        for(x = 0; x < width; x++) {
            ca = (double)(x - widthSub) / (div * zoom) + aCentre;
            cb = (double)(y - heightSub) / (div * zoom) + bCentre;
            i = mandelbrot(ca, cb, iterations);
            if(i == iterations) {
                pixArr[pixIdx(x,y)].red = 0; //Red
                pixArr[pixIdx(x,y)].green = 0; //Green
                pixArr[pixIdx(x,y)].blue = 0; //Blue
                pixArr[pixIdx(x,y)].lum = 0; //Luminance
            }
            else {
                pixArr[pixIdx(x,y)].red = (uint8_t)(((double)(242 - 11) / 19) * (i % 20)) + 11; //Red
                pixArr[pixIdx(x,y)].green = (uint8_t)(((double)(154 - 41) / 19) * (i % 20)) + 41; //Green
                pixArr[pixIdx(x,y)].blue = (uint8_t)(((double)(99 - 150) / 19) * (i % 20)) + 150; //Blue
                pixArr[pixIdx(x,y)].lum = (uint8_t)((pixArr[pixIdx(x,y)].red * 0.3) + (pixArr[pixIdx(x,y)].green * 0.59) + (pixArr[pixIdx(x,y)].blue * 0.11)); //Luminance
            }
        }
    }
    
    //Contrast map
    //Top row
    pixArr[pixIdx(0,0)].contr = max(max(pixArr[pixIdx(0,0)].lum, pixArr[pixIdx(1,0)].lum),pixArr[pixIdx(0,1)].lum)
    - min(min(pixArr[pixIdx(0,0)].lum, pixArr[pixIdx(1,0)].lum), pixArr[pixIdx(0,1)].lum);
    for(x = 1; x < width - 1; x++) {
        pixArr[pixIdx(x,0)].contr = max(max(max(pixArr[pixIdx(x,0)].lum, pixArr[pixIdx(x+1,0)].lum), pixArr[pixIdx(x-1,0)].lum), pixArr[pixIdx(x,1)].lum)
        - min(min(min(pixArr[pixIdx(x,0)].lum, pixArr[pixIdx(x+1,0)].lum), pixArr[pixIdx(x-1,0)].lum), pixArr[pixIdx(x,1)].lum);
    }
    pixArr[pixIdx(width-1,0)].contr = max(max(pixArr[pixIdx(width-1,0)].lum, pixArr[pixIdx(width-2,0)].lum),pixArr[pixIdx(width-1,1)].lum)
    - min(min(pixArr[pixIdx(width-1,0)].lum, pixArr[pixIdx(width-2,0)].lum), pixArr[pixIdx(width-1,1)].lum);
    //Middle
    for(y = 1; y < height - 1; y++) {
        pixArr[pixIdx(0,y)].contr = max(max(max(pixArr[pixIdx(0,y)].lum, pixArr[pixIdx(1,y)].lum), pixArr[pixIdx(0,y+1)].lum), pixArr[pixIdx(0,y-1)].lum)
        - min(min(min(pixArr[pixIdx(0,y)].lum, pixArr[pixIdx(1,y)].lum), pixArr[pixIdx(0,y+1)].lum), pixArr[pixIdx(0,y-1)].lum);
        for(x = 1; x < width - 1; x++) {
            pixArr[pixIdx(x,y)].contr = max(max(max(max(pixArr[pixIdx(x,y)].lum, pixArr[pixIdx(x+1,y)].lum), pixArr[pixIdx(x-1,y)].lum), pixArr[pixIdx(x,y+1)].lum), pixArr[pixIdx(x,y-1)].lum)
            - min(min(min(min(pixArr[pixIdx(x,y)].lum, pixArr[pixIdx(x+1,y)].lum), pixArr[pixIdx(x-1,y)].lum), pixArr[pixIdx(x,y+1)].lum), pixArr[pixIdx(x,y-1)].lum);
        }
        pixArr[pixIdx(width-1,y)].contr = max(max(max(pixArr[pixIdx(width-1,y)].lum, pixArr[pixIdx(width-2,y)].lum), pixArr[pixIdx(width-1,y+1)].lum), pixArr[pixIdx(width-1,y-1)].lum)
        - min(min(min(pixArr[pixIdx(width-1,y)].lum, pixArr[pixIdx(width-2,y)].lum), pixArr[pixIdx(width-1,y+1)].lum), pixArr[pixIdx(width-1,y-1)].lum);
    }
    //Bottom row
    pixArr[pixIdx(0,height-1)].contr = max(max(pixArr[pixIdx(0,height-1)].lum, pixArr[pixIdx(1,height-1)].lum), pixArr[pixIdx(0,height-2)].lum)
    - min(min(pixArr[pixIdx(0,height-1)].lum, pixArr[pixIdx(1,height-1)].lum), pixArr[pixIdx(0,height-2)].lum);
    for(x = 1; x < width - 1; x++) {
        pixArr[pixIdx(x,height-1)].contr = max(max(max(pixArr[pixIdx(x,height-1)].lum, pixArr[pixIdx(x+1,height-1)].lum), pixArr[pixIdx(x-1,height-1)].lum), pixArr[pixIdx(x,height-2)].lum)
        - min(min(min(pixArr[pixIdx(x,height-1)].lum, pixArr[pixIdx(x+1,height-1)].lum), pixArr[pixIdx(x-1,height-1)].lum), pixArr[pixIdx(x,height-2)].lum);
    }
    pixArr[pixIdx(width-1,height-1)].contr = max(max(pixArr[pixIdx(width-1,height-1)].lum, pixArr[pixIdx(width-2,height-1)].lum), pixArr[pixIdx(width-1,height-2)].lum)
    - min(min(pixArr[pixIdx(width-1,height-1)].lum, pixArr[pixIdx(width-2,height-1)].lum), pixArr[pixIdx(width-1,height-2)].lum);

    //Contrast threshold
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (pixArr[pixIdx(x,y)].contr < 16) pixArr[pixIdx(x,y)].contr = 0;
        }
    }

    //Blend factor
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (pixArr[pixIdx(x,y)].contr != 0) {




            }
        }
    }
/*
    int index = 54;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            bytes[index] = pixArr[pixIdx(x,y)].contr; //Blue
            index++;
            bytes[index] = pixArr[pixIdx(x,y)].contr; //Green
            index++;
            bytes[index] = pixArr[pixIdx(x,y)].contr; //Red
            index++;
        }
    }*/

    
    int index = 54;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            bytes[index] = pixArr[pixIdx(x,y)].blue; //Blue
            index++;
            bytes[index] = pixArr[pixIdx(x,y)].green; //Green
            index++;
            bytes[index] = pixArr[pixIdx(x,y)].red; //Red
            index++;
        }
    }
    
    fwrite(bytes, 1, byteCount, fp);
    fclose(fp);
    free(bytes);
    free(pixArr);
    return 0;
}