#pragma once
#include "RenderItemStruct.h"

class Enemy
{
	public:
		Enemy();
		Enemy(int stage, int health);

		~Enemy();
		bool DealDamage(int damage);
		virtual void Movement() = 0;

	private:
		int hp;
		RenderItem* geoObject;
		float tt = 0;
};
