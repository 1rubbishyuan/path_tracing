// #ifndef LIGHTGROUP_H
// #define LIGHTGROUP_H
// #include <iostream>
// #include <vector>
// #include "object3d.hpp"
// #include "Vector3f.h"
// #include "group.hpp"
// #include "hit.hpp"
// using namespace std;

// class LightGroup
// {
// public:
//     LightGroup(int _lightsNum) : lightsNum(_lightsNum) {}

//     void addLight(Object3D *light)
//     {
//         lights.push_back(light);
//         lightsNum++;
//     }
//     Vector3f getLightsEnergy(const Vector3f &point, Group *baseGroup)
//     {
//         Vector3f energy = Vector3f(1, 1, 1);
//         bool getEnergy = false;
//         for (int i = 0; i < lightsNum; i++)
//         {
//             Hit hit;
//             Vector3f line = lights[i]->randomFace() - point;
//             Ray ray = Ray(point, line.normalized());
//             bool isIntersect = baseGroup->intersect(ray, hit, 0.0001, 0);
//             if (isIntersect && abs(hit.getT() - line.length()) < 0.001)
//             {
//                 getEnergy = true;
//                 energy = energy * hit.getMaterial()->getEmitColor(Vector3f::ZERO, nullptr);
//             }
//         }
//         if (!getEnergy)
//         {
//             return Vector3f::ZERO;
//         }
//         return energy;
//     }

// private:
//     vector<Object3D *> lights;
//     int lightsNum;
// };
// #endif