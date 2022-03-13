#include "Mob.h"

#include "Mob.h"


using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

Mob::Mob()
{
	hp = 5;
}

Mob::~Mob()
{

}

void Mob::Setup(RenderItem* geo, Camera* player)
{
	geoObject = geo;
	playerObject = player;

	XMStoreFloat4x4(&geoObject->position, XMMatrixTranslation(posX, posY, posZ));
	geoObject->NumFramesDirty = gNumFrameResources;
}

void Mob::Movement() {
	XMFLOAT3  playerPos = playerObject->GetPosition3f();
	if (playerPos.x > posX) posX += 0.01f;
	else if (playerPos.x < posX) posX += -0.01f;
	else posX += 0;

	if (playerPos.y > posY) posY += 0.01f;
	else if (playerPos.y < posY) posY += -0.01f;
	else posY += 0;

	if (playerPos.z > posZ) posZ += 0.01f;
	else if (playerPos.z < posZ) posZ += -0.01f;
	else posZ += 0;

	XMStoreFloat4x4(&geoObject->position, XMMatrixTranslation(posX, posY, posZ));
	geoObject->NumFramesDirty = gNumFrameResources;
	//tt += 0.01f;
}

//void Mob::UpdateMovement()
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

