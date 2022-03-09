#pragma once

#include <string>
#include <SimpleMath.h>

struct ObstacleData
{
	std::string filename;
	std::string geoName;
	std::string subGeoName;
	DirectX::SimpleMath::Vector3 boundingBox;
};

namespace gc
{
	const size_t NUM_OBSTACLE = 4;


	const ObstacleData OBSTACLE_DATA[NUM_OBSTACLE]
	{
		"Data/Models/obst_02_10_06.obj", "obst00Geo", "obst00",{02.0f,10.0f,06.0f},
		"Data/Models/obst_06_10_06.obj", "obst01Geo", "obst01",{06.0f,10.0f,06.0f},
		"Data/Models/obst_08_10_08.obj", "obst02Geo", "obst02",{08.0f,10.0f,08.0f},
		"Data/Models/obst_10_17_06.obj", "obst03Geo", "obst03",{10.0f,17.0f,06.0f}
	};

}
