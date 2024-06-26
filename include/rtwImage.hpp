#ifndef RTW_IMAGE_H
#define RTW_IMAGE_H

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <iostream>
// #define STB_IMAGE_IMPLEMENTATION
// #define STBI_FAILURE_USERMSG
#include "stb_image.h"
using namespace std;
class RtwImage
{
public:
    RtwImage(char *image_filename)
    {
        // cout << image_filename << endl;
        load(std::string(image_filename));
    }
    ~RtwImage()
    {
        delete[] bdata;
        // STBI_FREE(fdata);
        delete[] fdata;
    }
    bool load(std::string filename)
    {
        int n = bytesPerPixel;
        fdata = stbi_loadf(filename.c_str(), &imageWidth, &imageHeight, &n, bytesPerPixel);
        if (fdata == nullptr)
            return false;
        bytesPerScanLine = imageWidth * bytesPerPixel;
        convert_to_bytes();
        return true;
    }
    int width()
    {
        return this->imageWidth;
    }
    int height()
    {
        return this->imageHeight;
    }
    const unsigned char *getPixelDate(int u, int v)
    {
        return bdata + v * bytesPerScanLine + u * bytesPerPixel;
    }

private:
    int imageWidth = 0, imageHeight = 0;
    const int bytesPerPixel = 3;
    float *fdata = nullptr;
    unsigned char *bdata = nullptr;
    int bytesPerScanLine = 0;
    static unsigned char float_to_byte(float value)
    {
        if (value <= 0.0)
            return 0;
        if (1.0 <= value)
            return 255;
        return static_cast<unsigned char>(256.0 * value);
    }

    void convert_to_bytes()
    {
        // Convert the linear floating point pixel data to bytes, storing the resulting byte
        // data in the `bdata` member.

        int total_bytes = imageWidth * imageHeight * bytesPerPixel;
        bdata = new unsigned char[total_bytes];

        // Iterate through all pixel components, converting from [0.0, 1.0] float values to
        // unsigned [0, 255] byte values.

        auto *bptr = bdata;
        auto *fptr = fdata;
        for (auto i = 0; i < total_bytes; i++, fptr++, bptr++)
            *bptr = float_to_byte(*fptr);
    }
};
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif