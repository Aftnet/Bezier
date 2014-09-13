#pragma once

#include "ProceduralCube.h"
#include "SkyTech.h"
#include "Texture.h"

class SkyBox : public ProceduralCube
{
public:
	SkyBox(void);
	~SkyBox(void);
	void Render(void);

private:
	SkyTech m_Technique;
	GLuint m_hTex[6];
};
