#include "TimerInterval.h"

class AmmoBox
{
public:
	AmmoBox();
	AmmoBox(float);
	~AmmoBox();
	float Consume();
	void Update(float);
	bool hasBeenConsumed = false;
private:
	float ammoPercentageToRestore = 50.0f;
	TimerInterval timer;
};