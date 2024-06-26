#ifndef CAMERA_H
#define CAMERA_H

#include "ray.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>
#include "utils.hpp"
class Camera
{
public:
    Camera(const Vector3f &center, const Vector3f &direction, const Vector3f &up, int imgW, int imgH)
    {
        this->center = center;
        this->direction = direction.normalized();
        this->horizontal = Vector3f::cross(this->direction, up).normalized();
        this->up = Vector3f::cross(this->horizontal, this->direction);
        this->width = imgW;
        this->height = imgH;
    }

    // Generate rays for each screen-space coordinate
    virtual Ray generateRay(const Vector2f &point) = 0;
    virtual ~Camera() = default;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

protected:
    // Extrinsic parameters
    Vector3f center;
    Vector3f direction;
    Vector3f up;
    Vector3f horizontal;
    // Intrinsic parameters
    int width;
    int height;
};

// TODO: Implement Perspective camera
// You can add new functions or variables whenever needed.
class PerspectiveCamera : public Camera
{

public:
    PerspectiveCamera(const Vector3f &center, const Vector3f &direction,
                      const Vector3f &up, int imgW, int imgH, float angle, float _defocus_angel, float _focus_dist) : Camera(center, direction, up, imgW, imgH), defocus_angle(_defocus_angel), focus_dist(_focus_dist)
    {
        // angle is in radian.
        std::cout << "center: ";
        center.print();
        this->angle = angle;
        this->horizontal = Vector3f::cross(this->direction, this->up).normalized();

        float defocusRadius = focus_dist * tan(defocus_angle * 3.1415926 / (2 * 180));
        defocus_u = horizontal * defocusRadius;
        defocus_v = up * defocusRadius;
    }

    Ray generateRay(const Vector2f &point) override
    {
        //
        float cx = this->width / 2;
        float cy = this->height / 2;
        float fx = 2 * tan(angle / 2) / this->width;
        float fy = 2 * tan(angle / 2) / this->height;
        Vector3f d_rc = Vector3f((point.x() - cx) * fx, (-point.y() + cy) * fy, 1);
        d_rc.normalize();
        // d_rc.print();
        Matrix3f R = Matrix3f(this->horizontal, -this->up.normalized(), this->direction.normalized(), true);
        Vector3f d_rw = R * d_rc;
        // this->center.print();
        // d_rw.print();
        Vector3f rayOrigin = defocus_angle <= 0 ? this->center : originSample();
        // this->center.print();
        // rayOrigin.print();
        // cout << "---" << endl;
        Vector3f rayDirection = center + d_rw.normalized() * focus_dist - rayOrigin;
        float time = Utils::generateRandomFloat(0, 1);
        // cout << time << endl;
        return Ray(rayOrigin, rayDirection.normalized(), time);
    }
    void setDefoucusInfo(float _defocus_angel, float _focus_dist)
    {
        this->defocus_angle = _defocus_angel;
        this->focus_dist = _focus_dist;
    }
    Vector3f originSample()
    {
        float s_x = Utils::generateRandomFloat(-1, 1);
        float s_y = Utils::generateRandomFloat(-1, 1);
        Vector3f unitVector = Vector3f(s_x, s_y, 0).normalized();
        return center + unitVector[0] * defocus_u + unitVector[1] * defocus_v;
    }

private:
    float defocus_angle = 0;
    float focus_dist = 1;
    float angle;
    Vector3f defocus_u;
    Vector3f defocus_v;
};

#endif // CAMERA_H
