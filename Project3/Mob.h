#pragma once
#include "Enemy.h"
class Point;

class Mob : public Enemy
{
public:
	Mob();
	~Mob();
	void Setup(Point* geo, Camera* player, BoundingBox* box);
	void Enemy::Movement(float dt);
	void Enemy::Update();
	//void Enemy::Setup(int stage, int health, RenderItem* geo);
private:
	Point* pointObject = nullptr;
};