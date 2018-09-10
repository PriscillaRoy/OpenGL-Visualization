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
const char *WINDOWTITLE = { "OpenGL / GLUT / GLUI Sample -- Priscilla Graphics - PeopleSoft ID : 1619570"  };
const char *GLUITITLE   = { "User Interface Window" };

// what the glui package defines as true and false:
const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };

// the escape key:
#define ESCAPE		0x1b

// initial window size:
const int INIT_WINDOW_SIZE = { 500 };

// size of the box:
const float BOXSIZE = { 2.f };

// multiplication factors for input interaction:
//  (these are known from previous experience)
const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };

const float TEMP_MIN = 0, TEMP_MAX = 100;
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
//const float BACKCOLOR[] = { 0.5, 0.5, 0.5, 0.};

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
enum MODELS
{
	BUNNY,
	FELINE,
	DRAGON,
	HAPPY,
	SPHERE,
	TORUS,
};
enum VFMODELS
{
	Bnoise,
	Bruno3,
	Cnoise,
	Dipole,
	Vnoise,
	
};

enum ColorSchemes
{
	Rainbow,
	BlueWhiteRed,
	OrangeWhiteCyan,
};


enum Param_types
{
	Magnitude,
	AngleV,
	X_Comp,
	Y_Comp
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
const int kernel_size = 50;
// fog parameters:
const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };

const int IMG_RES = 512;
unsigned char noise_tex[IMG_RES][IMG_RES][3];
unsigned char vec_img[IMG_RES][IMG_RES][3];
unsigned char LIC_tex[IMG_RES][IMG_RES][3];

//
// non-constant global variables:
//
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



GLUI_RadioGroup *group2;
GLUI_Checkbox *color_plot_check;


float	Scale, Scale2;		// scaling factors
int	WhichColor;		// index into Colors[]
int	WhichProjection;	// ORTHO or PERSP
int	Xmouse, Ymouse;		// mouse values
float	Xrot, Yrot;		// rotation angles in degrees
float	TransXYZ[3];		// set by glui translation widgets

// function prototypes:
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

void    Choose_Object_VF();
void	Color_Object();
void	Choose_Feature();
void	simulate_colors(Polyhedron*);
void	simulate_arrows(Polyhedron*);
void	simulate_lic(Polyhedron*);
void	gen_noise_tex();
void	render_vec_img(Polyhedron*);


int feature = 0;
int ObjectId = 0;
int ObjectId_VF = 0;
int ObID = 0;
int VF_Parameters = 0;
int VF_plots = 1;
int VF_arrows = 0;
int VF_lic = 0;
double radius_factor = 0.5;
int display_mode = 0; 
int VColor = 0;

char object_name[128]= "bnoise";

Polyhedron *poly = NULL;
//extern PlyFile *in_ply;
//extern FILE *this_file;

