#ifndef AABB_H
#define AABB_H
#include <iostream>
#include "Vector3f.h"
#include "ray.hpp"
#include "hit.hpp"
using namespace std;

class Interval
{
public:
    Interval() { min = max = 0; }
    Interval(float _min, float _max) : min(_min), max(_max) {}
    Interval(const Interval &interval0, const Interval &interval1)
    {
        this->min = interval0.min <= interval1.min ? interval0.min : interval1.min;
        this->max = interval0.max >= interval1.max ? interval0.max : interval1.max;
    }
    Interval expand(float delta)
    {
        float padding = delta / 2;
        return Interval(min - padding, max + padding);
    }
    float min;
    float max;
};

class AABB
{
public:
    AABB() {}
    AABB(Interval _x, Interval _y, Interval _z) : x(_x), y(_y), z(_z) {}
    AABB(Vector3f start, Vector3f end)
    {
        x = start.x() <= end.x() ? Interval(start.x(), end.x()) : Interval(end.x(), start.x());
        y = start.y() <= end.y() ? Interval(start.y(), end.y()) : Interval(end.y(), start.y());
        z = start.z() <= end.z() ? Interval(start.z(), end.z()) : Interval(end.z(), start.z());
    }
    AABB(const AABB &box0, const AABB &box1)
    {
        x = Interval(box0.x, box1.x);
        y = Interval(box0.y, box1.y);
        z = Interval(box0.z, box1.z);
    }
    const Interval &axis(int n) const
    {
        if (n == 0)
            return x;
        if (n == 1)
            return y;
        return z;
    }
    bool intersect(const Ray &ray, float tmin)
    {
        const Vector3f &origin = ray.getOrigin();
        const Vector3f &direction = ray.getDirection();
        for (int i = 0; i < 3; i++)
        {
            const Interval ax = axis(i);
            float adinv = 1.0 / direction[i];
            float t0 = (ax.min - direction[i]) * adinv;
            float t1 = (ax.max - direction[i]) * adinv;
            if (t0 > t1)
            {
                return false;
            }
        }
        return true;
    }
    Interval x, y, z;
};
#endif