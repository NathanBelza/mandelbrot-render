#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "renderFrame.hpp"
#include "mandelbrotCheck.hpp"

uint32_t pixIdx(uint16_t x,uint16_t y) {return x + width * y;}

//Get contrast of pixel compared to neighboring pixels
uint8_t getContr(pixData* pixArr,uint16_t x,uint16_t y);

int8_t renderFrame(const char fileName[], double aCentre, double bCentre, double zoom) {
    FILE* fp;
    fp = fopen(fileName, "wb");
    if(!fp) return -1;
    uint32_t rowBytes = ((3 * width + 3) / 4) * 4; //3 bytes per pixel + ensure multiple of 4 bytes
    uint64_t byteCount = 14 + 40 + rowBytes * height;
    uint8_t* bytes = (uint8_t*)calloc(byteCount, 1); //Create array for bytes in bitmap file
    if(!bytes) {
        fclose(fp);
        return -2;
    }

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
    if(!pixArr) {
        fclose(fp);
        free(bytes);
        return -3;
    }
    uint16_t x, y;
    const double div = width / 4;
    const double widthSub = width / 2;
    const double heightSub = height / 2;
    double ca, cb;
    uint32_t i;
    for(y = 0; y < height; y++) {
        for(x = 0; x < width; x++) {
            ca = (double)(x - widthSub) / (div * zoom) + aCentre; //Convert pixel coordinates onto complex plane (in a + bi)
            cb = (double)(y - heightSub) / (div * zoom) + bCentre;
            i = mandelbrot(ca, cb, iterations); //Check how many iterations until value blows up to infinity
            if(i == iterations) {
                pixArr[pixIdx(x,y)].red = 0; //Red
                pixArr[pixIdx(x,y)].green = 0; //Green
                pixArr[pixIdx(x,y)].blue = 0; //Blue
                pixArr[pixIdx(x,y)].lum = 0; //Luminance
            }
            else {
                pixArr[pixIdx(x,y)].red = (uint8_t)(((double)(242 - 11) / 19) * (i % 20)) + 11; //Red - add colour based on iterations
                pixArr[pixIdx(x,y)].green = (uint8_t)(((double)(154 - 41) / 19) * (i % 20)) + 41; //Green - add colour based on iterations
                pixArr[pixIdx(x,y)].blue = (uint8_t)(((double)(99 - 150) / 19) * (i % 20)) + 150; //Blue - add colour based on iterations
                pixArr[pixIdx(x,y)].lum = (uint8_t)((pixArr[pixIdx(x,y)].red * 0.3) + (pixArr[pixIdx(x,y)].green * 0.59) + (pixArr[pixIdx(x,y)].blue * 0.11)); //Luminance calculation
            }
        }
    }
    
    //Contrast map
    for(y = 0; y < height; y++) {
        for(x = 0; x < width; x++) {
            pixArr[pixIdx(x,y)].contr = getContr(pixArr,x,y);
            if(pixArr[pixIdx(x,y)].contr < contrastThreshold) pixArr[pixIdx(x,y)].contr = 0;
        }
    }

    //Blend factor - TODO
    for(y = 0; y < height; y++) {
        for(x = 0; x < width; x++) {
            if(pixArr[pixIdx(x,y)].contr != 0) {




            }
        }
    }

    int index = 54;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            bytes[index] = pixArr[pixIdx(x,y)].contr; //Blue
            index++;
            bytes[index] = pixArr[pixIdx(x,y)].contr; //Green
            index++;
            bytes[index] = pixArr[pixIdx(x,y)].contr; //Red
            index++;
        }
    }

    /* //Need to make output two BMPs
    int index = 54;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            bytes[index] = pixArr[pixIdx(x,y)].blue; //Blue
            index++;
            bytes[index] = pixArr[pixIdx(x,y)].green; //Green
            index++;
            bytes[index] = pixArr[pixIdx(x,y)].red; //Red
            index++;
        }
    }*/
    
    fwrite(bytes, 1, byteCount, fp);
    fclose(fp);
    free(bytes);
    free(pixArr);
    return 0;
}

uint8_t getContr(pixData* pixArr,uint16_t x,uint16_t y) {
    uint8_t maxLum = 0, minLum = 255;
    uint8_t lum;
    int16_t nx, ny;
    for(int8_t dy = -1; dy <= 1; dy++) {
        for(int8_t dx = -1; dx <= 1; dx++) {
            if(dx != 0 && dy != 0) continue; //Skip pixel if diagonal to centre
            nx = x + dx;
            ny = y + dy;
            if(nx >= 0 && nx < width && ny >= 0 && ny < height) { //Check for pixel not out of image bounds
                lum = pixArr[pixIdx((int16_t)x+dx,y+dy)].lum;
                if(lum > maxLum) maxLum = lum;
                if(lum < minLum) minLum = lum;
            }
        }
    }
    return maxLum - minLum;
}