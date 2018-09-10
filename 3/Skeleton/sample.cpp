//latest
#include <stdio.h>
#include <iostream>
// yes, I know stdio.h is not good C++, but I like the *printf()
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>
#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif


// You need to adjust the location of these header files according to your configuration


#include <GL/include/gl.h>
#include <GL/include/glu.h>
#include <GL/include/glut.h>
#include <GL/include/glui.h>
#include "Skeleton.h"



//#include "glui.h"

//
//
//	This is a sample OpenGL / GLUT / GLUI program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with radio buttons
//
//	The left mouse button allows rotation
//	The middle mouse button allows scaling
//	The glui window allows:
//		1. The 3d object to be transformed
//		2. The projection to be changed
//		3. The color of the axes to be changed
//		4. The axes to be turned on and off
//		5. The transformations to be reset
//		6. The program to quit
//
//	Author: Joe Graphics

// constants:
//
// NOTE: There are a bunch of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch() statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch() statements.  Those are #defines.
//
//
// This source code has been modified by Guoning Chen since its release

// title of these windows:
const char *WINDOWTITLE = { "OpenGL / GLUT / GLUI Sample -- Priscilla Graphics - PeopleSoft ID : 1619570" };

const char *GLUITITLE = { "User Interface" };

// what the glui package defines as true and false:
const int GLUITRUE = { true };
const int GLUIFALSE = { false };

// the escape key:
#define ESCAPE		0x1b

#define NX 128
#define NY 128
#define NZ 128


// For textures
#define X 0
#define Y 1
#define Z 2

// initial window size:
const int INIT_WINDOW_SIZE = { 600 };

// size of the box:
const float BOXSIZE = { 2.f };

// multiplication factors for input interaction:
//  (these are known from previous experience)
const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };

const int MINUS = { 0 };
const int PLUS = { 1 };

const float TEMPMIN = 0, TEMPMAX = 100;
static float DISTMIN, DISTMAX;
static float GRADMIN, GRADMAX;


// able to use the left mouse for either rotation or scaling,
// in case have only a 2-button mouse:
enum LeftButton
{
	ROTATE,
	SCALE
};

// minimum allowable scale factor:
const float MINSCALE = { 0.05f };

// active mouse buttons (or them together):
const int LEFT = { 4 };
const int MIDDLE = { 2 };
const int RIGHT = { 1 };
enum Slide{
	Temp_Sliders,
	Gradient_Slider,
	X_Slider,
	Y_Slider,
	Z_Slider,
	Distance_Slider,
	Opacity_Slider,
};

// which projection:
enum Projections
{
	ORTHO,
	PERSP
};

// which button:
enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):
const float BACKCOLOR[] = { 0., 0., 0., 0. };
//const float BACKCOLOR[] = { 0.5, 0.5, 0.5, 0.};

// line width for the axes:
const GLfloat AXES_WIDTH = { 3. };


struct node
{
	float x, y, z;
	float T;
	float r, g, b; 
	float rad; 
	float dTdx, dTdy, dTdz; 
	float grad; 
};
node Nodes[NX][NY][NZ];

struct sources
{
	double xc, yc, zc; // 
	double a; 
} Sources[] =
{
	{ 1.00f, 0.00f, 0.00f, 90.00f },
	{ -1.00f, 0.30f, 0.00f, 120.00f },
	{ 0.10f, 1.00f, 0.00f, 120.00f },
	{ 0.00f, 0.40f, 1.00f, 170.00f },
};
float scalar[NX*NY*NZ];

struct contour
{
	float x, y, z;
	float scalar;
};
contour contours[NX*NY];
// the color numbers:
// this order must match the radio button order
enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};
enum ColorSchemes
{
	Rainbow,
	BlueWhiteRed,
	OrangeWhiteCyan,
};

enum Tech
{
	VSlicing,
	WireFraming,
	VRendering,
};
// the color definitions:
// this order must match the radio button order
const GLfloat Colors[8][3] =
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};

// fog parameters:
const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE = { GL_LINEAR };
const GLfloat FOGDENSITY = { 0.30f };
const GLfloat FOGSTART = { 1.5 };
const GLfloat FOGEND = { 4. };

//
// non-constant global variables:
//
int	ActiveButton;		// current button that is down
GLuint	AxesList;		// list to hold the axes
int	AxesOn;			// != 0 means to draw the axes
int	DebugOn;			// != 0 means to print debugging info
int	DepthCueOn;		// != 0 means to use Grey Scale depth cueing
GLUI *	Glui;			// instance of glui window
int	GluiWindow;		// the glut id for the glui window
int	LeftButton;		// either ROTATE or SCALE
GLuint	BoxList;		// object display list
int	MainWindow;		// window id for main graphics window
GLfloat	RotMatrix[4][4];	// set by glui rotation widget

GLUI_StaticText *XLabel;
GLUI_StaticText *YLabel;
GLUI_StaticText *ZLabel;
GLUI_StaticText *TempLabel;
GLUI_StaticText *GradLabel;
GLUI_StaticText *DistLabel;
GLUI_StaticText *OpacLabel;
GLUI_Checkbox *checkXY;
GLUI_Checkbox *checkYZ;
GLUI_Checkbox *checkZX;
GLUI_Spinner *sliceX;
GLUI_Spinner *sliceY;
GLUI_Spinner *sliceZ;
GLUI_Spinner *contour_scalar_spin;
GLUI_HSlider * XSlider;
GLUI_HSlider * YSlider;
GLUI_HSlider * ZSlider;
GLUI_HSlider *temp_slider;
GLUI_HSlider *grad_sliders;
GLUI_HSlider *dist_sliders;
GLUI_HSlider *opac_slider;
int Major;	/* X, Y, or Z */
int Xside, Yside, Zside;	//which side is visible, PLUS or MINUS
float	Scale, Scale2;		// scaling factors
int	WhichColor;		// index into Colors[]
int	WhichProjection;	// ORTHO or PERSP
int	Xmouse, Ymouse;		// mouse values
float	Xrot, Yrot;		// rotation angles in degrees
float	TransXYZ[3];		// set by glui translation widgets

// function prototypes:
void	Animate(void);
void	Buttons(int);
void	Display(void);
void	DoRasterString(float, float, float, char *);
void	DoStrokeString(float, float, float, float, char *);
float	ElapsedSeconds(void);
void	InitGlui(void);
void	InitGraphics(void);
void	InitLists(void);
void	Keyboard(unsigned char, int, int);
void	MouseButton(int, int, int, int);
void	MouseMotion(int, int);
void	Reset(void);
void	Resize(int, int);
void	Visibility(int);
void	DetermineVisibility();

void	Arrow(float[3], float[3]);
void	Cross(float[3], float[3], float[3]);
float	Dot(float[3], float[3]);
float	Unit(float[3], float[3]);
void	Axes(float);
void	HsvRgb(float[3], float[3]);
void    Slider1();
void    Slider2();
void    Display_Model(void);
void    set_view(GLenum mode, Polyhedron *poly);
void    set_scene(GLenum mode, Polyhedron *poly);
void    display_shape(GLenum mode, Polyhedron *this_poly);
void    temporary();
void	generate_data();
void	Color_Object();
void	Choose_Technique();
void	Sliders(int);
float*  MinMax(float, int);

const char * ZFORMAT = { "Z Axis: %5.2f - %5.2f" };
const char * YFORMAT = { "Y Axis: %5.2f - %5.2f" };
const char * XFORMAT = { "X Axis: %5.2f - %5.2f" };
const char * TEMPFORMAT = { "Temperature: %5.2f - %5.2f" };
const char * GRADFORMAT = { "Gradient: %5.2f - %5.2f" };
const char * DISTFORMAT = { "Distance: %5.2f - %5.2f" };
const char * OPACITYFORMAT = { "Opacity: %5.2f" };

double radius_factor = 0.9;
int VColor = 0;
int TName = 0;
int ObjectId = 0;
int ObID = 0;
int TId = 0;
float XLowHigh[2] = { -1, 1 };
float YLowHigh[2] = { -1, 1 };
float ZLowHigh[2] = { -1, 1 };
float TempLowHigh[2];
float GradLowHigh[2];
float DistLowHigh[2];
float MaxAlpha = 1;
int XY_visible = 1;
int YZ_visible = 1;
int XZ_visible = 1;
int showbilinear = 0;
int planeX = NX / 2;
int planeY = NY / 2;
int planeZ = NZ / 2;

float contourValue = 15.0;
unsigned char TextureXY[NZ][NX][NY][4];
unsigned char TextureXZ[NY][NX][NZ][4];
unsigned char TextureYZ[NX][NY][NZ][4];


Polyhedron *poly = NULL;
//extern PlyFile *in_ply;
//extern FILE *this_file;

