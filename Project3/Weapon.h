#include <string>

class Weapon
{
public:
	Weapon();
	~Weapon();
	void Setup(std::string n, int d);
	int GetDamage();
	std::string GetName();

private:
	std::string name = "Pistol";
	int damage = 20;
};