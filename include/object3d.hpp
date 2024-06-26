#ifndef OBJECT3D_H
#define OBJECT3D_H

#include "ray.hpp"
#include "hit.hpp"
#include "material.hpp"
#include "aabb.hpp"
// Base class for all 3d entities.
static int num = 0;
class Object3D
{
public:
    Object3D() : material(nullptr) {}

    virtual ~Object3D() = default;

    explicit Object3D(Material *material)
    {
        this->material = material;
    }

    // Intersect Ray with this object. If hit, store information in hit structure.
    virtual bool intersect(const Ray &r, Hit &h, float tmin, int type) = 0;
    virtual void getUV(Vector3f point, float &u, float &v) {};
    virtual AABB boundingBox() { return AABB(); };
    virtual Vector3f randomFace() { return Vector3f::ZERO; };
    Material *material;

protected:
    int Id;
    // bool enable_reflect;
    // bool enable_refract;
};

#endif