// main program:
int main(int argc, char *argv[])
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)
	glutInit(&argc, argv);

	// Load the model and data here
	//FILE *this_file = fopen("D:/Masters/Fall 2014/Visualization/assign2/data_files_assign2/diesel_field1.ply", "r");
	FILE *this_file = fopen("../models/diesel_field1.ply", "r");
	poly = new Polyhedron(this_file);
	fclose(this_file);
	//mat_ident( rotmat );	

	poly->initialize(); // initialize everything

	poly->calc_bounding_sphere();
	poly->calc_face_normals_and_area();
	poly->average_normals();

	// setup all the graphics stuff:
	InitGraphics();

	// create the display structures that will not change:
	InitLists();

	// init all the global variables used by Display():
	// this will also post a redisplay
	// it is important to call this before InitGlui()
	// so that the variables that glui will control are correct
	// when each glui widget is created
	Reset();

	// setup all the user interface stuff:
	InitGlui();

	// draw the scene once and wait for some interaction:
	// (will never return)
	glutMainLoop();

	// finalize the object if loaded
	if (poly != NULL)
		poly->finalize();

	// this is here to make the compiler happy:
	return 0;
}

//
// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display() from here -- let glutMainLoop() do it
//

void
Animate(void)
{
	// put animation stuff in here -- change some global variables
	// for Display() to find:
	// force a call to Display() next time it is convenient:
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

//
// glui buttons callback:
//
void
Buttons(int id)
{
	switch (id)
	{
	case RESET:
		Reset();
		Glui->sync_live();
		glutSetWindow(MainWindow);
		glutPostRedisplay();
		break;

	case QUIT:
		// gracefully close the glui window:
		// gracefully close out the graphics:
		// gracefully close the graphics window:
		// gracefully exit the program:
		Glui->close();
		glutSetWindow(MainWindow);
		glFinish();
		glutDestroyWindow(MainWindow);
		exit(0);
		break;
	default:
		fprintf(stderr, "Don't know what to do with Button ID %d\n", id);
	}

}
//
// draw the complete scene:
//
void Display(void)
{
	GLsizei vx, vy, v;		// viewport dimensions
	GLint xl, yb;		// lower-left corner of viewport
	GLfloat scale2;		// real glui scale factor

	if (DebugOn != 0)
	{
		fprintf(stderr, "Display\n");
	}

	// set which window we want to do the graphics into:
	glutSetWindow(MainWindow);

	// erase the background:
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// specify shading to be flat:
	glShadeModel(GL_FLAT);

	// set the viewport to a square centered in the window:
	vx = glutGet(GLUT_WINDOW_WIDTH);
	vy = glutGet(GLUT_WINDOW_HEIGHT);
	v = vx < vy ? vx : vy;			// minimum dimension
	xl = (vx - v) / 2;
	yb = (vy - v) / 2;
	glViewport(xl, yb, v, v);

	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D() IF YOU ARE DOING 2D !
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (WhichProjection == ORTHO)
		glOrtho(-3., 3., -3., 3., 0.1, 1000.);
	else
		gluPerspective(90., 1., 0.1, 1000.);

	// place the objects into the scene:
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// set the eye position, look-at position, and up-vector:
	// IF DOING 2D, REMOVE THIS -- OTHERWISE ALL YOUR 2D WILL DISAPPEAR !
	gluLookAt(0., 0., 3., 0., 0., 0., 0., 1., 0.);

	// translate the objects in the scene:
	// note the minus sign on the z value
	// this is to make the appearance of the glui z translate
	// widget more intuitively match the translate behavior
	// DO NOT TRANSLATE IN Z IF YOU ARE DOING 2D !
	glTranslatef((GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2]);

	// rotate the scene:
	// DO NOT ROTATE (EXCEPT ABOUT Z) IF YOU ARE DOING 2D !
	glRotatef((GLfloat)Yrot, 0., 1., 0.);
	glRotatef((GLfloat)Xrot, 1., 0., 0.);
	glMultMatrixf((const GLfloat *)RotMatrix);

	// uniformly scale the scene:
	glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);
	scale2 = 1. + Scale2;		// because glui translation starts at 0.
	if (scale2 < MINSCALE)
		scale2 = MINSCALE;
	glScalef((GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2);

	// set the fog parameters:
	// DON'T NEED THIS IF DOING 2D !
	if (DepthCueOn != 0)
	{
		glFogi(GL_FOG_MODE, FOGMODE);
		glFogfv(GL_FOG_COLOR, FOGCOLOR);
		glFogf(GL_FOG_DENSITY, FOGDENSITY);
		glFogf(GL_FOG_START, FOGSTART);
		glFogf(GL_FOG_END, FOGEND);
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}

	// Let us disable lighting right now
	glDisable(GL_LIGHTING);

	// possibly draw the axes:
	if (AxesOn != 0)
	{
		glColor3fv(&Colors[WhichColor][0]);
		glCallList(AxesList);
	}

	// set the color of the object:
	glColor3fv(Colors[WhichColor]);

	// draw the current object:
	//glCallList( BoxList );

	/*// draw some gratuitous text that just rotates on top of the scene:
	glDisable( GL_DEPTH_TEST );
	glColor3f( 0., 1., 1. );
	DoRasterString( 0., 1., 0., "Text That Moves" );

	// draw some gratuitous text that is fixed on the screen:
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	// this is called "percent units", and is just a convenience
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0., 100.,     0., 100. );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glColor3f( 1., 1., 1. );
	DoRasterString( 5., 5., 0., "Text That Doesn't" );*/

	// Render the loaded object
	set_view(GL_RENDER, poly);
	glTranslatef(0.0, 0.0, -3.0);
	//glTranslatef(0.0, 0.0, 0.0);
	glTranslatef((GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2]);
	//glTranslatef(poly->center.entry[0], poly->center.entry[1], poly->center.entry[2]);
	glScalef((GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2);
	glRotatef((GLfloat)Yrot, 0., 1., 0.);
	glRotatef((GLfloat)Xrot, 1., 0., 0.);
	glMultMatrixf((const GLfloat *)RotMatrix);
	//glScalef(1.0/poly->radius, 1.0/poly->radius, 1.0/poly->radius);
	//glTranslatef(-poly->center.entry[0], -poly->center.entry[1], -poly->center.entry[2]);

	DetermineVisibility();
	display_shape(GL_RENDER, poly);

	// swap the double-buffered framebuffers:
	glutSwapBuffers();

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush() here, not glFinish() !
	glFlush();
}
//
// use glut to display a string of characters using a raster font:
void
DoRasterString(float x, float y, float z, char *s)
{
	char c;			// one character to print

	glRasterPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);
	for (; (c = *s) != '\0'; s++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
	}
}



//
// use glut to display a string of characters using a stroke font:
//

void
DoStrokeString(float x, float y, float z, float ht, char *s)
{
	char c;			// one character to print
	float sf;		// the scale factor

	glPushMatrix();
	glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
	sf = ht / (119.05 + 33.33);
	glScalef((GLfloat)sf, (GLfloat)sf, (GLfloat)sf);
	for (; (c = *s) != '\0'; s++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
	}
	glPopMatrix();
}
//
// return the number of seconds since the start of the program:
//

float
ElapsedSeconds(void)
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet(GLUT_ELAPSED_TIME);

	// convert it to seconds:

	return (float)ms / 1000.;
}

//
// initialize the glui window:
//

