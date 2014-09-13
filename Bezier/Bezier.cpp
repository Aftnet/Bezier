#include "Bezier.h"

CompositeBezierBase::CompositeBezierBase(void):
m_uiCtrlPtsNo(0),
m_uiNumSplines(0),
m_pCtrlPtsArr(NULL)
{
}

CompositeBezierBase::~CompositeBezierBase(void)
{
	delete[] m_pCtrlPtsArr;
}

UINT CompositeBezierBase::GetNumControlPoints(void) const
{
	return m_uiCtrlPtsNo;
}

const Vector4& CompositeBezierBase::GetControlPoint(UINT Index) const
{
	if(Index<m_uiCtrlPtsNo)
	{
		return m_pCtrlPtsArr[Index];
	}
	else
	{
		return m_pCtrlPtsArr[m_uiCtrlPtsNo-1];
	}
}

Vector4 CompositeBezierBase::GetInterpolatedPos(float Param)
{
	//Determine which of the splines comprising the composite spline to use
	UINT startindex = UINT(Param/m_fStep);

	//Recalculate parameter in the selected spline scale
	Param -= m_fStep*float(startindex);
	Param = Param/m_fStep;

	//Calculate One minus the parameter value (uesful multiple times later on)
	float Param2 = Param*Param;
	float Param3 = Param2*Param;
	float OneMinusParam = 1.0f - Param;
	float OneMinusParam2 = OneMinusParam*OneMinusParam;
	float OneMinusParam3 = OneMinusParam2*OneMinusParam;

	//Determine starting control point for chosen spline
	startindex = 3*startindex;

	//Interpolate
	return (OneMinusParam3*m_pCtrlPtsArr[startindex])+(3.0f*OneMinusParam2*Param*m_pCtrlPtsArr[startindex+1])+(3.0f*OneMinusParam*Param2*m_pCtrlPtsArr[startindex+2])+(Param3*m_pCtrlPtsArr[((startindex+3)%m_uiCtrlPtsNo)]);
}

Vector4 CompositeBezierBase::GetInterpolatedTan(float Param)
{
	//Determine which of the splines comprising the composite spline to use
	UINT startindex = UINT(Param/m_fStep);

	//Recalculate parameter in the selected spline scale
	Param -= m_fStep*float(startindex);
	Param = Param/m_fStep;

	//Calculate One minus the parameter value (uesful multiple times later on)
	float Param2 = Param*Param;
	float OneMinusParam = 1.0f - Param;
	float OneMinusParam2 = OneMinusParam*OneMinusParam;

	//Determine starting control point for chosen spline
	startindex = 3*startindex;

	//Interpolate
	return (((-3.0f*OneMinusParam2)*m_pCtrlPtsArr[startindex])+(((3.0f-9.0f*Param)*OneMinusParam)*m_pCtrlPtsArr[startindex+1])+((6.0f*Param-9.0f*Param2)*m_pCtrlPtsArr[startindex+2])+((3.0f*Param2)*m_pCtrlPtsArr[((startindex+3)%m_uiCtrlPtsNo)])).Normalise();
}


bool BezierLine::SetNumControlPoints(UINT Value)
{
	if(Value > 3 && ((Value-4)%3) == 0)
	{
		m_uiCtrlPtsNo = Value;
		m_uiNumSplines = ((m_uiCtrlPtsNo-4)/3)+1;
		m_fStep = 1.0f/float(m_uiNumSplines);
		
		if(m_pCtrlPtsArr!=NULL)
		{
			delete[] m_pCtrlPtsArr;
		}
		m_pCtrlPtsArr = new Vector4[m_uiCtrlPtsNo];

		return true;
	}
	else
	{
		return false;
	}
}

bool BezierLine::SetControlPoint(UINT Index, const Vector4& Value)
{
	if(Index<m_uiCtrlPtsNo)
	{
		Vector4 Displacement = Value - m_pCtrlPtsArr[Index];
		m_pCtrlPtsArr[Index] = Value;
		//Special handling to constrain control points to form C1 composite splines
		if((Index!=0) && (Index!=(m_uiCtrlPtsNo-1)) && ((Index%3)==0))
		{
			m_pCtrlPtsArr[Index-1] += Displacement;
			m_pCtrlPtsArr[Index+1] += Displacement;
		}
		if((Index>1) && (Index<(m_uiCtrlPtsNo-4)) && (((Index+1)%3)==0))
		{
			m_pCtrlPtsArr[Index+2] = (2.0f*m_pCtrlPtsArr[Index+1]) - m_pCtrlPtsArr[Index];
		}
		if((Index>3) && (Index<(m_uiCtrlPtsNo-2)) && (((Index-1)%3)==0))
		{
			m_pCtrlPtsArr[Index-2] = (2.0f*m_pCtrlPtsArr[Index-1]) - m_pCtrlPtsArr[Index];
		}
		return true;
	}
	else
	{
		return false;
	}
}


bool BezierLoop::SetNumControlPoints(UINT Value)
{
	if(Value > 5 && (Value%3) == 0)
	{
		m_uiCtrlPtsNo = Value;
		m_uiNumSplines = (m_uiCtrlPtsNo/3);
		m_fStep = 1.0f/float(m_uiNumSplines);
		
		if(m_pCtrlPtsArr!=NULL)
		{
			delete[] m_pCtrlPtsArr;
		}
		m_pCtrlPtsArr = new Vector4[m_uiCtrlPtsNo];

		return true;
	}
	else
	{
		return false;
	}
}

bool BezierLoop::SetControlPoint(UINT Index, const Vector4& Value)
{
	if(Index<m_uiCtrlPtsNo)
	{
		UINT PrevIndex = (Index+m_uiCtrlPtsNo-1)%m_uiCtrlPtsNo;
		UINT Prev2Index = (Index+m_uiCtrlPtsNo-2)%m_uiCtrlPtsNo;
		UINT NextIndex = (Index+1)%m_uiCtrlPtsNo;
		UINT Next2Index = (Index+2)%m_uiCtrlPtsNo;
		Vector4 Displacement = Value - m_pCtrlPtsArr[Index];
		m_pCtrlPtsArr[Index] = Value;
		//Special handling to constrain control points to form C1 composite splines
		if((Index%3)==0)
		{
			m_pCtrlPtsArr[PrevIndex] += Displacement;
			m_pCtrlPtsArr[NextIndex] += Displacement;
		}
		if(((Index+1)%3)==0)
		{
			m_pCtrlPtsArr[Next2Index] = (2.0f*m_pCtrlPtsArr[NextIndex]) - m_pCtrlPtsArr[Index];
		}
		if(((Index+2)%3)==0)
		{
			m_pCtrlPtsArr[Prev2Index] = (2.0f*m_pCtrlPtsArr[PrevIndex]) - m_pCtrlPtsArr[Index];
		}
		return true;
	}
	else
	{
		return false;
	}
}


