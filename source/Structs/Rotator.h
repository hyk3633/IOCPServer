#pragma once

#include <sstream>
#include <cmath>

using namespace std;

struct Vector3D;

struct Rotator
{
	float pitch, yaw, roll;

	Rotator();
	Rotator(float p, float y, float r);

	Vector3D GetForwardVector();

	friend istream& operator>>(istream& stream, Rotator& rot);

	friend ostream& operator<<(ostream& stream, Rotator& rot);

};