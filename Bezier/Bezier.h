#pragma once

#include "Vector.h"

class CompositeBezierBase
{
public:
	UINT GetNumControlPoints(void) const;
	const Vector4& GetControlPoint(UINT Index) const;
	Vector4 GetInterpolatedPos(float Param);
	Vector4 GetInterpolatedTan(float Param);

	virtual bool SetNumControlPoints(UINT Value)=0;
	virtual bool SetControlPoint(UINT Index, const Vector4& Value)=0;

protected:
	CompositeBezierBase(void);
	~CompositeBezierBase(void);

	UINT m_uiCtrlPtsNo, m_uiNumSplines;
	Vector4 *m_pCtrlPtsArr;

	float m_fStep;
};


class BezierLine : public CompositeBezierBase
{
public:
	bool SetNumControlPoints(UINT Value);
	bool SetControlPoint(UINT Index, const Vector4& Value);	
};


class BezierLoop : public CompositeBezierBase
{
public:
	bool SetNumControlPoints(UINT Value);
	bool SetControlPoint(UINT Index, const Vector4& Value);
};
