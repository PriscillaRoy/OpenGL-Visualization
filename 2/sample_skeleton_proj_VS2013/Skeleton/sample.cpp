#include <stdio.h>
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
//	Author: Priscilla Graphics
//


//
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
const char *GLUITITLE   = { "User Interface Window" };


// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };


// the escape key:

#define ESCAPE		0x1b

#define NX 50
#define NY 50
#define NZ 50
#define TEMPMAX 100
#define TEMPMIN 0
#define MINN 1
#define MAXN 100



// initial window size:

const int INIT_WINDOW_SIZE = { 600 };


// size of the box:

const float BOXSIZE = { 2.f };



// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


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

const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };


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


// line width for the axes:

const GLfloat AXES_WIDTH   = { 3. };


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

enum ColorScheme
{
	Rainbow,
	BlueWhiteRed,
	OrangeWhiteCyan,

};
// the object numbers:
// 

enum MODELS
{
	diesel_field1,
	distance_field1,
	distance_field2,
	iceland_current_field,
	torus_field,
	SIMULATED_TEMPERATURE,
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
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };



//
// non-constant global variables:
//
enum Sliders{
	Temp_Slider,
	Grad_Slider,
	X_Slider,
	Y_Slider,
};

int isoCounters = 0;
int contmain = 0;


GLUI_Spinner *Contour_spinner;
GLUI_Spinner *scalar_Value_Spinner;   

const char * TEMPFORMAT = { "Temperature: %5.2f - %5.2f" };
float TempLowHigh[2] = { TEMPMIN, TEMPMAX };
GLUI_HSlider * TempSlider;
GLUI_StaticText * TempLabel;

const char * GRADFORMAT = { "Gradient: %5.2f - %5.2f" };
float GradLowHigh[2];
GLUI_HSlider * GradSlider;
GLUI_StaticText * GradLabel;

const char * XFORMAT = { "X: %5.2f - %5.2f" };
float XLowHigh[2] = { -1, 1 };
GLUI_HSlider * XSlider;
GLUI_StaticText * XLabel;

const char * YFORMAT = { "Y: %5.2f - %5.2f" };
float YLowHigh[2] = { -1, 1 };
GLUI_HSlider * YSlider;
GLUI_StaticText * YLabel;



float minTemp = TEMPMAX;
int TorusAdjust;
float gradMin;
float gradMax;
int temparature = 1;
int nIntersections;
int discrete = 0;




// structures

float source[4][3] = { { 1.00, 0.00, 90.00 },
{ -1.00, -0.30, 140.00 },
{ -0.10, 1.00, 110.00 },
{ 0.00, 0.40, 170.00 } };



struct sources
{
	float xc, yc, zc;
	float a; // temperature value of the source
} Sources[] =
{
	{ 1.00f, 0.00f, 0.00f, 90.00f },
	{ -1.00f, 0.30f, 0.00f, 120.00f },
	{ 0.00f, 1.00f, 0.00f, 120.00f },
	{ 0.00f, 0.40f, 1.00f, 170.00f },
};



int	ActiveButton;		// current button that is down
GLuint	AxesList;		// list to hold the axes
int	AxesOn;			// != 0 means to draw the axes
int	DebugOn;			// != 0 means to print debugging info
int	DepthCueOn;		// != 0 means to use intensity depth cueing
GLUI *	Glui;			// instance of glui window
int	GluiWindow;		// the glut id for the glui window
int	LeftButton;		// either ROTATE or SCALE
GLuint	BoxList;		// object display list
int	MainWindow;		// window id for main graphics window
GLfloat	RotMatrix[4][4];	// set by glui rotation widget
float	Scale, Scale2;		// scaling factors
int	WhichColor;		// index into Colors[]
int	WhichProjection;	// ORTHO or PERSP
int	Xmouse, Ymouse;		// mouse values
float	Xrot, Yrot;		// rotation angles in degrees
float	TransXYZ[3];		// set by glui translation widgets


//
// function prototypes:
//

void	Animate( void );
void	Buttons( int );
void	Display( void );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( void );
void	InitGlui( void );
void	InitGraphics( void );
void	InitLists( void );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( void );
void	Resize( int, int );
void	Visibility( int );
void	Edge_creation(float[2], float[2], float temp);

void	Arrow( float [3], float [3] );
void	Cross( float [3], float [3], float [3] );
float	Dot( float [3], float [3] );
float	Unit( float [3], float [3] );
void	Axes( float );
void	HsvRgb( float[3], float [3] );

void    Display_Model(void);
void    set_view(GLenum mode, Polyhedron *poly);
void    set_scene(GLenum mode, Polyhedron *poly);
void    display_shape(GLenum mode, Polyhedron *this_poly);
void    Choose_Object();
void	Model_Object();

float*  findrgb(float, float, float);
float   Temperature(float, float, float);
void    Grid_creation(void);
void    Gradient_finding();
void    display_temp(int);
void    Display_selection();
void    Sliders(int);
bool    Shade(float);
void	sSpinner();
void	nSpinner();
void	show_Contours();
//float* choose_Edge_Color(float temp, float, float);
void poly_contours(Polyhedron* this_poly);

int no_Cont = 1;
float scalar_Cont = 1.564;

enum type{
	POINT_CLOUD,
	COLOR_PLOT,
};



double radius_factor = 0.9;
int display_mode = 0; 

int ObjectId = SIMULATED_TEMPERATURE;
int ObID = 0;
char object_name[128]= "diesel_field1";
char color_name[128] = "Rainbow";
int displayType = COLOR_PLOT; 


Polyhedron *poly = NULL;
//extern PlyFile *in_ply;
//extern FILE *this_file;

typedef struct{
	int no_Of_Intersections;
	float pointA[4];
	float pointB[4];
	float tempAB[4];
}line_Points;

typedef struct{
	int no_Of_Intersections;
	float pointX[3];
	float pointY[3];
	float pointZ[3];
	float tempS[3];
}triangle_Points;

line_Points intersections(int, int, float);
triangle_Points tri_Intersections(Triangle*, float);
void process_Intersections(line_Points);
void process_Tri_Intersections(triangle_Points);
void Edge_creation_poly(float[3], float[3]);

typedef struct{
	float x, y, z;
	float t;
	float mag;
	float dir;
	float grad;
	float r, g, b;
}node;

node Nodes[NX][NY];







//
// main program:
//

