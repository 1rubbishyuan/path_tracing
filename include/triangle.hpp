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
				hit.set(t, this->material, this->normal, ray.pointAtParameter(t));
				// if (type == 0)
				// 	this->material->getDiffuseColor().print();
				// if (this->material->isTextured())
				// {
				// 	getUV((ray.pointAtParameter(t) - this->center) / radius, h.u, h.v);
				// 	this->material->changeColorAccordingTexture(h.getU(), h.getV(), h.getPoint());
				// }
				// cout << t << endl;
				// cout << h.u << " " << h.v << endl;
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
