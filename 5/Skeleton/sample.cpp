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

//
// constants:
//
// NOTE: There are a bunch of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch() statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch() statements.  Those are #defines.
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

// initial window size:

const int INIT_WINDOW_SIZE = { 400 };

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

// the object numbers:
// 


// the object numbers:
// 






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

const int numX = 50;
const int numY = 50;
const int numZ = 50;

struct tempNode
{
	float tempx, tempy, tempz;
	float tempvx, tempvy, tempvz;
	float tempr, tempg, tempb;
};
enum MODELS
{
	VECTORFIELD1,
	VECTORFIELD2,
	VECTORFIELD3
};
enum PROBE
{
	OFF,
	SINGLE,
	MULTIPLE,
	RIBBON,
};
tempNode tempNodeList[numX][numY][numZ];
tempNode Field1List[numX][numY][numZ];
tempNode Field2List[numX][numY][numZ];
tempNode Field3List[numX][numY][numZ];

const int MINUS = {0};
const int PLUS = {1};

#define X 0
#define Y 1
#define Z 2

float xcordminmax[2]={-1.0,1.0};
float ycordminmax[2]={-1.0,1.0};
float zcordminmax[2]={-1.0,1.0};
float inc = 0.0f;
float afactor = 0.0f;
int stepsize = 50;

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };

//
// non-constant global variables:
//

int	ActiveButton;		// current button that is down
GLuint	AxesList;		// list to hold the axes
int	AxesOn;			// != 0 means to draw the axes
int	drawBox;			// != 0 means to draw the Box
int	DebugOn;			// != 0 means to print debugging info
int	DepthCueOn;		// != 0 means to use intensity depth cueing
int	drawArrow;    // != 0 means display 3D arrow plot 
int	drawStreamLine;   // != 0 means display 3D stream line 
int	XYPlaneOn;		// != 0 means to draw XY Plane
int	XZPlaneOn;		// != 0 means to draw XZ Plane
int	YZPlaneOn;		// != 0 means to draw YZ Plane
GLUI *	Glui;			// instance of glui window
int	GluiWindow;		// the glut id for the glui window
int	LeftButton;		// either ROTATE or SCALE
GLuint	BoxList;		// object display list
int	MainWindow;		// window id for main graphics window
GLfloat	RotMatrix[4][4];	// set by glui rotation widget
float	Scale, Scale2;		// scaling factors
int	WhichColor;		// index into Colors[]
int probe;
int	ProjType;	// ORTHO or PERSP
int	Xmouse, Ymouse;		// mouse values
float	Xrot, Yrot;		// rotation angles in degrees
float	TransXYZ[3];		// set by glui translation widgets
float live_xz_tran[2];
float live_x_tran;
float live_y_tran;
float live_z_tran;

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
void	Sliders( int );
void	Spinners( int );

void	Arrow( float [3], float [3] );
void	Cross( float [3], float [3], float [3] );
float	Dot( float [3], float [3] );
float	Unit( float [3], float [3] );
void	Axes( float );
void	HsvRgb( float[3], float [3] );
void    colormap(float , float &, float &, float &);

void    set_view(GLenum mode);
void    display_shape(GLenum mode);
void	datagen(void);
void	ChooseVF(void);
void	buildArrowhead(double [3], float [3]);
void	buildStreamline(float, float , float, float);
void	buildRibbon(float, float , float, float);
void	appendVector(float &, float &, float &);

//Functions to generate vector field datasets
void	getVF1( float x, float y, float z, float &vxp, float &vyp, float &vzp );
void	getVF2(float x, float y, float z, float &vxp, float &vyp, float &vzp);
void	getVF3(float x, float y, float z, float &vxp, float &vyp, float &vzp);


double radius_factor = 0.9;
int display_mode = 0; 
int ObId = 0;
float xtran=0.0;
float ytran=0.0;
bool tempObject=FALSE;

//extern PlyFile *in_ply;
//extern FILE *this_file;

//
// main program:
//

