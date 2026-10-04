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


void image::get_contr() {
    for (std::size_t y = 0; y < height; y++) {
        for (std::size_t x = 0; x < width; x++) {

            std::uint8_t lum = pixel_data[coord_to_index(x, y)].lum;
            std::uint8_t min_lum = lum, max_lum = lum;

            static constexpr std::pair<int,int> offsets[] = {{1,0},{-1,0},{0,1},{0,-1}};
            for (auto [dx, dy] : offsets) {
                std::uint8_t l = pixel_data[coord_to_index(x + dx, y + dy)].lum;
                min_lum = std::min(min_lum, l);
                max_lum = std::max(max_lum, l);
            }

            pixel_data[x + width * y].contr = max_lum - min_lum;
        }
    }
}


void image::apply_fxaa() {
    for (std::size_t y = 0; y < height; y++) {
        for (std::size_t x = 0; x < width; x++) {
            pixel& p = pixel_data[coord_to_index(x,y)];
            p.lum = (p.red * 0.3) + (p.green * 0.59) + (p.blue * 0.11); // Luminance calculation
        }
    }

    get_contr();

    for(std::size_t y = 0; y < height; y++) {
        for(std::size_t x = 0; x < width; x++) {
            pixel& p = pixel_data[coord_to_index(x,y)];
            p.fxaa_red = p.red;
            p.fxaa_green = p.green;
            p.fxaa_blue = p.blue;

            if (p.contr < contrast_threshold) {
                continue;
            }

            float blend_factor = get_pix_blend_factor(x,y);
            edge e = get_edge_dir(x,y);

            std::int8_t step = e.is_positive ? 1 : -1;
            const pixel& n = pixel_data[coord_to_index(x + (e.is_horizontal ? 0 : step), y + (e.is_horizontal ? step : 0))];

            // blend pixel across edge direction
            p.fxaa_red = static_cast<std::uint8_t> (std::round(p.red + (n.red - p.red) * blend_factor));
            p.fxaa_green = static_cast<std::uint8_t> (std::round(p.green + (n.green - p.green) * blend_factor));
            p.fxaa_blue = static_cast<std::uint8_t> (std::round(p.blue + (n.blue - p.blue) * blend_factor));
        }
    }
}


float image::get_pix_blend_factor(std::uint32_t x, std::uint32_t y) {

    float blend_factor = 0;
    std::size_t nx, ny; // neighbours

    for (std::int8_t dy = -1; dy <= 1; dy++) {
        for (std::int8_t dx = -1; dx <= 1; dx++) {
            nx = x + dx;
            ny = y + dy;

            if (dx == 0 && dy == 0) {
                continue;
            } else if (dx != 0 && dy != 0) {
                blend_factor += pixel_data[coord_to_index(nx,ny)].lum; // Weighted average of neighboring pixels
            } else {
                blend_factor += 2 * pixel_data[coord_to_index(nx,ny)].lum;
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


edge image::get_edge_dir(std::uint32_t x, std::uint32_t y) {
    edge e = {0};
    std::uint8_t l[3][3] = {0};

    std::size_t nx, ny; // neighbours
    for(std::int8_t dy = -1; dy <= 1; dy++) {
        for(std::int8_t dx = -1; dx <= 1; dx++) {
            nx = x + dx;
            ny = y + dy;
            
            l[dx+1][dy+1] = pixel_data[coord_to_index(nx,ny)].lum;
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
    
    e.is_horizontal = horizontal >= vertical;

    if (e.is_horizontal) {
        // Check if ln is bigger than ls
        e.is_positive = std::abs(l[1][2] - l[1][1]) >= std::abs(l[1][0] - l[1][1]);
    } else {
        // Check if le is bigger than lw
        e.is_positive = std::abs(l[2][1] - l[1][1]) >= std::abs(l[0][1] - l[1][1]);
    }

    return e;
}