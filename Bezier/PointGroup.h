#pragma once

#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>
#include <math.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "glext.h"
#include "vector.h"
#include "MonochromeTech.h"

class PointGroup
{
public:

	enum eUsage{STATIC, DYNAMIC};
	enum eDrawStyle{POINTS, LINE, LOOP};

	PointGroup(eUsage Usage, eDrawStyle DrawStyle);
	~PointGroup(void);
	eDrawStyle GetDrawStyle(void) const;
	void SetDrawStyle(eDrawStyle Value);
	GLuint GetNumPoints(void) const;
	void SetNumPoints(GLuint Value);
	const Vector4& GetColor(void) const;
	void SetColor(const Vector4& Value);
	void SetColor(float r, float g, float b, float a);
	GLfloat GetSize(void) const;
	void SetSize(GLfloat Value);
	const Vector4& GetPoint(GLuint index) const;
	bool SetPoint(GLuint index, const Vector4& Value);
	void GenerateVBO(void);
	void Render(void);
	void RenderSection(GLuint StartIndex, GLuint PointsToRender);


protected:
	eUsage m_eUsage;
	eDrawStyle m_eDrawStyle;

	Vector4 m_vColor;
	GLfloat m_fSize;

	GLuint m_uiNumPoints;
	Vector4 * m_pPoints;
	GLuint m_hVBOPointBuffer;

	MonochromeTech m_Technique;
	

	//OpenGL extensions function pointers
	PFNGLGENBUFFERSARBPROC glGenBuffersARB;					// VBO Name Generation Procedure
	PFNGLBINDBUFFERARBPROC glBindBufferARB;					// VBO Bind Procedure
	PFNGLBUFFERDATAARBPROC glBufferDataARB;					// VBO Data Loading Procedure
	PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;			// VBO Deletion Procedure
};
