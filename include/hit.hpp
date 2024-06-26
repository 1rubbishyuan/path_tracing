#ifndef HIT_H
#define HIT_H

#include <vecmath.h>
#include "ray.hpp"
// #include "object3d.hpp"
// #include <iostream>
// #include <vector>
// using namespace std;
class Material;

class Hit
{
public:
    // constructors
    Hit()
    {
        material = nullptr;
        t = 1e38;
    }

    Hit(float _t, Material *m, const Vector3f &n)
    {
        t = _t;
        material = m;
        normal = n;
    }

    Hit(const Hit &h)
    {
        t = h.t;
        material = h.material;
        normal = h.normal;
    }

    // destructor
    ~Hit() = default;

    float getT() const
    {
        return t;
    }

    Material *getMaterial() const
    {
        return material;
    }
    int getHitObjectId() const
    {
        return hitObjectId;
    }
    const Vector3f &getNormal() const
    {
        return normal;
    }
    void set(float _t, Material *m, const Vector3f &n, const Vector3f &_point)
    {
        t = _t;
        point = _point;
        material = m;
        normal = n;
    }

    void setObjectId(int obId)
    {
        this->hitObjectId = obId;
    }

    const Vector3f getPoint()
    {
        return this->point;
    }
    float getU()
    {
        return this->u;
    }
    float getV()
    {
        return this->v;
    }
    float u;
    float v;

private:
    float t;
    Material *material;
    Vector3f normal;
    Vector3f point;
    int hitObjectId;
    // std::vector<Object3D *> all_object;
};

inline std::ostream &operator<<(std::ostream &os, const Hit &h)
{
    os << "Hit <" << h.getT() << ", " << h.getNormal() << ">";
    return os;
}

#endif // HIT_H
