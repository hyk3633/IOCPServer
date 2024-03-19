#include "Rotator.h"
#include "Vector3D.h"
#include "../Math/Math.h"

Rotator::Rotator() : pitch(0), yaw(0), roll(0) 
{

}

Rotator::Rotator(float p, float y, float r) : pitch(p), yaw(y), roll(r)
{

}

Vector3D Rotator::GetForwardVector()
{
	float rPitch = DegreesToRadian(pitch);
	float rYaw = DegreesToRadian(yaw);

	float sP = sin(rPitch);
	float cP = cos(rPitch);
	float sY = sin(rYaw);
	float cY = cos(rYaw);

	return Vector3D{ cP * cY, cP * sY, sP };
}

istream& operator>>(istream& stream, Rotator& rot)
{
	stream >> rot.pitch >> rot.yaw >> rot.roll;
	return stream;
}

ostream& operator<<(ostream& stream, Rotator& rot)
{
	stream << rot.pitch << "\n" << rot.yaw << "\n" << rot.roll << "\n";
	return stream;
}