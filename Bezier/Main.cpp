//Windows Example Code
//Matthew Bett 2005
//Shazam!

#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>
#include <math.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "SkyBox.h"
#include "MS3Dmodel.h"
#include "PointGroup.h"
#include "Bezier.h"
#include "time.h"



#define COLOUR_DEPTH 32	//Colour depth

float pi = 3.14159265358979323846f;
float pi_over_2 = 1.57079632679489661923f;

HWND        ghwnd;
HDC			ghdc;
HGLRC		ghrc;			//hardware RENDERING CONTEXT
HINSTANCE	ghInstance;
RECT		gRect;
RECT		screenRect;

int S_WIDTH	 =	800;		//client area resolution
int S_HEIGHT =	600;


bool		keys[256];

typedef struct Mousestr
{
	Mousestr(){x=y=prevx=prevy=0;};
	bool rbutton, lbutton;
	int x,y,prevx,prevy;
} Mouse;

Mouse MouseStat;

clock_t timedelta,currtime,prevtime;

Vector4 RecordedCamPos, CameraPos, CameraLook, CameraUp;
float alpha = 0.0f;
float beta = 0.0f;

//Light parameters
GLfloat SunLight_Ambient[4] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat SunLight_Diffuse[4] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat SunLight_Specular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat SunLight_Position[4]= {10.0f, 10.0f, 0.0f, 0.0f};

//Fog parameters
GLfloat FogColor[4]	= {0.0f, 0.0f, 0.0f, 1.0f};


//Application specific members
enum eAppState {CAMERA_FREE, CAMERA_OBJECTBOUND, MOVING_CONTROL_POINT};
eAppState AppState;
bool PickingInvoked = false;
UINT SelCtrlPtIndex;
GLdouble SelCtrlPtZDepth;
BezierLoop CompBez;

PointGroup *ControlPoints, *Curve;
float ReferenceLv;
SkyBox *Sky;
MS3Dmodel *Model;
Vector4 ModelPos, ModelRot, ModelTan;
float ModelParam;



//Functions prototypes
HWND CreateOurWindow(LPWSTR strWindowName, int width, int height, DWORD dwStyle, bool bFullScreen, HINSTANCE hInstance);
bool SetPixelFormat(HDC hdc);
void ResizeGLWindow(int width, int height);
void InitializeOpenGL(int width, int height);
void Init(HWND hwnd);
void CreateScene();
void DrawScene();
void Cleanup();
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow);
bool PickControlPoint(UINT &CtrlPtIndexOut, double &ZDepthOut);
void MoveModel(float timedelta);
void CalculateModelTransform(void);



void Init(HWND hwnd)
{
	ghwnd = hwnd;
	GetClientRect(ghwnd, &gRect);	//get rect into our handy global rect
	InitializeOpenGL(gRect.right, gRect.bottom);// initialise openGL

	//OpenGL settings
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);				// Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glEnable(GL_LIGHTING);                              // Enable Lighting
	glEnable(GL_TEXTURE_2D);							// Enable texturing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);



	
	ControlPoints = new PointGroup(PointGroup::DYNAMIC, PointGroup::LINE);
	Curve = new PointGroup(PointGroup::DYNAMIC, PointGroup::LINE);
	Sky = new SkyBox;
	Model = new MS3Dmodel("Models\\Airplane.ms3d");
	//App initialization
	AppState = CAMERA_FREE;
	CreateScene();

	CameraPos = Vector4(0.0f,30.0f,0.0f,1.0f);
	currtime=prevtime=clock();
}

