#include "Boss.h"


Boss::Boss()
{

}

Boss::~Boss()
{

}

void Boss::Setup(int s, int h, RenderItem* geo)
{
	stage = s;
	health = h;
	geoObject = geo;
}

void Boss::UpdateMovement()
{
	float posX = 0.0f;	float scaleX = 1.0f;
	float posY = 1.0f;	float scaleY = 5.0f;
	float posZ = 0.0f;	float scaleZ = 1.0f;


	XMStoreFloat4x4(&geoObject->position, XMMatrixTranslation(posX, posY + tt, posZ));
	geoObject->NumFramesDirty = gNumFrameResources;
	tt += 0.01f;
}

bool Boss::DealDamage(int damage)
{
	health -= damage;
	if (health <= 0)
		return true;
	else return false;
}

