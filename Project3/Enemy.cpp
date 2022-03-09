#include "Enemy.h"

Enemy::Enemy()
{

}

Enemy::~Enemy()
{

}

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