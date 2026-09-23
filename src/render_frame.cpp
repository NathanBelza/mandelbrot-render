#include <cstdint>
#include <cstddef>
#include <vector>
#include <cmath>
#include <fstream>

#include "render_frame.hpp"

std::int8_t mandel_image::render_frame(std::string file_name, render_data render) {

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

    populate_image(render);
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

// check if a point is in the mandelbrot set, returns number of iterations until divergence
std::size_t mandel_image::mandelbrot_check(double ca, double cb, std::size_t iter) {
    double za = 0, zb = 0, za2 = 0, zb2 = 0;

    std::size_t i;
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

    const double div = width / 4.0; // So that zoom of 1 corresponds to x range of -2 to 2
    const double width_sub = width / 2.0;
    const double height_sub = height / 2.0;

    double ca = 0, cb = 0;
    for(std::size_t y = 0; y < height; y++) {
        for(std::size_t x = 0; x < width; x++) {
            // Convert pixel coordinates onto complex plane (in a + bi)
            ca = static_cast<double> (x - width_sub) / (div * render.zoom) + render.a_centre;
            cb = static_cast<double> (y - height_sub) / (div * render.zoom) + render.b_centre;
            std::size_t i = mandelbrot_check(ca, cb, render.iterations); // Check how many iterations until value blows up to infinity

            if(i == render.iterations) {
                pixel_data[coord_to_index(x,y)].red = 0;
                pixel_data[coord_to_index(x,y)].green = 0;
                pixel_data[coord_to_index(x,y)].blue = 0;
                pixel_data[coord_to_index(x,y)].lum = 0; // Luminance
            }
            else {
                pixel_data[coord_to_index(x,y)].red = (std::uint8_t)(((double)(242 - 11) / 19) * (i % 20) + 11); // Red - add colour based on iterations
                pixel_data[coord_to_index(x,y)].green = (std::uint8_t)(((double)(154 - 41) / 19) * (i % 20) + 41); // Green - add colour based on iterations
                pixel_data[coord_to_index(x,y)].blue = (std::uint8_t)(((double)(99 - 150) / 19) * (i % 20) + 150); // Blue - add colour based on iterations
                pixel_data[coord_to_index(x,y)].lum = (pixel_data[coord_to_index(x,y)].red * 0.3) + (pixel_data[coord_to_index(x,y)].green * 0.59) + (pixel_data[coord_to_index(x,y)].blue * 0.11); // Luminance calculation
            }
        }
    }
    get_contr();
}

void mandel_image::apply_fxaa() {
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

void mandel_image::get_contr() {
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

float mandel_image::get_pix_blend_factor(std::size_t x, std::size_t y) {
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

void mandel_image::get_edge_dir(std::size_t x, std::size_t y) {
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