int
main( int argc, char *argv[] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	Grid_creation();
	Gradient_finding();


	glutInit( &argc, argv );

	// Load the model and data here
	FILE *this_file = fopen("../models/diesel_field1.ply", "r");
	poly = new Polyhedron (this_file);
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
Animate( void )
{
	// put animation stuff in here -- change some global variables
	// for Display() to find:



	// force a call to Display() next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay();
}




//
// glui buttons callback:
//

void
Buttons( int id )
{
	switch( id )
	{
		case RESET:
			Reset();

			no_Cont = 1;
			discrete = 0; 
			temparature = 1;
			isoCounters = 0;
			ObID = 0; // RAINBOW;
			//	displayType = COLOR_PLOT;
			XLowHigh[0] = -1;
			XLowHigh[1] = 1;
			YLowHigh[0] = -1;
			YLowHigh[1] = 1;
			TempLowHigh[0] = TEMPMIN;
			TempLowHigh[1] = TEMPMAX;
			GradLowHigh[0] = gradMin;
			GradLowHigh[1] = gradMax;
			char str[32];
			sprintf(str, TEMPFORMAT, TempLowHigh[0], TempLowHigh[1]);
			TempLabel->set_text(str);
			sprintf(str, GRADFORMAT, GradLowHigh[0], GradLowHigh[1]);
			GradLabel->set_text(str);
			sprintf(str, XFORMAT, XLowHigh[0], XLowHigh[1]);
			XLabel->set_text(str);
			sprintf(str, YFORMAT, YLowHigh[0], YLowHigh[1]);
			YLabel->set_text(str);

			Glui->sync_live();
			glutSetWindow( MainWindow );
			glutPostRedisplay();
			break;

		case QUIT:
			// gracefully close the glui window:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:

			Glui->close();
			glutSetWindow( MainWindow );
			glFinish();
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Button ID %d\n", id );
	}

}



//
// draw the complete scene:
//

void
Display( void )
{
	GLsizei vx, vy, v;		// viewport dimensions
	GLint xl, yb;		// lower-left corner of viewport
	GLfloat scale2;		// real glui scale factor

	if( DebugOn != 0 )
	{
		fprintf( stderr, "Display\n" );
	}


	// set which window we want to do the graphics into:

	glutSetWindow( MainWindow );


	// erase the background:

	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );


	// specify shading to be flat:

	glShadeModel( GL_FLAT );


	// set the viewport to a square centered in the window:

	vx = glutGet( GLUT_WINDOW_WIDTH );
	vy = glutGet( GLUT_WINDOW_HEIGHT );
	v = vx < vy ? vx : vy;			// minimum dimension
	xl = ( vx - v ) / 2;
	yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D() IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	if( WhichProjection == ORTHO )
		glOrtho( -3., 3.,     -3., 3.,     0.1, 1000. );
	else
		gluPerspective( 90., 1.,	0.1, 1000. );


	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();


	// set the eye position, look-at position, and up-vector:
	// IF DOING 2D, REMOVE THIS -- OTHERWISE ALL YOUR 2D WILL DISAPPEAR !

	gluLookAt( 0., 0., 3.,     0., 0., 0.,     0., 1., 0. );


	// translate the objects in the scene:
	// note the minus sign on the z value
	// this is to make the appearance of the glui z translate
	// widget more intuitively match the translate behavior
	// DO NOT TRANSLATE IN Z IF YOU ARE DOING 2D !

	glTranslatef((GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2]);


	// rotate the scene:
	// DO NOT ROTATE (EXCEPT ABOUT Z) IF YOU ARE DOING 2D !

	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );
	glMultMatrixf( (const GLfloat *) RotMatrix );


	// uniformly scale the scene:

	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );
	scale2 = 1. + Scale2;		// because glui translation starts at 0.
	if( scale2 < MINSCALE )
		scale2 = MINSCALE;
	glScalef( (GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2 );


	// set the fog parameters:
	// DON'T NEED THIS IF DOING 2D !

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	// Let us disable lighting right now
	glDisable(GL_LIGHTING);


	// possibly draw the axes:

	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[WhichColor][0] );
		glCallList( AxesList );
	}


	// set the color of the object:

	glColor3fv( Colors[WhichColor] );


	// draw the current object:

	//glCallList( BoxList );


	// draw some gratuitous text that just rotates on top of the scene:

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
	DoRasterString( 5., 5., 0., "Text That Doesn't" );


	// Render the loaded object
	set_view(GL_RENDER, poly);
	
	glTranslatef(0.0, 0.0, -3.0);

	glTranslatef( (GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2] );
	
	glTranslatef(poly->center.entry[0], poly->center.entry[1], poly->center.entry[2]);

	glScalef( (GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2 );
	
	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );
	glMultMatrixf( (const GLfloat *) RotMatrix );
	

	glScalef(1.0/poly->radius, 1.0/poly->radius, 1.0/poly->radius);
	glTranslatef(-poly->center.entry[0], -poly->center.entry[1], -poly->center.entry[2]);

	//display_shape(GL_RENDER, poly);

	if (temparature == 1){
		display_temp(displayType);
	}
	else{
		display_shape(GL_RENDER, poly);
	}

	// swap the double-buffered framebuffers:

	glutSwapBuffers();


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush() here, not glFinish() !

	glFlush();
}



//
// use glut to display a string of characters using a raster font:
//

void
DoRasterString( float x, float y, float z, char *s )
{
	char c;			// one character to print

	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}



//
// use glut to display a string of characters using a stroke font:
//

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	char c;			// one character to print
	float sf;		// the scale factor

	glPushMatrix();
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		sf = ht / ( 119.05 + 33.33 );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix();
}



//
// return the number of seconds since the start of the program:
//

float
ElapsedSeconds( void )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.;
}



//
// initialize the glui window:
//

