/*
	COMPUTAÇÃO GRÁFICA
	ENGENHARIA INFORMÁTICA 
	INSTITUTO POLITÉCNICO DE BRAGANÇA

	Mini Shinobi
	C/C++ & OpenGL

	Bernardo Lopes
	Tiago Padrão
*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h> // Used for the random number generator (RNG) algorithm
#include "mersenne-twister.h" // RNG algorithm
#include "soil\SOIL.h" // Library to load image files into textures http://www.lonesock.net/soil.html
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glut.h>

/*
	Mersenne Twister algorithm imported from https://github.com/cslarsen/mersenne-twister
	Files imported: mersenne-twister.h; mersenne-twister.cpp
*/

typedef struct // Defines a new structure type to be used in the rng() function
{
	GLfloat xi;
	GLfloat xf;
	GLfloat r;
} points;

GLfloat stick_growth[3]; // Value added to the stick height while the mouse is clicked
GLint score = 0, highscore = 0; // Current game score and user high score
GLfloat height = 250.0f, width = 250.0f; // Default max orthogonal grid values
GLint gamestate = 0; // Starting game state
GLint oranges = 0; // Number of oranges collected
GLfloat rotation; // Stick rotation angle
GLint stage; // Game stage - determines the next platform to be generated
GLfloat stepsize = 0.077f; // Movement speed (x-axys)
GLint mouse_down = -1; // Left mouse button state
GLfloat x_shinobi, x_shinobi_start[3], y_shinobi = 1.0f, x_shinobi_final[3]; // Variables for shinobi position control
GLint k; // Variable to control stick drawing and some aspects of shinobi positioning
GLuint texture[25]; // Array of textures
const GLint font = (GLint)GLUT_BITMAP_HELVETICA_18; // Default font for bitmap text
points random[9]; // Array of random x and y coordinates
GLfloat midpoint; // Value of the midpoint of a given platform
GLint window_width = 600, window_height = 1000; // Default window size
FILE *fp; // File pointer for load and save operations
GLint savedata[6]; // Array to store load and save data
char string_score[10]; // String to display the score
char string_highscore[10]; // String to display the highscore
char string_oranges[10]; // String to display the number of oranges
GLint orange_hit = 0; // Variable to control orange collision
GLint shinobimoving, flip_shinobi = 0; // Variables to control shinobi flip
GLfloat xi_orange; // Variable to define orange position
GLint background; // Variable to define the background image
GLint perfect_control = 0, game_start = 0;  // Variables to control the "perfect" mechanic
GLint store = 0; // Variable to control store opening
GLint selected_shinobi = 1;  // Variable to control selected shinobi
GLint shinobi_bought[3]; // Variable to control shinobis bought

/*
	Game states modify the behaviour of the game depending on certain conditions

	LIST OF GAME STATES

	0 - Main Menu (or Store)
	1 - Game Start (or waiting for user input)
	2 - Game processing the user action
	3 - Death Menu
*/

points rng() // Function that returns a random number, based on the Mersenne Twister algorithm 
{
	GLint seed = time(NULL); // Uses current clock time as seed for the generator
	srand(seed);
	GLfloat random = randf_cc();

	if (random < 0.2) random = 0.2;
	if (random < 0.5) random += 0.15;

	points coordinates; // Creates a new variable of the "points" type

	coordinates.r = random; // Stores the random generated value to be used in other functions
	coordinates.xi = x_shinobi + (161 * random); // xi
	if (145 * random < 20) coordinates.xi = x_shinobi + 45; // Minimum distance between shinobi and platform of 45
	if (random * 70 < 15) random += (15 - random); // Minimum platform size of 15
	coordinates.xf = coordinates.xi + (random * 70); // xf
	if (coordinates.xf - coordinates.xi > 115) coordinates.xf = coordinates.xi + 87; // If width is greater than 115, then width = 87

	return coordinates; // Returns platform's xi and xf
}

