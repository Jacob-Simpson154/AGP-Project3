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

void Mob::Setup(RenderItem* geo, Camera* player, BoundingBox* box)
{
	geoObject = geo;
	playerObject = player;
	hitbox = box;

	hitbox->Center = XMFLOAT3(posX, posY, posZ);

	XMStoreFloat4x4(&geoObject->position, XMMatrixTranslation(posX, posY, posZ));
	geoObject->NumFramesDirty = gNumFrameResources;
}

void Mob::Movement() {
	FollowTarget(playerObject->GetPosition3f());
	XMStoreFloat4x4(&geoObject->position, XMMatrixTranslation(posX, posY, posZ));
	hitbox->Center = XMFLOAT3(posX, posY, posZ);
	geoObject->NumFramesDirty = gNumFrameResources;
}

void Mob::Update()
{
	
}

