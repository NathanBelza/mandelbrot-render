#include <stdint.h>
#include "renderFrame.hpp"

int mandelbrot(double ca, double cb, uint32_t iter) {
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