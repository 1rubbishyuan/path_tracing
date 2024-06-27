#include "mesh.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <utility>
#include <sstream>
#include <vector>
#include "bvhNode.hpp"
bool Mesh::intersect(const Ray &r, Hit &h, float tmin, int type)
{

    // Optional: Change this brute force method into a faster one.
    // std::cout << "lll" << endl;
    bool result = false;
    // for (int triId = 0; triId < (int)t.size(); ++triId)
    // {
    //     // TriangleIndex &triIndex = t[triId];
    //     // Triangle triangle(v[triIndex[0]],
    //     //                   v[triIndex[1]], v[triIndex[2]], material);
    //     // triangle.normal = n[triId];
    //     result |= triangles[triId]->intersect(r, h, tmin, type);
    // }
    result = this->meshRoot->intersect(r, h, tmin, 1145);
    if (result)
    {
        // cout << this->Id << endl;
        h.setObjectId(this->Id);
    }
    return result;
}

Mesh::Mesh(const char *filename, Material *material) : Object3D(material)
{

    // Optional: Use tiny obj loader to replace this simple one.
    this->Id = num;
    num++;
    std::ifstream f;
    f.open(filename);
    if (!f.is_open())
    {
        std::cout << "Cannot open " << filename << "\n";
        return;
    }
    std::string line;
    std::string vTok("v");
    std::string fTok("f");
    std::string texTok("vt");
    char bslash = '/', space = ' ';
    std::string tok;
    int texID;
    int sum = 0;
    while (true)
    {
        std::getline(f, line);
        if (f.eof())
        {
            break;
        }
        if (line.size() < 3)
        {
            continue;
        }
        if (line.at(0) == '#')
        {
            continue;
        }
        std::stringstream ss(line);
        ss >> tok;
        if (tok == vTok)
        {
            Vector3f vec;
            ss >> vec[0] >> vec[1] >> vec[2];
            v.push_back(vec);
        }
        else if (tok == fTok)
        {
            if (line.find(bslash) != std::string::npos)
            {
                std::replace(line.begin(), line.end(), bslash, space);
                std::stringstream facess(line);
                TriangleIndex trig;
                facess >> tok;
                for (int ii = 0; ii < 3; ii++)
                {
                    facess >> trig[ii] >> texID;
                    trig[ii]--;
                }
                t.push_back(trig);
            }
            else
            {
                TriangleIndex trig;
                for (int ii = 0; ii < 3; ii++)
                {
                    ss >> trig[ii];
                    sum = std::max(sum, trig[ii]);
                    trig[ii]--;
                }
                t.push_back(trig);
            }
        }
        else if (tok == texTok)
        {
            Vector2f texcoord;
            ss >> texcoord[0];
            ss >> texcoord[1];
        }
    }
    computeNormal(sum);
    // cout << sum << endl;
    f.close();
    for (int triId = 0; triId < (int)t.size(); ++triId)
    {
        TriangleIndex &triIndex = t[triId];
        Triangle *triangle = new Triangle(v[triIndex[0]],
                                          v[triIndex[1]], v[triIndex[2]], material);
        triangle->normal = n[triId];
        triangle->setVerticeNormals(verticeNormals[triIndex[0]], verticeNormals[triIndex[1]], verticeNormals[triIndex[2]]);
        this->triangles.push_back(triangle);
        this->bbox = AABB(this->bbox, triangle->boundingBox());
    }
    this->meshRoot = new BvhNode(triangles, 0, t.size() - 1);
}

void Mesh::computeNormal(int sum)
{
    n.resize(t.size());
    for (int triId = 0; triId < (int)t.size(); ++triId)
    {
        TriangleIndex &triIndex = t[triId];
        Vector3f a = v[triIndex[1]] - v[triIndex[0]];
        Vector3f b = v[triIndex[2]] - v[triIndex[0]];
        b = Vector3f::cross(a, b);
        n[triId] = b / b.length();
    }
    std::vector<std::vector<Vector3f>> normalsOfVertice;
    std::vector<std::vector<float>> squiresOfVertice;
    normalsOfVertice.resize(sum);
    squiresOfVertice.resize(sum);
    this->verticeNormals.resize(sum);
    for (int i = 0; i < t.size(); i++)
    {
        TriangleIndex &triIndex = t[i];
        Vector3f ab = v[triIndex[1]] - v[triIndex[0]];
        Vector3f ac = v[triIndex[2]] - v[triIndex[0]];
        float squire = Vector3f::cross(ab, ac).length() / 2;
        for (int j = 0; j < 3; j++)
        {
            // 这个三角形的三个顶点的编号,如此一来也可以在这里计算面积
            normalsOfVertice[triIndex[j]].push_back(n[i]);
            squiresOfVertice[triIndex[j]].push_back(squire);
        }
    }
    for (int i = 0; i < sum; i++)
    {
        Vector3f verticeNormal;
        float denom = 0;
        for (int j = 0; j < normalsOfVertice[i].size(); j++)
        {
            verticeNormal += squiresOfVertice[i][j] * normalsOfVertice[i][j];
            denom += squiresOfVertice[i][j];
        }
        this->verticeNormals[i] = verticeNormal / denom;
    }
}
