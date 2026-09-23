#include <cstdint>
#include <cstddef>
#include <cstdbool>
#include <cstdlib>
#include <cmath>

#include "render_frame.hpp"

int8_t mandel_image::render_frame(std::string file_name, render_data render) {

    FILE* fp;
    fp = fopen(file_name.c_str(), "wb");
    if(!fp) {
        return -1;
    }

    const uint32_t row_bytes = ((3 * width + 3) / 4) * 4; // 3 bytes per pixel + ensure multiple of 4 bytes
    const uint64_t byte_count = 14 + 40 + row_bytes * height;
    uint8_t* bytes = (uint8_t*) calloc((size_t) byte_count, 1); // Create array for bytes in bitmap file
    if(!bytes) {
        fclose(fp);
        return -2;
    }

    // Generate .BMP header data
    bytes[0x0] = 'B';
    bytes[0x1] = 'M';
    // File size
    bytes[0x2] = byte_count & 0xFF;
    bytes[0x3] = byte_count >> 8 & 0xFF;
    bytes[0x4] = byte_count >> 16 & 0xFF;
    bytes[0x5] = byte_count >> 24 & 0xFF;
    bytes[0xA] = 54; // Header bytes
    // DIB Header
    bytes[0xE] = 40; // DIB Header Bytes
    // Pixel width
    bytes[0x12] = width & 0xFF;
    bytes[0x13] = width >> 8 & 0xFF;
    bytes[0x14] = width >> 16 & 0xFF;
    bytes[0x15] = width >> 24 & 0xFF;
    // Pixel height
    bytes[0x16] = height & 0xFF;
    bytes[0x17] = height >> 8 & 0xFF;
    bytes[0x18] = height >> 16 & 0xFF;
    bytes[0x19] = height >> 24 & 0xFF;
    bytes[0x1A] = 1; // Colour planes
    bytes[0x1C] = 24; // Bits per pixel
    // Pixel array size
    bytes[0x22] = (byte_count - 54) & 0xFF;
    bytes[0x23] = (byte_count - 54) >> 8 & 0xFF;
    bytes[0x24] = (byte_count - 54) >> 16 & 0xFF;
    bytes[0x25] = (byte_count - 54) >> 24 & 0xFF;
    // Horizontal print_res
    bytes[0x26] = print_res & 0xFF;
    bytes[0x27] = print_res >> 8 & 0xFF;
    bytes[0x28] = print_res >> 16 & 0xFF;
    bytes[0x29] = print_res >> 24 & 0xFF;
    // Vertical print_res
    bytes[0x2A] = print_res & 0xFF;
    bytes[0x2B] = print_res >> 8 & 0xFF;
    bytes[0x2C] = print_res >> 16 & 0xFF;
    bytes[0x2D] = print_res >> 24 & 0xFF;

    populate_image(render);
    apply_fxaa();

    // Need to make output two BMPs
    int x,y;
    int indx = 54;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            bytes[indx++] = pixel_data[coord_to_index(x,y)].fxaa_blue;
            bytes[indx++] = pixel_data[coord_to_index(x,y)].fxaa_green;
            bytes[indx++] = pixel_data[coord_to_index(x,y)].fxaa_red;
        }
        for(uint8_t pad = 0; pad < row_bytes - get_width() * 3; pad++) {
            bytes[indx++] = 0x00; // Make sure rows have a multiple of 4 bytes
        }
    }
    
    fwrite(bytes, 1, byte_count, fp);
    fclose(fp);
    free(bytes);
    return 0;
}

uint32_t mandel_image::mandelbrot_check(double ca, double cb, uint32_t iter) {
    double za = 0, zb = 0, za2 = 0, zb2 = 0;

    uint32_t i;
    for (i = 0; i < iter; i++) {
        zb = (2 * za * zb) + cb;
        za = (za2 - zb2) + ca;

        za2 = za * za;
        zb2 = zb * zb;

        if (za2 + zb2 > 4) {
            break;
        }
    }
    return i;
}