void CreateScene()
{
	GLuint i;
	
	//Initialize composite bezier
	CompBez.SetNumControlPoints(9);
	for(i=0;i<CompBez.GetNumControlPoints();i++)
	{
		CompBez.SetControlPoint(i,Vector4(20.0f*cosf(float(i)*pi/3.0f),0.0f,20.0f*sinf(float(i)*pi/3.0f),1.0f));
	}

	//Set positions for control points
	ControlPoints->SetNumPoints(CompBez.GetNumControlPoints());
	for(i=0;i<ControlPoints->GetNumPoints();i++)
	{
		ControlPoints->SetPoint(i,CompBez.GetControlPoint(i));
	}

	//Setup Curve
	float param;
	Vector4 interpolatedpos;
	Curve->SetNumPoints(100);
	Curve->SetColor(Vector4(1.0f,1.0f,0.0f,1.0f));
	for(i=0;i<Curve->GetNumPoints();i++)
	{
		param = float(i)/float(Curve->GetNumPoints()-1);
		if(i==Curve->GetNumPoints()) {param=1.0f;}
		interpolatedpos = CompBez.GetInterpolatedPos(param);
		Curve->SetPoint(i,interpolatedpos);
		//Record the reference level (point in the curve with highest Y position value)
		//While not related to the curve this is done here to cut down on calculations
		if(i==0)
		{
			ReferenceLv = interpolatedpos.y;
		}
		else
		{
			if(interpolatedpos.y > ReferenceLv)
			{
				ReferenceLv = interpolatedpos.y;
			}
		}
	}
	Curve->GenerateVBO();

	//Position model
	CalculateModelTransform();
}

