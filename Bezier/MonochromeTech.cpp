#include "MonochromeTech.h"

MonochromeTech::MonochromeTech(void)
:TechBase("Shaders\\MonochromeVS.txt","Shaders\\MonochromePS.txt")
{
}

MonochromeTech::~MonochromeTech(void)
{
}

void MonochromeTech::SetColor(Vector4 Value)
{
	GLint hUniform = glGetUniformLocationARB(m_hProgram, "color");
	glUniform4fARB(hUniform, Value.x, Value.y, Value.z, Value.w);
}