void
InitGlui( void )
{
	GLUI_Panel *panel;
	GLUI_RadioGroup *group;
	GLUI_Rotation *rot;
	GLUI_RadioGroup *ColorGroup;
	GLUI_Translation *trans, *scale;


	// setup the glui window:

	glutInitWindowPosition( INIT_WINDOW_SIZE + 50, 0 );
	Glui = GLUI_Master.create_glui( (char *) GLUITITLE );


	Glui->add_statictext( (char *) GLUITITLE );
	Glui->add_separator();

	Glui->add_checkbox( "Axes", &AxesOn );

	Glui->add_checkbox( "Perspective", &WhichProjection );

	Glui->add_checkbox( "Intensity Depth Cue", &DepthCueOn );

	// Add a rollout for the axes color
	GLUI_Rollout *rollout = Glui->add_rollout(" Axes Color ", 0);

	//panel = Glui->add_panel(  "Axes Color" );
	//GLUI_Rollout *rollout = Glui->add_rollout_to_panel(panel,  "Axes Color", 1 );
		//group = Glui->add_radiogroup_to_panel( panel, &WhichColor );

		group = Glui->add_radiogroup_to_panel( rollout, &WhichColor );
			Glui->add_radiobutton_to_group( group, "Red" );
			Glui->add_radiobutton_to_group( group, "Yellow" );
			Glui->add_radiobutton_to_group( group, "Green" );
			Glui->add_radiobutton_to_group( group, "Cyan" );
			Glui->add_radiobutton_to_group( group, "Blue" );
			Glui->add_radiobutton_to_group( group, "Magenta" );
			Glui->add_radiobutton_to_group( group, "White" );
			Glui->add_radiobutton_to_group( group, "Black" );


	// Add a list for the different models
	//rollout = Glui->add_rollout(" Models ", 0);
	panel = Glui->add_panel(  "Choose object to open " );
			GLUI_Listbox *obj_list = Glui->add_listbox_to_panel(panel, "Objects", &ObjectId, -1, ( GLUI_Update_CB) Choose_Object);
			obj_list->add_item (0, "diesel_field1");
			obj_list->add_item (1, "distance_field1");
			obj_list->add_item (2, "distance_field2");
			obj_list->add_item (3, "iceland_current_field");
			obj_list->add_item (4, "torus_field");
			obj_list->add_item(5, "SIMULATED_TEMPERATURE");

	panel = Glui->add_panel("Choose Color Scheme ");
	GLUI_Listbox *obj_list_1 = Glui->add_listbox_to_panel(panel, "ColorScheme", &ObID, -1, (GLUI_Update_CB) Model_Object);
			obj_list_1->add_item(0, "Rainbow");
			obj_list_1->add_item(1, "Blue White Red");
			obj_list_1->add_item(2, "Orange White Cyan");
			//obj_list_1->add_item(3, "Random Color Scale");
			Glui->add_column_to_panel(panel, GLUITRUE);
			Glui->add_checkbox_to_panel(panel, "Discrete", &discrete);

			panel = Glui->add_panel("Temparature simulation");
			Glui->add_checkbox_to_panel(panel, "Simulation", &temparature);
			Glui->add_column_to_panel(panel, GLUITRUE);
			group = Glui->add_radiogroup_to_panel(panel, &displayType, -1, (GLUI_Update_CB)Display_selection);
			Glui->add_radiobutton_to_group(group, "Point Cloud");
			Glui->add_radiobutton_to_group(group, "Color Plot");

			char str[128];

			panel = Glui->add_panel("GLUI Range Sliders");
			XSlider = Glui->add_slider_to_panel(panel, true, GLUI_HSLIDER_FLOAT, XLowHigh,
				X_Slider, (GLUI_Update_CB)Sliders);
			XSlider->set_float_limits(-1, 1);
			XSlider->set_w(200);
			sprintf(str, XFORMAT, XLowHigh[0], XLowHigh[1]);
			XLabel = Glui->add_statictext_to_panel(panel, str);

			YSlider = Glui->add_slider_to_panel(panel, true, GLUI_HSLIDER_FLOAT, YLowHigh,
				Y_Slider, (GLUI_Update_CB)Sliders);
			YSlider->set_float_limits(-1, 1);
			YSlider->set_w(200);
			sprintf(str, YFORMAT, YLowHigh[0], YLowHigh[1]);
			YLabel = Glui->add_statictext_to_panel(panel, str);

			TempSlider = Glui->add_slider_to_panel(panel, true, GLUI_HSLIDER_FLOAT, TempLowHigh,
				Temp_Slider, (GLUI_Update_CB)Sliders);
			TempSlider->set_float_limits(TEMPMIN, TEMPMAX);
			TempSlider->set_w(200);
			sprintf(str, TEMPFORMAT, TempLowHigh[0], TempLowHigh[1]);
			TempLabel = Glui->add_statictext_to_panel(panel, str);

			GradSlider = Glui->add_slider_to_panel(panel, true, GLUI_HSLIDER_FLOAT, GradLowHigh,
				Grad_Slider, (GLUI_Update_CB)Sliders);
			GradSlider->set_float_limits(gradMin, gradMax);
			GradSlider->set_w(200);
			sprintf(str, GRADFORMAT, GradLowHigh[0], GradLowHigh[1]);
			GradLabel = Glui->add_statictext_to_panel(panel, str);

			panel = Glui->add_panel("iso-contours");
			Glui->add_checkbox_to_panel(panel, "Contours", &isoCounters);
			Glui->add_column_to_panel(panel, GLUITRUE);
			scalar_Value_Spinner = Glui->add_spinner_to_panel(panel, "Scalar Value", GLUI_SPINNER_FLOAT, &scalar_Cont, -1, (GLUI_Update_CB)sSpinner);
			Contour_spinner = Glui->add_spinner_to_panel(panel, "Contours Count", GLUI_SPINNER_INT, &no_Cont, -1, (GLUI_Update_CB)nSpinner);
			Contour_spinner->set_int_limits(MINN, MAXN);
   

	panel = Glui->add_panel( "Object Transformation" );

		rot = Glui->add_rotation_to_panel( panel, "Rotation", (float *) RotMatrix );

		// allow the object to be spun via the glui rotation widget:

		rot->set_spin( 1.0 );


		Glui->add_column_to_panel( panel, GLUIFALSE );
		scale = Glui->add_translation_to_panel( panel, "Scale",  GLUI_TRANSLATION_Y , &Scale2 );
		scale->set_speed( 0.005f );

		Glui->add_column_to_panel( panel, GLUIFALSE );
		trans = Glui->add_translation_to_panel( panel, "Trans XY", GLUI_TRANSLATION_XY, &TransXYZ[0] );
		trans->set_speed( 0.05f );

		Glui->add_column_to_panel( panel, GLUIFALSE );
		trans = Glui->add_translation_to_panel( panel, "Trans Z",  GLUI_TRANSLATION_Z , &TransXYZ[2] );
		trans->set_speed( 0.05f );

	Glui->add_checkbox( "Debug", &DebugOn );


	panel = Glui->add_panel( "", GLUIFALSE );

	Glui->add_button_to_panel( panel, "Reset", RESET, (GLUI_Update_CB) Buttons );

	Glui->add_column_to_panel( panel, GLUIFALSE );

	Glui->add_button_to_panel( panel, "Quit", QUIT, (GLUI_Update_CB) Buttons );


	// tell glui what graphics window it needs to post a redisplay to:

	Glui->set_main_gfx_window( MainWindow );


	// set the graphics window's idle function:

	GLUI_Master.set_glutIdleFunc( NULL );
	Glui->sync_live();
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}



//
// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions
//

void
InitGraphics( void )
{
	// setup the display mode:
	// ( *must* be done before call to glutCreateWindow() )
	// ask for color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );


	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );


	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );


	// setup the clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );


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

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	//glutDisplayFunc( Display_Model );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( 0, NULL, 0 );

	// DO NOT SET THE GLUT IDLE FUNCTION HERE !!
	// glutIdleFunc( NULL );
	// let glui take care of it in InitGlui()
}




//
// initialize the display lists that will not change:
//

