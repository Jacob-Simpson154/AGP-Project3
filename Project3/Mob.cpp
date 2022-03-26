#include "Mob.h"

#include "Mob.h"

#include "FrameResource.h"
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

void Mob::Setup(Point* geo, Camera* player, BoundingBox* box)
{
	assert(box);
	// todo remove deadcode
	//geoObject = geo;
	pointObject = geo;
	playerObject = player;
	hitbox = box;

	hitbox->Center = XMFLOAT3(posX, posY, posZ);

	// todo remove deadcode
	//XMStoreFloat4x4(&geoObject->position, XMMatrixTranslation(posX, posY, posZ));
	//geoObject->NumFramesDirty = gNumFrameResources;
	pointObject->Pos = { posX, posY, posZ };
}

void Mob::Movement() 
{
	if (isActive)
	{
		assert(hitbox);
		assert(playerObject);
		FollowTarget(playerObject->GetPosition3f());
		pointObject->Pos = { posX, posY, posZ };

		// todo remove dead code
		//XMStoreFloat4x4(&geoObject->position, XMMatrixTranslation(posX, posY, posZ));
		//geoObject->NumFramesDirty = gNumFrameResources;
		hitbox->Center = XMFLOAT3(posX, posY, posZ);
	}
}

void Mob::Update()
{
	
}