int
main( int argc, char *argv[] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	ObId = VECTORFIELD1;
	datagen();
	memcpy(Field1List, tempNodeList, sizeof(Field1List));

	ObId = VECTORFIELD2;
	datagen();		
	memcpy(Field2List, tempNodeList, sizeof(Field1List));

	ObId = VECTORFIELD3;
	datagen();
	memcpy(Field3List, tempNodeList, sizeof(Field1List));

	// setup all the graphics stuff:
	ObId = VECTORFIELD1;
	memcpy(tempNodeList, Field1List, sizeof(tempNodeList));

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
	char str[256];
	switch( id )
	{
		case RESET:
			Reset();
			drawStreamLine = GLUIFALSE;
			drawArrow = GLUIFALSE;
			probe = OFF;
			ObId = VECTORFIELD1;
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
	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );
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
	if (ProjType == ORTHO)
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

	glTranslatef( (GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2] );

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

	if (drawBox != 0)
	{
		glColor3fv( &Colors[WhichColor][0] );
		glCallList( BoxList );
	}

	// set the color of the object:

	glColor3fv( Colors[WhichColor] );

	// Render the loaded object
	set_view(GL_RENDER);
	
	glTranslatef(xtran, ytran, -5.0);

	glTranslatef( (GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2] );
	
	glScalef( (GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2 );
	
	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );
	glMultMatrixf( (const GLfloat *) RotMatrix );	

	display_shape(GL_RENDER);

	// swap the double-buffered framebuffers:

	glutSwapBuffers();

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush() here, not glFinish() !

	glFlush();
}

//
// initialize the glui window:
//

void InitGlui( void )
{
	GLUI_Panel *panel;
	GLUI_RadioGroup *group, *colormap_group, *surface_group, *volume_group, *probe_group;
	GLUI_Rotation *rot;
	GLUI_Translation *trans, *scale;	

	// setup the glui window:

	glutInitWindowPosition( INIT_WINDOW_SIZE + 50, 0 );
	Glui = GLUI_Master.create_glui( (char *) GLUITITLE );

	Glui->add_statictext( (char *) GLUITITLE );
	Glui->add_separator();

	panel=Glui->add_panel(" Axis and Projections ");

	Glui->add_checkbox_to_panel( panel, "Axes", &AxesOn );

	Glui->add_checkbox_to_panel( panel, "BOX", &drawBox );

	Glui->add_checkbox_to_panel(panel, "PERSPECTIVE", &ProjType);

	Glui->add_checkbox_to_panel(panel, "ARROWS", &drawArrow);

	Glui->add_checkbox_to_panel(panel, "STREAMLINES", &drawStreamLine);

	// Add a list for the different vector fileds
	panel = Glui->add_panel(  "Choose Vector Field" );
	GLUI_Listbox *obj_list = Glui->add_listbox_to_panel(panel, "Objects", &ObId, -1, (GLUI_Update_CB)ChooseVF);
	obj_list->add_item (0, "VECTORFIELD-1");
	obj_list->add_item (1, "VECTORFIELD-2");
	obj_list->add_item (2, "VECTORFIELD-3");

	panel = Glui->add_panel("Probe" );
	probe_group = Glui->add_radiogroup_to_panel(panel, &probe);
	Glui->add_radiobutton_to_group( probe_group, "OFF" );
	Glui->add_radiobutton_to_group( probe_group, "SINGLE" );
	Glui->add_radiobutton_to_group( probe_group, "MULTIPLE" );
	Glui->add_radiobutton_to_group( probe_group, "RIBBON" );
	Glui->add_radiobutton_to_group( probe_group, "BLOB" );

	panel = Glui->add_panel_to_panel(panel, "Move the Probe");
	Glui->add_column_to_panel( panel, GLUIFALSE );
	trans = Glui->add_translation_to_panel( panel, "Trans XY", GLUI_TRANSLATION_XY, &live_z_tran);
	trans->set_speed( 0.05f );

	Glui->add_column_to_panel( panel, GLUIFALSE );
	trans = Glui->add_translation_to_panel( panel, "Trans ZX", GLUI_TRANSLATION_XY, &live_y_tran);
	trans->set_speed( 0.05f );

	Glui->add_column_to_panel( panel, GLUIFALSE );
	trans = Glui->add_translation_to_panel( panel, "Trans YZ", GLUI_TRANSLATION_XY, &live_x_tran );
	trans->set_speed( 0.05f );
	
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

}

