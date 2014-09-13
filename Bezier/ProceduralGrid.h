#pragma once

#include "BaseGeometry.h"
#include "PerlinNoise.h"

class ProceduralGrid : public BaseGeometry
{
public:
	ProceduralGrid(GLuint width, GLuint height, GLfloat tilesize, GLuint htexturetiles, GLuint vtexturetiles);
	~ProceduralGrid(void);
	virtual void Render(void);
	Vector4 GetGridSize(void) const;
	float GetMedianHeight(void) const;
	float GetPointHeight(float xpos, float zpos) const;
	Vector4 GetPointNormal(float xpos, float zpos) const;
	void Reset(float height = 0.0f);
	void Smooth(GLuint iterations, GLuint centerweight);
	void RandomNoise(float magnitude, int levels);
	void Fault(GLuint iterations, float initdisplacement, float dampening);
	void PerlinNoise(float scale, float magnification);
	

protected:
	enum VertexAttribute {POSITION, NORMAL, TEXCOORDINATE};
	bool GetVertexAttrib(VertexAttribute attr, GLuint column, GLuint row, Vector4 &output) const;
	void NormalGen(void);
	void CalculateMedianHeight(void);
	GLuint m_iWidth, m_iHeight;
	GLfloat m_fHsize,m_fVsize,m_fMedianHeight,m_fTileSize;
};
