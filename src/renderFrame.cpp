#include <stdint.h>
#include <cstdlib>
#include <cmath>
#include <stdbool.h>
#include "renderFrame.hpp"

int8_t mandelImage::renderFrame(std::string fileName, renderData render) {
    FILE* fp;
    fp = fopen(fileName.c_str(), "wb");
    if(!fp) return -1;
    const uint32_t rowBytes = ((3 * width + 3) / 4) * 4; //3 bytes per pixel + ensure multiple of 4 bytes
    const uint64_t byteCount = 14 + 40 + rowBytes * height;
    uint8_t* bytes = (uint8_t*)calloc((size_t)byteCount, 1); //Create array for bytes in bitmap file
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

    populateImage(render);
    applyFXAA();

    //Need to make output two BMPs
    int x,y;
    int indx = 54;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            bytes[indx++] = pixelData[coordToIndex(x,y)].fxaaBlue; //Blue
            bytes[indx++] = pixelData[coordToIndex(x,y)].fxaaGreen; //Green
            bytes[indx++] = pixelData[coordToIndex(x,y)].fxaaRed; //Red
        }
        for(uint8_t pad = 0; pad < rowBytes - getWidth() * 3; pad++) bytes[indx++] = 0x00; //Make sure rows have a multiple of 4 bytes
    }
    
    fwrite(bytes, 1, byteCount, fp);
    fclose(fp);
    free(bytes);
    return 0;
}

uint32_t mandelImage::mandelbrotCheck(double ca, double cb, uint32_t iter) {
    double za = 0, zb = 0, za2 = 0, zb2 = 0;
    uint32_t i;
    for (i = 0; i < iter; i++) {
        zb = (2 * za * zb) + cb;
        za = (za2 - zb2) + ca;
        za2 = za * za;
        zb2 = zb * zb;
        if (za2 + zb2 > 4) break;
    }
    return i;
}

void mandelImage::populateImage(renderData render) {
    pixelData.resize(width * height);

    uint16_t x, y;
    const double div = width / 4;
    const double widthSub = width / 2;
    const double heightSub = height / 2;
    double ca, cb;
    uint32_t i;
    for(y = 0; y < height; y++) {
        for(x = 0; x < width; x++) {
            ca = (double)(x - widthSub) / (div * render.zoom) + render.aCentre; //Convert pixel coordinates onto complex plane (in a + bi)
            cb = (double)(y - heightSub) / (div * render.zoom) + render.bCentre;
            i = mandelbrotCheck(ca, cb, render.iterations); //Check how many iterations until value blows up to infinity
            if(i == render.iterations) {
                pixelData[coordToIndex(x,y)].red = 0; //Red
                pixelData[coordToIndex(x,y)].green = 0; //Green
                pixelData[coordToIndex(x,y)].blue = 0; //Blue
                pixelData[coordToIndex(x,y)].lum = 0; //Luminance
            }
            else {
                pixelData[coordToIndex(x,y)].red = (uint8_t)(((double)(242 - 11) / 19) * (i % 20) + 11); //Red - add colour based on iterations
                pixelData[coordToIndex(x,y)].green = (uint8_t)(((double)(154 - 41) / 19) * (i % 20) + 41); //Green - add colour based on iterations
                pixelData[coordToIndex(x,y)].blue = (uint8_t)(((double)(99 - 150) / 19) * (i % 20) + 150); //Blue - add colour based on iterations
                pixelData[coordToIndex(x,y)].lum = (pixelData[coordToIndex(x,y)].red * 0.3) + (pixelData[coordToIndex(x,y)].green * 0.59) + (pixelData[coordToIndex(x,y)].blue * 0.11); //Luminance calculation
            }
        }
    }
    getContr();
}

void mandelImage::applyFXAA() {
    float blendFactor;
    for(uint32_t y = 0; y < height; y++) {
        for(uint32_t x = 0; x < width; x++) {
            pixelData[coordToIndex(x,y)].fxaaRed = pixelData[coordToIndex(x,y)].red;
            pixelData[coordToIndex(x,y)].fxaaGreen = pixelData[coordToIndex(x,y)].green;
            pixelData[coordToIndex(x,y)].fxaaBlue = pixelData[coordToIndex(x,y)].blue;

            if(pixelData[coordToIndex(x,y)].contr != 0) {
                blendFactor = getPixBlendFactor(x,y);
                getEdgeDir(x,y);
                if(edgeDir && edgeSign && y < height-1) { //Blend north
                    pixelData[coordToIndex(x,y)].fxaaRed = pixelData[coordToIndex(x,y)].red * (1 - blendFactor) + pixelData[coordToIndex(x,y+1)].red * blendFactor;
                    pixelData[coordToIndex(x,y)].fxaaGreen = pixelData[coordToIndex(x,y)].green * (1 - blendFactor) + pixelData[coordToIndex(x,y+1)].green * blendFactor;
                    pixelData[coordToIndex(x,y)].fxaaBlue = pixelData[coordToIndex(x,y)].blue * (1 - blendFactor) + pixelData[coordToIndex(x,y+1)].blue * blendFactor;
                } else if(edgeDir && !edgeSign && y > 0) { //Blend south
                    pixelData[coordToIndex(x,y)].fxaaRed = pixelData[coordToIndex(x,y)].red * (1 - blendFactor) + pixelData[coordToIndex(x,y-1)].red * blendFactor;
                    pixelData[coordToIndex(x,y)].fxaaGreen = pixelData[coordToIndex(x,y)].green * (1 - blendFactor) + pixelData[coordToIndex(x,y-1)].green * blendFactor;
                    pixelData[coordToIndex(x,y)].fxaaBlue = pixelData[coordToIndex(x,y)].blue * (1 - blendFactor) + pixelData[coordToIndex(x,y-1)].blue * blendFactor;
                } else if(!edgeDir && edgeSign && x < width-1) { //Blend east
                    pixelData[coordToIndex(x,y)].fxaaRed = pixelData[coordToIndex(x,y)].red * (1 - blendFactor) + pixelData[coordToIndex(x+1,y)].red * blendFactor;
                    pixelData[coordToIndex(x,y)].fxaaGreen = pixelData[coordToIndex(x,y)].green * (1 - blendFactor) + pixelData[coordToIndex(x+1,y)].green * blendFactor;
                    pixelData[coordToIndex(x,y)].fxaaBlue = pixelData[coordToIndex(x,y)].blue * (1 - blendFactor) + pixelData[coordToIndex(x+1,y)].blue * blendFactor;
                } else if(!edgeDir && !edgeSign && x > 0) { //Blend west
                    pixelData[coordToIndex(x,y)].fxaaRed = pixelData[coordToIndex(x,y)].red * (1 - blendFactor) + pixelData[coordToIndex(x-1,y)].red * blendFactor;
                    pixelData[coordToIndex(x,y)].fxaaGreen = pixelData[coordToIndex(x,y)].green * (1 - blendFactor) + pixelData[coordToIndex(x-1,y)].green * blendFactor;
                    pixelData[coordToIndex(x,y)].fxaaBlue = pixelData[coordToIndex(x,y)].blue * (1 - blendFactor) + pixelData[coordToIndex(x-1,y)].blue * blendFactor;
                } else {
                    pixelData[coordToIndex(x,y)].fxaaRed = pixelData[coordToIndex(x,y)].red;
                    pixelData[coordToIndex(x,y)].fxaaGreen = pixelData[coordToIndex(x,y)].green;
                    pixelData[coordToIndex(x,y)].fxaaBlue = pixelData[coordToIndex(x,y)].blue;
                }
            }
        }
    }
}