void
InitGlui(void)
{
	GLUI_Panel *panel;
	GLUI_Panel *panel1;
	GLUI_RadioGroup *group;
	GLUI_Rotation *rot;
	GLUI_Translation *trans, *scale;


	float *distCentrevalues = new float[NX*NY*NZ];
	int iDistC = 0;

	float *gradCentreValues = new float[NX*NY*NZ];
	int igradC = 0;
	// setup the glui window:

	glutInitWindowPosition(INIT_WINDOW_SIZE + 50, 0);
	Glui = GLUI_Master.create_glui((char *)GLUITITLE);


	Glui->add_statictext((char *)GLUITITLE);
	Glui->add_separator();

	Glui->add_checkbox("Axes", &AxesOn);

	Glui->add_checkbox("Perspective", &WhichProjection);

	Glui->add_checkbox("Intensity Scale Depth Cue", &DepthCueOn);

	// Add a rollout for the axes color
	GLUI_Rollout *rollout = Glui->add_rollout(" Axes Color ", 0);

	//panel = Glui->add_panel(  "Axes Color" );
	//GLUI_Rollout *rollout = Glui->add_rollout_to_panel(panel,  "Axes Color", 1 );
	//group = Glui->add_radiogroup_to_panel( panel, &WhichColor );

	group = Glui->add_radiogroup_to_panel(rollout, &WhichColor);
	Glui->add_radiobutton_to_group(group, "Red");
	Glui->add_radiobutton_to_group(group, "Yellow");
	Glui->add_radiobutton_to_group(group, "Green");
	Glui->add_radiobutton_to_group(group, "Cyan");
	Glui->add_radiobutton_to_group(group, "Blue");
	Glui->add_radiobutton_to_group(group, "Magenta");
	Glui->add_radiobutton_to_group(group, "White");
	Glui->add_radiobutton_to_group(group, "Black");


	//**ADD a list of different color maps implemented
	generate_data();
	temporary();
	panel = Glui->add_panel("Choose Color Scheme");
	GLUI_RadioGroup *color_list = Glui->add_radiogroup_to_panel(panel, &ObID, -1, (GLUI_Update_CB)Color_Object);
	Glui->add_radiobutton_to_group(color_list, "Rainbow");
	Glui->add_radiobutton_to_group(color_list, "Blue White Red");
	Glui->add_radiobutton_to_group(color_list, "Orange White Cyan");
	//Glui->add_checkbox( "New Color Mapping", &new_mapping );




	// sliders
	Slider1();
	Slider2();

	//Volume slicing
	panel = Glui->add_panel("Cutting Planes");
	checkXY = Glui->add_checkbox_to_panel(panel, "XY", &XY_visible);
	Glui->add_column_to_panel(panel, true);

	checkYZ = Glui->add_checkbox_to_panel(panel, "YZ", &YZ_visible);
	Glui->add_column_to_panel(panel, true);

	checkZX = Glui->add_checkbox_to_panel(panel, "XZ", &XZ_visible);
	char str[128];
	panel1 = Glui->add_panel("GLUI Range Sliders");
	XSlider = Glui->add_slider_to_panel(panel1, true, GLUI_HSLIDER_FLOAT, XLowHigh, X_Slider, (GLUI_Update_CB)Sliders);
	XSlider->set_float_limits(-1, 1);
	XSlider->set_w(200);
	sprintf(str, XFORMAT, XLowHigh[0], XLowHigh[1]);
	XLabel = Glui->add_statictext_to_panel(panel1, str);

	YSlider = Glui->add_slider_to_panel(panel1, true, GLUI_HSLIDER_FLOAT, YLowHigh, Y_Slider, (GLUI_Update_CB)Sliders);
	YSlider->set_float_limits(-1, 1);
	YSlider->set_w(200);
	sprintf(str, YFORMAT, YLowHigh[0], YLowHigh[1]);
	YLabel = Glui->add_statictext_to_panel(panel1, str);

	ZSlider = Glui->add_slider_to_panel(panel1, true, GLUI_HSLIDER_FLOAT, ZLowHigh, Z_Slider, (GLUI_Update_CB)Sliders);
	ZSlider->set_float_limits(-1, 1);
	ZSlider->set_w(200);
	sprintf(str, ZFORMAT, ZLowHigh[0], ZLowHigh[1]);
	ZLabel = Glui->add_statictext_to_panel(panel1, str);

	TempLowHigh[0] = TEMPMIN;
	TempLowHigh[1] = TEMPMAX;
	temp_slider = Glui->add_slider_to_panel(panel1, true, GLUI_HSLIDER_FLOAT, TempLowHigh, Temp_Sliders, (GLUI_Update_CB)Sliders);
	temp_slider->set_float_limits(TEMPMIN, TEMPMAX);
	temp_slider->set_w(200);
	sprintf(str, TEMPFORMAT, TempLowHigh[0], TempLowHigh[1]);
	TempLabel = Glui->add_statictext_to_panel(panel1, str);


	//Dist Slider	


	DistLowHigh[0] = DISTMIN;
	DistLowHigh[1] = DISTMAX + 2.0;
	dist_sliders = Glui->add_slider_to_panel(panel1, true, GLUI_HSLIDER_FLOAT, DistLowHigh, Distance_Slider, (GLUI_Update_CB)Sliders);

	dist_sliders->set_float_limits(DISTMIN - 0.001, DISTMAX);
	dist_sliders->set_w(200);

	sprintf(str, DISTFORMAT, DistLowHigh[0], DistLowHigh[1]);
	DistLabel = Glui->add_statictext_to_panel(panel1, str);
	//Grad Slider


	GradLowHigh[0] = GRADMIN - 0.001;
	GradLowHigh[1] = GRADMAX - 1.0;
	grad_sliders = Glui->add_slider_to_panel(panel1, true, GLUI_HSLIDER_FLOAT, GradLowHigh, Gradient_Slider, (GLUI_Update_CB)Sliders);
	grad_sliders->set_w(200);

	grad_sliders->set_float_limits(GRADMIN - 0.001, GRADMAX);
	sprintf(str, GRADFORMAT, GradLowHigh[0], GradLowHigh[1]);
	GradLabel = Glui->add_statictext_to_panel(panel1, str);

	panel = Glui->add_panel("Choose Visualisation Technique");
	GLUI_Listbox *obj_list_2 = Glui->add_listbox_to_panel(panel, "Techniques", &TId, -1, (GLUI_Update_CB)Choose_Technique);
	obj_list_2->add_item(0, "Cutting Plane");
	obj_list_2->add_item(1, "Wire-Frame");
	obj_list_2->add_item(2, "Volume Rendering");

	//Wire-Frames

	panel = Glui->add_panel("Wire-Frames ");
	contour_scalar_spin = Glui->add_spinner_to_panel(panel, "color", GLUI_SPINNER_FLOAT, &contourValue);
	contour_scalar_spin->set_float_limits(0, 100);

	panel = Glui->add_panel(" Texture Volume Rendering ");
	Glui->add_checkbox_to_panel(panel, "Bilinear", &showbilinear);
	opac_slider = Glui->add_slider_to_panel(panel, false, GLUI_HSLIDER_FLOAT, &MaxAlpha, Opacity_Slider, (GLUI_Update_CB)Sliders);
	opac_slider->set_float_limits(0, 3);
	sprintf(str, OPACITYFORMAT, MaxAlpha);
	OpacLabel = Glui->add_statictext_to_panel(panel, str);

	panel = Glui->add_panel("Object Transformation");
	rot = Glui->add_rotation_to_panel(panel, "Rotation", (float *)RotMatrix);
	rot->set_spin(1.0);

	Glui->add_column_to_panel(panel, GLUIFALSE);
	scale = Glui->add_translation_to_panel(panel, "Scale", GLUI_TRANSLATION_Y, &Scale2);
	scale->set_speed(0.005f);

	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Trans XY", GLUI_TRANSLATION_XY, &TransXYZ[0]);
	trans->set_speed(0.05f);

	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Trans Z", GLUI_TRANSLATION_Z, &TransXYZ[2]);
	trans->set_speed(0.05f);

	Glui->add_checkbox("Debug", &DebugOn);

	panel = Glui->add_panel("", GLUIFALSE);
	Glui->add_button_to_panel(panel, "Reset", RESET, (GLUI_Update_CB)Buttons);
	Glui->add_column_to_panel(panel, GLUIFALSE);
	Glui->add_button_to_panel(panel, "Quit", QUIT, (GLUI_Update_CB)Buttons);

	// tell glui what graphics window it needs to post a redisplay to:
	Glui->set_main_gfx_window(MainWindow);

	// set the graphics window's idle function:
	GLUI_Master.set_glutIdleFunc(NULL);
}

//
// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions
//

void
InitGraphics(void)
{
	// setup the display mode:
	// ( *must* be done before call to glutCreateWindow() )
	// ask for color, double-buffering, and z-buffering:

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);


	// set the initial window configuration:

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);


	// open the window and set its title:

	MainWindow = glutCreateWindow(WINDOWTITLE);
	glutSetWindowTitle(WINDOWTITLE);


	// setup the clear values:	
	glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);

	// setup the callback routines:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow(MainWindow);
	glutDisplayFunc(Display);
	//glutDisplayFunc( Display_Model );
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(NULL);
	glutVisibilityFunc(Visibility);
	glutEntryFunc(NULL);
	glutSpecialFunc(NULL);
	glutSpaceballMotionFunc(NULL);
	glutSpaceballRotateFunc(NULL);
	glutSpaceballButtonFunc(NULL);
	glutButtonBoxFunc(NULL);
	glutDialsFunc(NULL);
	glutTabletMotionFunc(NULL);
	glutTabletButtonFunc(NULL);
	glutMenuStateFunc(NULL);
	glutTimerFunc(0, NULL, 0);

	// DO NOT SET THE GLUT IDLE FUNCTION HERE !!
	// glutIdleFunc( NULL );
	// let glui take care of it in InitGlui()
}




//
// initialize the display lists that will not change:
//

