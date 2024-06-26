#ifndef BRDF_H
#define BRDF_H
#include <iostream>
#include "Vector3f.h"
class Brdf
{
public:
    // const float M_PI = 1;
    static float schlickFresnel(float cosTheta, float F0)
    {
        return F0 + (1.0f - F0) * std::pow(1.0f - cosTheta, 5.0f);
    }

    static float ggxDistribution(const Vector3f &normal, const Vector3f &halfVector, float alpha)
    {
        float NdotH = std::max(0.0f, Vector3f::dot(normal, halfVector));
        float alpha2 = alpha * alpha * alpha * alpha;
        float denom = (NdotH * NdotH * (alpha2 - 1.0f) + 1.0f);
        return alpha2 / (3.14 * denom * denom);
    }

    static float smithGeometry(float NdotV, float alpha)
    {
        // float alpha2 = alpha * alpha;
        // float denom = NdotV + std::sqrt(alpha2 + (1.0f - alpha2) * NdotV * NdotV);
        // return 2.0f * NdotV / denom;
        float r = alpha + 1;
        float k = r * r / 8.0;
        float nom = NdotV;
        float denom = NdotV * (1 - k) + k;
        return nom / denom;
    }

    static float cookTorranceBRDF(const Vector3f &incoming, const Vector3f &outgoing, const Vector3f &normal, float alpha, float F0)
    {
        Vector3f halfVector = (-incoming + outgoing).normalized();
        // if (normal.z() > 0.8)
        // {
        //     std::cout << "a: " << 11 << " " << std::endl;
        //     incoming.print();
        //     outgoing.print();
        //     normal.print();
        //     halfVector.print();
        // }
        float NdotL = std::max(0.0f, Vector3f::dot(normal, -incoming));
        float NdotV = std::max(0.0f, Vector3f::dot(normal, outgoing));
        float NdotH = std::max(0.0f, Vector3f::dot(normal, halfVector));
        float VdotH = std::max(0.0f, Vector3f::dot(outgoing, halfVector));

        float D = ggxDistribution(normal, halfVector, alpha);
        float G = smithGeometry(NdotL, alpha) * smithGeometry(NdotV, alpha);
        float F = schlickFresnel(VdotH, F0);
        // std::cout << D << " " << G << " " << F << " " << NdotL << " " << NdotV << " " << (F * D * G) / (4.0f * NdotL * NdotV) << std::endl;
        return (F * D * G) / std::max(4.0f * NdotL * NdotV, float(0.001));
    }
};

class BaseBrdf
{
public:
    BaseBrdf(float _alpha, float _F0) : alpha(_alpha), F0(_F0) {}
    virtual float getBrdfValue(const Vector3f &incoming, const Vector3f &outgoing, const Vector3f &normal) = 0;
    float getF0()
    {
        return this->F0;
    }

protected:
    float alpha;
    float F0;

private:
};

class cookTorrance : public BaseBrdf
{
public:
    cookTorrance(float _alpha, float _F0) : BaseBrdf(_alpha, _F0) {}
    float schlickFresnel(float cosTheta, float F0)
    {
        return F0 + (1.0f - F0) * std::pow(1.0f - cosTheta, 5.0f);
    }

    float ggxDistribution(const Vector3f &normal, const Vector3f &halfVector, float alpha)
    {
        float NdotH = std::max(0.0f, Vector3f::dot(normal, halfVector));
        float alpha2 = alpha * alpha;
        float denom = (NdotH * NdotH * (alpha2 - 1.0f) + 1.0f);
        return alpha2 / (3.14 * denom * denom);
    }

    float smithGeometry(float NdotV, float alpha)
    {
        float r = alpha + 1;
        float k = r * r / 8.0;
        float nom = NdotV;
        float denom = NdotV * (1 - k) + k;
        return nom / denom;
    }
    float getBrdfValue(const Vector3f &incoming, const Vector3f &outgoing, const Vector3f &normal) override
    {
        // incoming.print();
        // outgoing.print();
        // cout << "---" << endl;
        Vector3f halfVector = (-incoming + outgoing).normalized();
        float NdotL = std::max(0.0f, Vector3f::dot(normal, -incoming));
        float NdotV = std::max(0.0f, Vector3f::dot(normal, outgoing));
        float NdotH = std::max(0.0f, Vector3f::dot(normal, halfVector));
        float VdotH = std::max(0.0f, Vector3f::dot(outgoing, halfVector));

        float D = ggxDistribution(normal, halfVector, alpha);
        float G = smithGeometry(NdotL, alpha) * smithGeometry(NdotV, alpha);
        float F = schlickFresnel(VdotH, F0);
        return (F * D * G) / std::max(4.0f * NdotL * NdotV, float(0.001));
    }

private:
};
#endif