#pragma once
#include "../Structs/Vector3D.h"
#include "../Structs/Rotator.h"
#include <memory>

class Character
{
public:

	Character() = default;

	Character(const int num);

	virtual ~Character();

	inline int GetNumber() const { return number; }

	Vector3D GetForwardVector();

	void SetLocation(const Vector3D& loc);

	inline Vector3D GetLocation() const { return location; }

	void SetRotation(const Rotator& rot);

	inline Rotator GetRotation() const { return rotation; }

	void SerializeLocation(std::ostream& stream);

	void DeserializeLocation(std::istream& stream);

	void SerializeRotation(std::ostream& stream);

	void DeserializeRotation(std::istream& stream);

	virtual void SerializeData(std::ostream& stream) = 0;

	virtual void TakeDamage(const float damage) = 0;

protected:



private:

	int number;

	Vector3D location;

	Rotator rotation;

};