#pragma once

#include <sstream>
#include <math.h>

using namespace std;

#define SERVER_PORT 9999
#define PACKET_SIZE 4096
#define GRID_DIST 50

#define PI 3.1415926535897932f

enum class EPacketType
{
	SIGNUP,
	LOGIN,
	SPAWNPLAYER,
	SYNCHPLAYER,
	SYNCHZOMBIE,
	PLAYERINPUTACTION,
	PACKETTYPE_MAX,
	PLAYERDISCONNECTED
};

struct Vector3D
{
	float X, Y, Z;

	Vector3D() : X(0), Y(0), Z(0) {}
	Vector3D(float x, float y, float z) : X(x), Y(y), Z(z) {}

	friend istream& operator>>(istream& stream, Vector3D& loc)
	{
		stream >> loc.X >> loc.Y >> loc.Z;
		return stream;
	}

	friend ostream& operator<<(ostream& stream, Vector3D& loc)
	{
		stream << loc.X << "\n" << loc.Y << "\n" << loc.Z << "\n";
		return stream;
	}

	friend Vector3D operator-(const Vector3D& a, const Vector3D& b)
	{
		return { a.X - b.X, a.Y - b.Y, a.Z - b.Z };
	}
	friend Vector3D operator+(const Vector3D& a, const Vector3D& b)
	{
		return { a.X + b.X, a.Y + b.Y, a.Z + b.Z };
	}
	friend Vector3D operator*(const Vector3D& a, const float& b)
	{
		return { a.X * b, a.Y * b, a.Z * b };
	}

	float GetMagnitude()
	{
		return sqrt(X * X + Y * Y + Z * Z);
	}

	float GetDistance(const Vector3D& other)
	{
		return (other - *this).GetMagnitude();
	}

	Vector3D Normalize()
	{
		float magnitude = this->GetMagnitude();
		return { X / magnitude, Y / magnitude, Z / magnitude };
	}

	Vector3D& MoveTo(const Vector3D& direction, const float speed)
	{
		X += direction.X * speed;
		Y += direction.Y * speed;
		Z += direction.Z * speed;
		return *this;
	}

	Vector3D& Truncate()
	{
		X = trunc(X * 1000) / 1000;
		Y = trunc(Y * 1000) / 1000;
		Z = trunc(Z * 1000) / 1000;
		return *this;
	}
};

struct Rotator
{
	float pitch, yaw, roll;

	Rotator() : pitch(0), yaw(0), roll(0) {}
	Rotator(float p, float y, float r) : pitch(p), yaw(y), roll(r) {}

	float DegreesToRadian(float degreeVal)
	{
		return degreeVal* (PI / 180.f);
	}

	Vector3D GetForwardVector()
	{
		float rPitch = DegreesToRadian(pitch);
		float rYaw = DegreesToRadian(yaw);

		float SP = sin(rPitch);
		float CP = cos(rPitch);
		float SY = sin(rYaw);
		float CY = cos(rYaw);

		return Vector3D{ CP * CY, CP * SY, SP };
	}

	friend istream& operator>>(istream& stream, Rotator& rot)
	{
		stream >> rot.pitch >> rot.yaw >> rot.roll;
		return stream;
	}

	friend ostream& operator<<(ostream& stream, Rotator& rot)
	{
		stream << rot.pitch << "\n" << rot.yaw << "\n" << rot.roll << "\n";
		return stream;
	}
};

struct Pos
{
	int y, x;

	Pos(int _y, int _x) : y(_y), x(_x) {}
	Pos() : y(0), x(0) {}
	Pos operator+(Pos& _Pos)
	{
		Pos ret{ y + _Pos.y , x + _Pos.x };
		return ret;
	}
	bool operator<(const Pos& other) const
	{
		if (y != other.y)
			return y < other.y;
		return x < other.x;
	}
	bool operator!=(const Pos& other) const
	{
		return (y != other.y || x != other.x);
	}
	friend Pos operator+(const Pos& a, Pos& b)
	{
		Pos ret{ a.y + b.y, a.x + b.x };
		return ret;
	}
	friend bool operator==(const Pos& L, const Pos& R) {
		return (L.y == R.y && L.x == R.x);
	}
	float GetDistance(Pos& other)
	{
		return sqrt(pow(x - other.x, 2)) + sqrt(pow(y - other.y, 2));
	}
};