#pragma once

#include "TechBase.h"
#include "Vector.h"

class MonochromeTech: public TechBase
{
public:
	MonochromeTech(void);
	~MonochromeTech(void);
	void SetColor(Vector4 value);
};
