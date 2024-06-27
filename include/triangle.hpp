#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include "aabb.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>
using namespace std;

// TODO: implement this class and add more fields as necessary,
class Triangle : public Object3D
{

public:
	Triangle() = delete;

	// a b c are three vertex positions of the triangle
	Triangle(const Vector3f &a, const Vector3f &b, const Vector3f &c, Material *m) : Object3D(m)
	{
		this->vertices[0] = a;
		this->vertices[1] = b;
		this->vertices[2] = c;
		this->normal = Vector3f::cross(a - b, a - c).normalized();
		AABB box1 = AABB(a, b);
		AABB box2 = AABB(a, c);
		this->bbox = AABB(box1, box2);
		this->Id = 15;
		// cout << this->Id << endl;
		num++;
	}

	bool intersect(const Ray &ray, Hit &hit, float tmin, int type) override
	{
		// std::cout << "i" << std::endl;
		Vector3f e1 = vertices[0] - vertices[1];
		Vector3f e2 = vertices[0] - vertices[2];
		Vector3f s = vertices[0] - ray.getOrigin();
		Vector3f unit_direction = ray.getDirection().normalized();
		Matrix3f m1 = Matrix3f(s, e1, e2, true);
		Matrix3f m2 = Matrix3f(unit_direction, s, e2, true);
		Matrix3f m3 = Matrix3f(unit_direction, e1, s, true);
		Matrix3f d = Matrix3f(unit_direction, e1, e2, true);
		float div = d.determinant();
		float t = m1.determinant() / div;
		float beta = m2.determinant() / div;
		float gama = m3.determinant() / div;
		if (t > 0 && 0 <= beta && beta <= 1 && 0 <= gama && gama <= 1 && beta + gama <= 1)
		{
			if (t < tmin)
			{
				return false;
			}
			if (t < hit.getT())
			{
				Vector3f hitNormal;
				Vector3f hitPoint = ray.pointAtParameter(t);
				if (type != 1145)
				{
					hitNormal = this->normal;
				}
				else
				{
					float lambda1, lambda2, lambda3;
					Vector3f v0 = vertices[1] - vertices[0];
					Vector3f v1 = vertices[2] - vertices[0];
					Vector3f v2 = hitPoint - vertices[0];
					float d00 = Vector3f::dot(v0, v0);
					float d01 = Vector3f::dot(v0, v1);
					float d11 = Vector3f::dot(v1, v1);
					float d20 = Vector3f::dot(v2, v0);
					float d21 = Vector3f::dot(v2, v1);
					float denom = d00 * d11 - d01 * d01;
					lambda2 = (d11 * d20 - d01 * d21) / denom;
					lambda3 = (d00 * d21 - d01 * d20) / denom;
					lambda1 = 1 - lambda2 - lambda3;
					hitNormal = (lambda1 * verticeNormals[0] + lambda2 * verticeNormals[1] + lambda3 * verticeNormals[2]).normalized();
				}
				hit.set(t, this->material, hitNormal, hitPoint);
				hit.setObjectId(this->Id);
				return true;
			}
			else
			{
				return false;
			}
		}
		return false;
	}
	Vector3f normal;
	Vector3f vertices[3];
	Vector3f verticeNormals[3];
	void setVerticeNormals(Vector3f n1, Vector3f n2, Vector3f n3)
	{
		this->verticeNormals[0] = n1;
		this->verticeNormals[1] = n2;
		this->verticeNormals[2] = n3;
	}
	void getUV(Vector3f point, float &u, float &v) override
	{
	}
	AABB boundingBox() override
	{
		return this->bbox;
	};

protected:
	AABB bbox;
};

#endif // TRIANGLE_H
