#include "TimerInterval.h"

class ShieldPowerup
{
public:
	ShieldPowerup();
	ShieldPowerup(float);
	~ShieldPowerup();
	float Consume();
	void Update(float);
	bool hasBeenConsumed = false;
private:
	float maxTimeActive = 30.f;
	TimerInterval timer;
};
