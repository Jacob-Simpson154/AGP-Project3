#include "TimerInterval.h"

class HealthBox
{
public:
	HealthBox();
	HealthBox(float);
	~HealthBox();
	float Consume();
	void Update(float);
	bool hasBeenConsumed = false;
private:
	float healthPercentageToRestore = 50.0f;
	TimerInterval timer;
};