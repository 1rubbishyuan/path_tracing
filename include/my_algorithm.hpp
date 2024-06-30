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
#include <thread>
#include <future>
#include <vector>
#include <omp.h>
#include <ctime>
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

    float p = 0.8;
    float rr = Utils::generateRandomFloat(0, 1);
    if (rr > p)
    {
        // cout << depth << endl;
        return Vector3f(0, 0, 0);
    }
    Hit hit;
    bool isIntersect = baseGroup->intersect(ray, hit, 0.000001, 0);
    Vector3f finalColor = Vector3f::ZERO; //= depth == 0 ? sceneparser->getBackgroundColor() : Vector3f::ZERO;
    Vector3f lightColor = Vector3f::ZERO;
    Vector3f directColor;
    Material *material = hit.getMaterial();
    if (isIntersect)
    {
        // finalColor = lastHit != 1 ? hit.getMaterial()->getDiffuseColor() : Vector3f::ZERO;
        // finalColor = hit.getMaterial()->getDiffuseColor();

        lightColor = material->getEmitColor();
        if (material->isALight())
        {
            // cout << 1 << endl;
            float boost = (depth != 0 || lastHit == 1) ? material->boost : 1;
            return material->getEmitColor() * boost;
        }
        // if (hit.getNormal().z() > 0.8)
        // {
        //     hit.getPoint().print();
        //     lightPoint.print();
        //     line.print();
        // }
        float mid1 = material->getPerDiffuse();
        float mid2 = material->getPerReflect() + mid1;
        float mid = Utils::generateRandomFloat(0, 1);

        if (mid < mid1)
        {
            if (lastHit != 1)
            {
                LightGroup *lightGroup = sceneparser->getLightGroup();
                finalColor = material->getDiffuseColor();
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
                    bool isIntersectl = baseGroup->intersect(lray, lhit, 0.000001, 1);
                    if (isIntersectl && abs(lhit.getT() - line.length()) < 0.1)
                    {

                        getEnergy = true;
                        float brdfValue = material->getBrdf()->getBrdfValue(ray.getDirection(), lray.getDirection(), hit.getNormal());

                        float distance = 1 / (line.length() * line.length());
                        float NdotL = max(0.0f, Vector3f::dot(hit.getNormal(), unit_line));
                        // directColor += brdfValue * float(1) / (1 + depth) * finalColor * MonteCarloIntersectionColor(sceneparser, lray, baseGroup, 0, 1) * distance * NdotL * 1500;

                        directColor += brdfValue * float(1) / (1 + depth) * finalColor * lhit.getMaterial()->getEmitColor() * distance * NdotL * 1500 * lhit.getMaterial()->boost;
                    }
                }

                Ray diffusedRay = getDifussionRay(hit, hit.getPoint());
                diffusedRay.time = ray.time;
                float brdfValue = material->getBrdf()->getBrdfValue(ray.getDirection(), diffusedRay.getDirection(), hit.getNormal());
                float NdotV = max(0.0f, Vector3f::dot(hit.getNormal(), -ray.getDirection()));
                Vector3f newColor = 1000 * brdfValue * NdotV * float(1) / (depth + 1) * MonteCarloIntersectionColor(sceneparser, diffusedRay, baseGroup, depth + 1, lastHit);
                newColor = Utils::clampV(newColor);
                // (finalColor * newColor).print();
                finalColor = finalColor * newColor + directColor; //(finalColor.x() * newColor.x(), finalColor.y() * newColor.y(), finalColor.z() * newColor.z());
            }
        }
        else if (mid <= mid2)
        {
            if (lastHit != 1)
            {
                finalColor = material->getDiffuseColor();
                Vector3f I = ray.getDirection();
                Vector3f N = hit.getNormal();
                directColor = Vector3f::ZERO;
                Vector3f reflectedDirection = (I - 2 * (Vector3f::dot(I, N)) * N).normalized(); // + hit.getMaterial()->getFuzz() * Utils::random_v()).normalized();
                Ray reflectedRay = Ray(hit.getPoint(), reflectedDirection, ray.time);
                // float brdfValue = hit.getMaterial()->getBrdf()->getBrdfValue(ray.getDirection(), reflectedRay.getDirection(), hit.getNormal());
                // cout << brdfValue << endl;
                float NdotV = max(0.0f, Vector3f::dot(hit.getNormal(), -ray.getDirection()));
                Vector3f newColor = NdotV * float(1) / (depth + 1) * MonteCarloIntersectionColor(sceneparser, reflectedRay, baseGroup, depth + 1, lastHit);
                newColor = Utils::clampV(newColor);
                // newColor.print();
                // if (lastHit != hit.getHitObjectId())
                finalColor = Vector3f(finalColor.x() * newColor.x(), finalColor.y() * newColor.y(), finalColor.z() * newColor.z());
            }
        }
        else
        {
            finalColor = material->getDiffuseColor();
            Vector3f I = ray.getDirection();
            Vector3f N = hit.getNormal();
            float refract_rate = material->getRefractRate();

            float c = Vector3f::dot(-I, N); /// (I.length() * N.length());
            if (c > 0)
            {
                // cout << lastHit << endl;
                refract_rate = 1 / refract_rate;
            }
            float s = sqrt(1.0 - c * c);
            Vector3f reflectedDirection;
            // cout << refract_rate * s << endl;
            if (refract_rate * s < 1 && !schlickApproximate(c, refract_rate))
            {
                // cout << 1;
                Vector3f rout = refract_rate * (I + c * N);
                reflectedDirection = rout - sqrt(fabs(1.0 - Vector3f::dot(rout, rout))) * N;
            }
            // reflectedDirection = -refract_rate * I + refract_rate * (Vector3f::dot(I, N) - sqrt(1 - refract_rate * refract_rate * (1 - c * c))) * N;
            else
            {
                // cout << 2;
                // float r0 = (1 - refract_rate) / (1 + refract_rate);
                // r0 = r0 * r0;
                // cout << refract_rate * s << " " << c<<" "<<r0 << " " << r0 + (1 - r0) * pow((1 - c), 5) << endl;
                reflectedDirection = (I - 2 * (Vector3f::dot(I, N)) * N).normalized(); //(I - 2 * (Vector3f::dot(I, N)) * N + hit.getMaterial()->getFuzz() * Utils::random_v()).normalized();
            }
            Ray reflectedRay = Ray(hit.getPoint(), reflectedDirection.normalized(), ray.time);
            // finalColor += MonteCarloIntersectionColor(sceneparser, reflectedRay, baseGroup, depth + 1, hit.getHitObjectId());
            Vector3f newColor = 2 * MonteCarloIntersectionColor(sceneparser, reflectedRay, baseGroup, depth + 1, lastHit);
            newColor = Utils::clampV(newColor);
            // if (lastHit != hit.getHitObjectId())
            finalColor = Vector3f(finalColor.x() * newColor.x(), finalColor.y() * newColor.y(), finalColor.z() * newColor.z());
            // newColor.print();
            // finalColor.print();
            // cout << "-----" << endl;
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
    return finalColor;
}