void
InitLists( void )
{
	float dx = BOXSIZE / 2.;
	float dy = BOXSIZE / 2.;
	float dz = BOXSIZE / 2.;

	// create the object:

	BoxList = glGenLists( 1 );
	glNewList( BoxList, GL_COMPILE );

		glBegin( GL_QUADS );

			glColor3f( 0., 0., 1. );
			glNormal3f( 0., 0.,  1. );
				glVertex3f( -dx, -dy,  dz );
				glVertex3f(  dx, -dy,  dz );
				glVertex3f(  dx,  dy,  dz );
				glVertex3f( -dx,  dy,  dz );

			glNormal3f( 0., 0., -1. );
				glTexCoord2f( 0., 0. );
				glVertex3f( -dx, -dy, -dz );
				glTexCoord2f( 0., 1. );
				glVertex3f( -dx,  dy, -dz );
				glTexCoord2f( 1., 1. );
				glVertex3f(  dx,  dy, -dz );
				glTexCoord2f( 1., 0. );
				glVertex3f(  dx, -dy, -dz );

			glColor3f( 1., 0., 0. );
			glNormal3f(  1., 0., 0. );
				glVertex3f(  dx, -dy,  dz );
				glVertex3f(  dx, -dy, -dz );
				glVertex3f(  dx,  dy, -dz );
				glVertex3f(  dx,  dy,  dz );

			glNormal3f( -1., 0., 0. );
				glVertex3f( -dx, -dy,  dz );
				glVertex3f( -dx,  dy,  dz );
				glVertex3f( -dx,  dy, -dz );
				glVertex3f( -dx, -dy, -dz );

			glColor3f( 0., 1., 0. );
			glNormal3f( 0.,  1., 0. );
				glVertex3f( -dx,  dy,  dz );
				glVertex3f(  dx,  dy,  dz );
				glVertex3f(  dx,  dy, -dz );
				glVertex3f( -dx,  dy, -dz );

			glNormal3f( 0., -1., 0. );
				glVertex3f( -dx, -dy,  dz );
				glVertex3f( -dx, -dy, -dz );
				glVertex3f(  dx, -dy, -dz );
				glVertex3f(  dx, -dy,  dz );

		glEnd();

	glEndList();


	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList();
}



//
// the keyboard callback:
//

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
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
			Buttons( QUIT );	// will not return here
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
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}


	// synchronize the GLUI display with the variables:

	Glui->sync_live();


	// force a call to Display():

	glutSetWindow( MainWindow );
	glutPostRedisplay();
}



//
// called when the mouse button transitions down or up:
//

void
MouseButton( int button, int state, int x, int y )
{
	int b;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}


	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
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
MouseMotion( int x, int y )
{
	int dx, dy;		// change in mouse coordinates

	if( DebugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );


	dx = x - Xmouse;		// change in mouse coords
	dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		switch( LeftButton )
		{
			case ROTATE:
				Xrot += ( ANGFACT*dy );
				Yrot += ( ANGFACT*dx );
				break;

			case SCALE:
				Scale += SCLFACT * (float) ( dx - dy );
				if( Scale < MINSCALE )
					Scale = MINSCALE;
				break;
		}
	}


	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay();
}



//
// reset the transformations and the colors:
//
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene
//

void
Reset( void )
{
	ActiveButton = 0;
	AxesOn = GLUITRUE;
	DebugOn = GLUIFALSE;
	TorusAdjust = GLUIFALSE;
	DepthCueOn = GLUIFALSE;
	LeftButton = ROTATE;
	Scale  = 1.0;
	Scale2 = 0.0;		// because we add 1. to it in Display()
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	TransXYZ[0] = TransXYZ[1] = TransXYZ[2] = 0.;

	                  RotMatrix[0][1] = RotMatrix[0][2] = RotMatrix[0][3] = 0.;
	RotMatrix[1][0]                   = RotMatrix[1][2] = RotMatrix[1][3] = 0.;
	RotMatrix[2][0] = RotMatrix[2][1]                   = RotMatrix[2][3] = 0.;
	RotMatrix[3][0] = RotMatrix[3][1] = RotMatrix[3][3]                   = 0.;
	RotMatrix[0][0] = RotMatrix[1][1] = RotMatrix[2][2] = RotMatrix[3][3] = 1.;
}



//
// called when user resizes the window:
//

void
Resize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );

	// don't really need to do anything since window size is
	// checked each time in Display():

	glutSetWindow( MainWindow );
	glutPostRedisplay();
}


