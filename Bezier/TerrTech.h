#pragma once

#include "TechBase.h"

class TerrTech: public TechBase
{
public:
	TerrTech();
	~TerrTech();
	void SetTexture0(GLuint hTex);
	void SetTexture1(GLuint hTex);
	void SetTexture2(GLuint hTex);
	void SetTexture3(GLuint hTex);
	void SetMedianHeight(GLfloat value);
};