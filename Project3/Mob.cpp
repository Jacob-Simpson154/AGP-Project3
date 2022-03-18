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
	FollowTarget(playerObject->GetPosition3f());
	XMStoreFloat4x4(&geoObject->position, XMMatrixTranslation(posX, posY, posZ));
	geoObject->NumFramesDirty = gNumFrameResources;
}