//
// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions
//

void InitGraphics( void )
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

void InitLists( void )
{
	float dx = BOXSIZE / 2.;
	float dy = BOXSIZE / 2.;
	float dz = BOXSIZE / 2.;

	// create the object:

	BoxList = glGenLists( 1 );
	glNewList( BoxList, GL_COMPILE );
	

		glBegin( GL_LINES );

			glVertex3f( -dx, -dy,  dz );
			glVertex3f(  dx, -dy,  dz );

			glVertex3f( -dx, -dy,  dz );
			glVertex3f( -dx,  dy,  dz );

			glVertex3f(  dx,  dy,  dz );
			glVertex3f( -dx,  dy,  dz );

			glVertex3f(  dx,  dy,  dz );
			glVertex3f(  dx, -dy,  dz );

			glVertex3f( -dx, -dy, -dz );
			glVertex3f(  dx, -dy, -dz );

			glVertex3f( -dx, -dy, -dz );
			glVertex3f( -dx,  dy, -dz );

			glVertex3f(  dx,  dy, -dz );
			glVertex3f( -dx,  dy, -dz );

			glVertex3f(  dx,  dy, -dz );
			glVertex3f(  dx, -dy, -dz );

			glVertex3f(  dx,  dy,  dz );
			glVertex3f(  dx,  dy, -dz );

			glVertex3f( -dx, -dy,  dz );
			glVertex3f( -dx, -dy, -dz );

			glVertex3f(  dx, -dy,  dz );
			glVertex3f(  dx, -dy, -dz );

			glVertex3f( -dx,  dy,  dz );
			glVertex3f( -dx,  dy, -dz );

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
			ProjType = ORTHO;
			break;

		case 'p':
		case 'P':
			ProjType = PERSP;
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
	drawBox = GLUITRUE;
	DebugOn = GLUIFALSE;
	DepthCueOn = GLUIFALSE;
	XYPlaneOn = GLUITRUE;
	XZPlaneOn = GLUITRUE;
	YZPlaneOn = GLUITRUE;
	LeftButton = ROTATE;
	Scale  = 1.0;
	Scale2 = 0.0;		// because we add 1. to it in Display()
	WhichColor = WHITE;
	ProjType = PERSP;
	Xrot = Yrot = 0.;
	live_x_tran = 0;
	live_y_tran = 0;
	live_z_tran = 0;
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
	float d;					// wing distance
	float x, y, z;				// point to plot
	float mag;					// magnitude of major direction
	float f;					// fabs of magnitude
	int axis;					// which axis is the major


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

void colormap(float scalar_value, float &red, float &green, float &blue)
{
	float hsv[3];
	float rgb[3];
	hsv[0]=240*(1-scalar_value);
	hsv[1]=1;
	hsv[2]=1;
	HsvRgb(hsv,rgb);
	red=rgb[0];
	green=rgb[1];
	blue=rgb[2];
}
void set_view(GLenum mode)
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

	if (ProjType == ORTHO)
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

void display_shape(GLenum mode)
{
	unsigned int i, j, k;
	GLfloat mat_diffuse[4];

	glEnable (GL_POLYGON_OFFSET_FILL);
	glPolygonOffset (1., 1.);

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);		
	glEnable(GL_COLOR_MATERIAL);
		
	mat_diffuse[0] = 0.6;
	mat_diffuse[1] = 0.8;
	mat_diffuse[2] = 0.7;
	mat_diffuse[3] = 1.0;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

	float mag=0.0f;

	if(drawStreamLine)
	{
		switch (probe)
		{
			case OFF:
				break;
			case SINGLE:
				buildStreamline(live_x_tran, live_y_tran, live_z_tran, 0.0);
				break;
			case MULTIPLE:
				int i = 0;
				while( i<numX) 
				{
					int j = 0;
					while(j<numY)
					{
						int k = 0;
						while(k<numZ)
						{			
							if(i%10 == 0 && j%10 == 0 && k%10 == 0)
							{
								mag = sqrt(pow(tempNodeList[i][j][k].tempvx,2)+pow(tempNodeList[i][j][k].tempvy,2)+pow(tempNodeList[i][j][k].tempvz,2));
								buildStreamline(tempNodeList[i][j][k].tempx, tempNodeList[i][j][k].tempy, tempNodeList[i][j][k].tempz, mag);
							}	
							k++;
						}
						j++;
					}
					i++;
				}
				break;
			case RIBBON:
				buildRibbon(live_x_tran, live_y_tran, live_z_tran, 0.0);
				break;
		}
	}

	if(drawArrow)
	{
		afactor = 2.5f;
		int i = 0;
		while( i<numX-1)
		{
			int j = 0;
			while(j<numY-1)
			{
				int k = 0;
				while(k<numZ-1)
				{
					double head[3]={0.0f,0.0f,0.0f};
					float tail[3]={0.0f,0.0f,0.0f};
					float direct[3]={0.0f,0.0f,0.0f};
					
					if(i%5 == 0 && j%5 == 0 && k%5 == 0)
					{
						head[0]=tempNodeList[i][j][k].tempx;
						head[1]=tempNodeList[i][j][k].tempy;
						head[2]=tempNodeList[i][j][k].tempz;
						direct[0]=tempNodeList[i][j][k].tempvx;
						direct[1]=tempNodeList[i][j][k].tempvy;
						direct[2]=tempNodeList[i][j][k].tempvz;
						buildArrowhead(head, direct);
					}	
					k++;
				}
				j++;
			}
			i++;
		}
	}
	
}



