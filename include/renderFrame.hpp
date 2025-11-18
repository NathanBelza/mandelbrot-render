#ifndef RENDER_FRAME
#define RENDER_FRAME

#define printRes 2835
#define width 1920
#define height 1080
#define iterations 1000

#define red 0
#define green 1
#define blue 2
#define lum 3
#define contr 4

void renderFrame(const char fileName[], double aCentre, double bCentre, double zoom);

#endif