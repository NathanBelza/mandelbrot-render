# CLI Mandelbrot Renderer

This project is a command line Mandelbrot renderer, for either a single image or animation.

Files are output as .bmp images, and animation can be stitched using a program such as Blender.

# Example Render

![alt text](media/sampleRender.bmp "Sample Render Image")

# How to use

- Input the real and imaginary parts of the centre point, separated by a space, eg.
```bash
What are the coordinates of the point to centre on?
-0.5 0.1
```
- Input the desired zoom value, eg.
```bash
What zoom level?
20
```
- Input 0 for single image, 1 for animation sequence

### Single Image (0)
- Single image render
```bash
img.bmp
```
- Image is output in the same directory as the executable.

### Animation Sequence (1)
- Input number of frames for animation, eg.
```bash
How many frames?
200
```
- Images will then output as:
```bash
1.bmp
2.bmp
3.bmp
...
200.bmp
```
- Images will output in the same directory as the executable.

# Build Instructions
- C++20 is required
```bash
mkdir build
cd build
cmake ..
cmake --build .
```
The executable will be output to
```bash
build/bin/mandelbrotRender
```