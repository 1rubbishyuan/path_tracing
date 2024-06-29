#define STB_IMAGE_IMPLEMENTATION
#include "rtwImage.hpp"

RtwImage::~RtwImage()
{
    delete[] bdata;
    STBI_FREE(fdata);
    // delete[] fdata;
}