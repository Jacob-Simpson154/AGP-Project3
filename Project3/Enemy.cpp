#include "Enemy.h"

Enemy::Enemy()
{
	stage = 0;
	std::mt19937 random_number_engine(time(0));
	std::uniform_int_distribution<int> distribution(-5, 5);

	posX = distribution(random_number_engine);
	posZ = distribution(random_number_engine);
	Sleep(1000);//? - Dirty method to prevent enemies from spawning on top of each other
}

Enemy::~Enemy()
{

}

//void Enemy::Setup(int s, int health, RenderItem* geo)
//{
//	stage = s;
//	hp = health;
//	geoObject = geo;
//}

bool Enemy::DealDamage(int damage)
{
	hp -= damage;
	if (hp <= 0)
		return true;
		//geoObject->material
	else return false;
}

void Enemy::FollowTarget(XMFLOAT3 targetPos)
{
	if (targetPos.x > posX) posX += 0.01f;
	else if (targetPos.x < posX) posX += -0.01f;
	else posX += 0;

	if (targetPos.y > posY) posY += 0.01f;
	else if (targetPos.y < posY) posY += -0.01f;
	else posY += 0;

	if (targetPos.z > posZ) posZ += 0.01f;
	else if (targetPos.z < posZ) posZ += -0.01f;
	else posZ += 0;

}