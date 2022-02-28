#include <string>

class Weapon
{
public:
	Weapon();
	~Weapon();
	void Setup(std::string n, int d, int a);
	int GetDamage();
	std::string GetName();
	void Reload();
	bool CanShoot();
	void Shoot();

private:
	std::string name = "Pistol";
	int ammo = 0;
	int maxAmmo = 0;
	int damage = 20;
};