#pragma once
#include "Enemy.h"

class Boss : public Enemy
{
public: 
	Boss();
	~Boss();
	void Setup(RenderItem* geo, Camera* player, BoundingBox* box);
	void Enemy::Movement();
	void Enemy::Update();
	void Pattern_1();
	float GetSpawnRate();
	bool SpawnReady();
	//void Enemy::Setup(int stage, int health, RenderItem* geo);
	/*void UpdateMovement();*/
private:
	/*int health = 100;
	int stage;
	RenderItem* geoObject;
	float tt = 0;*/
	bool spawn;
	float spawnRate = 1;
};