#pragma once

class Enemy
{
	public:
		Enemy();
		Enemy(int stage, int health);

		~Enemy();
		void DealDamage();
		virtual void Movement() = 0;

	private:
		int hp;
		//RenderItem struct //For keeping track of Geometry Object
};
