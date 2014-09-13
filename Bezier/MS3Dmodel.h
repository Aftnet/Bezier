#pragma once

#include "BaseGeometry.h"
#include "MS3Dff.h"
#include <iostream>
#include <fstream>
#include "Libs\SOIL.h"
#include "PhongTechs.h"

class MS3Dmodel : public BaseGeometry
{
public:
	MS3Dmodel(char * filename);
	~MS3Dmodel(void);
	void Render(void);

protected:
	GLuint *m_hTex;

private:
	PhongTech m_Tech;
	PhongNoTexTech m_TechNoTex;
};
