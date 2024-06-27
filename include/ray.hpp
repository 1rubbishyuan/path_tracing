#ifndef RAY_H
#define RAY_H

#include <cassert>
#include <iostream>
#include <Vector3f.h>

// Ray class mostly copied from Peter Shirley and Keith Morley
class Ray
{
public:
    Ray() = delete;
    Ray(const Vector3f &orig, const Vector3f &dir)
    {
        origin = orig;
        direction = dir;
        min = 0;
        max = 1e20;
    }

    Ray(const Ray &r)
    {
        origin = r.origin;
        direction = r.direction;
        time = r.time;
        min = 0;
        max = 1e20;
    }
    Ray(const Vector3f &orig, const Vector3f &dir, float _time) : origin(orig), direction(dir)
    {
        // cout << _time << endl;
        this->time = _time;
        min = 0;
        max = 1e20;
    }

    const Vector3f &getOrigin() const
    {
        return origin;
    }

    const Vector3f &getDirection() const
    {
        return direction;
    }

    Vector3f pointAtParameter(float t) const
    {
        return origin + direction * t;
    }
    float time;
    float min;
    float max;

private:
    Vector3f origin;
    Vector3f direction;
};

inline std::ostream &operator<<(std::ostream &os, const Ray &r)
{
    os << "Ray <" << r.getOrigin() << ", " << r.getDirection() << ">";
    return os;
}

#endif // RAY_H
