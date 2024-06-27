#ifndef MYALGORITHM_H
#define MYALGORITHM_H
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <algorithm>
#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"
#include "utils.hpp"
#include "brdf.hpp"
#include <string>
using namespace std;

// bool isShadow(int x, int y, SceneParser *sceneparser)
// {
//     bool ans = false;
//     Group *baseGroup = sceneparser->getGroup();
//     for (int i = 0; i < sceneparser->getNumLights(); i++)
//     {
//         Light *light = sceneparser->getLight(i);
//         Hit hit;
//         light->getIllumination()
//         // bool isIntersect = baseGroup->intersect(ray, hit, 0, 0);
//     }
// }

Vector3f IntersectionColor(SceneParser *sceneparser, Ray &ray, Group *baseGroup, int depth, int lastHit)
{
    if (depth >= 3)
    {
        return Vector3f::ZERO;
    }
    Hit hit;
    bool isIntersect = baseGroup->intersect(ray, hit, 0, depth > 0 ? 1 : 0);
    Vector3f finalColor = depth == 0 ? sceneparser->getBackgroundColor() : Vector3f::ZERO;
    if (isIntersect)
    {
        finalColor = Vector3f::ZERO;
        for (int li = 0; li < sceneparser->getNumLights(); ++li)
        {
            Light *light = sceneparser->getLight(li);
            Vector3f L, lightColor;
            light->getIllumination(ray.pointAtParameter(hit.getT()), L, lightColor);
            finalColor += hit.getMaterial()->Shade(ray, hit, L, lightColor);
            if (hit.getMaterial()->isReflective())
            {
                Vector3f I = ray.getDirection().normalized();
                Vector3f N = hit.getNormal().normalized();
                Vector3f reflectedDirection = I - 2 * (Vector3f::dot(I, N)) * N;
                Ray reflectedRay = Ray(ray.pointAtParameter(hit.getT()), reflectedDirection);
                finalColor += IntersectionColor(sceneparser, reflectedRay, baseGroup, depth + 1, hit.getHitObjectId());
            }
            if (hit.getMaterial()->isRefract())
            {
                Vector3f I = ray.getDirection().normalized();
                Vector3f N = hit.getNormal().normalized();
                float refract_rate = hit.getMaterial()->getRefractRate();
                float c = Vector3f::dot(I, N) / (I.length() * N.length());
                // 折射率在两个面之间的变换没有实现
                Vector3f reflectedDirection;
                if (lastHit != hit.getHitObjectId())
                    reflectedDirection = -refract_rate * I + (refract_rate * (Vector3f::dot(I, N) - sqrt(1 - refract_rate * refract_rate * (1 - c * c))) * N);
                else
                {
                    refract_rate = 1 / refract_rate;
                    reflectedDirection = -refract_rate * I + (refract_rate * (Vector3f::dot(I, N) - sqrt(1 - refract_rate * refract_rate * (1 - c * c))) * N);
                }
                Ray reflectedRay = Ray(ray.pointAtParameter(hit.getT()), reflectedDirection.normalized());
                finalColor += IntersectionColor(sceneparser, reflectedRay, baseGroup, depth + 1, hit.getHitObjectId());
            }
        }
    }
    return finalColor;
}

Image WhittedStyle(SceneParser *sceneparser)
{
    Camera *camera = sceneparser->getCamera();
    Image image = Image(camera->getWidth(), camera->getHeight());
    for (int x = 0; x < camera->getWidth(); ++x)
    {
        for (int y = 0; y < camera->getHeight(); ++y)
        {
            Group *baseGroup = sceneparser->getGroup();
            Vector3f finalColor;
            for (int i = 0; i < 10; i++)
            {
                int s_x = x + Utils::generateRandomInt(-1, 1);
                int s_y = y + Utils::generateRandomInt(-1, 1);
                Ray camRay = sceneparser->getCamera()->generateRay(Vector2f(s_x, s_y));
                finalColor += IntersectionColor(sceneparser, camRay, baseGroup, 0, -1);
            }
            finalColor = finalColor / 10;
            image.SetPixel(x, y, finalColor);
        }
    }
    return image;
}

Image Pure(SceneParser *sceneparser)
{
    Camera *camera = sceneparser->getCamera();
    Image image = Image(camera->getWidth(), camera->getHeight());
    for (int x = 0; x < camera->getWidth(); ++x)
    {
        for (int y = 0; y < camera->getHeight(); ++y)
        {
            Ray camRay = sceneparser->getCamera()->generateRay(Vector2f(x, y));
            Group *baseGroup = sceneparser->getGroup();
            Hit hit;
            bool isIntersect = baseGroup->intersect(camRay, hit, 0, 0);
            if (isIntersect)
            {

                Vector3f finalColor = Vector3f::ZERO;
                for (int li = 0; li < sceneparser->getNumLights(); ++li)
                {
                    Light *light = sceneparser->getLight(li);
                    Vector3f L, lightColor;
                    light->getIllumination(camRay.pointAtParameter(hit.getT()), L, lightColor);
                    finalColor += hit.getMaterial()->Shade(camRay, hit, L, lightColor);
                }
                image.SetPixel(x, y, finalColor);
            }
            else
            {
                image.SetPixel(x, y, sceneparser->getBackgroundColor());
            }
        }
    }
    // cout << "sdasad" << endl;
    return image;
}