GLvoid initGL()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Set clearing color to white

	/*
	if (rng().r < 0.25) background = 10;
	if (rng().r >= 0.25 && rng().r < 0.5) background = 8;
	if (rng().r >= 0.5 && rng().r < 0.75) background = 9;
	if (rng().r >= 0.75) background = 7; // Randomly select a background from the 4 possible ones
	*/

	//TEST CODE
	background = 7;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Enable Alpha settings (for opacity control)

	glGenTextures(25, texture); // Generates 25 texture names

	// Texture importing code based on http://www.gamasutra.com/view/feature/3361/understanding_and_using_opengl_.php?print=1

	glBindTexture(GL_TEXTURE_2D, texture[0]); // Binds the texture

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); // Defines texture behaviour (clamp to edge or repeat)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Defines texture filtering mode

	texture[0] = SOIL_load_OGL_texture // Loads the texture from a .png using SOIL
		(
		"textures/shinobi1.png", // Default shinobi texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[1]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[1] = SOIL_load_OGL_texture
		(
		"textures/orange.png", // orange texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[2]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[2] = SOIL_load_OGL_texture
		(
		"textures/title.png", // Game title texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[3]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[3] = SOIL_load_OGL_texture
		(
		"textures/play.png", // Play button texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[4]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[4] = SOIL_load_OGL_texture
		(
		"textures/gameover.png", // Game over texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[5]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[5] = SOIL_load_OGL_texture
		(
		"textures/menu.png", // Main menu icon texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[6]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[6] = SOIL_load_OGL_texture
		(
		"textures/repeat.png", // New game icon texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	if (background == 7) // Only loads the texture if it corresponds to the selected background
	{
		glBindTexture(GL_TEXTURE_2D, texture[7]);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		texture[7] = SOIL_load_OGL_texture
			(
			"textures/background1.png", // Background #1 texture
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
			);
	}

	if (background == 8)
	{
		glBindTexture(GL_TEXTURE_2D, texture[8]);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		texture[8] = SOIL_load_OGL_texture
			(
			"textures/background2.png", // Background #2 texture
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
			);
	}

	if (background == 9)
	{
		glBindTexture(GL_TEXTURE_2D, texture[9]);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		texture[9] = SOIL_load_OGL_texture
			(
			"textures/background3.png", // Background #3 texture
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
			);
	}

	if (background == 10)
	{
		glBindTexture(GL_TEXTURE_2D, texture[10]);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		texture[10] = SOIL_load_OGL_texture
			(
			"textures/background4.png", // Background #4 texture
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
			);
	}

	glBindTexture(GL_TEXTURE_2D, texture[11]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[11] = SOIL_load_OGL_texture
		(
		"textures/score.png", // Score box texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[12]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[12] = SOIL_load_OGL_texture
		(
		"textures/shinobi1moving.png", // Default shinobi moving texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[13]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[13] = SOIL_load_OGL_texture
		(
		"textures/shinobi2.png", // Square shinobi texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[14]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[14] = SOIL_load_OGL_texture
		(
		"textures/shinobi2moving.png", // Square shinobi moving texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[15]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[15] = SOIL_load_OGL_texture
		(
		"textures/shinobi3.png", // Ghost shinobi texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[16]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[16] = SOIL_load_OGL_texture
		(
		"textures/shinobi3moving.png", // Ghost shinobi moving texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[17]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[17] = SOIL_load_OGL_texture
		(
		"textures/shinobi4.png", // Guerrilla shinobi texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[18]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[18] = SOIL_load_OGL_texture
		(
		"textures/shinobi4moving.png", // Guerrilla shinobi moving texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[19]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[19] = SOIL_load_OGL_texture
		(
		"textures/store.png", // Store texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[20]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[20] = SOIL_load_OGL_texture
		(
		"textures/store1.png", // Store icon #1 texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[21]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[21] = SOIL_load_OGL_texture
		(
		"textures/store2.png", // Store icon #2 texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[22]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[22] = SOIL_load_OGL_texture
		(
		"textures/store3.png", // Store icon #3 texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	glBindTexture(GL_TEXTURE_2D, texture[23]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture[23] = SOIL_load_OGL_texture
		(
		"textures/store4.png", // Store icon #4 texture
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);
}

GLvoid drawBackground() // Draws the background image
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[background]);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2d(0, 0); glVertex2f(0.0f, 0.0f);
	glTexCoord2d(1, 0); glVertex2f(width, 0.0f);
	glTexCoord2d(1, 1); glVertex2f(width, height);
	glTexCoord2d(0, 1); glVertex2f(0.0f, height);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

GLvoid renderBitmapString(GLfloat x, GLfloat y, GLvoid *font, const char *string) // Function that renders a bitmap string. [IMPORTED] Source: http://www.lighthouse3d.com/tutorials/glut-tutorial/bitmap-fonts/
{
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f); 
	const char *c;
	glRasterPos2f(x, y);
	for (c = string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}

GLvoid drawPlatform(GLfloat xi, GLfloat xf, GLfloat yi, GLfloat yf) // Draws a platform with a "perfect" indicator
{
	glBegin(GL_QUADS);
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	glVertex2f(xi, yi);
	glVertex2f(xf, yi);
	glVertex2f(xf, yf);
	glVertex2f(xi, yf);
	glEnd();

	GLfloat perfect = xi + (xf - xi) / 2;

	glBegin(GL_QUADS);
	glColor4f(0.0f, 0.3f, 1.0f, 1.0f);
	glVertex2f(perfect - 3.0f, yf - 2.5f);
	glVertex2f(perfect + 3.0f, yf - 2.5f);
	glVertex2f(perfect + 3.0f, yf);
	glVertex2f(perfect - 3.0f, yf);
	glEnd();
}

GLvoid drawInitialPlatform(GLfloat xi, GLfloat xf, GLfloat yi, GLfloat yf) // Draws the starting platform (or any other platform without a "perfect" indicator)
{
	glBegin(GL_QUADS);
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	glVertex2f(xi, yi);
	glVertex2f(xf, yi);
	glVertex2f(xf, yf);
	glVertex2f(xi, yf);
	glEnd();
}

GLvoid drawStick(GLfloat xi, GLfloat yi) // Draws the stick
{
	GLfloat size = 2.0f; // Stick width

	glBegin(GL_QUADS);
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	glVertex2f(xi, yi);
	glVertex2f(xi + size, yi);
	glVertex2f(xi + size, yi + stick_growth[k]); // Stick height starts at 0 and increases with stick_growth
	glVertex2f(xi, yi + stick_growth[k]);
	glEnd();
}

GLvoid success() // Function to increase the score
{
	score++; // Increases the score

	if (orange_hit == 1) oranges++; // If a orange was caught, increases the orange counter

	printf("Score: %d\n", score); // Prints the score to the console
}

GLvoid nextStage() // Function called if the user landed succesfully on the platform
{
	success(); // Increases the score
	
	orange_hit = 0; 
	shinobimoving = 0;
	y_shinobi = 125.0f; 
	perfect_control = 0;
	game_start = 0; // Resets control variables

	stage = (stage + 1) % 9; // Increases the stage

	glTranslatef(-(x_shinobi - x_shinobi_start[k]), 0.0f, 0.0f); // Advances the camera
	
	random[stage].xi = rng().xi;
	random[stage].xf = rng().xf; // New platform coordinates
	random[stage].r = rng().r; // New random value

	k = (k + 1) % 3; // Increases the value of k
	if (k == 0) stick_growth[1] = 0.0f;
	if (k == 1) stick_growth[2] = 0.0f;
	if (k == 2) stick_growth[0] = 0.0f; // Clears the value of stick_growth that was used two stages before

	midpoint = random[stage].xi + (random[stage].xf - random[stage].xi) / 2; // Determines the new plaform's midpoint

	gamestate = 1; // Changes the game state to 1, waiting for user input
}

GLvoid newGame() // Function that starts a new game
{
	glLoadIdentity(); // Resets the camera

	stage = 0;
	score = 0; 
	orange_hit = 0;
	rotation = 0.0f;
	shinobimoving = 0; // Resets control variables
	
	for (int aux = 0; aux <= 3; aux++)
	{
		stick_growth[aux] = 0.0f; // Resets stick_growth
	}

	x_shinobi = (width / 5) -20.0f;
	y_shinobi = 125.0f; // Places the shinobi at the starting position

	random[0].xi = rng().xi;
	random[0].xf = rng().xf; // Determines the first platform coordinates
	midpoint = random[stage].xi + (random[stage].xf - random[stage].xi) / 2; // Determines the new plaform's midpoint
	
	game_start = 1; // Reset control variable
}

GLvoid saveGame() // Function that saves variables to a file called savegame.dat
{
	if ((fp = (fopen("savegame.dat", "wb"))) == NULL)
	{
		return;
	}

	savedata[0] = highscore;
	savedata[1] = oranges;
	savedata[2] = selected_shinobi;
	savedata[3] = shinobi_bought[0];
	savedata[4] = shinobi_bought[1];
	savedata[5] = shinobi_bought[2]; // Copies the variables to the save data array

	fwrite(savedata, sizeof(int), 6, fp); // Writes to the file the highscore, oranges number, selected shinobi and status of bought shinobis

	fclose(fp); // Closes the file
}

GLvoid loadSave()
{
	if ((fp = (fopen("savegame.dat", "rb"))) == NULL) // Checks if a previous save exists and other erros in file opening
	{
		printf("Couldn't find previous save game\n\n");
		return;
	}

	fread(savedata, sizeof(int), 6, fp); // Reads the save data from the file

	highscore = savedata[0];
	oranges = savedata[1];
	selected_shinobi = savedata[2];
	shinobi_bought[0] = savedata[3];
	shinobi_bought[1] = savedata[4];
	shinobi_bought[2] = savedata[5]; // Copies the save data to the desired variables

	fclose(fp);

	printf("Save game loaded. You have %d oranges and your highest score is %d\n\n", oranges, highscore); // Displays a message if succesfull
}

GLvoid drawDeathMenu() // Draws the death menu
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[4]); // Draws the Game Over text

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2d(0, 0); glVertex2f((width/2) -50.0f, (height/2)+100.0f);
	glTexCoord2d(1, 0); glVertex2f((width / 2) + 50.0f, (height / 2) + 100.0f);
	glTexCoord2d(1, 1); glVertex2f((width / 2) + 50.0f, (height / 2) + 200.0f);
	glTexCoord2d(0, 1); glVertex2f((width / 2) - 50.0f, (height / 2) + 200.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[11]); // Draws the score box

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2d(0, 0); glVertex2f((width / 2) - 50.0f, (height / 2) + 100.0f);
	glTexCoord2d(1, 0); glVertex2f((width / 2) + 50.0f, (height / 2) + 100.0f);
	glTexCoord2d(1, 1); glVertex2f((width / 2) + 50.0f, (height / 2));
	glTexCoord2d(0, 1); glVertex2f((width / 2) - 50.0f, (height / 2));
	glEnd();

	glDisable(GL_TEXTURE_2D);

	sprintf(string_score, "%d", score); 
	sprintf(string_highscore, "%d", highscore); // Copies the score and highscore to strings

	renderBitmapString(width / 2 - 12.0f, (height / 2) + 75.0f, (GLvoid *)font, "SCORE");
	renderBitmapString(width / 2 - 2.0f, (height / 2) + 60.0f, (GLvoid *)font, string_score);
	renderBitmapString(width / 2 - 9.0f, (height / 2) + 35.0f, (GLvoid *)font, "BEST");
	renderBitmapString(width / 2 - 3.0f, (height / 2) + 20.0f, (GLvoid *)font, string_highscore); // Displays the score and highscore

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[5]); // Draws the main menu icon

	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2d(1, 0); glVertex2f((width / 2) - 70.0f, (height / 2) - 100.0f);
	glTexCoord2d(0, 0); glVertex2f((width / 2) - 20.0f, (height / 2) - 100.0f);
	glTexCoord2d(0, 1); glVertex2f((width / 2) - 20.0f, (height / 2) - 50.0f);
	glTexCoord2d(1, 1); glVertex2f((width / 2) - 70.0f, (height / 2) - 50.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[6]); // Draws the new game icon

	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2d(0, 0); glVertex2f((width / 2) + 20.0f, (height / 2) - 100.0f);
	glTexCoord2d(1, 0); glVertex2f((width / 2) + 70.0f, (height / 2) - 100.0f);
	glTexCoord2d(1, 1); glVertex2f((width / 2) + 70.0f, (height / 2) - 50.0f);
	glTexCoord2d(0, 1); glVertex2f((width / 2) + 20.0f, (height / 2) - 50.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	if (game_start == 0)
	{
		printf("\nHope you enjoyed the game. Play again?\n\n");
		saveGame(); // Saves the data to the save file
	}

	game_start = 1;
}

GLvoid drawOrange(GLfloat x, GLfloat y) // Draws a orange
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[1]); // Apply texture to orange

	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
	glTexCoord2d(0, 0); glVertex2f(x - 5.0f, y);
	glTexCoord2d(1, 0); glVertex2f(x + 5.0f, y);
	glTexCoord2d(1, 1); glVertex2f(x + 5.0f, y + 10.0f);
	glTexCoord2d(0, 1); glVertex2f(x - 5.0f, y + 10.0f); // Draws the orange in the given coordinates and draws the texture
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

GLvoid drawRandomorange() // Draws a orange in a random location
{
	if (random[stage].r > 0.48)
	{
		if (random[stage].r < 0.7)
		{
			xi_orange = random[stage].xi - (90.0f*random[stage].r);
			drawOrange(xi_orange, 110.0f);
		}

		if (random[stage].r >= 0.7)
		{
			xi_orange = random[stage].xi - (50.0f*random[stage].r);
			drawOrange(xi_orange, 110.0f);
		}
	}
}

GLvoid drawTitle() // Draws the title in the main menu
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[2]);

	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2d(0, 0); glVertex2f((width/2)-90.0f, (height/2)+20.0f);
	glTexCoord2d(1, 0); glVertex2f((width / 2) + 90.0f, (height / 2)+20.0f);
	glTexCoord2d(1, 1); glVertex2f((width / 2) + 90.0f, (height / 2)+200.0f);
	glTexCoord2d(0, 1); glVertex2f((width / 2) - 90.0f, (height / 2)+200.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

GLvoid drawPlayButton() // Draws the play button in the main menu
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[3]);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2d(0, 0); glVertex2f((width / 2.f) - 50.0f, (height / 2.f) - 50.0f); // Draws a textured square with the text "Play" that will be used
	glTexCoord2d(1, 0); glVertex2f((width / 2.f) + 50.0f, (height / 2.f) - 50.0f); // as the clicking area for the play button
	glTexCoord2d(1, 1); glVertex2f((width / 2.f) + 50.0f, (height / 2.f) + 50.0f);
	glTexCoord2d(0, 1); glVertex2f((width / 2.f) - 50.0f, (height / 2.f) + 50.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

GLvoid drawShinobi(GLfloat x, GLfloat y) // Draws the shinobi
{
	GLint aux = 0;

	if (selected_shinobi == 1) // Different textures for the different shinobis
	{
		aux = 0;
		if (shinobimoving % 2) aux = 12; // Auxiliary variable that switches textures to give a sense of movement
	}

	if (selected_shinobi == 2)
	{
		aux = 13;
		if (shinobimoving % 2) aux = 14; 
	}

	if (selected_shinobi == 3)
	{
		aux = 15;
		if (shinobimoving % 5) aux = 16; 
	}

	if (selected_shinobi == 4)
	{
		aux = 17;
		if (shinobimoving % 2) aux = 18; 
	}

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[aux]); // Draws the shinobi with the selected texture

	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2d(0, 0); glVertex2f((x - 10.0f), y);
	glTexCoord2d(1, 0); glVertex2f(x + 10.0f, y);
	glTexCoord2d(1, 1); glVertex2f(x + 10.0f, y + 20.0f);
	glTexCoord2d(0, 1); glVertex2f(x - 10.0f, y + 20.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

GLvoid drawStore()
{
	glPushMatrix(); // Saves the current matrix in the stack

	glLoadIdentity(); // Prevents the counter from moving as the scene moves

	sprintf(string_oranges, "%d", oranges); // Copies oranges to a string

	drawOrange(width - 20.0f, height - 20.0f); // Draws the orange icon
	renderBitmapString(width - 38.0f, height - 18.0f, (GLvoid *)font, string_oranges); // Draws the orange counter

	glPopMatrix(); // Restores the matrix from the stack

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[20]); // Store icon #1

	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2d(0, 0); glVertex2f(width / 2 - 60.0f, height/2 + 10.0f);
	glTexCoord2d(1, 0); glVertex2f(width / 2 - 10.0f, height / 2 + 10.0f);
	glTexCoord2d(1, 1); glVertex2f(width / 2 - 10.0f, height / 2 + 60.0f);
	glTexCoord2d(0, 1); glVertex2f(width / 2 - 60.0f, height / 2 + 60.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[21]); // Store icon #2

	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2d(0, 0); glVertex2f(width / 2 + 10.0f, height / 2 + 10.0f);
	glTexCoord2d(1, 0); glVertex2f(width / 2 + 60.0f, height / 2 + 10.0f);
	glTexCoord2d(1, 1); glVertex2f(width / 2 + 60.0f, height / 2 + 60.0f);
	glTexCoord2d(0, 1); glVertex2f(width / 2 + 10.0f, height / 2 + +60.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[22]); // Store icon #3

	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2d(0, 0); glVertex2f(width / 2 - 60.0f, height / 2 - 60.0f);
	glTexCoord2d(1, 0); glVertex2f(width / 2 - 10.0f, height / 2 - 60.0f);
	glTexCoord2d(1, 1); glVertex2f(width / 2 - 10.0f, height / 2 - 10.0f);
	glTexCoord2d(0, 1); glVertex2f(width / 2 - 60.0f, height / 2 - 10.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[23]); // Store icon #4

	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2d(0, 0); glVertex2f(width / 2 + 10.0f, height / 2 - 60.0f);
	glTexCoord2d(1, 0); glVertex2f(width / 2 + 60.0f, height / 2 - 60.0f);
	glTexCoord2d(1, 1); glVertex2f(width / 2 + 60.0f, height / 2 - 10.0f);
	glTexCoord2d(0, 1); glVertex2f(width / 2 + 10.0f, height / 2 - 10.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[5]); // Draws the main menu icon

	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2d(1, 0); glVertex2f((width / 2) - 25.0f, 30.0f);
	glTexCoord2d(0, 0); glVertex2f((width / 2) + 25.0f, 30.0f);
	glTexCoord2d(0, 1); glVertex2f((width / 2) + 25.0f, 80.0f);
	glTexCoord2d(1, 1); glVertex2f((width / 2) - 25.0f, 80.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);

}

GLvoid drawMenu() // Draws the main menu
{
	drawPlayButton(); // Draws the play button
	drawInitialPlatform((width / 2.f) - 40.0f, (width / 2.f) + 40.0f, 0.0f, 80.0f); // Draws a platform
	drawShinobi((width / 2), 80.0f); // Draws the shinobi
	drawTitle(); // Draws the title

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[19]); // Draws the store icon

	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2d(0, 0); glVertex2f(width / 2 - 25.0f, height / 2 - 70.0f);
	glTexCoord2d(1, 0); glVertex2f(width / 2 + 25.0f, height / 2 - 70.0f);
	glTexCoord2d(1, 1); glVertex2f(width / 2 + 25.0f, height / 2 - 20.0f);
	glTexCoord2d(0, 1); glVertex2f(width / 2 - 25.0f, height / 2 - 20.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

GLvoid rotateStick(GLfloat angle, GLfloat xi, GLfloat yi) // Rotates the stick 90º clockwise
{
	GLfloat size = 5.0f; // Stick width
	
	glPushMatrix(); // glPushMatrix() and glPopMatrix() used to isolate the transformations from the other objects
	glTranslatef(xi + size, yi, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glTranslatef(-(xi + size), -yi, 0.0f);
	drawStick(xi + size, yi);
	glPopMatrix();
}

GLvoid drawFlippedShinobi(GLfloat x, GLfloat y) // Draws the shinobi on the other side of the stick
{
	GLint aux = 0;

	if (selected_shinobi == 1)
	{
		aux = 0;
		if (shinobimoving % 2) aux = 12; 
	}

	if (selected_shinobi == 2)
	{
		aux = 13;
		if (shinobimoving % 2) aux = 14; 
	}

	if (selected_shinobi == 3)
	{
		aux = 15;
		if (shinobimoving % 2) aux = 16; 
	}

	if (selected_shinobi == 4)
	{
		aux = 17;
		if (shinobimoving % 2) aux = 18; 
	}

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[aux]);

	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2d(0, 1); glVertex2f((x - 10.0f), y);
	glTexCoord2d(1, 1); glVertex2f(x + 10.0f, y);
	glTexCoord2d(1, 0); glVertex2f(x + 10.0f, y + 22.0f);
	glTexCoord2d(0, 0); glVertex2f(x - 10.0f, y + 22.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

GLvoid drawCounters() // Draws counters for oranges and current score
{
	glPushMatrix(); // Saves the current matrix in the stack

	glLoadIdentity(); // Prevents the counter from moving as the scene moves

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[11]); // Draws the score counter background

	GLfloat cf = 12.0f;
	if (score > 9) cf = 15.0f; // Adjusts the counter size

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2d(0, 0); glVertex2f((width / 2) - 7.0f, height - 38.0f);
	glTexCoord2d(1, 0); glVertex2f((width / 2) + cf, height - 38.0f);
	glTexCoord2d(1, 1); glVertex2f((width / 2) + cf, height - 58.0f);
	glTexCoord2d(0, 1); glVertex2f((width / 2) - 7.0f, height - 58.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	sprintf(string_score, "%d", score); // Copies score to a string

	renderBitmapString(width/2, height - 50.0f, (GLvoid *)font, string_score); // Displays the score

	sprintf(string_oranges, "%d", oranges); // Copies oranges to a string

	drawOrange(width - 20.0f, height - 20.0f); // Draws the orange icon
	renderBitmapString(width - 38.0f, height - 18.0f, (GLvoid *)font, string_oranges); // Draws the orange counter

	glPopMatrix(); // Restores the matrix from the stack
}

GLvoid display() // Function that effectively does all the drawing, guided by gamestates and calling the other functions
{
	glClear(GL_COLOR_BUFFER_BIT); // Clears the scene
	
	glPushMatrix();
	glLoadIdentity();
	drawBackground(); // Draws the background and prevents it from moving
	glPopMatrix();

	if (gamestate == 0) // Checks if the game state is set to 0
	{
		if (store == 0) drawMenu(); // If the store is not open, draws the main menu
		if (store == 1) drawStore(); // Draws the store if the "store" button was clicked
		if (mouse_down == 3) newGame();
		if (mouse_down == 0) gamestate = 1; // Starts a new game if the "Play" button was clicked
		
	}

	if (gamestate == 1 || gamestate == 2) // Checks if the game is running (not on a menu)
	{
		if (flip_shinobi == 0) drawShinobi(x_shinobi, y_shinobi); // Draws the shinobi
		if (flip_shinobi == 1) drawFlippedShinobi(x_shinobi, y_shinobi-22.0f); // Draws the flipped shinobi if the user clicked the mouse

		drawInitialPlatform(0.0f, (width / 5), 0.0f, 125.0f); // Draws the starting platform

		if (stage == 0) // Checks if the user just started the game
		{
			if (game_start == 1) renderBitmapString(width / 3, 310, (GLvoid *)font, "Click the left mouse button");
			if (game_start == 1) renderBitmapString(width / 3 + 10.0f, 300, (GLvoid *)font, "to stretch out the stick"); // Displays the initial help text

			if (game_start == 0) drawInitialPlatform(random[8].xi, random[8].xf, 0.0f, 125.0f); // Also draws the platform from the last stage, since it will appear on screen
		}
		
		if (stage == 1) 
		{
			glPushMatrix();
			glLoadIdentity();
			if (game_start == 1) renderBitmapString(width / 3 - 20.0f, 310, (GLvoid *)font, "Click the left mouse button while walking");
			if (game_start == 1) renderBitmapString(width / 3, 300, (GLvoid *)font, "on the stick to flip the shinobi"); // Displays the second help text
			glPopMatrix();
		}
		
		if (stage != 0) 
		{
			drawInitialPlatform(random[stage-1].xi, random[stage-1].xf, 0.0f, 125.0f); // Also draws the platform from the last stage, since it will appear on screen
			
			if (orange_hit == 0) drawRandomorange(); // After getting to the second platform, oranges may appear. The variable makes them disappear if caught by the shinobi
		}

		drawPlatform(random[stage].xi, random[stage].xf, 0.0f, 125.0f); // Draws the next platform

		if (k == 0)
		{
			k = 2;
			rotateStick(rotation, x_shinobi_start[k], 126.0f);
			k = 0;
		}

		if (k != 0)
		{
			k--;
			rotateStick(rotation, x_shinobi_start[k], 126.0f); // Temporarily decreases k's value to draw the last stick, since it may appear on screen
			k++;
		}

		drawCounters(); // Draws the score and orange counters
	}

	if (gamestate == 1) // Checks if the game state is set to 1 (waiting for user input)
	{
		x_shinobi_start[k] = x_shinobi; // Defines the new starting position
		drawStick(x_shinobi_start[k] + 10.0f, 125.0f); // Draws the new stick
		if (mouse_down == 1) stick_growth[k] += 0.07f; // Increases the stick height while the mouse is clicked
	}

	if (gamestate == 2) // Checks if the game state is set to 2 (game processing the user action)
	{
		rotateStick(rotation, x_shinobi_start[k], 126.0f); // Draws the rotated stick
		rotation -= 1.0f; // Increases the rotation angle
		if (rotation <= -90.0f) rotation = -90.0f; // Rotates the stick until it is rotated 90º clockwise

		if ((x_shinobi_start[k] + 5.0f + stick_growth[k]) >= (midpoint - 3.0f) && (x_shinobi_start[k] + 5.0f + stick_growth[k]) <= (midpoint + 3.0f) && perfect_control == 0) // Checks if the stick landed on the midpoint of the platform
		{
			printf("Perfect!\n");
			score++; // Landing on the middle of the platform grants a "perfect" and +1 score
			printf("Score: %d\n", score);
			perfect_control = 1;
		}

		if (flip_shinobi == 1 && x_shinobi > random[stage].xi - 6.0f) y_shinobi -= 0.45; // If the shinobi hits the platform while flipped, it dies

		if (flip_shinobi == 1 && x_shinobi + 10.0f > xi_orange) orange_hit = 1; // Checks if a orange was caught

		if (x_shinobi < x_shinobi_start[k] + stick_growth[k])
		{
			x_shinobi += stepsize; // Moves the shinobi until it reaches the end of the stick
			shinobimoving++;
		}

		if (mouse_down == 1 && x_shinobi < random[stage].xi - 6.0f) flip_shinobi = 1; // If the shinobi is on the stick, clicking the mouse will flip it
		if (mouse_down == 2) flip_shinobi = 0; // If the shinobi is flipped, clicking the mouse will revert it back to its original position

		if (x_shinobi > x_shinobi_start[k] + stick_growth[k])
		{
			x_shinobi_final[k] = x_shinobi; // When the shinobi reaches the end of the stick, defines the end position

			if (x_shinobi_final[k] < random[stage].xi - 6.0f || x_shinobi_final[k] > random[stage].xf - 6.0f)
			{
				y_shinobi -= 0.45f; // If the end position is outside of the platform, the shinobi falls
			}

			if (x_shinobi_final[k] >= random[stage].xi - 6.0f && x_shinobi_final[k] <= random[stage].xf - 6.0f) // Checks if the end position is inside the platform
			{
				mouse_down = 0; // Prevents flipping on the platform

				if (x_shinobi + 15.0f <= random[stage].xf) 
				{
					x_shinobi += stepsize; // Advances the shinobi to a position close to the end of the platform
					shinobimoving++;
				}

				if (x_shinobi + 15.0f > random[stage].xf)
				{
					nextStage(); // When the shinobi reaches the specified position, the game advances to the next stage
				}
			}
		}
	}

	if (y_shinobi <= 0.0f) // If the shinobi landed outside the platform and fell, changes the game state to 3 (death)
	{
		gamestate = 3;
	}

	if (gamestate == 3) // Checks if the game state is set to 3
	{
		if (score > highscore) highscore = score; // Updates the high score
		
		glLoadIdentity(); // Restores the default matrix to draw the death menu

		drawDeathMenu(); // Draws the death menu and saves the game

		if (mouse_down == 3) newGame(); 
		if (mouse_down == -1) gamestate = 1; // If the user clicked the "repeat" button, starts a new game

		if (mouse_down == 4)
		{
			newGame();
			gamestate = 0; // If the user clicked the "main menu" button, opens the main menu
		}
	}

	glutSwapBuffers();

	glutPostRedisplay();
}

GLvoid ChangeSize(GLsizei	w, GLsizei h) // Called by GLUT library when the window has changed size
{
	// Prevent a divide by zero	
	if (h == 0)
		h = 1;

	// Set Viewport to window dimensions	
	glViewport(0, 0, w, h);

	// Reset coordinate system
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Establish clipping volume (left, right, bottom, top, near, far)
	if (w <= h)
	{
		glOrtho(0.0f, 250.0f, 0.0f, 250.0f*h / w, 1.0, -1.0);
		height = 250.0f*h / w; // Stores the new dimensions in variables, so they can be used for objects' positions
	}
	else
	{
		glOrtho(0.0f, 250.0f*w / h, 0.0f, 250.0f, 1.0, -1.0);
		width = 250.0f*w / h; // Stores the new dimensions in variables, so they can be used for objects' positions
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

GLvoid HandleMouse(GLint button, GLint state, GLint x, GLint y) // Function that controls all mouse events
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) // Checks if the left mouse button was pressed
	{
		if (gamestate == 0)
		{
			if (x >= 235 && x <= 356 && y >= 438 && y <= 560 && store == 0) mouse_down = 3; // Play button clicked
			// TEST CODE 
			/*
			printf("x: %d\n", x);
			printf("y: %d\n", y);
			*/
			if (x >= 243 && x <= 359 && y >= 588 && y <= 622) store = 1; // Store button clicked

			if (store == 1) // Checks if the store is open
			{
				if (x >= 241 && x <= 359 && y >= 807 && y <= 926)
				{
					store = 0; // Main menu button clicked, closes the store
				}

				if (x >= 160 && x <= 274 && y >= 360 && y <= 468) // Default shinobi clicked
				{
					selected_shinobi = 1;
					printf("shinobi selected\n");
				}

				if (x >= 325 && x <= 439 && y >= 358 && y <= 467) // Square shinobi clicked
				{
					if (shinobi_bought[0] == 0) // Checks if the shinobi was bought before
					{
						if (oranges < 10) printf("Not enough oranges\n"); // Nothing happens if the user has not enough oranges

						if (oranges >= 10) 
						{
							oranges -= 10;
							shinobi_bought[0] = 1;
							selected_shinobi = 2;

							printf("New shinobi unlocked\n"); // If the user has enough oranges, the shinobi is bought and selected
						}
					}

					if (shinobi_bought[0] == 1)
					{
						selected_shinobi = 2;
						printf("shinobi selected\n"); // If the shinobi was bought before, no oranges are spent and the shinobi is selected
					}
				}

				if (x >= 159 && x <= 273 && y >= 525 && y <= 639) // Ghost shinobi clicked
				{
					if (shinobi_bought[1] == 0)
					{
						if (oranges < 20) printf("Not enough oranges\n");

						if (oranges >= 20) 
						{
							oranges -= 20;
							shinobi_bought[1] = 1;
							selected_shinobi = 3;

							printf("New shinobi unlocked\n");
						}
					}

					if (shinobi_bought[1] == 1)
					{
						selected_shinobi = 3;
						printf("shinobi selected\n");
					}
				}

				if (x >= 325 && x <= 443 && y >= 522 && y <= 640)  // Guerrilla shinobi clicked
				{
					if (shinobi_bought[2] == 0)
					{
						if (oranges < 30) printf("Not enough oranges\n");

						if (oranges >= 30)
						{
							oranges -= 30;
							shinobi_bought[2] = 1;
							selected_shinobi = 4;

							printf("New shinobi unlocked\n");
						}
					}

					if (shinobi_bought[2] == 1) 
					{
						selected_shinobi = 4; 
						printf("shinobi selected\n");
					}
				}
			}
		}
		
		if (gamestate == 1) mouse_down = 1;  // Stretches out the stick

		if (gamestate == 2)
		{
			mouse_down = 1;
			if (mouse_down == 1 && flip_shinobi == 1) mouse_down = 2; // Flips the shinobi
		}

		if (gamestate == 3) // Death menu
		{
			if (x >= 349 && x <= 466 && y >= 622 && y <= 739) mouse_down = 3; // New game button clicked
			if (x >= 132 && x <= 249 && y >= 622 && y <= 739) mouse_down = 4; // Main Menu button clicked
		}
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) // Checks if the left mouse button was released
	{
		if (gamestate == 0)
		{
			if (x >= 235 && x <= 356 && y >= 438 && y <= 560 && store == 0) mouse_down = 0;
		}

		if (gamestate == 1)
		{
			mouse_down = 0;
			gamestate = 2;
		}

		if (gamestate == 2 && mouse_down == 2)
		{
			mouse_down = 0;
		}

		if (gamestate == 3)
		{
			if (x >= 349 && x <= 466 && y >= 622 && y <= 739) mouse_down = -1; 
			if (x >= 132 && x <= 249 && y >= 622 && y <= 739) mouse_down = 0; 
		}
	}
}

// TEST CODE
GLvoid HandleKeyboard(unsigned char key, GLint x, GLint y)
{
	switch (key) {
	case 'a':
		oranges += 20;
		break;
	default:
		break;
	}
}

int main(int argc, char** argv)
{
	loadSave(); // Loads an existing save

	glutInit(&argc, argv); // Initializes GLUT
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA); // Sets the buffers
	glutInitWindowSize(window_width, window_height);   // Sets the initial window's width and height
	glutInitWindowPosition(650, 45); // Position the initial window's top-left corner
	glutCreateWindow("Mini Shinobi");  // Create window with the given title

	initGL();                       // OpenGL initialization

	glutDisplayFunc(display);       // Register callback handler for window re-paint event
	glutReshapeFunc(ChangeSize);	// Register callback handler for window resize event

	glutMouseFunc(HandleMouse);     // Register callback handler for mouse event
	
	// TEST CODE
	glutKeyboardFunc(HandleKeyboard);

	glutMainLoop();                 // Enter infinitely event-processing loop
	
}