#ifndef SPHERE_H
#define SPHERE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>
#include "aabb.hpp"
#include "utils.hpp"
// TODO: Implement functions and add more fields as necessary

class Sphere : public Object3D
{
public:
    Sphere()
    {
        // unit ball at the center
        this->center = Vector3f(0);
        this->centerFixed = center;
        this->radius = 1;
        this->Id = num;
        Vector3f rVector = Vector3f(radius, radius, radius);
        this->bbox = AABB(center - rVector, center + rVector);
        this->isMoving = false;
        num++;
    }

    Sphere(const Vector3f &center, float radius, Material *material) : Object3D(material)
    {
        //
        this->center = center;
        this->centerFixed = center;
        this->radius = radius;
        this->Id = num;
        Vector3f rVector = Vector3f(radius, radius, radius);
        this->bbox = AABB(center - rVector, center + rVector);
        this->isMoving = false;
        num++;
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin, int type) override
    {
        // 判断r是否与这个球有交，并更新hit
        // std::cout << "k" << std::endl;
        // cout << r.time << endl;
        this->center = isMoving ? centerFixed + r.time * moveVector : centerFixed;
        Vector3f l = this->center - r.getOrigin();
        Vector3f unit_direction = r.getDirection().normalized();
        float t_p = Vector3f::dot(l, unit_direction);
        float length_l = l.length();
        if (t_p < 0 && length_l > this->radius)
        {
            // std::cout << "sphere0" << endl;
            return false;
        }
        if (abs(length_l - this->radius) < 0.0001)
        {
            // if (tmin <= 0)
            // {
            //     Vector3f n = (r.getOrigin() - this->center).normalized();
            //     h.set(0, this->material, n);
            //     return true;
            // }
            // std::cout << "sphere1" << endl;
            return false;
        }
        float length_d = sqrt(length_l * length_l - t_p * t_p);
        if (length_d >= this->radius)
        {
            // std::cout << "sphere2" << endl;

            return false;
        }
        float t_pi = sqrt(this->radius * this->radius - length_d * length_d);
        float t = 0;
        if (length_l > this->radius)
            t = t_p - t_pi;
        else if (length_l < this->radius)
            t = t_p + t_pi;
        Vector3f n = (unit_direction * t + r.getOrigin() - this->center).normalized();
        // std::cout << "sphere: " << t << std::endl;
        if (h.getT() > t && t > tmin)
        {
            // std::cout << "sphere_t: " << t << " " << length_l - this->radius << std::endl;
            h.set(t, this->material, n, r.pointAtParameter(t));
            getUV((r.pointAtParameter(t) - this->center) / radius, h.u, h.v);
            if (this->material->isTextured())
            {
                // cout << "as" << endl;
                this->material->changeColorAccordingTexture(h.getU(), h.getV(), h.getPoint());
            }

            // cout << h.u << " " << h.v << endl;
            h.setObjectId(this->Id);
            return true;
        }
        else
        {
            // std::cout << "sphere_f: " << t << " " << length_l - this->radius << std::endl;
            return false;
        }
    }
    void getUV(Vector3f point, float &u, float &v) override
    {
        float pi = 3.1415926535;
        float x = point.x();
        float y = point.y();
        float theta = acos(-point.y());
        float phi = atan2(-point.z(), point.x()) + pi;
        u = phi / (2 * pi);
        v = theta / pi;
    }
    AABB boundingBox() override
    {
        return this->bbox;
    };
    Vector3f randomFace() override
    {
        Vector3f rv = Utils::random_v();
        return center + rv * radius;
    };

    void setMoveVector(Vector3f _moveVector)
    {
        isMoving = true;
        moveVector = _moveVector;
        Vector3f rVector = Vector3f(radius, radius, radius);
        AABB bbox2 = AABB(centerFixed - rVector + _moveVector, centerFixed + rVector + _moveVector);
        bbox = AABB(bbox, bbox2);
    }

protected:
    Vector3f center;
    Vector3f centerFixed;
    float radius;
    AABB bbox;
    bool isMoving;
    Vector3f moveVector;
};

#endif