void DrawScene() 
{	
	GLuint i;

	//Get time delta

	currtime = clock();
	timedelta = currtime - prevtime;
	prevtime = currtime;

	//Move the plane

	CalculateModelTransform();
	MoveModel(float(timedelta));
	CalculateModelTransform();

	//Check if a control point has been selected (needs to be here as it uses the matrix stack)
	if(PickingInvoked == true)
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(CameraPos.x,CameraPos.y,CameraPos.z,CameraLook.x,CameraLook.y,CameraLook.z,CameraUp.x,CameraUp.y,CameraUp.z);
		
		if(PickControlPoint(SelCtrlPtIndex,SelCtrlPtZDepth))
		{
			AppState = MOVING_CONTROL_POINT;
		}
		PickingInvoked = false;
	}
	

	//Free camera behavior
	if(AppState == CAMERA_FREE)
	{
		if(MouseStat.rbutton == true)
		{
			alpha -= 0.04f* float((MouseStat.x - MouseStat.prevx));
			beta -= 0.04f* float((MouseStat.y - MouseStat.prevy));
			MouseStat.prevx = MouseStat.x;
			MouseStat.prevy = MouseStat.y;

			if(alpha>(2.0f*pi))
			{
				alpha-=(2.0f*pi);
			}
			if(alpha<0)
			{
				alpha+=(2.0f*pi);
			}
			if(beta>((pi/2.0f)-0.01f))
			{
				beta=((pi/2.0f)-0.01f);
			}
			if(beta<(0.01f-(pi/2.0f)))
			{
				beta=(0.01f-(pi/2.0f));
			}
		}
		
		Vector4 Temp;

		CameraLook = Vector4(0.0f,0.0f,-1.0f,1.0f);
		Temp = CameraLook;
		CameraLook.y = Temp.y * cosf(beta) - Temp.z * sinf(beta);
		CameraLook.z = Temp.y * sinf(beta) + Temp.z * cosf(beta);
		Temp = CameraLook;
		CameraLook.x = Temp.x * cosf(alpha) + Temp.z * sinf(alpha); 
		CameraLook.z = Temp.z * cosf(alpha) - Temp.x * sinf(alpha);

		CameraUp = Vector4(0.0f,1.0f,0.0f,1.0f);
		Temp = CameraUp;
		CameraUp.y = Temp.y * cosf(beta) - Temp.z * sinf(beta);
		CameraUp.z = Temp.y * sinf(beta) + Temp.z * cosf(beta);
		Temp = CameraUp;
		CameraUp.x = Temp.x * cosf(alpha) + Temp.z * sinf(alpha); 
		CameraUp.z = Temp.z * cosf(alpha) - Temp.x * sinf(alpha);

		if(keys['W'])
		{
			CameraPos += CameraLook * 0.05f * float(timedelta);
		}

		if(keys['S'])
		{
			CameraPos -= CameraLook * 0.05f * float(timedelta);
		}

		if(keys['A'])
		{
			CameraPos -= CameraLook.Cross(Vector4(0.0f,1.0f,0.0f,1.0f)).Normalise() * 0.05f * float(timedelta);
		}

		if(keys['D'])
		{
			CameraPos += CameraLook.Cross(Vector4(0.0f,1.0f,0.0f,1.0f)).Normalise() * 0.05f * float(timedelta);
		}

		CameraLook += CameraPos;

		if(keys['Z'])
		{
			RecordedCamPos = CameraPos;
			AppState = CAMERA_OBJECTBOUND;
		}
	}
	//Moving control point behaviour
	else if(AppState == MOVING_CONTROL_POINT)
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(CameraPos.x,CameraPos.y,CameraPos.z,CameraLook.x,CameraLook.y,CameraLook.z,CameraUp.x,CameraUp.y,CameraUp.z);

		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		GLdouble modelview[16];
		glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

		GLdouble projection[16];
		glGetDoublev(GL_PROJECTION_MATRIX, projection);

		GLdouble Posx,Posy,Posz;
		gluUnProject(GLdouble(MouseStat.x),GLdouble(gRect.bottom - MouseStat.y),SelCtrlPtZDepth,modelview,projection,viewport,&Posx,&Posy,&Posz);
		
		//Move the selected control point
		CompBez.SetControlPoint(SelCtrlPtIndex,Vector4(float(Posx),float(Posy),float(Posz),1.0f));

		//Move the points representing the spline's control points
		for(i=0;i<CompBez.GetNumControlPoints();i++)
		{
			ControlPoints->SetPoint(i,CompBez.GetControlPoint(i));
		}

		//Modify the displayed curve to reflect the new composite bezier
		float param;
		Vector4 interpolatedpos;
		for(i=0;i<Curve->GetNumPoints();i++)
		{
			param = float(i)/float(Curve->GetNumPoints()-1);
			if(i==Curve->GetNumPoints()) {param=1.0f;}
			interpolatedpos = CompBez.GetInterpolatedPos(param);
			Curve->SetPoint(i,interpolatedpos);
			//Record the reference level (point in the curve with highest Y position value)
			//While not related to the curve this is done here to cut down on calculations
			if(i==0)
			{
				ReferenceLv = interpolatedpos.y;
			}
			else
			{
				if(interpolatedpos.y > ReferenceLv)
				{
					ReferenceLv = interpolatedpos.y;
				}
			}
		}
		Curve->GenerateVBO();

		//Allow camera to move again when control point has been positioned
		if(MouseStat.lbutton == false)
		{
			AppState = CAMERA_FREE;
		}
	}
	else if(AppState == CAMERA_OBJECTBOUND)
	{
		CameraPos = ModelPos - (ModelTan * 20.0f);
		CameraLook = ModelPos;
		CameraUp = ModelTan.Cross(Vector4(0.0f,1.0f,0.0f,1.0f));
		CameraUp = CameraUp.Cross(ModelTan);

		if(keys['X'])
		{
			CameraPos = RecordedCamPos;
			AppState = CAMERA_FREE;
		}
	}

	



	//Actual Rendering Starts Here
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// Clear The Screen And The Depth Buffer

	//Setup Light
	glLightfv(GL_LIGHT0, GL_AMBIENT,  SunLight_Ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  SunLight_Diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR,  SunLight_Specular);
	glLightfv(GL_LIGHT0, GL_POSITION, SunLight_Position);
	glEnable(GL_LIGHT0);

	//Apply Camera (View) Transform
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(CameraPos.x,CameraPos.y,CameraPos.z,CameraLook.x,CameraLook.y,CameraLook.z,CameraUp.x,CameraUp.y,CameraUp.z);
	//Render skybox (has to be done first)
	glPushMatrix();
	glTranslatef(CameraPos.x,CameraPos.y,CameraPos.z);
	Sky->Render();
	glPopMatrix();

	//Render the rest of the immovable objects
	glPushMatrix();
	ControlPoints->SetDrawStyle(PointGroup::POINTS);
	ControlPoints->SetSize(4.0f);
	ControlPoints->SetColor(1.0f,0.0f,0.0f,1.0f);
	ControlPoints->Render();
	ControlPoints->SetDrawStyle(PointGroup::LOOP);
	ControlPoints->SetSize(1.0f);
	ControlPoints->SetColor(0.0f,1.0f,0.0f,1.0f);
	ControlPoints->Render();
	Curve->SetSize(2.0f);
	Curve->Render();
	glPopMatrix();

	//Render the Plane
	glPushMatrix();
	glTranslatef(ModelPos.x,ModelPos.y,ModelPos.z);
	glRotatef(ModelRot.y, 0,1,0);
	glRotatef(ModelRot.x, 1,0,0);
	glRotatef(ModelRot.z, 0,0,1);
	Model->Render();
	glPopMatrix();

	// Swap the frame buffers.
	SwapBuffers(ghdc);
}		

