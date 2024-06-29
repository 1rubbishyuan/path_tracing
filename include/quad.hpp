#ifndef QUAD_H
#define QUAD_H

#include "object3d.hpp"
#include "aabb.hpp"
#include "utils.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>

class Quad : public Object3D
{
public:
    Quad(Vector3f _origin, Vector3f _u, Vector3f _v, Material *material) : Object3D(material), origin(_origin), uVector(_u), vVector(_v)
    {
        Vector3f mid = Vector3f::cross(uVector, vVector);
        normal = mid.normalized();
        d = Vector3f::dot(normal, origin);
        w = mid / Vector3f::dot(mid, mid);
        AABB bbox1 = AABB(origin, origin + uVector + vVector);
        AABB bbox2 = AABB(origin + uVector, origin + vVector);
        bbox = AABB(bbox1, bbox2);
        this->Id = num;
        num++;
    }
    ~Quad() {}
    bool intersect(const Ray &r, Hit &h, float tmin, int type) override
    {
        // cout << 1 << endl;
        float judge = Vector3f::dot(this->normal, r.getDirection().normalized());
        float t = (this->d - Vector3f::dot(this->normal, r.getOrigin())) /
                  (judge);
        // if (type == 1)
        //     cout << t << endl;
        // if (type == 12)
        // {
        //     cout << judge << endl;
        // }
        if (judge > 0 && type != 12)
        {
            return false;
        }
        if (t < 0 || t < tmin)
            return false;
        Vector3f n = this->normal.normalized();
        if (t < h.getT())
        {
            // cout << 2 << endl;
            Vector3f p = r.pointAtParameter(t) - origin;
            float t_alpha = Vector3f::dot(w, Vector3f::cross(p, vVector));
            float t_beta = Vector3f::dot(w, Vector3f::cross(uVector, p));

            if (t_alpha > 1 || t_alpha < 0 || t_beta > 1 || t_beta < 0)
            {

                return false;
            }
            // cout << t_alpha << " " << t_beta << endl;
            this->alpha = t_alpha;
            this->beta = t_beta;
            getUV(Vector3f::ZERO, h.u, h.v);
            // if (n.z() > 0.8)
            // {
            //     r.pointAtParameter(t).print();
            // }
            if (this->material->hasNTexture)
            {
                Vector3f nInfo = this->material->getNinfo(h.getU(), h.getV());
                // nInfo.print();
                nInfo = (2 * nInfo - Vector3f(1, 1, 1)).normalized();
                Vector3f on = n;
                n = nInfo[0] * uVector.normalized() + nInfo[1] * vVector.normalized() + nInfo[2] * n;
                if (Vector3f::dot(on, n) < 0)
                {
                    // n.print();
                    // (255 * (nInfo + Vector3f(1, 1, 1)) / 2).print();
                    n = -n;
                }
                n = n.normalized();
            }
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
        u = alpha;
        v = beta;
    }
    AABB boundingBox() override
    {
        return this->bbox;
    };
    Vector3f randomFace() override
    {
        float a = Utils::generateRandomFloat(0, 1);
        float b = Utils::generateRandomFloat(0, 1);
        return origin + a * uVector + b * vVector;
    };

private:
    Vector3f origin;
    Vector3f uVector, vVector, w;
    Vector3f normal;
    float d;
    float alpha;
    float beta;
    AABB bbox;
};

#endif // PLANE_H
