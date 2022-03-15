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


struct ObjData
{
	std::string filename;
	std::string geoName;
	std::string subGeoName;
	DirectX::SimpleMath::Vector3 position;
};

namespace gc
{
	const size_t NUM_OBSTACLE = 4;
	const size_t NUM_UI_SPRITES = 7;

	

	const ObstacleData OBSTACLE_DATA[NUM_OBSTACLE]
	{
		"Data/Models/obst_02_10_06.obj", "obst00Geo", "obst00",{02.0f,10.0f,06.0f},
		"Data/Models/obst_06_10_06.obj", "obst01Geo", "obst01",{06.0f,10.0f,06.0f},
		"Data/Models/obst_08_10_08.obj", "obst02Geo", "obst02",{08.0f,10.0f,08.0f},
		"Data/Models/obst_10_17_06.obj", "obst03Geo", "obst03",{10.0f,17.0f,06.0f}
	};

	enum {
		CHAR_0 = 0, CHAR_1, CHAR_2, CHAR_3, CHAR_4, CHAR_5,
		CHAR_6, CHAR_7, CHAR_8, CHAR_9, CHAR_SPC, CHAR_PRD,
		CHAR_AMM0, CHAR_PTS, CHAR_SEC, CHAR_UNUSED1, CHAR_UNUSED2,
		CHAR_UNUSED3, CHAR_UNUSED4, CHAR_UNUSED5,
		CHAR_COUNT,

		WORD_HEALTH = 0,
		WORD_AMMO,
		WORD_MONSTER,
		WORD_TIMELEFT,
		WORD_COUNT
	};

	const ObjData UI_SPRITE_DATA[NUM_UI_SPRITES]
	{
		"Data/Models/UI_Controls.obj", "uiControlGeo", "uiControl",			{00.60f,-0.70f,01.00f},
		"Data/Models/UI_FinLose.obj", "uiFinLoseGeo", "uiFinLose",			{01.00f,01.00f,00.00f}, 
		"Data/Models/UI_FinWin.obj", "uiFinWinGeo", "uiFinWin",				{01.00f,00.80f,00.00f},
		"Data/Models/UI_Health.obj", "uiHealthGeo", "uiHealth",				{00.00f,00.90f,00.00f}, // todo make red
		"Data/Models/UI_Health.obj", "uiHealth2Geo", "uiHealth2",			{00.00f,00.90f,00.00f}, // todo make green, scale based on remaining health
		"Data/Models/UI_Objective.obj", "uiObjectiveGeo", "uiObjective",	{-1.00f,01.00f,00.00f}, 
		"Data/Models/UI_CrossHair.obj", "uiCrossHairGeo", "uiCrossHair",	{00.00f,00.00f,00.00f}
	};

	// characters available in texture 
	const int UI_NUM_CHAR = 20;
	// words available in texture
	const int UI_NUM_WORD = 20;

	// number of chars render items
	const size_t UI_NUM_RITEM_CHAR = 32;
	// number of words render items
	const size_t UI_NUM_RITEM_WORD = 22;
	
	// increment distance between chars
	const float UI_CHAR_SPACING = 0.07f;

	// uv increment in y texture axis
	const float UI_CHAR_INC = 1.0f / (float)UI_NUM_CHAR;
	const float UI_WORD_INC = 1.0f / (float)UI_NUM_WORD;

	const ObjData UI_CHAR = { "Data/Models/UI_Char.obj", "uiCharGeo", "uiChar",		{-0.50f,00.50f,00.0f} };
	const ObjData UI_WORD = { "Data/Models/UI_Word.obj", "uiWordGeo", "uiWord",		{-0.80f,-0.90f,00.0f} };

}