void Cleanup()
{
	delete Sky;
	delete Curve;
	delete ControlPoints;
	delete Model;

	if (ghrc)
	{
		wglMakeCurrent(NULL, NULL);	// free rendering memory
		wglDeleteContext(ghrc);		// Delete our OpenGL Rendering Context
	}

	if (ghdc) 
		ReleaseDC(ghwnd, ghdc);			// Release our HDC from memory

	UnregisterClass(L"FirstWindowClass", ghInstance);// Free the window class

	PostQuitMessage (0);		// Post a QUIT message to the window
}

HWND CreateOurWindow(LPWSTR strWindowName, int width, int height, DWORD dwStyle, bool bFullScreen, HINSTANCE hInstance)
{
	HWND hwnd;

	WNDCLASS wcex;

	memset(&wcex, 0, sizeof(WNDCLASS));
	wcex.style			= CS_HREDRAW | CS_VREDRAW;		
	wcex.lpfnWndProc	= WndProc;		
	wcex.hInstance		= hInstance;						
	wcex.hIcon			= LoadIcon(NULL, IDI_APPLICATION);; 
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);		
	wcex.hbrBackground	= (HBRUSH) (COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;	
	wcex.lpszClassName	= L"FirstWindowClass";	


	RegisterClass(&wcex);// Register the class

	dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	ghInstance	= hInstance;// Assign our global hInstance to the window's hInstance

	//Set the Client area of the window to be our resolution.
	RECT glwindow;
	glwindow.left		= 0;		
	glwindow.right		= width;	
	glwindow.top		= 0;		
	glwindow.bottom		= height;	

	AdjustWindowRect( &glwindow, dwStyle, false);

	//Create the window
	hwnd = CreateWindow(L"FirstWindowClass", 
		strWindowName, 
		dwStyle, 
		0, 
		0,
		glwindow.right  - glwindow.left,
		glwindow.bottom - glwindow.top, 
		NULL,
		NULL,
		hInstance,
		NULL
		);

	if(!hwnd) return NULL;// If we could get a handle, return NULL

	ShowWindow(hwnd, SW_SHOWNORMAL);	
	UpdateWindow(hwnd);					
	SetFocus(hwnd);						

	return hwnd;
}

bool SetPixelFormat(HDC hdc) 
{ 
	PIXELFORMATDESCRIPTOR pfd = {0}; 
	int pixelformat; 

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);	// Set the size of the structure
	pfd.nVersion = 1;							// Always set this to 1
	// Pass in the appropriate OpenGL flags
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; 
	pfd.dwLayerMask = PFD_MAIN_PLANE;			// standard mask (this is ignored anyway)
	pfd.iPixelType = PFD_TYPE_RGBA;				// RGB and Alpha pixel type
	pfd.cColorBits = COLOUR_DEPTH;				// Here we use our #define for the color bits
	pfd.cDepthBits = COLOUR_DEPTH;				// Ignored for RBA
	pfd.cAccumBits = 0;							// nothing for accumulation
	pfd.cStencilBits = 0;						// nothing for stencil

	//Gets a best match on the pixel format as passed in from device
	if ( (pixelformat = ChoosePixelFormat(hdc, &pfd)) == false ) 
	{ 
		MessageBox(NULL, L"ChoosePixelFormat failed", L"Error", MB_OK); 
		return false; 
	} 

	//sets the pixel format if its ok. 
	if (SetPixelFormat(hdc, pixelformat, &pfd) == false) 
	{ 
		MessageBox(NULL, L"SetPixelFormat failed", L"Error", MB_OK); 
		return false; 
	} 

	return true;
}

