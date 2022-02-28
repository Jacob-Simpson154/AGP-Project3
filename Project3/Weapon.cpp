#include "Weapon.h"

Weapon::Weapon()
{

}

Weapon::~Weapon()
{

}

void Weapon::Setup(std::string n, int d, int a)
{
	name = n;
	damage = d;
	maxAmmo = a;
	ammo = maxAmmo;
}

int Weapon::GetDamage()
{
	return damage;
}

std::string Weapon::GetName()
{
	return name;
}

void Weapon::Reload()
{
	ammo = maxAmmo;
}

bool Weapon::CanShoot()
{
	if (ammo > 0)
		return true;
	else return false;
}

void Weapon::Shoot()
{
	ammo--;
}