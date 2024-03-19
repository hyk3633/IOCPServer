#pragma once
#include "../Structs/Vector3D.h"
#include "../Structs/Rotator.h"

class Character
{
public:

	Character();

	~Character();

	Vector3D GetForwardVector();

	void SetLocation(const Vector3D& location);

	Vector3D GetLocation() const;

	void SetRotation(const Rotator& rotation);

	Rotator GetRotation() const;

protected:



private:



};