void ResizeGLWindow(int width, int height)// Initialize The GL Window
{
	if (height==0)// Prevent A Divide By Zero error
	{
		height=1;// Make the Height Equal One
	}

	glViewport(0,0,width,height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//calculate aspect ratio
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height, 1 ,10000.0f);

	glMatrixMode(GL_MODELVIEW);// Select The Modelview Matrix
	glLoadIdentity();// Reset The Modelview Matrix
}

void InitializeOpenGL(int width, int height) 
{  
	ghdc = GetDC(ghwnd);//  sets  global HDC

	if (!SetPixelFormat(ghdc))//  sets  pixel format
		PostQuitMessage (0);


	ghrc = wglCreateContext(ghdc);	//  creates  rendering context from  hdc
	wglMakeCurrent(ghdc, ghrc);		//	Use this HRC.

	ResizeGLWindow(width, height);	// Setup the Screen
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{


	switch (message)											
	{														
	case WM_CREATE:	
		break;

	case WM_SIZE:
		//resize the open gl window when the window is resized
		ResizeGLWindow(LOWORD(lParam),HIWORD(lParam));
		GetClientRect(hwnd, &gRect);
		break;	

	case WM_KEYDOWN:
		keys[wParam]=true;
		break;

	case WM_KEYUP:
		keys[wParam]=false;
		break;

	case WM_LBUTTONDOWN:
		MouseStat.lbutton = true;
		if(AppState == CAMERA_FREE)
		{
			PickingInvoked = true;
		}
		break;

	case WM_LBUTTONUP:
		MouseStat.lbutton = false;
		break;

	case WM_RBUTTONDOWN:
		MouseStat.rbutton = true;
		break;

	case WM_RBUTTONUP:
		MouseStat.rbutton = false;
		break;

	case WM_MOUSEMOVE:
		//if(MK_LBUTTON & wParam) {MouseStat.lbutton = true;} else  {MouseStat.lbutton = false;}
		//if(MK_RBUTTON & wParam) {MouseStat.rbutton = true;} else  {MouseStat.rbutton = false;}
		MouseStat.prevx = MouseStat.x;
		MouseStat.prevy = MouseStat.y;
		MouseStat.x = int(LOWORD(lParam));
		MouseStat.y = int(HIWORD(lParam));
		break;

	case WM_PAINT:
		break;		

	case WM_DESTROY:	
		PostQuitMessage(0);	
		break;				
	}													

	return DefWindowProc (hwnd, message, wParam, lParam);		

}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
					PSTR szCmdLine, int nCmdShow)			
{	
	HWND		hwnd;
	MSG         msg;	

	//initialise and create window
	hwnd = CreateOurWindow(L"Terragen", S_WIDTH, S_HEIGHT, 0, false, hInstance);	
	if(hwnd == NULL) return true;

	//initialise opengl and other settings
	Init(hwnd);

	while(true)					
	{							
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			if (msg.message==WM_QUIT)
				break;
			TranslateMessage (&msg);							
			DispatchMessage (&msg);
		}

		else
		{		
			//any intensive proccessing for the app,  do it here. 
			DrawScene();
		}
	}

	return msg.wParam ;										
}

