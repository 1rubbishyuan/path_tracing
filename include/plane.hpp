#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>
// TODO: Implement Plane representing an infinite plane
// function: ax+by+cz=d
// choose your representation , add more fields and fill in the functions

class Plane : public Object3D
{
public:
    Plane()
    {
        this->Id = num;
        num++;
    }

    Plane(const Vector3f &normal, float d, Material *m) : Object3D(m)
    {
        this->normal = normal.normalized();
        this->d = d;
        this->Id = num;
        num++;
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin, int type) override
    {
        float t = (this->d + Vector3f::dot(this->normal, r.getOrigin())) /
                  (Vector3f::dot(this->normal, r.getDirection().normalized()));
        // std::cout << "j: " << t << std::endl;
        if (t < 0 || t < tmin)
            return false;
        Vector3f n = this->normal.normalized();
        if (t < h.getT())
        {
            // std::cout << "plane: " << t << std::endl;
            h.set(t, this->material, n, r.pointAtParameter(t));
            if (this->material->isTextured())
            {
                this->material->changeColorAccordingTexture(h.getU(), h.getV(), h.getPoint());
            }

            h.setObjectId(this->Id);
            return true;
        }
        else
        {
            return false;
        }
    }
    void getUV(Vector3f point, float &u, float &v) override
    {
    }

protected:
    Vector3f normal;
    float d;
};

#endif // PLANE_H