void mandel_image::populate_image(render_data render) {
    pixel_data.resize(width * height);

    uint16_t x, y;
    const double div = width / 4.0;
    const double width_sub = width / 2.0;
    const double height_sub = height / 2.0;

    double ca, cb;
    uint32_t i;
    for(y = 0; y < height; y++) {
        for(x = 0; x < width; x++) {
            ca = (double)(x - width_sub) / (div * render.zoom) + render.a_centre; // Convert pixel coordinates onto complex plane (in a + bi)
            cb = (double)(y - height_sub) / (div * render.zoom) + render.b_centre;
            i = mandelbrot_check(ca, cb, render.iterations); // Check how many iterations until value blows up to infinity
            if(i == render.iterations) {
                pixel_data[coord_to_index(x,y)].red = 0;
                pixel_data[coord_to_index(x,y)].green = 0;
                pixel_data[coord_to_index(x,y)].blue = 0;
                pixel_data[coord_to_index(x,y)].lum = 0; // Luminance
            }
            else {
                pixel_data[coord_to_index(x,y)].red = (uint8_t)(((double)(242 - 11) / 19) * (i % 20) + 11); // Red - add colour based on iterations
                pixel_data[coord_to_index(x,y)].green = (uint8_t)(((double)(154 - 41) / 19) * (i % 20) + 41); // Green - add colour based on iterations
                pixel_data[coord_to_index(x,y)].blue = (uint8_t)(((double)(99 - 150) / 19) * (i % 20) + 150); // Blue - add colour based on iterations
                pixel_data[coord_to_index(x,y)].lum = (pixel_data[coord_to_index(x,y)].red * 0.3) + (pixel_data[coord_to_index(x,y)].green * 0.59) + (pixel_data[coord_to_index(x,y)].blue * 0.11); // Luminance calculation
            }
        }
    }
    get_contr();
}

void mandel_image::apply_fxaa() {
    float blend_factor;
    for(uint32_t y = 0; y < height; y++) {
        for(uint32_t x = 0; x < width; x++) {
            pixel_data[coord_to_index(x,y)].fxaa_red = pixel_data[coord_to_index(x,y)].red;
            pixel_data[coord_to_index(x,y)].fxaa_green = pixel_data[coord_to_index(x,y)].green;
            pixel_data[coord_to_index(x,y)].fxaa_blue = pixel_data[coord_to_index(x,y)].blue;

            if(pixel_data[coord_to_index(x,y)].contr != 0) {
                blend_factor = get_pix_blend_factor(x,y);
                get_edge_dir(x,y);
                if(edge_dir && edge_sign && y < height-1) { // Blend north
                    pixel_data[coord_to_index(x,y)].fxaa_red = pixel_data[coord_to_index(x,y)].red * (1 - blend_factor) + pixel_data[coord_to_index(x,y+1)].red * blend_factor;
                    pixel_data[coord_to_index(x,y)].fxaa_green = pixel_data[coord_to_index(x,y)].green * (1 - blend_factor) + pixel_data[coord_to_index(x,y+1)].green * blend_factor;
                    pixel_data[coord_to_index(x,y)].fxaa_blue = pixel_data[coord_to_index(x,y)].blue * (1 - blend_factor) + pixel_data[coord_to_index(x,y+1)].blue * blend_factor;
                } else if(edge_dir && !edge_sign && y > 0) { // Blend south
                    pixel_data[coord_to_index(x,y)].fxaa_red = pixel_data[coord_to_index(x,y)].red * (1 - blend_factor) + pixel_data[coord_to_index(x,y-1)].red * blend_factor;
                    pixel_data[coord_to_index(x,y)].fxaa_green = pixel_data[coord_to_index(x,y)].green * (1 - blend_factor) + pixel_data[coord_to_index(x,y-1)].green * blend_factor;
                    pixel_data[coord_to_index(x,y)].fxaa_blue = pixel_data[coord_to_index(x,y)].blue * (1 - blend_factor) + pixel_data[coord_to_index(x,y-1)].blue * blend_factor;
                } else if(!edge_dir && edge_sign && x < width-1) { // Blend east
                    pixel_data[coord_to_index(x,y)].fxaa_red = pixel_data[coord_to_index(x,y)].red * (1 - blend_factor) + pixel_data[coord_to_index(x+1,y)].red * blend_factor;
                    pixel_data[coord_to_index(x,y)].fxaa_green = pixel_data[coord_to_index(x,y)].green * (1 - blend_factor) + pixel_data[coord_to_index(x+1,y)].green * blend_factor;
                    pixel_data[coord_to_index(x,y)].fxaa_blue = pixel_data[coord_to_index(x,y)].blue * (1 - blend_factor) + pixel_data[coord_to_index(x+1,y)].blue * blend_factor;
                } else if(!edge_dir && !edge_sign && x > 0) { // Blend west
                    pixel_data[coord_to_index(x,y)].fxaa_red = pixel_data[coord_to_index(x,y)].red * (1 - blend_factor) + pixel_data[coord_to_index(x-1,y)].red * blend_factor;
                    pixel_data[coord_to_index(x,y)].fxaa_green = pixel_data[coord_to_index(x,y)].green * (1 - blend_factor) + pixel_data[coord_to_index(x-1,y)].green * blend_factor;
                    pixel_data[coord_to_index(x,y)].fxaa_blue = pixel_data[coord_to_index(x,y)].blue * (1 - blend_factor) + pixel_data[coord_to_index(x-1,y)].blue * blend_factor;
                } else {
                    pixel_data[coord_to_index(x,y)].fxaa_red = pixel_data[coord_to_index(x,y)].red;
                    pixel_data[coord_to_index(x,y)].fxaa_green = pixel_data[coord_to_index(x,y)].green;
                    pixel_data[coord_to_index(x,y)].fxaa_blue = pixel_data[coord_to_index(x,y)].blue;
                }
            }
        }
    }
}

