#ifndef GROUP_H
#define GROUP_H

#include "object3d.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include "aabb.hpp"
#include <iostream>
#include <vector>
#include "bvhNode.hpp"
// TODO: Implement Group - add data structure to store a list of Object*
class Group : public Object3D
{

public:
    Group()
    {
    }

    explicit Group(int num_objects)
    {
        this->all_object.resize(num_objects);
        this->num_objects = num_objects;
    }

    ~Group() override
    {
    }

    bool intersect(const Ray &r, Hit &h, float tmin, int type) override
    {
        bool is_intersect = false;
        for (int i = 0; i < this->num_objects; i++)
        {
            // if (type == 1)
            // {
            //     cout << is_intersect << endl;
            // }
            // std::cout << i << std::endl;
            if (!is_intersect)
            {
                is_intersect = all_object[i]->intersect(r, h, tmin, type);
                // std::cout << i << std::endl;
            }
            else
            {
                bool m = all_object[i]->intersect(r, h, tmin, type);
            }
            // cout << h.getT() << endl;
        }
        return is_intersect;
        // return root->intersect(r, h, tmin, type);
    }

    void addObject(int index, Object3D *obj)
    {
        all_object[index] = obj;
        // cout << "sad" << endl;
        // cout << this->bbox.x.max << endl;
        this->bbox = AABB(this->bbox, obj->boundingBox());
        if (index == num_objects - 1)
        {
            root = new BvhNode(all_object, 0, index);
        }
    }

    int getGroupSize()
    {
        return this->num_objects;
    }
    void getUV(Vector3f point, float &u, float &v) override
    {
    }
    AABB boundingBox() override
    {
        return this->bbox;
    };
    std::vector<Object3D *> all_object;

private:
    int num_objects;
    AABB bbox;
    BvhNode *root;
};

class LightGroup
{
public:
    LightGroup(int _lightsNum) : lightsNum(_lightsNum) {}

    void addLight(Object3D *light)
    {
        // cout << lightsNum << endl;
        lights.push_back(light);
        lightsNum++;
    }
    // Vector3f getLightsEnergy(const Vector3f &point, Group *baseGroup)
    // {
    //     Vector3f energy = Vector3f(1, 1, 1);
    //     bool getEnergy = false;
    //     for (int i = 0; i < lightsNum; i++)
    //     {
    //         Hit hit;
    //         Vector3f line = lights[i]->randomFace() - point;
    //         Ray ray = Ray(point, line.normalized());
    //         bool isIntersect = baseGroup->intersect(ray, hit, 0.0001, 0);
    //         if (isIntersect && abs(hit.getT() - line.length()) < 0.001)
    //         {
    //             getEnergy = true;
    //             energy = energy * hit.getMaterial()->getEmitColor();
    //         }
    //     }
    //     if (!getEnergy)
    //     {
    //         return Vector3f::ZERO;
    //     }
    //     return energy;
    // }
    vector<Object3D *> lights;
    int lightsNum;
};
#endif