void
InitLists(void)
{
	float dx = BOXSIZE / 2.;
	float dy = BOXSIZE / 2.;
	float dz = BOXSIZE / 2.;

	// create the object:

	BoxList = glGenLists(1);
	glNewList(BoxList, GL_COMPILE);

	glBegin(GL_QUADS);

	glColor3f(0., 0., 1.);
	glNormal3f(0., 0., 1.);
	glVertex3f(-dx, -dy, dz);
	glVertex3f(dx, -dy, dz);
	glVertex3f(dx, dy, dz);
	glVertex3f(-dx, dy, dz);

	glNormal3f(0., 0., -1.);
	glTexCoord2f(0., 0.);
	glVertex3f(-dx, -dy, -dz);
	glTexCoord2f(0., 1.);
	glVertex3f(-dx, dy, -dz);
	glTexCoord2f(1., 1.);
	glVertex3f(dx, dy, -dz);
	glTexCoord2f(1., 0.);
	glVertex3f(dx, -dy, -dz);

	glColor3f(1., 0., 0.);
	glNormal3f(1., 0., 0.);
	glVertex3f(dx, -dy, dz);
	glVertex3f(dx, -dy, -dz);
	glVertex3f(dx, dy, -dz);
	glVertex3f(dx, dy, dz);

	glNormal3f(-1., 0., 0.);
	glVertex3f(-dx, -dy, dz);
	glVertex3f(-dx, dy, dz);
	glVertex3f(-dx, dy, -dz);
	glVertex3f(-dx, -dy, -dz);

	glColor3f(0., 1., 0.);
	glNormal3f(0., 1., 0.);
	glVertex3f(-dx, dy, dz);
	glVertex3f(dx, dy, dz);
	glVertex3f(dx, dy, -dz);
	glVertex3f(-dx, dy, -dz);

	glNormal3f(0., -1., 0.);
	glVertex3f(-dx, -dy, dz);
	glVertex3f(-dx, -dy, -dz);
	glVertex3f(dx, -dy, -dz);
	glVertex3f(dx, -dy, dz);

	glEnd();

	glEndList();


	// create the axes:

	/*AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
	glLineWidth( AXES_WIDTH );
	Axes( 1.5 );
	glLineWidth( 1. );
	glEndList();*/
}

// the keyboard callback:
//

void
Keyboard(unsigned char c, int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

	switch (c)
	{
	case 'o':
	case 'O':
		WhichProjection = ORTHO;
		break;

	case 'p':
	case 'P':
		WhichProjection = PERSP;
		break;

	case 'q':
	case 'Q':
	case ESCAPE:
		Buttons(QUIT);	// will not return here
		break;			// happy compiler

	case 'r':
	case 'R':
		LeftButton = ROTATE;
		break;

	case 's':
	case 'S':
		LeftButton = SCALE;
		break;

	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}


	// synchronize the GLUI display with the variables:

	Glui->sync_live();


	// force a call to Display():

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}



//
// called when the mouse button transitions down or up:
//

void
MouseButton(int button, int state, int x, int y)
{
	int b;			// LEFT, MIDDLE, or RIGHT

	if (DebugOn != 0)
		fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);


	// get the proper button bit mask:

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		b = LEFT;		break;

	case GLUT_MIDDLE_BUTTON:
		b = MIDDLE;		break;

	case GLUT_RIGHT_BUTTON:
		b = RIGHT;		break;

	default:
		b = 0;
		fprintf(stderr, "Unknown mouse button: %d\n", button);
	}


	// button down sets the bit, up clears the bit:

	if (state == GLUT_DOWN)
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}
}



//
// called when the mouse moves while a button is down:
//

void
MouseMotion(int x, int y)
{
	int dx, dy;		// change in mouse coordinates

	if (DebugOn != 0)
		fprintf(stderr, "MouseMotion: %d, %d\n", x, y);


	dx = x - Xmouse;		// change in mouse coords
	dy = y - Ymouse;

	if ((ActiveButton & LEFT) != 0)
	{
		switch (LeftButton)
		{
		case ROTATE:
			Xrot += (ANGFACT*dy);
			Yrot += (ANGFACT*dx);
			break;

		case SCALE:
			Scale += SCLFACT * (float)(dx - dy);
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;
		}
	}


	if ((ActiveButton & MIDDLE) != 0)
	{
		Scale += SCLFACT * (float)(dx - dy);

		// keep object from turning inside-out or disappearing:

		if (Scale < MINSCALE)
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}



//
// reset the transformations and the colors:
//
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene
//

void
Reset(void)
{
	ActiveButton = 0;
	AxesOn = GLUITRUE;
	DebugOn = GLUIFALSE;
	DepthCueOn = GLUIFALSE;
	LeftButton = ROTATE;
	Scale = 1.0;
	Scale2 = 0.0;		// because we add 1. to it in Display()
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	TransXYZ[0] = TransXYZ[1] = TransXYZ[2] = 0.;

	RotMatrix[0][1] = RotMatrix[0][2] = RotMatrix[0][3] = 0.;
	RotMatrix[1][0] = RotMatrix[1][2] = RotMatrix[1][3] = 0.;
	RotMatrix[2][0] = RotMatrix[2][1] = RotMatrix[2][3] = 0.;
	RotMatrix[3][0] = RotMatrix[3][1] = RotMatrix[3][3] = 0.;
	RotMatrix[0][0] = RotMatrix[1][1] = RotMatrix[2][2] = RotMatrix[3][3] = 1.;
}



//
// called when user resizes the window:
//

void
Resize(int width, int height)
{
	if (DebugOn != 0)
		fprintf(stderr, "ReSize: %d, %d\n", width, height);

	// don't really need to do anything since window size is
	// checked each time in Display():

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


//
// handle a change to the window's visibility:
//

void
Visibility(int state)
{
	if (DebugOn != 0)
		fprintf(stderr, "Visibility: %d\n", state);

	if (state == GLUT_VISIBLE)
	{
		glutSetWindow(MainWindow);
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}




//////////////////////////////////////////  EXTRA HANDY UTILITIES:  /////////////////////////////

// size of wings as fraction of length:

#define WINGS	0.10


// axes:

#define X	1
#define Y	2
#define Z	3


// x, y, z, axes:

static float axx[3] = { 1., 0., 0. };
static float ayy[3] = { 0., 1., 0. };
static float azz[3] = { 0., 0., 1. };


void
Arrow(float tail[3], float head[3])
{
	float u[3], v[3], w[3];		// arrow coordinate system
	float d;			// wing distance
	float x, y, z;			// point to plot
	float mag;			// magnitude of major direction
	float f;			// fabs of magnitude
	int axis;			// which axis is the major


	// set w direction in u-v-w coordinate system:

	w[0] = head[0] - tail[0];
	w[1] = head[1] - tail[1];
	w[2] = head[2] - tail[2];


	// determine major direction:

	axis = X;
	mag = fabs(w[0]);
	if ((f = fabs(w[1]))  > mag)
	{
		axis = Y;
		mag = f;
	}
	if ((f = fabs(w[2]))  > mag)
	{
		axis = Z;
		mag = f;
	}


	// set size of wings and turn w into a Unit vector:
	d = WINGS * Unit(w, w);

	// draw the shaft of the arrow:
	glBegin(GL_LINE_STRIP);
	glVertex3fv(tail);
	glVertex3fv(head);
	glEnd();

	// draw two sets of wings in the non-major directions:

	if (axis != X)
	{
		Cross(w, axx, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}


	if (axis != Y)
	{
		Cross(w, ayy, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}

	if (axis != Z)
	{
		Cross(w, azz, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}
}



float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}



void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];

	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];

	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}



float
Unit(float vin[3], float vout[3])
{
	float dist, f;

	dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];

	if (dist > 0.0)
	{
		dist = sqrt(dist);
		f = 1. / dist;
		vout[0] = f * vin[0];
		vout[1] = f * vin[1];
		vout[2] = f * vin[2];
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}

	return dist;
}



// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = {
	0.f, 1.f, 0.f, 1.f
};

static float xy[] = {
	-.5f, .5f, .5f, -.5f
};

static int xorder[] = {
	1, 2, -3, 4
};


static float yx[] = {
	0.f, 0.f, -.5f, .5f
};

static float yy[] = {
	0.f, .6f, 1.f, 1.f
};

static int yorder[] = {
	1, 2, 3, -2, 4
};


static float zx[] = {
	1.f, 0.f, 1.f, 0.f, .25f, .75f
};

static float zy[] = {
	.5f, .5f, -.5f, -.5f, 0.f, 0.f
};

static int zorder[] = {
	1, 2, 3, 4, -5, 6
};


// fraction of the length to use as height of the characters:

const float LENFRAC = 0.10f;


// fraction of length to use as start location of the characters:

const float BASEFRAC = 1.10f;


//
//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)
//

