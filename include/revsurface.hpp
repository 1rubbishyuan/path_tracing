#ifndef REVSURFACE_HPP
#define REVSURFACE_HPP
#define PI 3.1415926
#include "object3d.hpp"
#include "curve.hpp"
#include <tuple>
#include <vector>
#include <cmath>
#include "aabb.hpp"
#include "group.hpp"
#include "quad.hpp"
#include "hit.hpp"
using namespace std;
class RevSurface : public Object3D
{
public:
    float xmax = 0;
    float ymax = 0;
    RevSurface(Curve *pCurve, Material *material) : pCurve(pCurve), Object3D(material)
    {
        // Check flat.
        for (const auto &cp : pCurve->getControls())
        {
            if (cp.z() != 0.0)
            {
                printf("Profile of revSurface must be flat on xy plane.\n");
                exit(0);
            }
            xmax = max(xmax, abs(cp.x()));
            ymax = max(ymax, abs(cp.y()));
        }
        // xmax += 0.5;
        // ymax += 0.5;
        this->bbox = AABB(Vector3f(-xmax, -ymax, -xmax), Vector3f(xmax, ymax, xmax));
        this->quadGroup = new Group(6);
        this->quadGroup->addObject(0, new Quad(Vector3f(-xmax, -ymax, xmax), Vector3f(2 * xmax, 0, 0), Vector3f(0, 2 * ymax, 0), this->material));
        this->quadGroup->addObject(1, new Quad(Vector3f(-xmax, -ymax, -xmax), Vector3f(0, 2 * ymax, 0), Vector3f(2 * xmax, 0, 0), this->material));
        this->quadGroup->addObject(2, new Quad(Vector3f(-xmax, -ymax, xmax), Vector3f(0, 2 * ymax, 0), Vector3f(0, 0, -2 * xmax), this->material));
        this->quadGroup->addObject(3, new Quad(Vector3f(xmax, -ymax, xmax), Vector3f(0, 0, -2 * xmax), Vector3f(0, 2 * ymax, 0), this->material));
        this->quadGroup->addObject(4, new Quad(Vector3f(-xmax, -ymax, xmax), Vector3f(0, 0, -2 * xmax), Vector3f(2 * xmax, 0, 0), this->material));
        this->quadGroup->addObject(5, new Quad(Vector3f(-xmax, ymax, xmax), Vector3f(2 * xmax, 0, 0), Vector3f(0, 0, -2 * xmax), this->material));
    }

    ~RevSurface() override
    {
        delete pCurve;
        delete quadGroup;
    }

