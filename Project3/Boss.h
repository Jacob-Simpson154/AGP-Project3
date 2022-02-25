#pragma once

class Boss
{
public: 
	Boss();
	~Boss();
	void Setup(int s, int h);
	bool DealDamage(int d);
private:
	int health = 100;
	int stage;
};