void Axes(float length)
{
	int i, j;			// counters
	float fact;			// character scale factor
	float base;			// character start location


	glBegin(GL_LINE_STRIP);
	glVertex3f(length, 0., 0.);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., length, 0.);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., 0., length);
	glEnd();

	fact = LENFRAC * length;
	base = BASEFRAC * length;

	glBegin(GL_LINE_STRIP);
	for (i = 0; i < 4; i++)
	{
		j = xorder[i];
		if (j < 0)
		{
			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(base + fact*xx[j], fact*xy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (i = 0; i < 5; i++)
	{
		j = yorder[i];
		if (j < 0)
		{
			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(fact*yx[j], base + fact*yy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (i = 0; i < 6; i++)
	{
		j = zorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(0.0, fact*zy[j], base + fact*zx[j]);
	}
	glEnd();

}

// routine to convert HSV to RGB
//
// Reference:  Foley, van Dam, Feiner, Hughes,
//		"Computer Graphics Principles and Practices,"
//		Additon-Wesley, 1990, pp592-593.

void HsvRgb(float hsv[3], float rgb[3])
{
	float h, s, v;			// hue, sat, value
	float r, g, b;			// red, green, blue
	float i, f, p, q, t;		// interim values

	// guarantee valid input:
	h = hsv[0] / 60.;
	while (h >= 6.)	h -= 6.;
	while (h <  0.) 	h += 6.;

	s = hsv[1];
	if (s < 0.)
		s = 0.;
	if (s > 1.)
		s = 1.;

	v = hsv[2];
	if (v < 0.)
		v = 0.;
	if (v > 1.)
		v = 1.;

	// if sat==0, then is a gray:
	if (s == 0.0)
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:	
	i = floor(h);
	f = h - i;
	p = v * (1. - s);
	q = v * (1. - s*f);
	t = v * (1. - (s * (1. - f)));
	switch ((int)i)
	{
	case 0:
		r = v;	g = t;	b = p;
		break;

	case 1:
		r = q;	g = v;	b = p;
		break;

	case 2:
		r = p;	g = v;	b = t;
		break;

	case 3:
		r = p;	g = q;	b = v;
		break;

	case 4:
		r = t;	g = p;	b = v;
		break;

	case 5:
		r = v;	g = p;	b = q;
		break;
	}

	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void set_view(GLenum mode, Polyhedron *poly)
{
	icVector3 up, ray, view;
	GLfloat light_ambient0[] = { 0.3, 0.3, 0.3, 1.0 };
	GLfloat light_diffuse0[] = { 0.7, 0.7, 0.7, 1.0 };
	GLfloat light_specular0[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_ambient1[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse1[] = { 0.5, 0.5, 0.5, 1.0 };
	GLfloat light_specular1[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_ambient2[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_diffuse2[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular2[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_position[] = { 0.0, 0.0, 0.0, 1.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular0);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular1);


	glMatrixMode(GL_PROJECTION);
	if (mode == GL_RENDER)
		glLoadIdentity();

	if (WhichProjection == ORTHO)
		glOrtho(-radius_factor, radius_factor, -radius_factor, radius_factor, 0.0, 40.0);
	else
		gluPerspective(45.0, 1.0, 0.1, 40.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	light_position[0] = 5.5;
	light_position[1] = 0.0;
	light_position[2] = 0.0;
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	light_position[0] = -0.1;
	light_position[1] = 0.0;
	light_position[2] = 0.0;
	glLightfv(GL_LIGHT2, GL_POSITION, light_position);
}

void set_scene(GLenum mode, Polyhedron *poly)
{
	glTranslatef(0.0, 0.0, -3.0);

	glScalef(1.0 / poly->radius, 1.0 / poly->radius, 1.0 / poly->radius);
	glTranslatef(-poly->center.entry[0], -poly->center.entry[1], -poly->center.entry[2]);
}




void CompositeXY()
{
	int x, y, z, zz;
	float alpha; /* opacity at this voxel */
	float r, g, b; /* running color composite */
	for (x = 0; x < NX; x++)
	{
		for (y = 0; y < NY; y++)
		{
			r = g = b = 0.;
			for (zz = 0; zz < NZ; zz++)
			{
				// which direction to composite: 
				if (Zside == PLUS)
					z = zz;
				else
					z = (NZ - 1) - zz;


				if (Nodes[x][y][z].x >= XLowHigh[0] && Nodes[x][y][z].x <= XLowHigh[1]
					&& Nodes[x][y][z].y >= YLowHigh[0] && Nodes[x][y][z].y <= YLowHigh[1]
					&& Nodes[x][y][z].z >= ZLowHigh[0] && Nodes[x][y][z].z <= ZLowHigh[1]
					&& Nodes[x][y][z].T > TempLowHigh[0] && Nodes[x][y][z].T <= TempLowHigh[1]
					&& Nodes[x][y][z].grad > GradLowHigh[0] && Nodes[x][y][z].grad <= GradLowHigh[1]
					&& Nodes[x][y][z].rad > DistLowHigh[0] && Nodes[x][y][z].rad <= DistLowHigh[1])
				{
					r = Nodes[x][y][z].r;
					g = Nodes[x][y][z].g;
					b = Nodes[x][y][z].b;
					alpha = MaxAlpha;
				}
				else
				{
					r = g = b = 0.;
					alpha = 0.;
				}
				TextureXY[zz][x][y][0] = (unsigned char)(255.*r + .5);
				TextureXY[zz][x][y][1] = (unsigned char)(255.*g + .5);
				TextureXY[zz][x][y][2] = (unsigned char)(255.*b + .5);
				TextureXY[zz][x][y][3] = (unsigned char)(255.*alpha + .5);
			}
		}
	}
}

void CompositeYZ()
{
	int x, xx, y, z;
	float alpha; /* opacity at this voxel */
	float r, g, b; /* running color composite */

	for (y = 0; y < NY; y++)
	{
		for (z = 0; z < NZ; z++)
		{
			r = g = b = 0.;
			for (xx = 0; xx < NX; xx++)
			{
				// which direction to composite: 
				if (Xside == PLUS)
					x = xx;
				else
					x = (NX - 1) - xx;
				if (Nodes[x][y][z].x >= XLowHigh[0] && Nodes[x][y][z].x <= XLowHigh[1]
					&& Nodes[x][y][z].y >= YLowHigh[0] && Nodes[x][y][z].y <= YLowHigh[1]
					&& Nodes[x][y][z].z >= ZLowHigh[0] && Nodes[x][y][z].z <= ZLowHigh[1]
					&& Nodes[x][y][z].T > TempLowHigh[0] && Nodes[x][y][z].T <= TempLowHigh[1]
					&& Nodes[x][y][z].grad > GradLowHigh[0] && Nodes[x][y][z].grad <= GradLowHigh[1]
					&& Nodes[x][y][z].rad > DistLowHigh[0] && Nodes[x][y][z].rad <= DistLowHigh[1])
				{
					r = Nodes[x][y][z].r;
					g = Nodes[x][y][z].g;
					b = Nodes[x][y][z].b;
					alpha = MaxAlpha;
				}
				else
				{
					r = g = b = 0.;
					alpha = 0.;
				}
				TextureYZ[xx][y][z][0] = (unsigned char)(255.*r + .5);
				TextureYZ[xx][y][z][1] = (unsigned char)(255.*g + .5);
				TextureYZ[xx][y][z][2] = (unsigned char)(255.*b + .5);
				TextureYZ[xx][y][z][3] = (unsigned char)(255.*alpha + .5);
			}
		}
	}
}
void CompositeXZ()
{
	int x, y, yy, z;
	float alpha; /* opacity at this voxel */
	float r, g, b; /* running color composite */


	for (z = 0; z < NZ; z++)
	{
		for (x = 0; x < NX; x++)
		{
			r = g = b = 0.;
			for (yy = 0; yy < NY; yy++)
			{
				// which direction to composite: 
				if (Yside == PLUS)
					y = yy;
				else
					y = (NY - 1) - yy;

				if (Nodes[x][y][z].x >= XLowHigh[0] && Nodes[x][y][z].x <= XLowHigh[1]
					&& Nodes[x][y][z].y >= YLowHigh[0] && Nodes[x][y][z].y <= YLowHigh[1]
					&& Nodes[x][y][z].z >= ZLowHigh[0] && Nodes[x][y][z].z <= ZLowHigh[1]
					&& Nodes[x][y][z].T > TempLowHigh[0] && Nodes[x][y][z].T <= TempLowHigh[1]
					&& Nodes[x][y][z].grad > GradLowHigh[0] && Nodes[x][y][z].grad <= GradLowHigh[1]
					&& Nodes[x][y][z].rad > DistLowHigh[0] && Nodes[x][y][z].rad <= DistLowHigh[1])
				{
					r = Nodes[x][y][z].r;
					g = Nodes[x][y][z].g;
					b = Nodes[x][y][z].b;
					alpha = MaxAlpha;
				}
				else
				{
					r = g = b = 0.;
					alpha = 0.;
				}
				TextureXZ[yy][x][z][0] = (unsigned char)(255.*r + .5);
				TextureXZ[yy][x][z][1] = (unsigned char)(255.*g + .5);
				TextureXZ[yy][x][z][2] = (unsigned char)(255.*b + .5);
				TextureXZ[yy][x][z][3] = (unsigned char)(255.*alpha + .5);
			}
		}
	}
}
void DetermineVisibility()
{
	float xr, yr;
	float cx, sx;
	float cy, sy;
	float nzx, nzy, nzz; /* z component of normal for x side, y side, and z side */
	xr = Xrot * (M_PI / 180.);
	yr = Yrot * (M_PI / 180.);
	cx = cos(xr);
	sx = sin(xr);
	cy = cos(yr);
	sy = sin(yr);
	nzx = -sy;
	nzy = sx * cy;
	nzz = cx * cy;
	/* which sides of the cube are showing:*/
	/* the Xside being shown to the user is MINUS or PLUS */
	Xside = (nzx > 0. ? PLUS : MINUS);
	Yside = (nzy > 0. ? PLUS : MINUS);
	Zside = (nzz > 0. ? PLUS : MINUS);
	/* which direction needs to be composited: */
	if (fabs(nzx) > fabs(nzy) && fabs(nzx) > fabs(nzz))
		Major = X;
	else if (fabs(nzy) > fabs(nzx) && fabs(nzy) > fabs(nzz))
		Major = Y;
	else
		Major = Z;
}

void generateVolumeX()
{

	float x0, dx;
	float xcoord;
	if (Xside == PLUS)
	{
		x0 = -1.;
		dx = 2. / (float)(NX - 1);
	}
	else
	{
		x0 = 1.;
		dx = -2. / (float)(NX - 1);
	}
	xcoord = x0;
	for (int x = 0; x < NX; x++)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, 4, NY, NZ, 0, GL_RGBA, GL_UNSIGNED_BYTE, &TextureYZ[x][0][0][0]);
		glBegin(GL_QUADS);
		glTexCoord2f(0., 0.);
		glVertex3f(xcoord, -1., -1.);
		glTexCoord2f(1., 0.);
		glVertex3f(xcoord, 1., -1.);
		glTexCoord2f(1., 1.);
		glVertex3f(xcoord, 1., 1.);
		glTexCoord2f(0., 1.);
		glVertex3f(xcoord, -1., 1.);
		glEnd();
		xcoord += dx;
	}

}
void generateVolumeY()
{
	float y0, dy;
	float ycoord;
	if (Yside == PLUS)
	{
		y0 = -1.;
		dy = 2. / (float)(NY - 1);
	}
	else
	{
		y0 = 1.;
		dy = -2. / (float)(NY - 1);
	}
	ycoord = y0;
	for (int y = 0; y < NY; y++)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, 4, NX, NZ, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, &TextureXZ[y][0][0][0]);
		glBegin(GL_QUADS);
		glTexCoord2f(0., 0.);
		glVertex3f(-1., ycoord, -1.);
		glTexCoord2f(1., 0.);
		glVertex3f(1., ycoord, -1.);
		glTexCoord2f(1., 1.);
		glVertex3f(1., ycoord, 1.);
		glTexCoord2f(0., 1.);
		glVertex3f(-1., ycoord, 1.);
		glEnd();
		ycoord += dy;
	}
}
void generateVolumeZ()
{
	float z0, dz;
	float zcoord;
	if (Zside == PLUS)
	{
		z0 = -1.;
		dz = 2. / (float)(NZ - 1);
	}
	else
	{
		z0 = 1.;
		dz = -2. / (float)(NZ - 1);
	}
	zcoord = z0;
	for (int z = 0; z < NZ; z++)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, 4, NX, NY, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, &TextureXY[z][0][0][0]);
		glBegin(GL_QUADS);
		glTexCoord2f(0., 0.);
		glVertex3f(-1., -1., zcoord);
		glTexCoord2f(1., 0.);
		glVertex3f(1., -1., zcoord);
		glTexCoord2f(1., 1.);
		glVertex3f(1., 1., zcoord);
		glTexCoord2f(0., 1.);
		glVertex3f(-1., 1., zcoord);
		glEnd();
		zcoord += dz;
	}
}

void generateVolume()
{
	CompositeXY();
	CompositeYZ();
	CompositeXZ();

	glDisable(GL_LIGHTING);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	GLfloat filter = GL_NEAREST;
	if (showbilinear)
		filter = GL_LINEAR;
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	if (Major == X)
		generateVolumeX();

	if (Major == Y)
		generateVolumeY();

	if (Major == Z)
		generateVolumeZ();



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}
void get_wireframe_lines(float *s, float *wx, float *wy, float *wz, float svalue, int length)
{
	int crossing = 0, k = 0, i;
	float t, xs[4], ys[4], zs[4];
	i = 0;
	while( i<length)
	{
		int j = i + 1;
		if (i>(length / 2)) j = 0;
		float mv = min(s[i], s[j]);
		float maxV = max(s[i], s[j]);
		if (svalue >= mv && svalue <= maxV)
		{
			crossing++;

		}
		i++;
	}
	if (crossing == 2)

	{
		i = 0;
		while( i<length)
		{
			int j = i + 1;
			if (i>(length / 2)) j = 0;
			float mv = min(s[i], s[j]);
			float maxV = max(s[i], s[j]);
			if (svalue >= mv && svalue <= maxV)
			{
				t = (svalue - s[i]) / (s[j] - s[i]);
				xs[k] = (1 - t)*wx[i] + t*wx[j];
				ys[k] = (1 - t)*wy[i] + t*wy[j];
				zs[k] = (1 - t)*wz[i] + t*wz[j];
				k++;
			}
			i++;
		}
		glBegin(GL_LINES);
		glColor3f(1.0, 1.0, 1.0);
		glVertex3f(xs[0], ys[0], zs[0]);
		glVertex3f(xs[1], ys[1], zs[1]);
		glEnd();

	}

	if (length == 4 && crossing == 4)
	{
		i = 0;
		while( i<length)
		{
			int j = i + 1;
			if (i>(length / 2)) j = 0;
			float mv = min(s[i], s[j]);
			float maxV = max(s[i], s[j]);
			if (svalue >= mv && svalue <= maxV)
			{
				t = (svalue - s[i]) / (s[j] - s[i]);
				xs[k] = (1 - t)*wx[i] + t*wx[j];
				ys[k] = (1 - t)*wy[i] + t*wy[j];
				zs[k] = (1 - t)*wz[i] + t*wz[j];
				k++;
			}
			i++;
		}

		float avg = (s[0] + s[1] + s[2] + s[3]) / 4;
		if ((s[0] < avg && svalue < avg) || (s[0] > avg && svalue > avg))
		{
			glBegin(GL_LINES);
			glColor3f(1.0, 1.0, 1.0);
			glVertex3f(xs[0], ys[0], zs[0]);
			glVertex3f(xs[3], ys[3], zs[3]);
			glEnd();

			glBegin(GL_LINES);
			glColor3f(1.0, 1.0, 1.0);
			glVertex3f(xs[1], ys[1], zs[1]);
			glVertex3f(xs[2], ys[2], zs[2]);
			glEnd();

		}
		else
		{
			glBegin(GL_LINES);
			glColor3f(1.0, 1.0, 1.0);
			glVertex3f(xs[0], ys[0], zs[0]);
			glVertex3f(xs[1], ys[1], zs[1]);
			glEnd();

			glBegin(GL_LINES);
			glColor3f(1.0, 1.0, 1.0);
			glVertex3f(xs[2], ys[2], zs[2]);
			glVertex3f(xs[3], ys[3], zs[3]);
			glEnd();
		}
	}


}
void generateWireFrames()
{
	float s[4], xval[4], yval[4], zval[4];
	int wx[4], wy[4], wz[4];
	for (int k = 0; k<NZ; k++)
		for (int i = 0; i<NX - 1; i++)
			for (int j = 0; j<NY - 1; j++)
			{
				wx[0] = i;	wx[1] = i; wx[2] = i + 1; wx[3] = i + 1;
				wy[0] = j; wy[1] = j + 1; wy[2] = j + 1; wy[3] = j;
				wz[0] = k; wz[1] = k; wz[2] = k; wz[3] = k;
				for (int i = 0; i<4; i++)
				{
					s[i] = Nodes[wx[i]][wy[i]][wz[i]].T;
					xval[i] = Nodes[wx[i]][wy[i]][wz[i]].x;
					yval[i] = Nodes[wx[i]][wy[i]][wz[i]].y;
					zval[i] = Nodes[wx[i]][wy[i]][wz[i]].z;
				}

				get_wireframe_lines(s, xval, yval, zval, contourValue, 4);
			}
	for (int j = 0; j<NY; j++)
		for (int i = 0; i<NX - 1; i++)
			for (int k = 0; k<NZ - 1; k++)
			{
				wx[0] = i;	wx[1] = i + 1; wx[2] = i + 1; wx[3] = i;
				wy[0] = j; wy[1] = j; wy[2] = j; wy[3] = j;
				wz[0] = k; wz[1] = k; wz[2] = k + 1; wz[3] = k + 1;
				for (int i = 0; i<4; i++)
				{
					s[i] = Nodes[wx[i]][wy[i]][wz[i]].T;
					xval[i] = Nodes[wx[i]][wy[i]][wz[i]].x;
					yval[i] = Nodes[wx[i]][wy[i]][wz[i]].y;
					zval[i] = Nodes[wx[i]][wy[i]][wz[i]].z;
				}

				get_wireframe_lines(s, xval, yval, zval, contourValue, 4);

			}
	for (int i = 0; i<NX; i++)
		for (int j = 0; j<NY - 1; j++)
			for (int k = 0; k<NZ - 1; k++)
			{
				wx[0] = i;	wx[1] = i; wx[2] = i; wx[3] = i;
				wy[0] = j; wy[1] = j; wy[2] = j + 1; wy[3] = j + 1;
				wz[0] = k; wz[1] = k + 1; wz[2] = k + 1; wz[3] = k;
				for (int i = 0; i<4; i++)
				{
					s[i] = Nodes[wx[i]][wy[i]][wz[i]].T;
					xval[i] = Nodes[wx[i]][wy[i]][wz[i]].x;
					yval[i] = Nodes[wx[i]][wy[i]][wz[i]].y;
					zval[i] = Nodes[wx[i]][wy[i]][wz[i]].z;
				}

				get_wireframe_lines(s, xval, yval, zval, contourValue, 4);
			}
}

float Temperature(float x, float y, float z)
{
	float t = 0.0;
	for (int i = 0; i < 4; i++)
	{
		float dx = x - Sources[i].xc;
		float dy = y - Sources[i].yc;
		float dz = z - Sources[i].zc;
		float rsqd = dx*dx + dy*dy + dz*dz;
		t += Sources[i].a * exp(-5.*rsqd);
	}
	if (t > TEMPMAX)
		t = TEMPMAX;
	return t;
}

float getGradient(float dx, float dy, float dz)
{
	return sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));
}

float* MinMax(float* values, int number_vertices)
{
	double min = MAXINT, max = MININT;
	int num = 0;

	while(num<number_vertices )
	{
		if (values[num] <= min)
			min = values[num];
		num++;
	}
	num = 0;
	while(num<number_vertices)
	{
		if (values[num] >= max)
			max = values[num];
		num++;
	}
	float* value = new float[2];
	value[0] = min;
	value[1] = max;
	return value;
}


void Slider2()
{
	float *gradCentreValues = new float[NX*NY*NZ];
	int igradC = 0, i = 0;
	while (i < NX){
		int j = 0;
		while (j < NY){
			int k = 0;
			while(k < NZ)
			{gradCentreValues[igradC++] = Nodes[i][j][k].grad;
				k++;}
			j++;}
		i++;}
	float* values = MinMax(gradCentreValues, NX*NY*NZ);
	GRADMIN = values[0];
	GRADMAX = values[1];
	delete gradCentreValues, values;
}

void Slider1()
{
	float *distCentreValues = new float[NX*NY*NZ];
	int iDistC = 0, i = 0, j,k;
	while (i < NX){
		j = 0;
		while( j < NY){
			k = 0;
			while(k < NZ){
				distCentreValues[iDistC++] = Nodes[i][j][k].rad;
				k++;
			}
			j++;
		}
		i++;
	}
	float* values = MinMax(distCentreValues, NX*NY*NZ);
	printf("rad: %f, %f: ", values[0], values[1]);
	DISTMIN = values[0];
	DISTMAX = values[1];
	delete distCentreValues, values;
}



float* Findrgb(float temp_s, float tMax, float tMin)
{
	float avg = (tMax - tMin) / 2;
	float *hsv;
	float *rgb;
	hsv = new float[3];
	rgb = new float[3];
	float maxV = tMax;
	switch (VColor)
	{
	case 0:
		hsv[0] = 240. - (240. * ((temp_s - tMin) / (tMax - tMin)));
		hsv[1] = 1; hsv[2] = 1;
		break;
	case 1:
		if (temp_s > avg)
		{
			hsv[0] = 0;
			hsv[1] = ((temp_s - avg) / (tMax - avg));
		}
		else if (temp_s < avg)
		{
			hsv[0] = 240;
			hsv[1] = (avg - temp_s) / (avg - tMin);
		}
		hsv[2] = 1;
		break;
	case 2:
		if (temp_s < avg)
		{
			hsv[0] = 20;
			hsv[1] = (avg - temp_s) / (avg - tMin);
		}

		else if (temp_s > avg)
		{
			hsv[0] = 180;
			hsv[1] = (temp_s - avg) / (tMax - avg);
		}

		hsv[2] = 1;
		break;
	}
	HsvRgb(hsv, rgb);
	delete hsv;
	return rgb;
}




void generate_data()
{
	float min_r = -1, max_r = 1;
	float r = max_r - min_r, val_x = min_r,val_y = min_r, val_z =min_r;
	float *xaxisV, *yaxisV, *zaxisV;
	xaxisV = new float[NX];
	yaxisV = new float[NY];
	zaxisV = new float[NZ];
	for (int i = 0; i<NX; i++)
	{
		xaxisV[i] = val_x;
		val_x += r / NX;
	}
	for (int i = 0; i<NY; i++)
	{
		yaxisV[i] = val_y;
		val_y += r / NY;
	}
	for (int i = 0; i<NZ; i++)
	{
		zaxisV[i] = val_z;
		val_z += r / NZ;
	}
	int index = 0;
	for (int i = 0; i<NX; i++)
		for (int j = 0; j<NY; j++)
			for (int k = 0; k<NZ; k++)
			{
				Nodes[i][j][k].x = xaxisV[i];
				Nodes[i][j][k].y = yaxisV[j];
				Nodes[i][j][k].z = zaxisV[k];
				Nodes[i][j][k].T = Temperature(xaxisV[i], yaxisV[j], zaxisV[k]);
				scalar[index++] = Nodes[i][j][k].T;
			}

	for (int i = 0; i<NX; i++)
		for (int j = 0; j<NY; j++)
			for (int k = 0; k<NZ; k++)
			{
				Nodes[i][j][k].dTdx = (Nodes[min(NX - 1, i + 1)][j][k].T - Nodes[max(0, i - 1)][j][k].T) / (Nodes[min(NX - 1, i + 1)][j][k].x - Nodes[max(0, i - 1)][j][k].x);
				Nodes[i][j][k].dTdy = (Nodes[i][min(NY - 1, j + 1)][k].T - Nodes[i][max(0, j - 1)][k].T) / (Nodes[i][min(NY - 1, j + 1)][k].y - Nodes[i][max(0, j - 1)][k].y);
				Nodes[i][j][k].dTdy = (Nodes[i][j][min(NZ - 1, k + 1)].T - Nodes[i][j][max(0, k - 1)].T) / (Nodes[i][j][min(NZ - 1, k + 1)].z - Nodes[i][j][max(0, k - 1)].z);
				Nodes[i][j][k].grad = getGradient(Nodes[i][j][k].dTdx, Nodes[i][j][k].dTdy, Nodes[i][j][k].dTdz);
				Nodes[i][j][k].rad = getGradient(Nodes[i][j][k].x, Nodes[i][j][k].y, Nodes[i][j][k].z);
			}
}

void simulate(int Xinit, int Yinit, int Zinit)
{

	float *rgb = new float[3];
	float* values = MinMax(scalar, NX*NY*NZ);
	for (int i = Xinit; i<NX; i++)
		for (int j = Yinit; j<NY; j++)
			for (int k = Zinit; k<NZ; k++)
			{
				if (Nodes[i][j][k].x >= XLowHigh[0] && Nodes[i][j][k].x <= XLowHigh[1]
					&& Nodes[i][j][k].y >= YLowHigh[0] && Nodes[i][j][k].y <= YLowHigh[1]
					&& Nodes[i][j][k].z >= ZLowHigh[0] && Nodes[i][j][k].z <= ZLowHigh[1]
					&& Nodes[i][j][k].T > TempLowHigh[0] && Nodes[i][j][k].T <= TempLowHigh[1]
					&& Nodes[i][j][k].grad > GradLowHigh[0] && Nodes[i][j][k].grad <= GradLowHigh[1]
					&& Nodes[i][j][k].rad > DistLowHigh[0] && Nodes[i][j][k].rad <= DistLowHigh[1])
					rgb = Findrgb(Nodes[i][j][k].T, values[0], values[1]);
				else
				{
					rgb[0] = 0;
					rgb[1] = 0;
					rgb[2] = 0;
				}
				Nodes[i][j][k].r = rgb[0];
				Nodes[i][j][k].g = rgb[1];
				Nodes[i][j][k].b = rgb[2];
			}
	delete rgb;
}

void generatePlaneX(int i, int Yinit, int Zinit){
	int j = Yinit, k;
	while( j < NY - 1){
		k = Zinit;
		while( k < NZ - 1)
		{
			glColor3f(Nodes[i][j][k].r, Nodes[i][j][k].g, Nodes[i][j][k].b);
			glVertex3d(Nodes[i][j][k].x, Nodes[i][j][k].y, Nodes[i][j][k].z);
			glColor3f(Nodes[i][j][k + 1].r, Nodes[i][j][k + 1].g, Nodes[i][j][k + 1].b);
			glVertex3d(Nodes[i][j][k + 1].x, Nodes[i][j][k + 1].y, Nodes[i][j][k + 1].z);
			glColor3f(Nodes[i][j + 1][k + 1].r, Nodes[i][j + 1][k + 1].g, Nodes[i][j + 1][k + 1].b);
			glVertex3d(Nodes[i][j + 1][k + 1].x, Nodes[i][j + 1][k + 1].y, Nodes[i][j + 1][k + 1].z);
			glColor3f(Nodes[i][j + 1][k].r, Nodes[i][j + 1][k].g, Nodes[i][j + 1][k].b);
			glVertex3d(Nodes[i][j + 1][k].x, Nodes[i][j + 1][k].y, Nodes[i][j + 1][k].z);
			k++;
		}
		j++;
	}
}
void generatePlaneY(int j,int Xinit,int Zinit){
	int i = Xinit,k;
	while (i < NX - 1){
		k = Zinit;
		while(k < NZ - 1)
		{
			glColor3f(Nodes[i][j][k].r, Nodes[i][j][k].g, Nodes[i][j][k].b);
			glVertex3d(Nodes[i][j][k].x, Nodes[i][j][k].y, Nodes[i][j][k].z);
			glColor3f(Nodes[i][j][k + 1].r, Nodes[i][j][k + 1].g, Nodes[i][j][k + 1].b);
			glVertex3d(Nodes[i][j][k + 1].x, Nodes[i][j][k + 1].y, Nodes[i][j][k + 1].z);
			glColor3f(Nodes[i + 1][j][k + 1].r, Nodes[i + 1][j][k + 1].g, Nodes[i + 1][j][k + 1].b);
			glVertex3d(Nodes[i + 1][j][k + 1].x, Nodes[i + 1][j][k + 1].y, Nodes[i + 1][j][k + 1].z);
			glColor3f(Nodes[i + 1][j][k].r, Nodes[i + 1][j][k].g, Nodes[i + 1][j][k].b);
			glVertex3d(Nodes[i + 1][j][k].x, Nodes[i + 1][j][k].y, Nodes[i + 1][j][k].z);
			k++;
		}
		i++;
	}
}

void generatePlaneZ(int k, int Xinit, int Yinit){
	int i = Xinit, j ;
	while (i < NX - 1){
		j = Yinit;
		while(j < NY - 1)
		{
			glColor3f(Nodes[i][j][k].r, Nodes[i][j][k].g, Nodes[i][j][k].b);
			glVertex3d(Nodes[i][j][k].x, Nodes[i][j][k].y, Nodes[i][j][k].z);
			glColor3f(Nodes[i][j + 1][k].r, Nodes[i][j + 1][k].g, Nodes[i][j + 1][k].b);
			glVertex3d(Nodes[i][j + 1][k].x, Nodes[i][j + 1][k].y, Nodes[i][j + 1][k].z);
			glColor3f(Nodes[i + 1][j + 1][k].r, Nodes[i + 1][j + 1][k].g, Nodes[i + 1][j + 1][k].b);
			glVertex3d(Nodes[i + 1][j + 1][k].x, Nodes[i + 1][j + 1][k].y, Nodes[i + 1][j + 1][k].z);
			glColor3f(Nodes[i + 1][j][k].r, Nodes[i + 1][j][k].g, Nodes[i + 1][j][k].b);
			glVertex3d(Nodes[i + 1][j][k].x, Nodes[i + 1][j][k].y, Nodes[i + 1][j][k].z);
			j++;
		}
		i++;
	}
}

void generateCuttingPlane()
{
	int number_vertices = NX*NY*NZ, Xinit = 0, Yinit = 0, Zinit = 0;
	
	float temp = 0;
	glEnable(GL_COLOR_MATERIAL);
	simulate(Xinit, Yinit, Zinit);
	glBegin(GL_QUADS);
	if (XY_visible == 1)
	{
		int k = planeZ;
		generatePlaneZ(k, Xinit, Yinit);
		
	}
	if (YZ_visible == 1)
	{
		int i = planeX;
		generatePlaneX(i, Yinit, Zinit);
	}
	if (XZ_visible == 1)
	{
		int j = planeY;
		generatePlaneY(j, Xinit, Zinit);
		
	}
	glEnd();
}

void Sliders(int id){
	char str[32];

	switch (id)
	{

	case X_Slider:{
		sprintf(str, XFORMAT, XLowHigh[0], XLowHigh[1]);
		XLabel->set_text(str);
	}
				  break;


	case Y_Slider:{
		sprintf(str, YFORMAT, YLowHigh[0], YLowHigh[1]);
		YLabel->set_text(str);
	}
				  break;
	case Z_Slider:{
		sprintf(str, ZFORMAT, ZLowHigh[0], ZLowHigh[1]);
		ZLabel->set_text(str);
	}
				  break;

	case Temp_Sliders:{
		sprintf(str, TEMPFORMAT, TempLowHigh[0], TempLowHigh[1]);
		TempLabel->set_text(str);
	}
					  break;


	case Gradient_Slider:{
		sprintf(str, GRADFORMAT, GradLowHigh[0], GradLowHigh[1]);
		GradLabel->set_text(str);
	}
						 break;

	case Distance_Slider:{
		if (XY_visible != 0)
			XY_visible = 1;
		sprintf(str, DISTFORMAT, DistLowHigh[0], DistLowHigh[1]);
		DistLabel->set_text(str);

	}
						 break;

	case Opacity_Slider:{
		CompositeXY();
		CompositeYZ();
		CompositeXZ();
		char str[128];
		sprintf(str, OPACITYFORMAT, MaxAlpha);
		OpacLabel->set_text(str);
	}
						break;

	}
	glutSetWindow(MainWindow);
	glutPostRedisplay();

}




void display_shape(GLenum mode, Polyhedron *this_poly)
{
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1., 1.);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	switch (TName){
	case 0: generateCuttingPlane();
		break;
	case 1: generateWireFrames();
		break;
	case 2: generateVolume();
		break;
	}
	/*switch (TId)

	for (i = 0; i<this_poly->ntris; i++) {
	if (mode == GL_SELECT)
	glLoadName(i + 1);

	Triangle *temp_t = this_poly->tlist[i];

	switch (display_mode) {
	case 0:
	if (i == this_poly->seed) {
	mat_diffuse[0] = 0.0;
	mat_diffuse[1] = 0.0;
	mat_diffuse[2] = 1.0;
	mat_diffuse[3] = 1.0;
	}
	else {
	mat_diffuse[0] = 0.6;
	mat_diffuse[1] = 0.8;
	mat_diffuse[2] = 0.7;
	mat_diffuse[3] = 1.0;
	}
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glBegin(GL_POLYGON);
	for (j = 0; j<3; j++) {

	Vertex *temp_v = temp_t->verts[j];
	glNormal3d(temp_v->normal.entry[0], temp_v->normal.entry[1], temp_v->normal.entry[2]);
	if (i == this_poly->seed)
	glColor3f(0.0, 0.0, 1.0);
	else
	glColor3f(1.0, 1.0, 0.0);
	glVertex3d(temp_v->x, temp_v->y, temp_v->z);
	}
	glEnd();
	break;

	case 6:
	glBegin(GL_POLYGON);
	for (j = 0; j<3; j++) {
	Vertex *temp_v = temp_t->verts[j];
	glNormal3d(temp_t->normal.entry[0], temp_t->normal.entry[1], temp_t->normal.entry[2]);
	glColor3f(1.0, 1.0, 1.0);
	glVertex3d(temp_v->x, temp_v->y, temp_v->z);
	}
	glEnd();
	break;

	case 10:
	glBegin(GL_POLYGON);
	for (j = 0; j<3; j++) {
	mat_diffuse[0] = 1.0;
	mat_diffuse[1] = 0.0;
	mat_diffuse[2] = 0.0;
	mat_diffuse[3] = 1.0;

	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

	Vertex *temp_v = temp_t->verts[j];
	glNormal3d(temp_t->normal.entry[0], temp_t->normal.entry[1], temp_t->normal.entry[2]);

	glColor3f(1.0, 0.0, 0.0);
	glVertex3d(temp_v->x, temp_v->y, temp_v->z);
	}
	glEnd();
	break;
	}
	}
	*/
}

void Display_Model(void)
{
	GLint viewport[4];
	int jitter;

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glGetIntegerv(GL_VIEWPORT, viewport);

	set_view(GL_RENDER, poly);
	set_scene(GL_RENDER, poly);
	display_shape(GL_RENDER, poly);
	glFlush();
	glutSwapBuffers();
	glFinish();
}


void temporary()
{
	CompositeXY();
	CompositeYZ();
	CompositeXZ();
	Glui->sync_live();
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void Color_Object()
{
	switch (ObID)
	{
	case Rainbow:
		VColor = 0;
		break;
	case BlueWhiteRed:
		VColor = 1;
		break;
	case OrangeWhiteCyan:
		VColor = 2;
		break;
	}
}

void Choose_Technique()
{
	switch (TId)
	{
	case VSlicing:
		TName = 0;
		break;
	case WireFraming:
		TName = 1;
		break;
	case VRendering:
		TName = 2;
		break;
	}
}


