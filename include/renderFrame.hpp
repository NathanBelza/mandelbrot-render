#ifndef RENDER_FRAME
#define RENDER_FRAME

#define printRes 2835
#define width 1920
#define height 1080
#define iterations 1000

struct pixData {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t lum;
    uint8_t contr;
};

int8_t renderFrame(const char fileName[], double aCentre, double bCentre, double zoom);

#endif