#include "Weapon.h"

Weapon::Weapon()
{

}

Weapon::~Weapon()
{

}

void Weapon::Setup(std::string n, int d)
{
	name = n;
	damage = d;
}

int Weapon::GetDamage()
{
	return damage;
}

std::string Weapon::GetName()
{
	return name;
}