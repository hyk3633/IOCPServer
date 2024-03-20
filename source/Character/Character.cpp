#include "Character.h"

using namespace std;

Character::Character(const int num) : number(num)
{

}

Character::~Character()
{

}

Vector3D Character::GetForwardVector()
{
	return rotation.GetForwardVector();
}

void Character::SetLocation(const Vector3D& loc)
{
	location = loc;
}

void Character::SetRotation(const Rotator& rot)
{
	rotation = rot;
}

void Character::SerializeLocation(ostream& stream)
{
	stream << location;
}

void Character::DeserializeLocation(istream& stream)
{
	stream >> location;
}

void Character::SerializeRotation(ostream& stream)
{
	stream << rotation;
}

void Character::DeserializeRotation(istream& stream)
{
	stream >> rotation;
}