void datagen()
{
	float rangeX=abs(xcordminmax[1] - xcordminmax[0]);
	float rangeY=abs(ycordminmax[1] - ycordminmax[0]);
	float rangeZ=abs(zcordminmax[1] - zcordminmax[0]);
	float dTdx = (float)rangeX/(float)(numX);
	float dTdy = (float)rangeY/(float)(numY);
	float dTdz = (float)rangeZ/(float)(numZ);
	float dx = xcordminmax[0]+dTdx;
	int i = 0;
	while(i<numX)
	{			
		float dy = ycordminmax[0]+dTdy;
		int j = 0;
		while(j<numY)
		{
			float dz = zcordminmax[0]+dTdz;
			int k = 0;
			while(k<numZ)
			{
				tempNodeList[i][j][k].tempx=dx;
				tempNodeList[i][j][k].tempy=dy;
				tempNodeList[i][j][k].tempz=dz;
				switch(ObId)
				{
					case VECTORFIELD1:
						getVF1(tempNodeList[i][j][k].tempx, tempNodeList[i][j][k].tempy, tempNodeList[i][j][k].tempz,
							tempNodeList[i][j][k].tempvx, tempNodeList[i][j][k].tempvy, tempNodeList[i][j][k].tempvz );
						break;

					case VECTORFIELD2:
						getVF2(tempNodeList[i][j][k].tempx, tempNodeList[i][j][k].tempy, tempNodeList[i][j][k].tempz,
							tempNodeList[i][j][k].tempvx, tempNodeList[i][j][k].tempvy, tempNodeList[i][j][k].tempvz );
						break;

					case VECTORFIELD3:
						getVF3(tempNodeList[i][j][k].tempx, tempNodeList[i][j][k].tempy, tempNodeList[i][j][k].tempz,
							tempNodeList[i][j][k].tempvx, tempNodeList[i][j][k].tempvy, tempNodeList[i][j][k].tempvz );
						break;
				}				
				dz=dz+dTdz;
				k++;
			}
			dy=dy+dTdy;
			j++;
		}
		dx=dx+dTdx;
		i++;
	}
}