bool PickControlPoint(UINT &CtrlPtIndexOut, double &ZDepthOut)
{
	bool result = false;

	UINT i;

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	GLdouble modelview[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	for(i=0;i<CompBez.GetNumControlPoints();i++)
	{
		Vector4 Pos = CompBez.GetControlPoint(i);
		GLdouble Xpos = 0;
		GLdouble Ypos = 0;
		GLdouble Zpos = 0; 
		int res = gluProject(Pos.x,Pos.y,Pos.z,modelview,projection,viewport,&Xpos,&Ypos,&Zpos);

		Ypos = gRect.bottom - Ypos;
		if(res == GL_TRUE)
		{
			if((abs(Xpos - GLdouble(MouseStat.x)) < 5) && (abs(Ypos - GLdouble(MouseStat.y)) < 5))
			{
				CtrlPtIndexOut = i;
				ZDepthOut = Zpos;
				result = true;
				return result;
			}
		}
	}

	return result;
}

void MoveModel(float timedelta)
{
	float speed = sqrt((0.0001f * (ReferenceLv - ModelPos.y)) + 0.001f);
	float dist = speed * timedelta;

	Vector4 Pos1, Pos2;
	float test = 0.0f;
	if(ModelParam < 0.0f)
	{
		ModelParam = ModelParam + float(int(ModelParam)+1);
	}
	if(ModelParam > 1.0f)
	{
		ModelParam = ModelParam - float(int(ModelParam));
	}
	Pos2 = CompBez.GetInterpolatedPos(ModelParam);
	while(test < dist)
	{
		Pos1 = Pos2;
		ModelParam += 0.00001f;
		if(ModelParam > 1.0f)
		{
			ModelParam = ModelParam - float(int(ModelParam));
		}
		Pos2 = CompBez.GetInterpolatedPos(ModelParam);
		test += (Pos2 - Pos1).Length();
	}
}

void CalculateModelTransform(void)
{
	float tempfl;
	bool reversed = false;
	Vector4 tempvec;

	if(ModelParam < 0.0f)
	{
		ModelParam = ModelParam + float(int(ModelParam)+1);
	}
	if(ModelParam > 1.0f)
	{
		ModelParam = ModelParam - float(int(ModelParam));
	}

	ModelPos = CompBez.GetInterpolatedPos(ModelParam);
	ModelTan = CompBez.GetInterpolatedTan(ModelParam);

	//Rotations about y and x axis: they are calculated so that the airplane model faces the direction of the
	//composite curve's tangent at its position
	//Calculate rotation angle about y axis
	tempvec = ModelTan;
	tempvec.y = 0.0f;
	tempvec.NormaliseSelf();
	ModelRot.y = acosf(Vector4(0.0f,0.0f,-1.0f,1.0f).Dot3(tempvec)) * (180/pi);
	if(tempvec.x > 0.0f) 
	{
		ModelRot.y = 0.0f - ModelRot.y;
	}

	//Calculate rotation angle about x axis
	tempvec = ModelTan;
	ModelRot.x = 90.0f - (acosf(Vector4(0.0f,1.0f,0.0f,1.0f).Dot3(tempvec)) * (180/pi));

	
	//Get difference between y axis rotation angles of plane's current position and one advanced by a small amount
	tempfl = ModelParam + 0.01f;
	if(tempfl < 0.0f)
	{
		tempfl = tempfl + float(int(tempfl)+1);
	}
	if(tempfl > 1.0f)
	{
		tempfl = tempfl - float(int(tempfl));
	}

	tempvec = CompBez.GetInterpolatedTan(tempfl);
	tempvec.y = 0.0f;
	tempvec.NormaliseSelf();
	tempfl = acosf(Vector4(0.0f,0.0f,-1.0f,1.0f).Dot3(tempvec)) * (180/pi);
	if(tempvec.x > 0.0f)
	{
		tempfl = 0.0f - tempfl;
	}

	tempfl = tempfl - ModelRot.y;

	//Get the actual difference, the smallest angle that needs to be travelled to reach the current orientation from the previous one
	//Taking into account that any angle alpha is equal to alpha - 2PI
	if(tempfl > 180.0f)
	{
		tempfl -= 360.0f; 
	}
	if(tempfl < -180.0f)
	{
		tempfl += 360.0f;
	}

	//Get the angle's module and record whether it was negative
	if(tempfl < 0.0f) 
	{
		reversed = true;
		tempfl = 0.0f - tempfl;
	}

	//Custom z rotation angle calculation
	if(tempfl > 90.0f ) {tempfl = 90.0f;}

	tempfl = tempfl/90.0f;
	tempfl = sqrt(tempfl);
	tempfl = tempfl*90.0f;

	//If the angle was less than zero change its sign back to what it was
	if(reversed == true)
	{
		tempfl = 0.0f - tempfl;
	}
	
	ModelRot.z = tempfl;
}
