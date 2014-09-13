#pragma once

#include "TechBase.h"

class PhongTech : public TechBase
{
public:
	PhongTech(void);
	~PhongTech(void);
	void SetTexture(GLuint hTex);
};

class PhongNoTexTech : public TechBase
{
public:
	PhongNoTexTech(void);
	~PhongNoTexTech(void);
};
