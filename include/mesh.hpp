#ifndef MESH_H
#define MESH_H

#include <vector>
#include "object3d.hpp"
#include "triangle.hpp"
#include "Vector2f.h"
#include "Vector3f.h"
#include "bvhNode.hpp"
#include "aabb.hpp"
class Mesh : public Object3D
{

public:
    Mesh(const char *filename, Material *m);

    struct TriangleIndex
    {
        TriangleIndex()
        {
            x[0] = 0;
            x[1] = 0;
            x[2] = 0;
        }
        int &operator[](const int i) { return x[i]; }
        // By Computer Graphics convention, counterclockwise winding is front face
        int x[3]{};
    };

    std::vector<Vector3f> v;
    std::vector<TriangleIndex> t;
    std::vector<Object3D *> triangles;
    std::vector<Vector3f> n;
    std::vector<Vector3f> verticeNormals;
    bool intersect(const Ray &r, Hit &h, float tmin, int type) override;
    BvhNode *meshRoot;
    AABB bbox;
    AABB boundingBox() override { return this->bbox; };

private:
    // Normal can be used for light estimation
    void computeNormal(int sum);
};

#endif
