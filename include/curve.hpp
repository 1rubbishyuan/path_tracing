#ifndef CURVE_HPP
#define CURVE_HPP

#include "object3d.hpp"
#include <vecmath.h>
#include <vector>
#include <utility>
#include <iostream>
#include <algorithm>
using namespace std;
struct CurvePoint
{
    Vector3f V; // Vertex
    Vector3f T; // Tangent  (unit)
};

// TODO (PA2): Implement Bernstein class to compute spline basis function.
//       You may refer to the python-script for implementation.
class Bernstein_BE
{
public:
    Bernstein_BE(std::vector<Vector3f> points) : controls(points)
    {
        n = controls.size();
        k = n - 1;
        // cout << k << endl;
    }

    CurvePoint evaluate(double t)
    {
        double **B = new double *[n];
        for (int i = 0; i < n; i++)
        {
            B[i] = new double[k + 1];
        }
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < k; j++)
            {
                B[i][j] = 0;
            }
        }
        for (int j = 1; j <= k; j++)
        {
            for (int i = 0; i < n; i++)
            {
                // cout << i << "  " << j << endl;
                if (j == 1)
                {
                    // cout << "sad" << endl;
                    B[i][j] = (1 - t) * (i == 0) + t * (i == 1);
                    // cout << "ss" << endl;
                }
                else if (i == 0)
                {
                    B[i][j] = (1 - t) * B[i][j - 1];
                }
                else
                {
                    B[i][j] = (1 - t) * B[i][j - 1] + t * B[i - 1][j - 1];
                }
            }
        }
        Vector3f V = Vector3f(0, 0, 0);
        Vector3f T = Vector3f(0, 0, 0);
        for (int i = 0; i < n; i++)
        {
            V = V + controls[i] * B[i][k];
            if (i != 0)
                T = T + controls[i] * (n * (B[i - 1][k - 1] - B[i][k - 1]));
            else
                T = T + controls[i] * (n * (0 - B[i][k - 1]));
        }
        CurvePoint ans;
        ans.V = V;
        ans.T = T;
        for (int i = 0; i < n; i++)
        {
            delete[] B[i];
        }
        delete[] B;
        return ans;
    }

private:
    std::vector<Vector3f> controls;
    int n;
    int k;
};

class Bernstein_BS
{
public:
    Bernstein_BS(std::vector<Vector3f> points) : controls(points)
    {
        n = controls.size();
        k = 3;
    }

    CurvePoint evaluate(double t)
    {
        double **B = new double *[n + k + 2];
        for (int i = 0; i < n + k + 2; i++)
        {
            B[i] = new double[4];
        }
        double num = n + k + 1;
        for (int i = 0; i < n + k + 2; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                B[i][j] = 0;
            }
        }
        for (int j = 0; j <= k; j++)
        {
            for (int i = 0; i < n + k + 1; i++)
            {
                // cout << i << "  " << j << "  ";
                if (j == 0)
                {
                    B[i][j] = (t >= double(i) / num && t < double(i + 1) / num) ? 1.0 : 0.0;
                    // cout << t << "  " << float(i) / num << "  " << float(i + 1) / num << "  " << B[i][j] << endl;
                }
                else
                {
                    B[i][j] = (double(t - double(i) / num) / (double(i + j) / num - double(i) / num)) * B[i][j - 1] +
                              (double(-t + double(i + j + 1) / num) / (double(i + j + 1) / num - double(i + 1) / num)) * B[i + 1][j - 1];
                    // cout << B[i][j] << endl;
                }
            }
        }
        Vector3f V = Vector3f(0, 0, 0);
        Vector3f T = Vector3f(0, 0, 0);
        for (int i = 0; i < n; i++)
        {
            V = V + controls[i] * B[i][3];
            T = T + controls[i] * (3 * (B[i][2] / ((float(i + 2) / num) - float(i) / num) - B[i + 1][2] / ((float(i + 2 + 1) / num) - float(i + 1) / num)));
        }
        CurvePoint ans;
        ans.V = V;
        ans.T = T;
        for (int i = 0; i < n; i++)
        {
            delete[] B[i];
        }
        delete[] B;
        return ans;
    }

private:
    std::vector<Vector3f> controls;
    int n;
    int k;
};
// The CurvePoint object stores information about a point on a curve
// after it has been tesselated: the vertex (V) and the tangent (T)
// It is the responsiblility of functions that create these objects to fill in all the data.

class Curve : public Object3D
{
protected:
public:
    std::vector<Vector3f> controls;
    explicit Curve(std::vector<Vector3f> points) : controls(std::move(points)) {}

    bool intersect(const Ray &r, Hit &h, float tmin, int type) override
    {
        return false;
    }

    std::vector<Vector3f> &getControls()
    {
        return controls;
    }
    virtual CurvePoint evaluate(float u) { return CurvePoint(); }
    virtual void discretize(int resolution, std::vector<CurvePoint> &data) = 0;
};

class BezierCurve : public Curve
{
public:
    explicit BezierCurve(const std::vector<Vector3f> &points) : Curve(points)
    {
        if (points.size() < 4 || points.size() % 3 != 1)
        {
            printf("Number of control points of BezierCurve must be 3n+1!\n");
            exit(0);
        }
    }

    void discretize(int resolution, std::vector<CurvePoint> &data) override
    {
        data.clear();
        for (int i = 0; i <= resolution; i++)
        {
            data.push_back(b.evaluate(float(i) / resolution));
        }
    }
    CurvePoint evaluate(float u) override
    {
        return b.evaluate(u);
    }

protected:
    Bernstein_BE b = Bernstein_BE(getControls());
};

class BsplineCurve : public Curve
{
public:
    BsplineCurve(const std::vector<Vector3f> &points) : Curve(points)
    {
        if (points.size() < 4)
        {
            printf("Number of control points of BspineCurve must be more than 4!\n");
            exit(0);
        }
    }

    void discretize(int resolution, std::vector<CurvePoint> &data) override
    {
        // resolution = 100;
        data.clear();
        for (int i = 0; i <= resolution * (getControls().size() + 3 + 1); i++)
        {
            if (i >= resolution * 3 && i <= resolution * getControls().size())
                data.push_back(b.evaluate(float(i) / (resolution * (getControls().size() + 3 + 1))));
        }
    }
    CurvePoint evaluate(float u) override
    {
        return b.evaluate(u);
    }

protected:
    Bernstein_BS b = Bernstein_BS(getControls());
};

#endif
