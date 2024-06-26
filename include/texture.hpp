#ifndef TEXTURE_H
#define TEXTURE_H

#include <iostream>
#include "Vector3f.h"
#include "rtwImage.hpp"
using namespace std;
class Texture
{

public:
    // texture(/* args */);
    virtual ~Texture() = default;
    virtual Vector3f getColor(float u, float v, Vector3f point) const = 0;
};

class SolidColor : public Texture
{
public:
    SolidColor(Vector3f &_color) : color(_color) {}

    Vector3f getColor(float u, float v, Vector3f point) const override
    {

        return this->color;
    }

private:
    Vector3f color;
};

class CheckerTexture : public Texture
{
public:
    CheckerTexture(float _scale, Vector3f colorEven, Vector3f colorOdd)
    {
        this->scale = _scale;
        this->evenTexture = new SolidColor(colorEven);
        this->oddTexture = new SolidColor(colorOdd);
    }

    Vector3f getColor(float u, float v, Vector3f point) const override
    {
        // cout << 1 << endl;
        int x = int(std::floor(scale * point.x()));
        int y = int(std::floor(scale * point.y()));
        int z = int(std::floor(scale * point.z()));

        if ((x + y + z) % 2 == 0)
        {
            return evenTexture->getColor(u, v, point);
        }
        else
        {
            return oddTexture->getColor(u, v, point);
        }
    }

private:
    float scale;
    SolidColor *evenTexture;
    SolidColor *oddTexture;
};

class ImageTexture : public Texture
{
public:
    ImageTexture(char *image_filename)
    {
        imagee = new RtwImage(image_filename);
    };
    Vector3f getColor(float u, float v, Vector3f point) const override
    {

        v = 1 - v;
        // cout << u << " " << v << endl;
        const unsigned char *pixel = imagee->getPixelDate(int(u * imagee->width()), int(v * imagee->height()));
        return Vector3f(float(pixel[0]) / 255.0, float(pixel[1]) / 255.0, float(pixel[3] / 255.0));
    }

private:
    RtwImage *imagee;
};

#endif