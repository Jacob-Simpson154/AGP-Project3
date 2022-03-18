#pragma once
#include "RenderItemStruct.h"
#include "Common/d3dApp.h"
#include "Common/MathHelper.h"
#include "Common/Camera.h"
#include <random>
#include <time.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

class Enemy
{
	public:
		Enemy();
		~Enemy();
		bool DealDamage(int damage);
		void FollowTarget(XMFLOAT3 targetPos);
		/*void Setup(int stage, int health, RenderItem* geo);*/
		virtual void Movement() = 0;

	//private:
		int hp;
		int stage;
		RenderItem* geoObject;
		Camera* playerObject;
		float tX = 0, tY, tZ, tt = 0;

		float posX = 0.0f;
		float posY = 5.0f;
		float posZ = 0.0f;
};