void getVF1(float x, float y, float z, float &vxp, float &vyp, float &vzp)
{
	vxp = -3 + 6.*x - 4.*x*(y+1.) - 4.*z;
	vyp = 12.*x - 4.*x*x - 12.*z + 4.*z*z;
	vzp = 3. + 4.*x - 4.*x*(y+1.) - 6.*z + 4.*(y+1.)*z;
}

void getVF2(float x, float y, float z, float &vxp, float &vyp, float &vzp)
{
	float a = pow(3,0.5);
	float b = pow(2,0.5);
	float c = 1.0;
	vxp = a*sin(x) + c*cos(y);
	vyp = b*sin(x) + a*cos(z);
	vzp = c*sin(y) + b*cos(x);
}

void getVF3(float x, float y, float z, float &vxp, float &vyp, float &vzp)
{
	vxp = -y;
	vyp = -z;
	vzp = x;
}

void ChooseVF()
{
	Reset();
	switch (ObId)
	{
		case VECTORFIELD1:
			memcpy(tempNodeList, Field1List, sizeof(tempNodeList));
			break;

		case VECTORFIELD2:
			memcpy(tempNodeList, Field2List, sizeof(tempNodeList));
			break;

		case VECTORFIELD3:
			memcpy(tempNodeList, Field3List, sizeof(tempNodeList));
			break;
	}
	Glui->sync_live();
	glutSetWindow( MainWindow );
	glutPostRedisplay();
}

void buildArrowhead(double head[3], float direct[3])
{
	glPushMatrix();
	glTranslatef(head[0], head[1], head[2]);
	glRotatef(atan2(direct[1], direct[0])*360/(2*M_PI), 0, 0, 1);
	glRotatef(atan2(direct[1], direct[2])*360/(2*M_PI), 1, 0, 0);
	glRotatef(atan2(direct[0], direct[2])*360/(2*M_PI), 0, 1, 0);
	float head1[3]={0.0f,0.0f,0.0f};
	float tail1[3]={0.0f,0.0f,0.0f};
	float mag = sqrt(pow(direct[0],2)+pow(direct[1],2)+pow(direct[2],2));
	float red, green, blue;
	colormap(mag, red, green, blue);
	glColor3f(red, green, blue);
	glScalef(afactor, afactor, afactor);
	head1[0] =  - (abs(xcordminmax[1] - xcordminmax[0])/(2*numX));
	head1[1] =  - (abs(ycordminmax[1] - ycordminmax[0])/(2*numY));
	head1[2] =  - (abs(zcordminmax[1] - zcordminmax[0])/(2*numZ));
	tail1[0] =  (abs(xcordminmax[1] - xcordminmax[0])/(2*numX));
	tail1[1] =  (abs(ycordminmax[1] - ycordminmax[0])/(2*numY));
	tail1[2] =  (abs(zcordminmax[1] - zcordminmax[0])/(2*numZ));
	Arrow(tail1, head1);
	glPopMatrix();
}

void buildStreamline(float seedx, float seedy, float seedz, float mag)
{
	int i=0;
	glLineWidth(2.0f);
	float red, green, blue, vx, vy, vz;
	colormap(mag, red, green, blue);
	float x=seedx,y=seedy,z=seedz;
	glColor3f(red, green, blue);
	glBegin(GL_LINE_STRIP);
	while( i < stepsize)
		{
			if(x < xcordminmax[0] || x > xcordminmax[1])
				break;
			if(y < ycordminmax[0] || y > ycordminmax[1])
				break;
			if(z < zcordminmax[0] || z > zcordminmax[1])
				break;
			glVertex3f(x,y,z);
			appendVector(x, y, z);
			i++;
		}
	glEnd();
	glLineWidth(1.0f);
}