Vector3f sample(int x, int y, SceneParser *sceneparser, Group *baseGroup)
{
    Vector3f finalColor;
    int s_x = x + Utils::generateRandomInt(-1, 1);
    int s_y = y + Utils::generateRandomInt(-1, 1);
    Ray camRay = sceneparser->getCamera()->generateRay(Vector2f(s_x, s_y)); // 有可能有问题
    finalColor = MonteCarloIntersectionColor(sceneparser, camRay, baseGroup, 0, 0);
    return finalColor;
}

Image MonteCarlo(SceneParser *sceneparser)
{
    clock_t start = clock();
    Camera *camera = sceneparser->getCamera();
    Image image = Image(camera->getWidth(), camera->getHeight());
    int k = 0;
    int sample_times = 100;
#pragma omp parallel for num_threads(4)
    for (int x = 0; x < camera->getWidth(); ++x)
    {
        for (int y = 0; y < camera->getHeight(); ++y)
        {
            vector<future<Vector3f>> futures;
            vector<thread> threads;
            Group *baseGroup = sceneparser->getGroup();
            Vector3f finalColor;
            for (int i = 0; i < sample_times; i++)
            {
                int s_x = x + Utils::generateRandomInt(-1, 1);
                int s_y = y + Utils::generateRandomInt(-1, 1);
                Ray camRay = sceneparser->getCamera()->generateRay(Vector2f(s_x, s_y)); // 有可能有问题
                finalColor += MonteCarloIntersectionColor(sceneparser, camRay, baseGroup, 0, 0);
            }
            finalColor = finalColor / sample_times;
            finalColor = Vector3f(sqrt(finalColor.x()), sqrt(finalColor.y()), sqrt(finalColor.z()));
            image.SetPixel(x, y, finalColor);
        }
    }
    clock_t end = clock();
    cout << "time: " << (end - start) / CLOCKS_PER_SEC << "s" << endl;
    return image;
}

#endif