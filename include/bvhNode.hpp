#ifndef BHVNODE_H
#define BHVNODE_H
#include <iostream>
#include "object3d.hpp"
#include "aabb.hpp"
// #include "utils.hpp"
#include <algorithm>
#include <vector>
#include <random>
using namespace std;
class BvhNode : public Object3D
{
public:
    int generateRandomInt_(int min, int max)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(min, max);
        int ans = dis(gen);
        return ans;
    }
    BvhNode(vector<Object3D *> &objects, int start, int end)
    {

        int axis = generateRandomInt_(0, 2);
        bool (*compare)(Object3D *, Object3D *);
        if (axis == 0)
        {
            compare = compareX;
        }
        else if (axis == 1)
        {
            compare = compareY;
        }
        else
        {
            compare = compareZ;
        }
        int scale = end - start + 1;
        if (scale == 1)
        {
            left = objects[start];
            right = objects[start];
        }
        else if (scale == 2)
        {
            left = objects[start];
            right = objects[end];
        }
        else
        {
            sort(objects.begin() + start, objects.begin() + end, compareX);
            int mid = int(start + end) / 2;
            left = new BvhNode(objects, start, mid);
            right = new BvhNode(objects, mid, end);
        }
        bbox = AABB(left->boundingBox(), right->boundingBox());
    }
    static bool compareX(Object3D *obj0, Object3D *obj1)
    {
        return obj0->boundingBox().x.min < obj1->boundingBox().x.min;
    }
    static bool compareY(Object3D *obj0, Object3D *obj1)
    {
        return obj0->boundingBox().y.min < obj1->boundingBox().y.min;
    }
    static bool compareZ(Object3D *obj0, Object3D *obj1)
    {
        return obj0->boundingBox().z.min < obj1->boundingBox().z.min;
    }
    bool intersect(const Ray &r, Hit &h, float tmin, int type) override
    {
        if (!bbox.intersect(r, tmin))
        {
            // cout << 2 << endl;
            return false;
        }
        // cout << 1 << endl;
        bool hitLeft = this->left->intersect(r, h, tmin, type);
        bool hitRight = this->right->intersect(r, h, tmin, type);
        return hitLeft || hitRight;
    }
    AABB boundingBox() override
    {
        return this->bbox;
    };

private:
    Object3D *left;
    Object3D *right;
    AABB bbox;
};
#endif