#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>
#include "texture.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include "scene_parser.hpp"
#include "lightGroup.hpp"
#include "brdf.hpp"
#include <iostream>

// TODO: Implement Shade function that computes Phong introduced in class.
class Material
{
public:
    // Material(const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO, float s = 0, bool e_reflect = false, bool e_refract = false, float r_rate = 0.0)
    //     : diffuseColor(d_color), specularColor(s_color), shininess(s), enable_reflet(e_reflect), enable_refract(e_refract), refract_rate(r_rate)
    // {
    // }
    Material(const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO, float s = 0, bool e_reflect = false, bool e_refract = false, float r_rate = 0, float _perDiffuse = 0, float _perReflect = 0, float _perRefract = 0, float _fuzz = 0)
        : diffuseColor(d_color), specularColor(s_color), shininess(s), enable_reflet(e_reflect), enable_refract(e_refract), refract_rate(r_rate), perDiffuse(_perDiffuse), perReflect(_perReflect), perRefract(_perRefract), fuzz(_fuzz)
    {
        hasTexture = false;
        isLight = false;
    }
    virtual ~Material() = default;

    virtual Vector3f getDiffuseColor() const
    {
        return diffuseColor;
    }
    virtual Vector3f getEmitColor()
    {

        return Vector3f::ZERO;
    }
    Vector3f Shade(const Ray &ray, const Hit &hit,
                   const Vector3f &dirToLight, const Vector3f &lightColor)
    {
        Vector3f shaded = Vector3f::ZERO;
        Vector3f R_i = 2 * (Vector3f::dot(hit.getNormal().normalized(), dirToLight.normalized())) * (hit.getNormal().normalized()) - dirToLight.normalized();
        Vector3f v1 = this->diffuseColor * (std::max(float(0), Vector3f::dot(dirToLight.normalized(), hit.getNormal().normalized())));
        Vector3f v2 = this->specularColor * pow(std::max(float(0), Vector3f::dot(-ray.getDirection().normalized(), R_i)), this->shininess);
        Vector3f ev = Vector3f(0.8, 0.8, 0.8);
        shaded = 0.01 * ev + lightColor * (v1 + v2);
        return shaded;
    }
    bool isReflective()
    {
        return this->enable_reflet;
    }
    bool isRefract()
    {
        return this->enable_refract;
    }
    float getRefractRate()
    {
        return this->refract_rate;
    }
    float getPerDiffuse()
    {
        return this->perDiffuse;
    }
    float getPerReflect()
    {
        return this->perReflect;
    }
    float getPerRefract()
    {
        return this->perRefract;
    }
    float getFuzz()
    {
        return this->fuzz;
    }
    bool isTextured()
    {
        return hasTexture;
    }
    void setTexture(Texture *_texture)
    {
        this->hasTexture = true;
        this->texture = _texture;
    }
    void changeColorAccordingTexture(float u, float v, Vector3f point)
    {
        this->diffuseColor = texture->getColor(u, v, point);
    }
    void setLightColor(Vector3f l_color)
    {
        this->isLight = true;
        this->lightColor = l_color;
    }
    bool isALight()
    {
        return this->isLight;
    }
    void setBrdf(BaseBrdf *_brdf)
    {
        this->brdf = _brdf;
    }
    BaseBrdf *getBrdf()
    {
        return this->brdf;
    }
    float boost;

protected:
    Vector3f diffuseColor;
    Vector3f specularColor;
    Texture *texture;
    bool hasTexture;
    bool isLight;
    float shininess;
    bool enable_reflet;
    bool enable_refract;
    float refract_rate;
    float perDiffuse;
    float perReflect;
    float perRefract;
    float fuzz;
    Vector3f lightColor;
    BaseBrdf *brdf;
};

class LightMaterial : public Material
{
public:
    LightMaterial(const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO, float s = 0, bool e_reflect = false, bool e_refract = false, float r_rate = 0, float _perDiffuse = 0, float _perReflect = 0, float _perRefract = 0, float _fuzz = 0)
        : Material(d_color, s_color, s, e_reflect, e_refract, r_rate, _perDiffuse, _perReflect, _perRefract, _fuzz)
    {
        hasTexture = false;
        isLight = false;
    }
    Vector3f getEmitColor() override
    {
        return this->lightColor;
    }

private:
};
#endif // MATERIAL_H