    bool intersect(const Ray &r, Hit &h, float tmin, int type) override
    {
        // (PA2 optional TODO): implement this for the ray-tracing routine using G-N iteration.
        Hit qhit;
        bool intersectq = this->quadGroup->intersect(r, qhit, 0.01, 99);
        if (!intersectq)
        {
            // cout << 1 << endl;
            return false;
        }
        float u = 0.6;
        // float vs[10] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1};
        float v = 0.1;
        float guessT = 6;
        Vector3f point = qhit.getPoint();
        u = (point.y() - ymax) / (-2 * ymax);
        Vector3f ppoint = Vector3f(point.x(), 0, point.z()).normalized();
        v = acos(-ppoint.x()) / (2 * PI);
        if (ppoint.z() > 0)
        {
            v = 1 - v;
            // cout << "1: " << v << endl;
        }
        // else
        // {
        //     cout << "2: " << v << endl;
        // }
        // cout << v << endl;
        // v = 0.03;
        // if (v < 0.1)
        // {
        //     cout << v << endl;
        //     r.getDirection().print();
        // }
        guessT = qhit.getT();
        float epsilon = 0.00001;
        int iterationTimes = 35;
        // int resolutipon = 100;
        // vector<CurvePoint> data;
        // pCurve->discretize(resolutipon, data);
        // cout << "--------" << endl;
        // float length = 1000;
        // for (int j = 0; j < 10; j++)
        // {
        //     v = vs[j];
        // cout << "-----" << endl;
        // qhit.getPoint().print();
        for (int j = 0; j < 2; j++)
        {
            if (j == 0)
            {
                v = v;
            }
            else
            {
                v = 1 - v;
            }
            for (int i = 0; i < iterationTimes; i++)
            {
                // cout << i << endl;
                // if (i > 10)
                // {
                //     cout << u << " " << v << endl;
                // }
                CurvePoint guessCurvePoint = pCurve->evaluate(u);
                // guessCurvePoint.V.print();
                Vector3f p = rotate_v(guessCurvePoint.V, 2 * v * PI);
                // cout << "----- " << " " << v << " " << 2 * v * PI << endl;
                // guessCurvePoint.V.print();
                // p.print();
                Vector3f guessResolution = r.pointAtParameter(guessT);
                // if (i == 0)
                // {
                // guessResolution.print();
                // }
                Vector3f Judge = p - guessResolution;
                // cout << i << " " << Judge.length() << endl;
                // p.print();
                // if (guessT != 0.5)
                //     cout << guessT << endl;
                // length = min(Judge.length(), length);
                // if (v < 0.5)
                //     cout << Judge.length() << " " << u << " " << v << " " << guessT << endl;
                if (Judge.length() < epsilon)
                {
                    // cout << 1 << endl;
                    // guessResolution.print();
                    // cout << Judge.length() << endl;
                    if (guessT < tmin)
                    {
                        // cout << 1 << endl;
                        return false;
                    }
                    Vector3f n1 = Vector3f(guessResolution.x(), 0, guessResolution.z());
                    Vector3f n2 = Vector3f::cross(n1, Vector3f(0, 1, 0));
                    Vector3f tang = rotate_v(guessCurvePoint.T, 2 * v * PI);
                    Vector3f normal = -Vector3f::cross(tang, n2).normalized();
                    h.set(guessT, this->material, normal, guessResolution);
                    h.u = u;
                    h.v = 1 - v;
                    return true;
                }
                Matrix3f J;
                J(0, 0) = guessCurvePoint.T.x() * cos(2 * v * PI);
                J(0, 1) = -2 * PI * guessCurvePoint.V.x() * sin(2 * v * PI);
                J(0, 2) = -r.getDirection().x();
                J(1, 0) = guessCurvePoint.T.y();
                J(1, 1) = 0;
                J(1, 2) = -r.getDirection().y();
                J(2, 0) = guessCurvePoint.T.x() * sin(2 * v * PI);
                J(2, 1) = 2 * PI * guessCurvePoint.V.x() * cos(2 * v * PI);
                J(2, 2) = -r.getDirection().z();
                // cout << J.determinant() << endl;
                // J.print();
                if (fabs(J.determinant()) < epsilon)
                {
                    // cout << J.determinant() << endl;
                    // cout << 2 << endl;
                    // return false;
                    break;
                }
                Matrix3f invJ = J.inverse();
                Vector3f delta = Vector3f(
                    invJ(0, 0) * Judge.x() + invJ(0, 1) * Judge.y() + invJ(0, 2) * Judge.z(),
                    invJ(1, 0) * Judge.x() + invJ(1, 1) * Judge.y() + invJ(1, 2) * Judge.z(),
                    invJ(2, 0) * Judge.x() + invJ(2, 1) * Judge.y() + invJ(2, 2) * Judge.z());
                // delta.print();
                // if (i >= 100)
                // {
                //     // cout << Judge.length() << " " << u << " " << v << " " << guessT << endl;
                //     // delta.print();
                // }
                u = u - delta.x();
                v = v - delta.y();
                guessT = guessT - delta.z();
                // cout << u << " " << v << " " << guessT << endl;
                // if (u < 0)
                //     u = 0;
                // if (u > 1)
                //     u = 1;
                // if (v < 0)
                //     v = 0;
                // if (v > 1)
                //     v = 1;
                // if (guessT < 0)
                //     guessT = 0;
                if (u < 0 || u > 1 || v < 0 || v > 1)
                {
                    // cout << 3 << endl;
                    // return false;
                    break;
                }
                // }
            }
        }
        // cout << length << endl;
        return false;
    }

    Vector3f rotate_v(Vector3f originPoint, float theta)
    {
        Vector3f ans;
        originPoint = originPoint - center;
        // ans = originPoint * cos(theta) + (Vector3f::cross(axis, originPoint)) * sin(theta) + axis * (Vector3f::dot(axis, originPoint)) * (1 - cos(theta));
        // cout << theta << endl;
        // originPoint.print();
        // ans.print();
        // center.print();
        ans = Vector3f(originPoint.x() * cos(theta), originPoint.y(), originPoint.x() * sin(theta));
        return ans + center;
    }
    void setAxis(Vector3f _axis)
    {
        this->axis = _axis;
    }
    void setCenter(Vector3f _center)
    {
        this->center = _center;
    }
    AABB boundingBox() override
    {
        // cout << this->bbox.x.min << " " << this->bbox.x.max << endl;
        // cout << this->bbox.y.min << " " << this->bbox.y.max << endl;
        // cout << this->bbox.z.min << " " << this->bbox.z.max << endl;
        return this->bbox;
    }

private:
    Curve *pCurve;
    Vector3f axis;
    Vector3f center;
    AABB bbox;
    Group *quadGroup;
};

#endif // REVSURFACE_HPP
