#include "Boss.h"

Boss::Boss()
{

}

Boss::~Boss()
{

}

void Boss::Setup(int s, int h)
{
	stage = s;
	health = h;
}

bool Boss::DealDamage(int damage)
{
	health -= damage;
	if (health <= 0)
		return true;
	else return false;
}

