#pragma once

#include "TechBase.h"

class WaterTech : public TechBase
{
public:
	WaterTech(void);
	~WaterTech(void);
	void SetTexture(GLuint hTex);
	void SetTime(float time);
};
