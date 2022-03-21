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
	tt += 0.0f;
	Pattern_1();
}

void Boss::Pattern_1()
{
	XMFLOAT3 playerPos = playerObject->GetPosition3f();
	XMFLOAT3 bossPos = XMFLOAT3(geoObject->position._41, geoObject->position._42, geoObject->position._43);

	//d = ((x2 - x1)^2 + (y2 - y1)^2 + (z2 - z1)^2) * 0.5      

	float playerToBoss = (((int)(bossPos.x - playerPos.x) ^ 2) + ((int)(bossPos.y - playerPos.y) ^ 2) + ((int)(bossPos.z - playerPos.z) ^ 2)) * 0.5f;
	
	if (playerToBoss <= 2) tt += 1;
}

