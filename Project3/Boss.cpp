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

