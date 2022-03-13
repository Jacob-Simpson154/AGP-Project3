#include "Boss.h"


using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

Boss::Boss()
{
	hp = 100;
	posX = 0;
	posZ = 0;
}

Boss::~Boss()
{

}

void Boss::Setup(RenderItem* geo, Camera* player)
{
	geoObject = geo;
	playerObject = player;
}

void Boss::Movement() {
	XMStoreFloat4x4(&geoObject->position, XMMatrixTranslation(posX, posY + tt, posZ));
	geoObject->NumFramesDirty = gNumFrameResources;
	tt += 0.01f;
}

//void Boss::UpdateMovement()
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

//bool Enemy::DealDamage(int damage)
//{
//	health -= damage;
//	if (health <= 0)
//		return true;
//	else return false;
//}

