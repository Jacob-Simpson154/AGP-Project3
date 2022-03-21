

class Sprint
{
public:
	Sprint();
	~Sprint();
	bool isSprinting = false;
	void Update();
	float maxSprint = 200.0f;
	float sprintScale = 2.0f; //Sprint speed, can be changed
};