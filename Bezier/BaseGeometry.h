#pragma once

//Renderable base class: stores and pumps geometric data about anything that can be rendered in 3D to the graphic card.
//It is meant to be used as a base for other specific classes to be derived from it.

#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>
#include <math.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "glext.h"
#include "vector.h"

struct MaterialStr
{
	MaterialStr(void) {GLuint i; for(i=0;i<4;i++) {Specular[i]=0.0f; Emissive[i]=0.0f; Diffuse[i]=0.0f; Ambient[i]=0.0f;} Shininess[0]=1.0f;}
	GLfloat Specular[4];
	GLfloat Emissive[4];
	GLfloat Diffuse[4];
	GLfloat Ambient[4];
	GLfloat Shininess[1];
};

#pragma pack(push, 1)
struct VertexStr
{
	Vector4 Pos, Norm, Tex;
};
#pragma pack(pop)

struct SubsetStr
{
	SubsetStr() {NumTriangles=0; hVBOIndexBuffer=0; pIndices=NULL;}
	~SubsetStr() {if(pIndices) {delete[] pIndices;}}

	GLuint NumTriangles;
	MaterialStr Material;
	GLuint * pIndices;
	
	GLuint hVBOIndexBuffer; 
};

class BaseGeometry
{
public:
protected:
	BaseGeometry(void);
	~BaseGeometry(void);
	void SetNumVertices(GLuint numvertices);
	void GenerateVertexBuffer(void);
	void SetNumSubsets(GLuint numsubsets);
	void ResizeSubset(GLuint numsubset, GLuint numtriangles);
	void GenerateSubsetIndicesBuffer(GLuint numsubset);
	void DrawSubset(GLuint numsubset) const;
	virtual void Render()=0;

	GLuint m_uiNumSubsets;
	SubsetStr * m_pSubsets;

	GLuint m_uiNumVertices;
	VertexStr * m_pVertices;
	GLuint m_hVBOVertexBuffer;

	//OpenGL extensions function pointers
	PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
	PFNGLGENBUFFERSARBPROC glGenBuffersARB;					// VBO Name Generation Procedure
	PFNGLBINDBUFFERARBPROC glBindBufferARB;					// VBO Bind Procedure
	PFNGLBUFFERDATAARBPROC glBufferDataARB;					// VBO Data Loading Procedure
	PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;			// VBO Deletion Procedure
};
