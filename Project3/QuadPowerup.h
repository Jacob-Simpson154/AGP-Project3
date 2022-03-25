#include "TimerInterval.h"

class QuadPowerup
{
public:
	QuadPowerup();
	QuadPowerup(float, float);
	~QuadPowerup();
	float Consume();
	void Update(float);
	bool hasBeenConsumed = false;
private:
	float maxTimeActive = 30.f;
	float damageMultiplier = 3.f;
	TimerInterval timer;
};
