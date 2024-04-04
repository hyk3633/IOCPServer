#include "Vector3D.h"
#include "Rotator.h"
#include "../Math/Math.h"

Vector3D::Vector3D() : X(0), Y(0), Z(0) 
{

}

Vector3D::Vector3D(float x, float y, float z) : X(x), Y(y), Z(z)
{

}

float Vector3D::GetMagnitude()
{
	return sqrt(X * X + Y * Y + Z * Z);
}

float Vector3D::GetDistance(const Vector3D& other)
{
	return (other - *this).GetMagnitude();
}

Vector3D Vector3D::Normalize()
{
	float magnitude = this->GetMagnitude();
	return { X / magnitude, Y / magnitude, Z / magnitude };
}

Vector3D& Vector3D::MoveTo(const Vector3D& direction, const float speed)
{
	X += direction.X * speed;
	Y += direction.Y * speed;
	Z += direction.Z * speed;
	return *this;
}

Vector3D& Vector3D::Truncate()
{
	X = trunc(X * 1000) * 0.001f;
	Y = trunc(Y * 1000) * 0.001f;
	Z = trunc(Z * 1000) * 0.001f;
	return *this;
}

Rotator Vector3D::Rotation()
{
	Rotator rotator;
	rotator.yaw = RadiansToDegrees(atan2(Y, X));
	rotator.pitch = RadiansToDegrees(atan2(Z, sqrt(X * X + Y * Y)));
	rotator.roll = 0;
	return rotator;
}

istream& operator>>(istream& stream, Vector3D& loc)
{
	stream >> loc.X >> loc.Y >> loc.Z;
	return stream;
}

ostream& operator<<(ostream& stream, Vector3D& loc)
{
	stream << loc.X << "\n" << loc.Y << "\n" << loc.Z << "\n";
	return stream;
}

Vector3D operator-(const Vector3D& a, const Vector3D& b)
{
	return { a.X - b.X, a.Y - b.Y, a.Z - b.Z };
}

Vector3D operator+(const Vector3D& a, const Vector3D& b)
{
	return { a.X + b.X, a.Y + b.Y, a.Z + b.Z };
}

Vector3D operator*(const Vector3D& a, const float& b)
{
	return { a.X * b, a.Y * b, a.Z * b };
}

double DotProduct(const Vector3D& a, const Vector3D& b)
{
	return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
}
