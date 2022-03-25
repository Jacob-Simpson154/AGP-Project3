#include "Boss.h"


using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

Boss::Boss()
{
	hp = 100;
	posX = 0;
	posZ = 0;
	spawn = true;
}

Boss::~Boss()
{

}

void Boss::Setup(RenderItem* geo, Camera* player, BoundingBox* box)
{
	geoObject = geo;
	playerObject = player;
	hitbox = box;
}

void Boss::Movement() {
	XMStoreFloat4x4(&geoObject->position, XMMatrixScaling(10, 10, 10) * XMMatrixTranslation(posX, posY + tt, posZ));
	geoObject->NumFramesDirty = gNumFrameResources;
	tt += 0.0f;
	Pattern_1();
}

void Boss::Update()
{
	if (hp >= 75) Pattern_1();
	//else if (hp >= 50) Pattern_2();
}

bool Boss::SpawnReady()
{
	return spawn;
}

float Boss::GetSpawnRate()
{
	return spawnRate;
}

void Boss::Pattern_1()
{
	XMFLOAT3 playerPos = playerObject->GetPosition3f();
	XMFLOAT3 bossPos = XMFLOAT3(geoObject->position._41, geoObject->position._42, geoObject->position._43);

	//d = ((x2 - x1)^2 + (y2 - y1)^2 + (z2 - z1)^2) * 0.5    

	float playerToBoss = ((pow((bossPos.x - playerPos.x), 2)) + (pow((bossPos.y - playerPos.y),2)) + (pow((bossPos.z - playerPos.z), 2))) * 0.5f;
	
	if (playerToBoss <= 10)
	{
		//tt += 1;
		playerObject->SetPosition(XMFLOAT3(playerObject->GetPosition3f().x * 4, playerObject->GetPosition3f().y, playerObject->GetPosition3f().z * 4));
		//XMVectorLerp();
	}
}

