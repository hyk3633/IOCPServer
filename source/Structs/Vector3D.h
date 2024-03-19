#pragma once

#include <sstream>
#include <cmath>

using namespace std;

struct Rotator;

struct Vector3D
{
	float X, Y, Z;

	Vector3D();
	Vector3D(float x, float y, float z);

	float GetMagnitude();

	float GetDistance(const Vector3D& other);

	Vector3D Normalize();

	Vector3D& MoveTo(const Vector3D& direction, const float speed);

	Vector3D& Truncate();

	Rotator Rotation();

	friend istream& operator>>(istream& stream, Vector3D& loc);

	friend ostream& operator<<(ostream& stream, Vector3D& loc);

	friend Vector3D operator-(const Vector3D& a, const Vector3D& b);
	
	friend Vector3D operator+(const Vector3D& a, const Vector3D& b);

	friend Vector3D operator*(const Vector3D& a, const float& b);

	friend double DotProduct(const Vector3D& a, const Vector3D& b);

};