#ifndef RENDER_FRAME
#define RENDER_FRAME

#include <vector>
#include <string>

#define printRes 2835
#define contrastThreshold 16

struct renderData {
    double aCentre;
    double bCentre;
    double zoom;
    uint32_t iterations;
};

struct Pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t fxaaRed;
    uint8_t fxaaGreen;
    uint8_t fxaaBlue;
    uint8_t lum;
    uint8_t contr;
};

class mandelImage {
    private:
    std::vector<Pixel> pixelData;
    uint32_t width;
    uint32_t height;
    bool edgeDir, edgeSign;
    public:
    int8_t renderFrame(std::string fileName, renderData render);
    void setWidth(uint32_t newWidth) {width = newWidth;}
    void setHeight(uint32_t newHeight) {height = newHeight;}
    uint32_t getWidth() {return width;}
    uint32_t getHeight() {return height;}
    void indexToCoord(uint32_t index, uint32_t& x, uint32_t& y) {
        x = index % width;
        y = (index - x) / width;
    }
    uint32_t coordToIndex(uint32_t x, uint32_t y) {return x + width * y;}
    private:
    void populateImage(renderData render);
    uint32_t mandelbrotCheck(double ca, double cb, uint32_t iter);
    void applyFXAA();
    void getContr();
    float getPixBlendFactor(uint32_t x, uint32_t y);
    void getEdgeDir(uint32_t x, uint32_t y);
    float smoothstep(float a) {
        if(a < 0) return 0;
        else if(a > 1.0) return 1.0;
        else return 3 * a * a - 2 * a * a * a;
    }
};

#endif