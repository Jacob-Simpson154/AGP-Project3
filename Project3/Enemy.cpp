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
	else return false;
}

//void Enemy::Movement()
//{
//	float posX = 0.0f;	float scaleX = 1.0f;
//	float posY = 1.0f;	float scaleY = 5.0f;
//	float posZ = 0.0f;	float scaleZ = 1.0f;
//
//
//	XMStoreFloat4x4(&geoObject->position, XMMatrixTranslation(posX, posY + tt, posZ));
//	geoObject->NumFramesDirty = gNumFrameResources;
//	tt += 0.01f;
//}