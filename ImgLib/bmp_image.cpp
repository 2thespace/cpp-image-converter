#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>
#include <iostream>

using namespace std;

namespace img_lib {

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

PACKED_STRUCT_BEGIN BitmapFileHeader {
    // поля заголовка Bitmap File Header
    BitmapFileHeader(int width, int height){
        size_ = 54 + GetBMPStride(width) * height;
   }
   char header_[2] = {'B', 'M'};
   uint32_t size_ = 0;
   uint32_t zero_field = 0x0000;
   uint32_t offset_ = 54;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
        // поля заголовка Bitmap Info Header
    BitmapInfoHeader(int width, int height)
        : width_(width), height_(height) {
        size_image_ = GetBMPStride(width) * height;
    }
    uint32_t size_ = 40; // размер второй части 40 байт
    int32_t width_;
    int32_t height_;
    uint16_t planes_ = 1;
    uint16_t bit_count_ = 24;
    uint32_t compression_ = 0;
    uint32_t size_image_ = {};
    int32_t x_pix_per_met_ = 11811;
    int32_t y_pix_per_met_ = 11811;
    int32_t clr_used_ = 0;
    int32_t clr_important_ = 0x1000000;
}
PACKED_STRUCT_END



// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image)
{
    std::ofstream out(file, ios::binary);
    const int w = image.GetWidth();
    const int h = image.GetHeight();
    BitmapFileHeader file_header{w, h};
    BitmapInfoHeader info_header(w, h);

    out.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    out.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));
    
    int stride = GetBMPStride(w);
    std::vector<char> buff(stride);

    for (int y = h - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < w; ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }
         out.write(reinterpret_cast<const char*>(buff.data()), buff.size());
    }

    return out.good();
}

// напишите эту функцию
Image LoadBMP(const Path& file) {
    ifstream ifs(file, ios::binary);
    int w, h;
    ifs.ignore(18); // пропускаем 18 байт полей файла до информации об изображении

    ifs.read(reinterpret_cast<char*>(&w), sizeof(w));
    ifs.read(reinterpret_cast<char*>(&h), sizeof(h));

    ifs.ignore(28);

    int stride = GetBMPStride(w);
    Image result(stride / 3, h, Color::Black());
    std::vector<char> buff(w * 3);

    for (int y = h - 1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), stride);

        for (int x = 0; x < w; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }

    return result;
}

}  // namespace img_lib