Ray getDifussionRay(Hit hit, Vector3f startPoint)
{
    Vector3f direction = (hit.getNormal() + Utils::random_v()).normalized();
    if (Vector3f::dot(direction, hit.getNormal()) > 0)
    {
        return Ray(startPoint, direction);
    }
    else
    {
        return Ray(startPoint, -direction);
    }
}

bool schlickApproximate(float cosTheta, float refractRate)
{
    float r0 = (1 - refractRate) / (1 + refractRate);
    r0 = r0 * r0;
    // cout << r0 + (1 - r0) * pow((1 - cosTheta), 5) << endl;
    // cout << generateRandomFloat(0, 1) << endl;
    return r0 + (1 - r0) * pow((1 - cosTheta), 5) > Utils::generateRandomFloat(0, 1);
}
Vector3f MonteCarloIntersectionColor(SceneParser *sceneparser, Ray &ray, Group *baseGroup, int depth, int lastHit)
{

    if (depth >= 4)
    {
        return Vector3f(0, 0, 0);
    }
    Hit hit;
    bool isIntersect = baseGroup->intersect(ray, hit, 0.01, 0);
    Vector3f finalColor; //= depth == 0 ? sceneparser->getBackgroundColor() : Vector3f::ZERO;
    Vector3f lightColor = Vector3f::ZERO;
    Vector3f directColor;
    LightGroup *lightGroup = sceneparser->getLightGroup();
    if (isIntersect)
    {
        finalColor = hit.getMaterial()->getDiffuseColor();
        lightColor = hit.getMaterial()->getEmitColor();
        if (hit.getMaterial()->isALight())
        {
            // cout << 1 << endl;
            float boost = depth == 0 ? 1 : hit.getMaterial()->boost;
            return hit.getMaterial()->getEmitColor() * boost;
        }
        // if (hit.getNormal().z() > 0.8)
        // {
        //     hit.getPoint().print();
        //     lightPoint.print();
        //     line.print();
        // }
        float mid1 = hit.getMaterial()->getPerDiffuse();
        float mid2 = hit.getMaterial()->getPerReflect() + mid1;
        float mid = Utils::generateRandomFloat(0, 1);

        if (mid < mid1)
        {
            for (int i = 0; i < lightGroup->lightsNum; i++)
            {
                Vector3f energy = Vector3f(1, 1, 1);
                bool getEnergy = false;
                Hit lhit;
                Vector3f lightPoint = lightGroup->lights[i]->randomFace();
                Vector3f startl = ray.pointAtParameter(hit.getT());
                Vector3f line = lightPoint - startl;
                Vector3f unit_line = line.normalized();
                Ray lray = Ray(startl, unit_line, 0);
                bool isIntersectl = baseGroup->intersect(lray, lhit, 0.1, 1);
                if (isIntersectl && abs(lhit.getT() - line.length()) < 0.1)
                {

                    getEnergy = true;
                    float brdfValue = hit.getMaterial()->getBrdf()->getBrdfValue(ray.getDirection(), lray.getDirection(), hit.getNormal());
                    // if (hit.getHitObjectId() == 15)
                    // {
                    //     hit.getNormal().print();
                    //     cout << isIntersectl << " " << lhit.getT() << " " << line.length() << " " << brdfValue << endl;
                    // }
                    float distance = 1 / (line.length() * line.length());
                    // energy = distance * brdfValue * energy * lhit.getMaterial()->getEmitColor(); // * abs(Vector3f::dot(unit_line, lhit.getNormal()));
                    float NdotL = max(0.0f, Vector3f::dot(hit.getNormal(), unit_line));
                    directColor += brdfValue * finalColor * lhit.getMaterial()->getEmitColor() * distance * NdotL * 1500 * lhit.getMaterial()->boost;
                }
                // if (!getEnergy)
                // {
                //     directColor = Vector3f::ZERO;
                // }
                // else
                // {
                //     // directColor += finalColor * energy;
                //     directColor += (finalColor)*energy;
                // }
            }
            Ray diffusedRay = getDifussionRay(hit, hit.getPoint());
            diffusedRay.time = ray.time;
            // float brdfValue = Brdf::cookTorranceBRDF(ray.getDirection(), diffusedRay.getDirection(), hit.getNormal(), 0.6, 0.04);
            float brdfValue = hit.getMaterial()->getBrdf()->getBrdfValue(ray.getDirection(), diffusedRay.getDirection(), hit.getNormal());
            float NdotV = max(0.0f, Vector3f::dot(hit.getNormal(), -ray.getDirection()));
            Vector3f newColor = 80 * brdfValue * NdotV * float(1) / (depth + 1) * MonteCarloIntersectionColor(sceneparser, diffusedRay, baseGroup, depth + 1, hit.getHitObjectId());
            // newColor.print();
            // cout << brdfValue << endl;
            // if (depth == 0)
            // newColor.print();
            newColor = Utils::clampV(newColor);
            if (lastHit != hit.getHitObjectId())
                finalColor = finalColor * newColor; //(finalColor.x() * newColor.x(), finalColor.y() * newColor.y(), finalColor.z() * newColor.z());
        }
        else if (mid <= mid2)
        {
            Vector3f I = ray.getDirection();
            Vector3f N = hit.getNormal();
            directColor = Vector3f::ZERO;
            Vector3f reflectedDirection = (I - 2 * (Vector3f::dot(I, N)) * N + hit.getMaterial()->getFuzz() * Utils::random_v()).normalized();
            Ray reflectedRay = Ray(hit.getPoint(), reflectedDirection, ray.time);
            float brdfValue = hit.getMaterial()->getBrdf()->getBrdfValue(ray.getDirection(), reflectedRay.getDirection(), hit.getNormal());
            // cout << brdfValue << endl;
            float NdotV = max(0.0f, Vector3f::dot(hit.getNormal(), -ray.getDirection()));
            Vector3f newColor = NdotV * float(1) / (depth + 1) * MonteCarloIntersectionColor(sceneparser, reflectedRay, baseGroup, depth + 1, hit.getHitObjectId());
            newColor = Utils::clampV(newColor);
            // newColor.print();
            if (lastHit != hit.getHitObjectId())
                finalColor = Vector3f(finalColor.x() * newColor.x(), finalColor.y() * newColor.y(), finalColor.z() * newColor.z());
        }
        else
        {
            Vector3f I = ray.getDirection();
            Vector3f N = hit.getNormal();
            float refract_rate = hit.getMaterial()->getRefractRate();
            if (lastHit == hit.getHitObjectId())
                refract_rate = 1 / refract_rate;

            float c = Vector3f::dot(-I, N) / (I.length() * N.length());
            float s = sqrt(1.0 - c * c);
            Vector3f reflectedDirection;
            if (refract_rate * s < 1 && !schlickApproximate(c, refract_rate))
                reflectedDirection = -refract_rate * I + (refract_rate * (Vector3f::dot(I, N) - sqrt(1 - refract_rate * refract_rate * (1 - c * c))) * N);
            else
                reflectedDirection = (I - 2 * (Vector3f::dot(I, N)) * N + hit.getMaterial()->getFuzz() * Utils::random_v()).normalized();

            Ray reflectedRay = Ray(hit.getPoint(), reflectedDirection.normalized(), ray.time);
            // finalColor += MonteCarloIntersectionColor(sceneparser, reflectedRay, baseGroup, depth + 1, hit.getHitObjectId());
            Vector3f newColor = float(1) / (depth + 1) * MonteCarloIntersectionColor(sceneparser, reflectedRay, baseGroup, depth + 1, hit.getHitObjectId());
            if (lastHit != hit.getHitObjectId())
                finalColor = Vector3f(finalColor.x() * newColor.x(), finalColor.y() * newColor.y(), finalColor.z() * newColor.z());
        }
        // }
    }
    else
    {
        return sceneparser->getBackgroundColor();
        // auto a = 0.5 * (ray.getDirection().y() + 1.0);
        // return (1.0 - a) * Vector3f(1.0, 1.0, 1.0) + a * Vector3f(0.5, 0.7, 1.0);
    }
    // finalColor.print();
    // lightColor.print();
    // (finalColor + lightColor).print();
    // cout << "--" << endl;
    // cout << "------------" << endl;
    // finalColor.print();
    // directColor.print();
    return finalColor + lightColor + directColor;
}

Image MonteCarlo(SceneParser *sceneparser)
{
    Camera *camera = sceneparser->getCamera();
    Image image = Image(camera->getWidth(), camera->getHeight());
    int k = 0;
    for (int x = 0; x < camera->getWidth(); ++x)
    {
        for (int y = 0; y < camera->getHeight(); ++y)
        {
            Group *baseGroup = sceneparser->getGroup();
            Vector3f finalColor;
            for (int i = 0; i < 10; i++)
            {
                int s_x = x + Utils::generateRandomInt(-1, 1);
                int s_y = y + Utils::generateRandomInt(-1, 1);
                Ray camRay = sceneparser->getCamera()->generateRay(Vector2f(s_x, s_y)); // 有可能有问题
                finalColor += MonteCarloIntersectionColor(sceneparser, camRay, baseGroup, 0, -1);
            }
            finalColor = finalColor / 10;
            finalColor = Vector3f(sqrt(finalColor.x()), sqrt(finalColor.y()), sqrt(finalColor.z()));
            // finalColor.print();
            // finalColor.print();
            image.SetPixel(x, y, finalColor);
            // cout << k++ << endl;
        }
    }
    return image;
}

#endif