void mandel_image::get_contr() {
    for(uint32_t i = 0; i < pixel_data.size(); i++) {
        uint8_t max_lum = 0, min_lum = 255;
        uint8_t lum;
        int16_t nx, ny;
        uint32_t x, y;
        for(int8_t dy = -1; dy <= 1; dy++) {
            for(int8_t dx = -1; dx <= 1; dx++) {
                if(dx != 0 && dy != 0) continue; // Skip pixel if diagonal to centre
                index_to_coord(i, x ,y);
                nx = x + dx;
                ny = y + dy;
                if(nx >= 0 && nx < width && ny >= 0 && ny < height) { // Check for pixel not out of image bounds
                    lum = pixel_data[coord_to_index(nx,ny)].lum;
                    if(lum > max_lum) max_lum = lum;
                    if(lum < min_lum) min_lum = lum;
                }
            }
        }
        pixel_data[coord_to_index(x,y)].contr = max_lum - min_lum;
    }
}

float mandel_image::get_pix_blend_factor(uint32_t x, uint32_t y) {
    float blend_factor = 0;
    int16_t nx, ny;
    for(int8_t dy = -1; dy <= 1; dy++) {
        for(int8_t dx = -1; dx <= 1; dx++) {
            nx = x + dx;
            ny = y + dy;
            if(nx >= 0 && nx < width && ny >= 0 && ny < height) { // Check for pixel not out of image bounds
                if(dx == 0 && dy == 0) continue;
                else if(dx != 0 && dy != 0) blend_factor += pixel_data[coord_to_index(nx,ny)].lum; // Weighted average of neighboring pixels
                else blend_factor += 2 * pixel_data[coord_to_index(nx,ny)].lum;
            }
        }
    }
    blend_factor *= 1.0/12;
    blend_factor = fabsf(blend_factor - pixel_data[coord_to_index(x,y)].lum); // Find contrast between weighted average and middle pixel
    if(pixel_data[coord_to_index(x,y)].contr == 0) return 0.0;
    blend_factor = smoothstep((float) blend_factor / pixel_data[coord_to_index(x,y)].contr);
    return blend_factor * blend_factor; // Squared smoothstep with clamping in 0-1
}

void mandel_image::get_edge_dir(uint32_t x, uint32_t y) {
    bool is_horizontal;
    uint8_t l[3][3] = {0};

    int16_t nx, ny;
    for(int8_t dy = -1; dy <= 1; dy++) {
        for(int8_t dx = -1; dx <= 1; dx++) {
            nx = x + dx;
            ny = y + dy;
            if(nx >= 0 && nx < width && ny >= 0 && ny < height) { // Check for pixel not out of image bounds
                l[dx+1][dy+1] = pixel_data[coord_to_index(nx,ny)].lum;
            }
        }
    }

    float horizontal =
    abs(l[1][2] + l[1][0] - 2 * l[1][1]) * 2 + //ln + ls - 2lm
    abs(l[2][2] + l[2][0] - 2 * l[2][1]) + //lne + lse - 2le
    abs(l[0][2] + l[0][0] - 2 * l[0][1]); //lnw + lsw - 2lw

    float vertical =
    abs(l[2][1] + l[0][1] - 2 * l[1][1]) * 2 + //le + lw - 2lm
    abs(l[2][2] + l[0][2] - 2 * l[1][2]) + //lne + lnw - 2ln
    abs(l[2][0] + l[0][0] - 2 * l[1][0]); //lse + lsw - 2ls
    
    is_horizontal = horizontal >= vertical;

    if(is_horizontal) {
        if(l[1][2] > l[1][0]) { // Check if ln is bigger than ls
            edge_sign = true;
        } else edge_sign = false;
    } else {
        if(l[2][1] > l[0][1]) { // Check if le is bigger than lw
            edge_sign = true;
        } else edge_sign = false;
    }

    edge_dir = is_horizontal;
}