void buildRibbon(float seedx, float seedy, float seedz, float mag)
{
	glLineWidth(2.0f);
	float red, green, blue, vx, vy, vz;
	colormap(mag, red, green, blue);
	float x=seedx,y=seedy,z=seedz;
	float x1=seedx + 0.1,y1=seedy + 0.1, z1=seedz + 0.1;
	float x2=seedx,y2=seedy,z2=seedz;
	float x3=seedx + 0.1,y3=seedy, z3=seedz;
	glColor3f(red, green, blue);
	int i=0;
	while(i< stepsize)
	{
		x2=x;y2=y;z2=z;
		x3=x1;y3=y1;z3=z1;
		if(x < xcordminmax[0] || x > xcordminmax[1])
			break;
		if(y < ycordminmax[0] || y > ycordminmax[1])
			break;
		if(z < zcordminmax[0] || z > zcordminmax[1])
			break;
		appendVector(x2, y2, z2);
		appendVector(x3, y3, z3);
		glBegin(GL_QUADS);
			glVertex3f(x,y,z);
			glVertex3f(x1,y1,z1);
			glVertex3f(x3,y3,z3);
			glVertex3f(x2,y2,z2);
		glEnd();
		x=x2;y=y2;z=z2;
		x1=x3;y1=x3;z1=z3;
		i++;
	}
}

void appendVector(float &x, float &y, float &z)
{
	float xa = x, ya = y, za = z, xb, yb, zb, xc, yc, zc, vxa, vya, vza, vxb, vyb, vzb, vxc, vyc, vzc;

	switch (ObId)
	{
		case VECTORFIELD1:
			getVF1(xa, ya, za, vxa, vya, vza);
			inc = 0.01;
			stepsize = 50;
			break;
		case VECTORFIELD2:
			getVF2(xa, ya, za, vxa, vya, vza);
			inc = 0.1;
			stepsize = 10;
			break;
		case VECTORFIELD3:
			getVF3(xa, ya, za, vxa, vya, vza);
			inc = 0.25;
			stepsize = 10;
			break;
	}
	xb = xa + inc * vxa;
	yb = xa + inc * vya;
	zb = xa + inc * vza;
	switch (ObId)
	{
		case VECTORFIELD1:
			getVF1(xb, yb, zb, vxb, vyb, vzb);
			break;
		case VECTORFIELD2:
			getVF2(xb, yb, zb, vxb, vyb, vzb);
			break;
		case VECTORFIELD3:
			getVF3(xb, yb, zb, vxb, vyb, vzb);
			break;
	}
	vxc = (vxa + vxb)/2.0f;
	vyc = (vya + vyb)/2.0f;
	vzc = (vza + vzb)/2.0f;

	xc = xa + inc * vxc;
	yc = ya + inc * vyc;
	zc = za + inc * vzc;

	x = xc;
	y = yc;
	z = zc;
}




/////Drawing the probe cube/////
void drawProbeCube(float *p0, float *p1, float *p2, float *p3,
float *p4, float *p5, float *p6, float *p7)
{
glColor3f(0, 1., 0);
//Front
glBegin(GL_QUADS);
glVertex3fv(p0);
glVertex3fv(p1);
glVertex3fv(p2);
glVertex3fv(p3);
glEnd();
//Rear
glBegin(GL_QUADS);
glVertex3fv(p4);
glVertex3fv(p5);
glVertex3fv(p6);
glVertex3fv(p7);
glEnd();
//Left
glBegin(GL_QUADS);
glVertex3fv(p0);
glVertex3fv(p1);
glVertex3fv(p5);
glVertex3fv(p4);
glEnd();
//Rigth
glBegin(GL_QUADS);
glVertex3fv(p3);
glVertex3fv(p2);
glVertex3fv(p6);
glVertex3fv(p7);
glEnd();
//Upper
glBegin(GL_QUADS);
glVertex3fv(p1);
glVertex3fv(p5);
glVertex3fv(p6);
glVertex3fv(p2);
glEnd();
//Buttom
glBegin(GL_QUADS);
glVertex3fv(p0);
glVertex3fv(p4);
glVertex3fv(p7);
glVertex3fv(p3);
glEnd();
}