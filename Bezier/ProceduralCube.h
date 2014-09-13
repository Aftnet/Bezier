#pragma once
#include "BaseGeometry.h"

class ProceduralCube : public BaseGeometry
{
public:
	ProceduralCube(GLfloat halfsize);
	~ProceduralCube(void);
	virtual void Render(void);
};
