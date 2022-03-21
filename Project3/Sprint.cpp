#include "Sprint.h"

Sprint::Sprint()
{

}

Sprint::~Sprint()
{

}

void Sprint::Update()
{
	if (isSprinting == true)
	{
		maxSprint--;
	}
	else if (isSprinting == false && maxSprint > 200.0f)
	{
		maxSprint++;
	}
}