void mandelImage::getContr() {
    for(uint32_t i = 0; i < pixelData.size(); i++) {
        uint8_t maxLum = 0, minLum = 255;
        uint8_t lum;
        int16_t nx, ny;
        uint32_t x, y;
        for(int8_t dy = -1; dy <= 1; dy++) {
            for(int8_t dx = -1; dx <= 1; dx++) {
                if(dx != 0 && dy != 0) continue; //Skip pixel if diagonal to centre
                indexToCoord(i, x ,y);
                nx = x + dx;
                ny = y + dy;
                if(nx >= 0 && nx < width && ny >= 0 && ny < height) { //Check for pixel not out of image bounds
                    lum = pixelData[coordToIndex(nx,ny)].lum;
                    if(lum > maxLum) maxLum = lum;
                    if(lum < minLum) minLum = lum;
                }
            }
        }
        pixelData[coordToIndex(x,y)].contr = maxLum - minLum;
    }
}

float mandelImage::getPixBlendFactor(uint32_t x, uint32_t y) {
    float blendFactor = 0;
    int16_t nx, ny;
    for(int8_t dy = -1; dy <= 1; dy++) {
        for(int8_t dx = -1; dx <= 1; dx++) {
            nx = x + dx;
            ny = y + dy;
            if(nx >= 0 && nx < width && ny >= 0 && ny < height) { //Check for pixel not out of image bounds
                if(dx == 0 && dy == 0) continue;
                else if(dx != 0 && dy != 0) blendFactor += pixelData[coordToIndex(nx,ny)].lum; //Weighted average of neighboring pixels
                else blendFactor += 2 * pixelData[coordToIndex(nx,ny)].lum;
            }
        }
    }
    blendFactor *= 1.0/12;
    blendFactor = fabsf(blendFactor - pixelData[coordToIndex(x,y)].lum); //Find contrast between weighted average and middle pixel
    if(pixelData[coordToIndex(x,y)].contr == 0) return 0.0;
    blendFactor = smoothstep((float) blendFactor / pixelData[coordToIndex(x,y)].contr);
    return blendFactor * blendFactor; //Squared smoothstep with clamping in 0-1
}

void mandelImage::getEdgeDir(uint32_t x, uint32_t y) {
    bool isHorizontal;
    uint8_t l[3][3] = {0};

    int16_t nx, ny;
    for(int8_t dy = -1; dy <= 1; dy++) {
        for(int8_t dx = -1; dx <= 1; dx++) {
            nx = x + dx;
            ny = y + dy;
            if(nx >= 0 && nx < width && ny >= 0 && ny < height) { //Check for pixel not out of image bounds
                l[dx+1][dy+1] = pixelData[coordToIndex(nx,ny)].lum;
            }
        }
    }

    float horizontal =
    fabs(l[1][2] + l[1][0] - 2 * l[1][1]) * 2 + //ln + ls - 2lm
    fabs(l[2][2] + l[2][0] - 2 * l[2][1]) + //lne + lse - 2le
    fabs(l[0][2] + l[0][0] - 2 * l[0][1]); //lnw + lsw - 2lw

    float vertical =
    fabs(l[2][1] + l[0][1] - 2 * l[1][1]) * 2 + //le + lw - 2lm
    fabs(l[2][2] + l[0][2] - 2 * l[1][2]) + //lne + lnw - 2ln
    fabs(l[2][0] + l[0][0] - 2 * l[1][0]); //lse + lsw - 2ls
    
    isHorizontal = horizontal >= vertical;

    if(isHorizontal) {
        if(l[1][2] > l[1][0]) { //Check if ln is bigger than ls
            edgeSign = true;
        } else edgeSign = false;
    } else {
        if(l[2][1] > l[0][1]) { //Check if le is bigger than lw
            edgeSign = true;
        } else edgeSign = false;
    }

    edgeDir = isHorizontal;
}