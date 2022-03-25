#pragma once
#include "Enemy.h"

class Mob : public Enemy
{
public:
	Mob();
	~Mob();
	void Setup(RenderItem* geo, Camera* player, BoundingBox* box);
	void Enemy::Movement();
	void Enemy::Update();
	//void Enemy::Setup(int stage, int health, RenderItem* geo);
private:

};