// main program:
int
main( int argc, char *argv[] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// Load the model and data here
	/*FILE *this_file = fopen("../models/bunny.ply", "r");
	poly = new Polyhedron (this_file);
	fclose(this_file);*/
	//mat_ident( rotmat );
	//**/
	FILE *this_file1 = fopen("../models/bnoise.ply", "r");
	poly = new Polyhedron (this_file1);
	fclose(this_file1);

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

void Display()
{
	printf("\n display");
	glViewport(0, 0, (GLsizei) 512, (GLsizei) 512);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	// Test noise texture (for debugging purpose)
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, IMG_RES, IMG_RES, 0, GL_RGB, GL_UNSIGNED_BYTE, noise_tex);
	// Test vector field image (for debugging purpose)
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, IMG_RES, IMG_RES, 0, GL_RGB, GL_UNSIGNED_BYTE, vec_img);
// Display LIC image using texture mapping

	if (VF_plots == 1)
		simulate_colors(poly);
	if (VF_arrows == 1)
		simulate_arrows(poly);
	if (VF_lic == 1)
	{
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glEnable(GL_TEXTURE_2D);
		glShadeModel(GL_SMOOTH);
		simulate_lic(poly);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, IMG_RES, IMG_RES, 0, GL_RGB, GL_UNSIGNED_BYTE, vec_img);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, IMG_RES, IMG_RES, 0,GL_RGB, GL_UNSIGNED_BYTE, LIC_tex);
		glBegin(GL_QUAD_STRIP);
			glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex2f(0.0, 1.0);
			glTexCoord2f(1.0, 0.0); glVertex2f(1.0, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex2f(1.0, 1.0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		if (VF_arrows == 1)
			simulate_arrows(poly);
	}
	glutSwapBuffers();
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
	GLUI_Panel *panel2;
	GLUI_Panel *panel3;
	GLUI_RadioGroup *group;
	GLUI_Rotation *rot;
	GLUI_Translation *trans, *scale;


	// setup the glui window:

	glutInitWindowPosition( INIT_WINDOW_SIZE + 50, 0 );
	Glui = GLUI_Master.create_glui( (char *) GLUITITLE );


	Glui->add_statictext( (char *) GLUITITLE );
	Glui->add_separator();

	panel = Glui->add_panel(  "Choose Data " );
	GLUI_Listbox *obj_list1 = Glui->add_listbox_to_panel(panel, "Data", &ObjectId_VF, -1, (GLUI_Update_CB)Choose_Object_VF);
			obj_list1->add_item (0, "bnoise");
			obj_list1->add_item (1, "bruno3");
			obj_list1->add_item (2, "cnoise");
			obj_list1->add_item (3, "dipole");
			obj_list1->add_item (4, "vnoise");

		
	panel2 = Glui->add_panel(  "Choose Color Scheme" );
	GLUI_RadioGroup *color_list = Glui->add_radiogroup_to_panel(panel2, &ObID, -1, (GLUI_Update_CB)Color_Object);
			Glui->add_radiobutton_to_group(color_list, "Rainbow");
			Glui->add_radiobutton_to_group(color_list, "Blue White Red");
			Glui->add_radiobutton_to_group(color_list, "Orange White Cyan");


				panel3 = Glui->add_panel(  "Vector Fields" );
				GLUI_Listbox *param_list = Glui->add_listbox_to_panel(panel3, "Features", &VF_Parameters, -1, (GLUI_Update_CB)Choose_Feature);
	    param_list->add_item (0, "Magnitude" );
		param_list->add_item (1, "Angle " );
		param_list->add_item (2, "X Component" );
		param_list->add_item (3, "Y Component" );

		color_plot_check = Glui->add_checkbox("Show Color Plot", &VF_plots);
		color_plot_check->enable();
		Glui->add_checkbox("Show Arrow Plots", &VF_arrows);
		Glui->add_checkbox("Show LIC Plot", &VF_lic);

	
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
float* MinMax(float* values, int number_vertices)
{
	double min = MAXINT, max = MININT;
	int num = 0;

	while (num<number_vertices)
	{
		if (values[num] <= min)
			min = values[num];
		num++;
	}
	num = 0;
	while (num<number_vertices)
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

float* Findrgb(float temp_s, float tMin, float tMax)
{	
	float avg = (tMin + tMax) / 2;
	float *hsv, *rgb;
	hsv = new float[3];
	rgb = new float[3];
	switch (VColor)
	{
	case 0: 
		hsv[0] = 240 * (1 - ((temp_s - tMin) / (tMax - tMin)));
		hsv[1]=1; hsv[2]=1;
		break;
	case 1:		
		if (temp_s > avg)
		{
			hsv[0] = 0;
			hsv[1] = ((temp_s - avg) / (tMax - avg));
		}
		else
		{
			hsv[0] = 240;
			hsv[1] = 1 - ((temp_s - tMin) / (avg - tMin));
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
	return rgb;
}

float* Feat_Value(Polyhedron *this_poly)
{
	int num_v = this_poly->nverts;
	float *feats = new float[num_v];
	int k = 0;
	while( k<num_v)
	{
		float vertx = this_poly->vlist[k]->vx;
		float verty = this_poly->vlist[k]->vy;
		switch (feature)
		{
		case Magnitude:
			feats[k] = sqrt(pow(vertx, 2) + pow(verty, 2));
			break;
		case AngleV:
			feats[k] = atan2(verty, vertx);
			break;
		case X_Comp:
			feats[k] = vertx;
			break;
		case Y_Comp:
			feats[k] = verty;
			break;
		}
		k++;
	}
	return feats;
}

float get_feature(Vertex *vertex)
{
	float vertx = vertex->vx;
	float verty = vertex->vy;
	float feats;
	switch (feature)
	{
	case Magnitude:
		feats = sqrt(pow(vertx, 2) + pow(verty, 2));
			break;
	case AngleV:
		feats = atan2(verty, vertx);
			break;
	case X_Comp:
		feats = vertx;
			break;
	case Y_Comp:
		feats = verty;
			break;
	}
	return feats;
}

void simulate_colors(Polyhedron *this_poly)
{
	int v_number = this_poly->nverts;
	float *colors = Feat_Value(this_poly);
	float *value = MinMax(colors, v_number);
	int i = 0;
	while(i<this_poly->ntris) 
	{
		Triangle *temp_t=this_poly->tlist[i];
		glEnable(GL_COLOR_MATERIAL);
		glBegin(GL_POLYGON);
		int j = 0;
		while( j<3) 
		{
			Vertex *temp_v = temp_t->verts[j];			
			float color = get_feature(temp_v);
			float *rgb = Findrgb(color, value[0], value[1]);
			glColor3f(rgb[0], rgb[1], rgb[2]);
			glVertex3d(temp_v->x, temp_v->y, temp_v->z);
			j++;
		}
		glEnd();
		i++;
	}	
}

void draw_arrow_head(float *head, float *direct, float length)
{
	glPushMatrix();
	glTranslatef(head[0], head[1], 0);
	glRotatef(atan2(direct[1], direct[0])*360/(2*M_PI), 0, 0, 1);
	glScalef(0.03, 0.03, 1);
	glColor3f(1.0, 1.0, 0.0);
	float head_height = 0.33*length;
	float head_width = 0.14*length;
	if (length < 1)
	{
		length=0.71;
		head_height = 0.4*length;
		head_width = 0.22*length;
	}
	glBegin(GL_LINES);
		glVertex2f(-length, 0);
		glVertex2f(0, 0);
	glEnd();

	glBegin(GL_TRIANGLES);
		glVertex2f(0, 0);
		glVertex2f(-head_height, head_width);
		glVertex2f(-head_height, -head_width);
	glEnd();
	glPopMatrix();
}

void simulate_arrows(Polyhedron *this_poly)
{
	float min_v = 1500, max_v=5000;
	float num_v = this_poly->nverts;
	float length = 1.5 - ((num_v - min_v) / (max_v - min_v));
	float head_height = 0.21*length;
	float head_width = 0.11*length;
	float* head = new float[2];
	float* direct = new float[2];
	for (int k = 0; k<num_v; k++)
	{
		head[0] = this_poly->vlist[k]->x;
		head[1] = this_poly->vlist[k]->y;
		direct[0] = this_poly->vlist[k]->vx;
		direct[1] = this_poly->vlist[k]->vy;
		draw_arrow_head(head, direct, length);
	}	
}

void simulate_lic(Polyhedron *this_poly)
{
	int num_v = this_poly->nverts;
	float *vertx = new float[num_v];
	float *verty = new float[num_v];
	float threshold = 0.0001;
	float x, y, next_i, next_j, vx1, vy1;
	int count,k=0;
	float* lic_color = new float[3];
	while (k < 3){
		lic_color[k] = 0;
		k++;
	}
		k = 0;
		while( k<num_v)
	{
		vertx[k] = this_poly->vlist[k]->vx;
		verty[k] = this_poly->vlist[k]->vy;
		k++;
	}
		float *valuesx_minmax = MinMax(vertx, num_v);
		float *valuesy_minmax = MinMax(verty, num_v);
		
	for (k = 0; k<3; k++)
	{
	for (int i = 0; i < IMG_RES; i++)
		for (int j = 0; j < IMG_RES; j++)
	{	// forward	
		x=i+0.5, y=j+0.5;
		count = 0;
		for (int k=0; k<kernel_size/2; k++)
	{				
		vy1 = valuesx_minmax[0] + (valuesx_minmax[1] - valuesx_minmax[0]) * vec_img[int(x)][int(y)][0] / 255.0;
		vx1 = valuesy_minmax[0] + (valuesy_minmax[1] - valuesy_minmax[0]) * vec_img[int(x)][int(y)][1] / 255.0;
		if ((sqrt(pow(vx1,2)+pow(vy1,2))) < threshold)
		break;
		vx1/= sqrt(pow(vx1,2)+pow(vy1,2));
		vy1/= sqrt(pow(vx1,2)+pow(vy1,2));
		next_i = x+vx1;
		next_j = y+vy1;
		if (abs((int)next_i-(int)x)>1) next_i=((x+next_i)/2);
		if (abs((int)next_j-(int)y)>1) next_j=((y+next_j)/2);

		if (next_i >= IMG_RES  || next_j >= IMG_RES || next_i < 0  || next_j < 0)
			break;			

		for (int m=0; m<3; m++)
		{
			lic_color[m] += noise_tex[(int)next_i][(int)next_j][m];
		}

		x = next_i;
		y = next_j;
		count++;
	}
	// backward
	x=i+0.5; y=j+0.5;
	for (int k=0; k<kernel_size/2; k++)
	{
		vy1 = valuesx_minmax[0] + (valuesx_minmax[1] - valuesx_minmax[0]) * vec_img[int(x)][int(y)][0] / 255.0;
		vx1 = valuesy_minmax[0] + (valuesy_minmax[1] - valuesy_minmax[0]) * vec_img[int(x)][int(y)][1] / 255.0;
		if ((sqrt(pow(vx1,2)+pow(vy1,2))) < threshold)
		break;
		vx1/= sqrt(pow(vx1,2)+pow(vy1,2));
		vy1/= sqrt(pow(vx1,2)+pow(vy1,2));

		next_i = x-vx1;
		next_j = y-vy1;

		if (abs((int)next_i-(int)x)>1) next_i=((x+next_i)/2);
		if (abs((int)next_j-(int)y)>1) next_j=((y+next_j)/2);

		if (next_i >= IMG_RES  || next_j >= IMG_RES || next_i < 0  || next_j < 0)
			break;		
		for (int m=0; m<3; m++)
		{
			lic_color[m] += noise_tex[(int)next_i][(int)next_j][m];
		}
		x = next_i;
		y = next_j;
		count++;
	}
	for (int k=0; k<3; k++)
		lic_color[k] += noise_tex[(int)i][(int)j][k];
	lic_color[0] /= (count + 1);
	lic_color[1] /= (count + 1);
	lic_color[2] /= (count + 1);
			
	for (int k=0; k<3; k++)
		LIC_tex[i][j][k] = lic_color[k];
		}
		 break;
	}
}

void render_vec_img( Polyhedron *this_poly)
{
	glViewport(0, 0, (GLsizei) 512, (GLsizei) 512);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawBuffer(GL_BACK);
	int i, j;

	// first search the max_vx, min_vx, max_vy, min_vy through the entire field
	int num_v = this_poly->nverts;
	float *vert_x = new float[num_v];
	float *vert_y = new float[num_v];
	for (int k = 0; k<num_v; k++)
	{
		vert_x[k] = this_poly->vlist[k]->vx;
		vert_y[k] = this_poly->vlist[k]->vy;
	}
	float *values_x = MinMax(vert_x, num_v);
	float *values_y = MinMax(vert_y, num_v);
	
	// render the mesh
	for (i=0; i<this_poly->ntris; i++) 
	{
		Triangle *temp_t=this_poly->tlist[i];
		float rgb[3];		
		glBegin(GL_TRIANGLES);
		j = 0;
			while( j<3)
			{
				Vertex *v = temp_t->verts[j];
				//determine the color for this vertex based on its vector value
				rgb[0] = (v->vx - values_x[0]) / (values_x[1] - values_x[0]);
				rgb[1] = (v->vy - values_y[0]) / (values_y[1] - values_y[0]);
				rgb[2] = 0.5; 
				glColor3f(rgb[0], rgb[1], rgb[2]);
				glVertex2f (v->x, v->y);
				j++;
			}
		glEnd();
	}
	// save the rendered image into the vec_img
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, 512, 512, GL_RGB, GL_UNSIGNED_BYTE, vec_img);
}

void gen_noise_tex ()
{
	for (int x = 0; x < IMG_RES; x++)
		for (int y = 0; y < IMG_RES; y++)
		{
			noise_tex[x][y][0] =
			noise_tex[x][y][1] = 
			noise_tex[x][y][2] = (unsigned char) 255*(rand() % 32768) / 32768.0;
		}
}

void display_shape(GLenum mode, Polyhedron *this_poly)
{
	glEnable (GL_POLYGON_OFFSET_FILL);
	glPolygonOffset (1., 1.);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_COLOR_MATERIAL);
	if (VF_plots == 1 )
		simulate_colors(this_poly);
	if (VF_arrows == 1)
		simulate_arrows(this_poly);
	if (VF_lic == 1)
	{
		glEnable(GL_TEXTURE_2D);
		glShadeModel(GL_SMOOTH);
		gen_noise_tex();
		render_vec_img(this_poly);
		simulate_lic(this_poly);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, IMG_RES, IMG_RES, 0,GL_RGB, GL_UNSIGNED_BYTE, LIC_tex);
		glBegin(GL_QUAD_STRIP);
			glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex2f(0.0, 1.0);
			glTexCoord2f(1.0, 0.0); glVertex2f(1.0, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex2f(1.0, 1.0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		if (VF_arrows == 1)
		simulate_arrows(this_poly);
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


void Choose_Object_VF()
{
	switch (ObjectId_VF){
	case Bnoise:
			strcpy(object_name, "bnoise");
			break;

		case Bruno3:
			strcpy(object_name, "bruno3");
			break;

		case Cnoise:
			strcpy(object_name, "cnoise");
			break;

		case Dipole:
			strcpy(object_name, "dipole");
			break;

		case Vnoise:
			strcpy(object_name, "vnoise");
			break;

				
	}

	poly->finalize();

    Reset();

	char tmp_str[512];
	sprintf (tmp_str, "../models/%s.ply", object_name);

	FILE *this_file = fopen(tmp_str, "r");
	poly = new Polyhedron (this_file);
	fclose(this_file);

	gen_noise_tex();
	render_vec_img(poly);

    ////Following codes build the edge information
	clock_t start, finish; //Used to show the time assumed
	start = clock(); //Get the first time

	poly->initialize(); // initialize everything
	poly->calc_bounding_sphere();
	poly->calc_face_normals_and_area();
	poly->average_normals();

	finish = clock(); //Get the current time after finished
	double t = (double)(finish - start)/CLOCKS_PER_SEC;

	printf("\n");
	printf("The number of the edges of the object %s is %d \n",object_name, poly->nedges);
	printf("The Euler Characteristics of the object %s is %d \n",object_name, (poly->nverts - poly->nedges + poly->ntris));
	printf("Time to building the edge link is %f seconds\n", t);

	Glui->sync_live();
	glutSetWindow( MainWindow );
	glutPostRedisplay();
}

void Color_Object()
{
	switch (ObID){
	
	case Rainbow:
		VColor = 0;
					break;
	
	case BlueWhiteRed:
		VColor = 1;
					break;

	case OrangeWhiteCyan:
		VColor = 2;
					break;

		
	}}

void Choose_Feature()
{
	switch(VF_Parameters)
	{
	case Magnitude:
		feature = 0;
		break;
	case AngleV:
		feature = 1;
		break;
	case X_Comp:
		feature = 2;
		break;
	case Y_Comp:
		feature = 3;
		break;
	}
}
