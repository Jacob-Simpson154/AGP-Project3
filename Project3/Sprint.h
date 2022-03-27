
class Sprint
{
public:
	Sprint();
	~Sprint();
	bool isSprinting;
	void Update(float);
	//float maxSprint = 100.0f;
	float sprintScale = 1.0f; //Sprint speed, can be changed
};