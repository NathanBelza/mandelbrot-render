#include <cstdint>
#include <cstddef>
#include <string>
#include <fstream>
#include <cmath>

#include "image.hpp"


std::int8_t image::save_bmp(std::string file_name) {

    std::ofstream file(file_name, std::ios::binary);
    if(!file) {
        return -1;
    }

    const std::size_t row_bytes = ((3 * width + 3) / 4) * 4; // 3 bytes per pixel + ensure multiple of 4 bytes
    const std::size_t byte_count = 14 + 40 + row_bytes * height;

    std::vector<std::uint8_t> bytes(byte_count, 0x00);

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

    apply_fxaa();

    // Need to make output two BMPs
    std::size_t indx = 54;
    for (std::size_t y = 0; y < height; y++) {
        for (std::size_t x = 0; x < width; x++) {
            bytes[indx++] = pixel_data[coord_to_index(x,y)].fxaa_blue;
            bytes[indx++] = pixel_data[coord_to_index(x,y)].fxaa_green;
            bytes[indx++] = pixel_data[coord_to_index(x,y)].fxaa_red;
        }
        for(std::uint8_t pad = 0; pad < row_bytes - get_width() * 3; pad++) {
            bytes[indx++] = 0x00; // Make sure rows have a multiple of 4 bytes
        }
    }
    
    file.write(reinterpret_cast<const char *> (bytes.data()), bytes.size());
    return 0;
}


void image::apply_fxaa() {
    float blend_factor = 0;
    for(std::size_t y = 0; y < height; y++) {
        for(std::size_t x = 0; x < width; x++) {
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


void image::get_contr() {
    for(std::size_t i = 0; i < pixel_data.size(); i++) {
        std::uint8_t max_lum = 0, min_lum = 255;
        std::uint8_t lum = 0;

        std::size_t nx = 0, ny = 0;
        std::size_t x = 0, y = 0;

        for(std::int8_t dy = -1; dy <= 1; dy++) {
            for(std::int8_t dx = -1; dx <= 1; dx++) {

                if(dx != 0 && dy != 0) {
                    continue; // Skip pixel if diagonal to centre
                }

                index_to_coord(i, x ,y);
                nx = x + dx;
                ny = y + dy;

                if(nx >= 0 && nx < width && ny >= 0 && ny < height) { // Check for pixel not out of image bounds
                    lum = pixel_data[coord_to_index(nx,ny)].lum;
                    if(lum > max_lum) {
                        max_lum = lum;
                    }
                    if(lum < min_lum) {
                        min_lum = lum;
                    }
                }
            }
        }
        pixel_data[coord_to_index(x,y)].contr = max_lum - min_lum;
    }
}


float image::get_pix_blend_factor(std::size_t x, std::size_t y) {
    float blend_factor = 0;
    std::size_t nx, ny;

    for(std::int8_t dy = -1; dy <= 1; dy++) {
        for(std::int8_t dx = -1; dx <= 1; dx++) {
            nx = x + dx;
            ny = y + dy;

            if(nx >= 0 && nx < width && ny >= 0 && ny < height) { // Check for pixel not out of image bounds
                if(dx == 0 && dy == 0) {
                    continue;
                } else if(dx != 0 && dy != 0) {
                    blend_factor += pixel_data[coord_to_index(nx,ny)].lum; // Weighted average of neighboring pixels
                } else {
                    blend_factor += 2 * pixel_data[coord_to_index(nx,ny)].lum;
                }
            }
        }
    }

    blend_factor *= 1.0/12.0;
    blend_factor = fabsf(blend_factor - pixel_data[coord_to_index(x,y)].lum); // Find contrast between weighted average and middle pixel

    if(pixel_data[coord_to_index(x,y)].contr == 0) {
        return 0.0;
    }
    blend_factor = smoothstep(blend_factor / pixel_data[coord_to_index(x,y)].contr);
    return blend_factor * blend_factor; // Squared smoothstep with clamping in 0-1
}


void image::get_edge_dir(std::size_t x, std::size_t y) {
    bool is_horizontal = false;
    std::uint8_t l[3][3] = {0};

    std::size_t nx, ny;
    for(std::int8_t dy = -1; dy <= 1; dy++) {
        for(std::int8_t dx = -1; dx <= 1; dx++) {
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

    if (is_horizontal) {
        if(l[1][2] > l[1][0]) { // Check if ln is bigger than ls
            edge_sign = true;
        } else {
            edge_sign = false;
        }
    } else {
        if(l[2][1] > l[0][1]) { // Check if le is bigger than lw
            edge_sign = true;
        } else {
            edge_sign = false;
        }
    }

    edge_dir = is_horizontal;
}