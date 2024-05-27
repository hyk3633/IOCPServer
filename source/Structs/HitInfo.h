#pragma once
#include <fstream>
#include "../Structs/Vector3D.h"
#include "../Structs/Rotator.h"

using std::istream;
using std::ostream;

struct HitInfo
{
	HitInfo() {}
	HitInfo(int number, bool isP, Vector3D hitLoc, Rotator hitRot) :
		characterNumber(number), isPlayer(isP), hitLocation(hitLoc), hitRotation(hitRot) {}

	int characterNumber;

	bool isPlayer;

	Vector3D hitLocation;

	Rotator hitRotation;

	friend ostream& operator<<(ostream& stream, HitInfo& hitInfo)
	{
		stream << hitInfo.characterNumber << "\n";
		stream << hitInfo.isPlayer << "\n";
		stream << hitInfo.hitLocation;
		stream << hitInfo.hitRotation;
		return stream;
	}

	friend istream& operator>>(istream& stream, HitInfo& hitInfo)
	{
		stream >> hitInfo.characterNumber;
		stream >> hitInfo.isPlayer;
		stream >> hitInfo.hitLocation;
		stream >> hitInfo.hitRotation;
		return stream;
	}
};