//
// handle a change to the window's visibility:
//

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
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
Arrow( float tail[3], float head[3] )
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
	mag = fabs( w[0] );
	if( (f=fabs(w[1]))  > mag )
	{
		axis = Y;
		mag = f;
	}
	if( (f=fabs(w[2]))  > mag )
	{
		axis = Z;
		mag = f;
	}


	// set size of wings and turn w into a Unit vector:

	d = WINGS * Unit( w, w );


	// draw the shaft of the arrow:

	glBegin( GL_LINE_STRIP );
		glVertex3fv( tail );
		glVertex3fv( head );
	glEnd();

	// draw two sets of wings in the non-major directions:

	if( axis != X )
	{
		Cross( w, axx, v );
		(void) Unit( v, v );
		Cross( v, w, u  );
		x = head[0] + d * ( u[0] - w[0] );
		y = head[1] + d * ( u[1] - w[1] );
		z = head[2] + d * ( u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
		x = head[0] + d * ( -u[0] - w[0] );
		y = head[1] + d * ( -u[1] - w[1] );
		z = head[2] + d * ( -u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
	}


	if( axis != Y )
	{
		Cross( w, ayy, v );
		(void) Unit( v, v );
		Cross( v, w, u  );
		x = head[0] + d * ( u[0] - w[0] );
		y = head[1] + d * ( u[1] - w[1] );
		z = head[2] + d * ( u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
		x = head[0] + d * ( -u[0] - w[0] );
		y = head[1] + d * ( -u[1] - w[1] );
		z = head[2] + d * ( -u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
	}



	if( axis != Z )
	{
		Cross( w, azz, v );
		(void) Unit( v, v );
		Cross( v, w, u  );
		x = head[0] + d * ( u[0] - w[0] );
		y = head[1] + d * ( u[1] - w[1] );
		z = head[2] + d * ( u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
		x = head[0] + d * ( -u[0] - w[0] );
		y = head[1] + d * ( -u[1] - w[1] );
		z = head[2] + d * ( -u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
	}
}



float
Dot( float v1[3], float v2[3] )
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}



void
Cross( float v1[3], float v2[3], float vout[3] )
{
	float tmp[3];

	tmp[0] = v1[1]*v2[2] - v2[1]*v1[2];
	tmp[1] = v2[0]*v1[2] - v1[0]*v2[2];
	tmp[2] = v1[0]*v2[1] - v2[0]*v1[1];

	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}



float
Unit( float vin[3], float vout[3] )
{
	float dist, f ;

	dist = vin[0]*vin[0] + vin[1]*vin[1] + vin[2]*vin[2];

	if( dist > 0.0 )
	{
		dist = sqrt( dist );
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

void
Axes( float length )
{
	int i, j;			// counters
	float fact;			// character scale factor
	float base;			// character start location


	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd();
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd();

	fact = LENFRAC * length;
	base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( i = 0; i < 4; i++ )
		{
			j = xorder[i];
			if( j < 0 )
			{
				
				glEnd();
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd();

	glBegin( GL_LINE_STRIP );
		for( i = 0; i < 5; i++ )
		{
			j = yorder[i];
			if( j < 0 )
			{
				
				glEnd();
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd();

	glBegin( GL_LINE_STRIP );
		for( i = 0; i < 6; i++ )
		{
			j = zorder[i];
			if( j < 0 )
			{
				
				glEnd();
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd();

}




//
// routine to convert HSV to RGB
//
// Reference:  Foley, van Dam, Feiner, Hughes,
//		"Computer Graphics Principles and Practices,"
//		Additon-Wesley, 1990, pp592-593.


void
HsvRgb( float hsv[3], float rgb[3] )
{
	float h, s, v;			// hue, sat, value
	float r, g, b;			// red, green, blue
	float i, f, p, q, t;		// interim values


	// guarantee valid input:

	h = hsv[0] / 60.;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;


	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}


	// get an rgb from the hue itself:
	
	i = floor( h );
	f = h - i;
	p = v * ( 1. - s );
	q = v * ( 1. - s*f );
	t = v * ( 1. - ( s * (1.-f) ) );

	switch( (int) i )
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

	glScalef(1.0/poly->radius, 1.0/poly->radius, 1.0/poly->radius);
	glTranslatef(-poly->center.entry[0], -poly->center.entry[1], -poly->center.entry[2]);
}


void display_shape(GLenum mode, Polyhedron *this_poly)
{
	unsigned int i, j;
	GLfloat mat_diffuse[4];

  glEnable (GL_POLYGON_OFFSET_FILL);
  glPolygonOffset (1., 1.);

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	float scalarMax = 0.0, scalarMin = 0.0;

	for (i = 0; i < this_poly->ntris; i++)
	{
		Triangle *temp_t = this_poly->tlist[i];
		for (j = 0; j < 3; j++){
			Vertex *temp_v = temp_t->verts[j];
			if (i == 0 && j == 0){
				scalarMax = scalarMin = temp_v->s;
			}
			if (temp_v->s > scalarMax)
				scalarMax = temp_v->s;
			if (temp_v->s < scalarMin)
				scalarMin = temp_v->s;
		}
	}


	if (isoCounters == 1){
		scalar_Value_Spinner->set_float_limits(scalarMin, scalarMax - 0.0001);
	}




	for (i=0; i<this_poly->ntris; i++) {
		if (mode == GL_SELECT)
      glLoadName(i+1);

		Triangle *temp_t=this_poly->tlist[i];

		switch (display_mode) {
		case 0:
			glEnable(GL_COLOR_MATERIAL);
			/*if (i == this_poly->seed) {
				mat_diffuse[0] = 0.0;
				mat_diffuse[1] = 0.0;
				mat_diffuse[2] = 1.0;
				mat_diffuse[3] = 1.0;
			} else {
				mat_diffuse[0] = 0.6;
				mat_diffuse[1] = 0.8;
				mat_diffuse[2] = 0.7;
				mat_diffuse[3] = 1.0;
			}
			glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);*/
			glEnable(GL_COLOR_MATERIAL);
			glBegin(GL_POLYGON);
			//poly->get_SMinSMax();
			float hsv[3], rgb[3];
			for (j=0; j<3; j++) {

				Vertex *temp_v = temp_t->verts[j];
				glNormal3d(temp_v->normal.entry[0], temp_v->normal.entry[1], temp_v->normal.entry[2]);
				/*if (i==this_poly->seed)
					glColor3f(0.0, 0.0, 1.0);
					
				else
					glColor3f(1.0, 1.0, 0.0);
				glVertex3d(temp_v->x, temp_v->y, temp_v->z);*/
				
				if (ObID == 0){
					hsv[0] = 240.0 - (240.0*(temp_v->s - this_poly->smin) /( this_poly->smax - this_poly->smin));
					hsv[1] = 1.0;
					hsv[2] = 1.0;
					HsvRgb(hsv, rgb);
					//glColor3f(rgb[0], rgb[1], rgb[2]);
					glColor3fv(rgb);
					glVertex3d(temp_v->x, temp_v->y, temp_v->z);
				}
				else if (ObID == 1){

					float hsv[3], rgb[3];
					float avg = (this_poly->smax - this_poly->smin) / 2;
					//float avg = (poly->smax - poly->smin) / 2;
					if (temp_v->s < avg)
					{
						hsv[0] = 240;
						hsv[1] = (avg - temp_v->s) / (avg - this_poly->smin);
					}

					else if (temp_v->s > avg)
					{
						hsv[0] = 0;
						hsv[1] = (temp_v->s - avg) / (this_poly->smax - avg);
					}
					//hsv[1] = (temp_v->s - this_poly->smin) / (this_poly->smax - this_poly->smin);
					
					hsv[2] = 1;
					HsvRgb(hsv, rgb);

					glColor3fv(rgb);
					glVertex3d(temp_v->x, temp_v->y, temp_v->z);


				}


				else if (ObID == 2)
				{

					float hsv[3], rgb[3];
					float avg = (this_poly->smax - this_poly->smin) / 2;
					//float avg = (poly->smax - poly->smin) / 2;
					if (temp_v->s < avg)
					{
						hsv[0] = 20;
						hsv[1] = (avg - temp_v->s) / (avg - this_poly->smin);
					}

					else if (temp_v->s > avg)
					{
						hsv[0] = 180;
						hsv[1] = (temp_v->s - avg) / (this_poly->smax - avg);
					}
					//hsv[1] = (temp_v->s - this_poly->smin) / (this_poly->smax - this_poly->smin);

					hsv[2] = 1;
					HsvRgb(hsv, rgb);

					glColor3fv(rgb);
					glVertex3d(temp_v->x, temp_v->y, temp_v->z);


				}
					
				
			}
			glEnd();
			if (isoCounters == 1){

				contmain = 1;
				triangle_Points temp = tri_Intersections(temp_t, scalar_Cont);

				process_Tri_Intersections(temp);
				contmain = 0;

				for (int k = 1; k < no_Cont; k++){

					float scalar = scalarMin + k*(scalarMax - scalarMin) / (no_Cont - 1);

					if (scalar == scalarMax){
						scalar -= 0.0001;
					}
					temp = tri_Intersections(temp_t, scalar);
					process_Tri_Intersections(temp);
				}
			}
			break;

		case 6:
			glBegin(GL_POLYGON);
			for (j=0; j<3; j++) {
				Vertex *temp_v = temp_t->verts[j];
				glNormal3d(temp_t->normal.entry[0], temp_t->normal.entry[1], temp_t->normal.entry[2]);
				glColor3f(1.0, 1.0, 1.0);
				glVertex3d(temp_v->x, temp_v->y, temp_v->z);
			}
			glEnd();
			break;

		case 10:
			glBegin(GL_POLYGON);
			for (j=0; j<3; j++) {
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
}


void Display_Model(void)
{
	GLint viewport[4];
	int jitter;

	glClearColor (1.0, 1.0, 1.0, 1.0);  // background for rendering color coding and lighting
	glGetIntegerv (GL_VIEWPORT, viewport);
 
	set_view(GL_RENDER, poly);
	set_scene(GL_RENDER, poly);
	display_shape(GL_RENDER, poly);
	glFlush();
	glutSwapBuffers();
	glFinish();
}

void Choose_Color(void){
	Glui->sync_live();
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void Model_Object(void)
{
	/*int w, h;
	switch (ObID){
	case 0:
		strcpy(color_name, "Rainbow");
		break;

	case 1:
		strcpy(color_name, "Blue White Red");
		break;

	case 2:
		strcpy(color_name, "Orange White Cyan");
		break;


	case 3:
		strcpy(color_name, "Random Color Scale");
		break;
	}*/
	Glui->sync_live();
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void Choose_Object(void)
{
	if (ObjectId != SIMULATED_TEMPERATURE){
		int w, h;
		TorusAdjust = 0;
		switch (ObjectId){
		case diesel_field1:
			strcpy(object_name, "diesel_field1");
			break;

		case distance_field1:
			strcpy(object_name, "distance_field1");
			break;

		case distance_field2:
			strcpy(object_name, "distance_field2");
			break;

		case iceland_current_field:
			strcpy(object_name, "iceland_current_field");
			break;

		case torus_field:
			strcpy(object_name, "torus_field");
			break;



			//Glui->add_radiobutton_to_group(group,"lucy");
			//Glui->add_radiobutton_to_group(group,"lion");
			//Glui->add_radiobutton_to_group(group,"heptoroid");
			//Glui->add_radiobutton_to_group(group,"igea");

			//case ICOSAHEDRON:
			//	//strcpy(object_name, "icosahedron");
			//	strcpy(object_name, "Armadillo");
			//	break;

			//case OCTAHEDRON:
			//	strcpy(object_name, "lucy");
			//	break;

			//case HEXAHEDRON:
			//	strcpy(object_name, "heptoroid");
			//	break;

			//case DODECAHEDRON:
			//	strcpy(object_name, "igea");
			//	break;

			//case TETRAHEDRON:
			//	strcpy(object_name, "brain");
			//	break;

		}

		poly->finalize();

		Reset();

		char tmp_str[512];

		sprintf(tmp_str, "../models/%s.ply", object_name);
		printf(object_name);

		FILE *this_file = fopen(tmp_str, "r");
		poly = new Polyhedron(this_file);
		fclose(this_file);

		////Following codes build the edge information
		clock_t start, finish; //Used to show the time assumed
		start = clock(); //Get the first time

		poly->initialize(); // initialize everything

		poly->calc_bounding_sphere();
		poly->calc_face_normals_and_area();
		poly->average_normals();

		finish = clock(); //Get the current time after finished
		double t = (double)(finish - start) / CLOCKS_PER_SEC;

		printf("\n");
		printf("The number of the edges of the object %s is %d \n", object_name, poly->nedges);
		printf("The Euler Characteristics of the object %s is %d \n", object_name, (poly->nverts - poly->nedges + poly->ntris));

		printf("Time to building the edge link is %f seconds\n", t);
	}
	Reset();
	Glui->sync_live();
	glutSetWindow( MainWindow );
	glutPostRedisplay();
}


float Temperature(float x, float y, float z){

	float t = 0.0;
	for (int i = 0; i < 4; i++)
	{
		float dx = x - Sources[i].xc;
		float dy = y - Sources[i].yc;
		float dz = z - Sources[i].zc;
		float rsqd = dx*dx + dy*dy + dz*dz;
		t += Sources[i].a * exp(-5 * rsqd);
	}

	if (t > TEMPMAX)
	{
		t = TEMPMAX;
	}
	else if (t< TEMPMIN)
	{
		t = TEMPMIN;
	}
	return t;
}

void Sliders(int id){
	char str[32];
	switch (id)
	{
	case Temp_Slider:{
		sprintf(str, TEMPFORMAT, TempLowHigh[0], TempLowHigh[1]);
		TempLabel->set_text(str);
	}
					 break;

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

	case Grad_Slider:{
		sprintf(str, GRADFORMAT, GradLowHigh[0], GradLowHigh[1]);
		GradLabel->set_text(str);
	}
					 break;

	}
	glutSetWindow(MainWindow);
	glutPostRedisplay();

}
bool shade(float grad){

	if (grad >= GradLowHigh[0] && grad <= GradLowHigh[1])
		return true;
	return false;
}
void display_temp(int type){

	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_COLOR_MATERIAL);

	switch (type){

	case 0:{

		glPointSize(3.0);
		glBegin(GL_POINTS);

		for (int i = 0; i < NX; i++){
			for (int j = 0; j < NY; j++){
				node temp = Nodes[i][j];
				if (temp.t >= TempLowHigh[0] && temp.t <= TempLowHigh[1] && temp.x >= XLowHigh[0] && temp.x <= XLowHigh[1] &&
					temp.y >= YLowHigh[0] && temp.y <= YLowHigh[1] && shade(temp.grad)){
					float* rgb = findrgb(Nodes[i][j].t, TEMPMAX, TEMPMIN);
					glColor3f(rgb[0], rgb[1], rgb[2]);
					glVertex2f(Nodes[i][j].x, Nodes[i][j].y);
				}
			}
		}
		glEnd();
	}
		   break;
	case 1:{

		for (int i = 0; i < NX - 1; i++){
			for (int j = 0; j < NY - 1; j++){
				for (int k = 0; k < NZ - 1; k++)
				{
					float *rgb;
					node temp = Nodes[i][j];
					if (temp.t >= TempLowHigh[0] && temp.t <= TempLowHigh[1] && temp.x >= XLowHigh[0] && temp.x <= XLowHigh[1] &&

						temp.y >= YLowHigh[0] && temp.y <= YLowHigh[1] && shade(temp.grad)){
						glBegin(GL_QUADS);

						rgb = findrgb(Nodes[i][j].t, TEMPMAX, TEMPMIN);
						glColor3f(rgb[0], rgb[1], rgb[2]);
						glVertex2f(Nodes[i][j].x, Nodes[i][j].y);

						rgb = findrgb(Nodes[i][j + 1].t, TEMPMAX, TEMPMIN);
						glColor3f(rgb[0], rgb[1], rgb[2]);
						glVertex2f(Nodes[i][j + 1].x, Nodes[i][j + 1].y);

						rgb = findrgb(Nodes[i + 1][j + 1].t, TEMPMAX, TEMPMIN);
						glColor3f(rgb[0], rgb[1], rgb[2]);
						glVertex2f(Nodes[i + 1][j + 1].x, Nodes[i + 1][j + 1].y);

						rgb = findrgb(Nodes[i + 1][j].t, TEMPMAX, TEMPMIN);
						glColor3f(rgb[0], rgb[1], rgb[2]);
						glVertex2f(Nodes[i + 1][j].x, Nodes[i + 1][j].y);
						glEnd();






						glBegin(GL_QUADS);

						rgb = findrgb(Nodes[i][j].t, TEMPMAX, TEMPMIN);
						glColor3f(rgb[0], rgb[1], rgb[2]);
						glVertex2f(Nodes[i][j].y, Nodes[i][j].z);

						rgb = findrgb(Nodes[i][j + 1].t, TEMPMAX, TEMPMIN);
						glColor3f(rgb[0], rgb[1], rgb[2]);
						glVertex2f(Nodes[i][j + 1].y, Nodes[i][j + 1].z);

						rgb = findrgb(Nodes[i + 1][j + 1].t, TEMPMAX, TEMPMIN);
						glColor3f(rgb[0], rgb[1], rgb[2]);
						glVertex2f(Nodes[i + 1][j + 1].y, Nodes[i + 1][j + 1].z);

						rgb = findrgb(Nodes[i + 1][j].t, TEMPMAX, TEMPMIN);
						glColor3f(rgb[0], rgb[1], rgb[2]);
						glVertex2f(Nodes[i + 1][j].y, Nodes[i + 1][j].z);
						glEnd();
					}
				}
			}
		}

	}
		   break;

	}
	if (isoCounters == 1){
		show_Contours();
	}
}

float* findrgb(float temp_s, float tMax, float tMin){

	float hsv[3], rgb[3];
	float avg = 0;
	switch (ObID)
	{
	case 0:

		hsv[0] = 240. - (240. * ((temp_s - tMin) / (tMax - tMin)));
		hsv[1] = 1.0;
		hsv[2] = 1.0;
		HsvRgb(hsv, rgb);
		return rgb;
		break;

	case 1:
		avg = (tMax - tMin) / 2;
		//float avg = (poly->smax - poly->smin) / 2;
		if (temp_s < avg)
		{
			hsv[0] = 240;
			hsv[1] = (avg - temp_s) / (avg - tMin);
		}

		else if (temp_s > avg)
		{
			hsv[0] = 0;
			hsv[1] = (temp_s - avg) / (tMax - avg);
		}
		//hsv[1] = (temp_v->s - this_poly->smin) / (this_poly->smax - this_poly->smin);

		hsv[2] = 1;
		/*if ((temp_s - tMin) / (tMax - tMin) < 0.33)
		{
			hsv[0] = 240.;
			hsv[1] = 1.;
			hsv[2] = 1.;

		}
		else if ((temp_s - tMin) / (tMax - tMin) < 0.66)
		{
			hsv[0] = 0.;
			hsv[1] = 0.;
			hsv[2] = 1.;

		}
		else
		{
			hsv[0] = 360.;
			hsv[1] = 1.;
			hsv[2] = 0.5;

		}
		*/
		HsvRgb(hsv, rgb);
		return rgb;
		break;

	case 2:

		/*if ((temp_s - tMin) / (tMax - tMin) < 0.5)
		{
			hsv[0] = 113.;
			hsv[1] = 1.0;
			hsv[2] = 0.36;

		}
		else
		{
			hsv[0] = 60.;
			hsv[1] = 1.0;
			hsv[2] = 1.0;

		}
		*/
		avg = (tMax - tMin) / 2;
		//float avg = (poly->smax - poly->smin) / 2;
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
		//hsv[1] = (temp_v->s - this_poly->smin) / (this_poly->smax - this_poly->smin);

		hsv[2] = 1;
		HsvRgb(hsv, rgb);
		return rgb;
		break;

	/*case 3:

		if ((temp_s - tMin) / (tMax - tMin) < 0.02)
		{
			hsv[0] = 312.;
			hsv[1] = 0.51;
			hsv[2] = 0.90;

		}
		else if ((temp_s - tMin) / (tMax - tMin) < 0.04)
		{
			hsv[0] = 264.;
			hsv[1] = 0.68;
			hsv[2] = 0.79;

		}
		else if ((temp_s - tMin) / (tMax - tMin) < 0.06)
		{
			hsv[0] = 92.;
			hsv[1] = 0.82;
			hsv[2] = 0.91;

		}
		else if ((temp_s - tMin) / (tMax - tMin) < 0.08)
		{
			hsv[0] = 24.;
			hsv[1] = 0.82;
			hsv[2] = 0.91;

		}
		else if ((temp_s - tMin) / (tMax - tMin) < 0.1)
		{
			hsv[0] = 58.;
			hsv[1] = 0.86;
			hsv[2] = 0.99;

		}
		else if ((temp_s - tMin) / (tMax - tMin) < 0.2)
		{
			hsv[0] = 176.;
			hsv[1] = 0.86;
			hsv[2] = 0.99;

		}
		else if ((temp_s - tMin) / (tMax - tMin) < 0.3)
		{
			hsv[0] = 240.;
			hsv[1] = 0.100;
			hsv[2] = 0.47;
		}
		else if ((temp_s - tMin) / (tMax - tMin) < 0.4)
		{
			hsv[0] = 350.;
			hsv[1] = 0.97;
			hsv[2] = 0.5;
		}
		else if ((temp_s - tMin) / (tMax - tMin) < 0.5)
		{
			hsv[0] = 40.;
			hsv[1] = 0.1;
			hsv[2] = 0.1;
		}
		else
		{

			hsv[0] = 102.;
			hsv[1] = 0.1;
			hsv[2] = 0.1;
		}

		HsvRgb(hsv, rgb);
		return rgb;
		break;
		*/
	}


}


triangle_Points tri_Intersections(Triangle* temp_t, float scalar){

	triangle_Points points;
	points.no_Of_Intersections = 0;

	for (int j = 0; j < 3; j++){
		int k = j + 1;
		if (k > 2){
			k = 0;
		}

		Vertex *tempA = temp_t->verts[j];
		Vertex *tempB = temp_t->verts[k];

		float a = tempA->s;
		float b = tempB->s;

		if (a == b && b == scalar){

			float p1[3] = { tempA->x, tempA->y, tempA->z };
			float p2[3] = { tempB->x, tempB->y, tempB->z };
			Edge_creation_poly(p1, p2);
		}
		else if (a == b && b != scalar){}
		else if (scalar >= min(a, b) && scalar <= max(a, b)){

			float t = (scalar - a) / (b - a);
			int ind = points.no_Of_Intersections;

			points.pointX[ind] = ((1 - t)*tempA->x) + (t*tempB->x);
			points.pointY[ind] = ((1 - t)*tempA->y) + (t*tempB->y);
			points.pointZ[ind] = ((1 - t)*tempA->z) + (t*tempB->z);
			points.tempS[ind] = (a + b) / 2;
			points.no_Of_Intersections++;
		}
	}
	return points;
}

void process_Tri_Intersections(triangle_Points points){

	if (points.no_Of_Intersections == 2){

		float p1[3] = { points.pointX[0], points.pointY[0], points.pointZ[0] };
		float p2[3] = { points.pointX[1], points.pointY[1], points.pointZ[1] };
		Edge_creation_poly(p1, p2);
	}
}

void Edge_creation_poly(float p1[3], float p2[3]){

	displayType = 1;

	glLineWidth(2.0);
	glBegin(GL_LINES);

	glColor3f(0.0, 0.0, 0.0);
	glVertex3f(p1[0], p1[1], p1[2]);
	glVertex3f(p2[0], p2[1], p2[2]);
	glEnd();
}


void Display_selection(){
	display_temp(displayType);
}

void show_Contours(){

	scalar_Value_Spinner->set_float_limits(TEMPMIN, TEMPMAX - 0.0001);

	for (int i = 0; i < NX - 1; i++){

		for (int j = 0; j < NY - 1; j++){

			contmain = 1;
			line_Points temp = intersections(i, j, scalar_Cont);
			process_Intersections(temp);
			contmain = 0;

			for (int k = 1; k < no_Cont; k++){
				float scalar = TEMPMIN + k*(TEMPMAX - TEMPMIN) / (no_Cont - 1);
				if (scalar == TEMPMAX){
					scalar -= 0.0001;
				}
				temp = intersections(i, j, scalar);
				process_Intersections(temp);
			}
		}
	}
}

line_Points intersections(int i, int j, float s){

	int grid_Matrix[4][2] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };

	line_Points this_points;
	this_points.no_Of_Intersections = 0;

	for (int k = 0; k < 4; k++){

		float a = Nodes[i][j].t;
		float b = Nodes[i + grid_Matrix[k][0]][j + grid_Matrix[k][1]].t;

		if (a == b && b == s){

			float p1[2] = { Nodes[i][j].x, Nodes[i][j].y };
			float p2[2] = { Nodes[i + grid_Matrix[k][0]][j + grid_Matrix[k][1]].x, Nodes[i + grid_Matrix[k][0]][j + grid_Matrix[k][1]].y };
			Edge_creation(p1, p2, (Nodes[i][j].t + Nodes[i + grid_Matrix[k][0]][j + grid_Matrix[k][1]].t) / 2);
		}
		else if (a == b&&b != s){}
		else if (s >= min(a, b) && s <= max(a, b)){

			float t = (s - a) / (b - a);

			this_points.pointA[this_points.no_Of_Intersections] = (1 - t)*Nodes[i][j].x + t*Nodes[i + grid_Matrix[k][0]][j + grid_Matrix[k][1]].x;
			this_points.pointB[this_points.no_Of_Intersections] = (1 - t)*Nodes[i][j].y + t*Nodes[i + grid_Matrix[k][0]][j + grid_Matrix[k][1]].y;
			this_points.tempAB[this_points.no_Of_Intersections] = (a + b) / 2;
			this_points.no_Of_Intersections++;
		}
		else{

		}
		i += grid_Matrix[k][0];
		j += grid_Matrix[k][1];
	}

	return this_points;

}

void process_Intersections(line_Points this_points){

	if (this_points.no_Of_Intersections == 2){

		float p1[2] = { this_points.pointA[0], this_points.pointB[0] };
		float p2[2] = { this_points.pointA[1], this_points.pointB[1] };
		Edge_creation(p1, p2, (this_points.tempAB[0] + this_points.tempAB[1]) / 2);
	}

	else if (this_points.no_Of_Intersections == 4){

		float scalarValue = 0.0;

		float p1[2] = { this_points.pointA[0], this_points.pointB[0] };
		float p2[2] = { this_points.pointA[1], this_points.pointB[1] };
		float p3[2] = { this_points.pointA[2], this_points.pointB[2] };
		float p4[2] = { this_points.pointA[3], this_points.pointB[3] };
		for (int i = 0; i < 4; i++){
			scalarValue += this_points.tempAB[i];
		}
		scalarValue = scalarValue / 4;
		if (scalar_Cont < scalarValue && this_points.tempAB[0] < scalarValue){

			Edge_creation(p1, p4, (this_points.tempAB[0] + this_points.tempAB[3]) / 2);
			Edge_creation(p2, p3, (this_points.tempAB[1] + this_points.tempAB[2]) / 2);
		}
		else{
			Edge_creation(p1, p2, (this_points.tempAB[0] + this_points.tempAB[1]) / 2);
			Edge_creation(p3, p4, (this_points.tempAB[2] + this_points.tempAB[3]) / 2);
		}
	}
}

void Edge_creation(float p1[2], float p2[2], float temp){

	glLineWidth(2.0);
	glBegin(GL_LINES);
	glColor3f(0.0, 0.0, 0.0);
	glVertex2f(p1[0], p1[1]);
	glVertex2f(p2[0], p2[1]);
	glEnd();
}

void Grid_creation(){

	float diff_x = 2.0 / NX;
	float diff_y = 2.0 / NY;
	float x_val = -1.0;
	float y_val = -1.0;


	for (int i = 0; i<NX; i++, x_val += diff_x)
	{
		y_val = -1.0;

		for (int j = 0; j <NY; j++, y_val += diff_y)

		{
			Nodes[i][j].x = x_val;
			Nodes[i][j].y = y_val;
			Nodes[i][j].z = 0;
			Nodes[i][j].t = Temperature(x_val, y_val, 0.0);
		}
	}
}



void Gradient_finding(){

	float min, max;

	for (int i = 0; i < NX; i++){

		for (int j = 0; j < NY; j++){

			float dx, dy, sqrd;

			dx = (Nodes[min(NX - 1, i + 1)][j].t - Nodes[max(0, i - 1)][j].t) / (Nodes[min(NX - 1, i + 1)][j].x - Nodes[max(0, i - 1)][j].x);
			dy = (Nodes[i][min(NY - 1, j + 1)].t - Nodes[i][max(0, j - 1)].t) / (Nodes[i][min(NY - 1, j + 1)].y - Nodes[i][max(0, j - 1)].y);
			sqrd = (dx*dx) + (dy*dy);
			Nodes[i][j].grad = sqrt(sqrd);

			if (i == 0 && j == 0){
				max = min = Nodes[i][j].grad;
			}

			if (Nodes[i][j].grad > max)
				max = Nodes[i][j].grad;

			if (Nodes[i][j].grad < min)
				min = Nodes[i][j].grad;
		}
	}

	GradLowHigh[1] = gradMax = max;
	GradLowHigh[0] = gradMin = min;
}

void sSpinner(){};

void nSpinner(){};
