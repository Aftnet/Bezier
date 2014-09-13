#include "PointGroup.h"

PointGroup::PointGroup(eUsage Usage, eDrawStyle DrawStyle):
m_eUsage(Usage),
m_eDrawStyle(DrawStyle),
m_vColor(Vector4(1.0f,1.0f,1.0f,1.0f)),
m_fSize(1.0f),
m_uiNumPoints(0),
m_pPoints(NULL),
m_hVBOPointBuffer(0)
{
	//Get required extensions function pointers
	glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) wglGetProcAddress("glGenBuffersARB");
	glBindBufferARB = (PFNGLBINDBUFFERARBPROC) wglGetProcAddress("glBindBufferARB");					
	glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress("glBufferDataARB");					
	glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) wglGetProcAddress("glDeleteBuffersARB");
}

PointGroup::~PointGroup(void)
{
	if(m_pPoints != NULL)
	{
		delete[] m_pPoints;
	}

	glDeleteBuffersARB(1,&m_hVBOPointBuffer);
}

PointGroup::eDrawStyle PointGroup::GetDrawStyle(void) const
{
	return m_eDrawStyle;
}

void PointGroup::SetDrawStyle(eDrawStyle Value)
{
	m_eDrawStyle = Value;
}

GLuint PointGroup::GetNumPoints(void) const
{
	return m_uiNumPoints;
}

void PointGroup::SetNumPoints(GLuint Value)
{
	if(m_pPoints != NULL)
	{
		delete[] m_pPoints;
	}

	m_uiNumPoints = Value;

	m_pPoints = new Vector4[m_uiNumPoints];
}

const Vector4& PointGroup::GetColor(void) const
{
	return m_vColor;
}

void PointGroup::SetColor(const Vector4& Value)
{
	m_vColor = Value;
}

void PointGroup::SetColor(float r, float g, float b, float a)
{
	m_vColor = Vector4(r,g,b,a);
}

GLfloat PointGroup::GetSize(void) const
{
	return m_fSize;
}
	
void PointGroup::SetSize(GLfloat Value)
{
	m_fSize = Value;
}

const Vector4& PointGroup::GetPoint(GLuint index) const
{
	if(index < m_uiNumPoints)
	{
		return m_pPoints[index];
	}
	else
	{
		return m_pPoints[m_uiNumPoints - 1];
	}
}

bool PointGroup::SetPoint(GLuint index, const Vector4& Value)
{
	if(index < m_uiNumPoints)
	{
		m_pPoints[index] = Value;
		return true;
	}
	else
	{
		return false;
	}
}

void PointGroup::GenerateVBO(void)
{
	if(m_eUsage == STATIC)
	{
		glDeleteBuffersARB(1,&m_hVBOPointBuffer);
		glGenBuffersARB(1,&m_hVBOPointBuffer);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,m_hVBOPointBuffer);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB,m_uiNumPoints*sizeof(Vector4),m_pPoints,GL_STATIC_DRAW_ARB);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	}
}

void PointGroup::Render(void)
{
	//Set shader
	m_Technique.Set();
	m_Technique.SetColor(m_vColor);
	//Set size
	glLineWidth(m_fSize);
	glPointSize(m_fSize);
	//Setup array usage states
	glEnableClientState(GL_VERTEX_ARRAY);
	//Setup arrays
	if(m_eUsage == STATIC)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,m_hVBOPointBuffer);
		glVertexPointer(4,GL_FLOAT,0,0);
	}
	else
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,m_hVBOPointBuffer);
		glVertexPointer(4,GL_FLOAT,0,(void*)m_pPoints);
	}
	//Draw
	switch(m_eDrawStyle)
	{
	case POINTS:
		glDrawArrays(GL_POINTS,0,m_uiNumPoints);
		break;
	case LINE:
		glDrawArrays(GL_LINE_STRIP,0,m_uiNumPoints);
		break;
	case LOOP:
		glDrawArrays(GL_LINE_LOOP,0,m_uiNumPoints);
		break;
	}
	//Restore previous state
	glDisableClientState(GL_VERTEX_ARRAY);
	//Bind with 0, so, switch back to normal pointer operation if VBOS have been used
	if(m_eUsage == STATIC)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}
	//Restore default size
	glPointSize(1.0f);
	glLineWidth(1.0f);
}

void PointGroup::RenderSection(GLuint StartIndex, GLuint PointsToRender)
{
	if((StartIndex + PointsToRender) < m_uiNumPoints)
	{
		//Set shader
		m_Technique.Set();
		m_Technique.SetColor(m_vColor);
		//Set size
		glLineWidth(m_fSize);
		glPointSize(m_fSize);
		//Setup array usage states
		glEnableClientState(GL_VERTEX_ARRAY);
		//Setup arrays
		if(m_eUsage == STATIC)
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,m_hVBOPointBuffer);
			glVertexPointer(4,GL_FLOAT,0,0);
		}
		else
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,m_hVBOPointBuffer);
			glVertexPointer(4,GL_FLOAT,0,(void*)m_pPoints);
		}
		//Draw
		if(m_eDrawStyle == POINTS)
		{
			glDrawArrays(GL_POINTS,StartIndex,PointsToRender);
		}
		else
		{
			glDrawArrays(GL_LINE_STRIP,StartIndex,PointsToRender);
		}
		//Restore previous state
		glDisableClientState(GL_VERTEX_ARRAY);
		//Bind with 0, so, switch back to normal pointer operation if VBOS have been used
		if(m_eUsage == STATIC)
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		}
		//Restore default size
		glPointSize(1.0f);
		glLineWidth(1.0f);
	}
}

