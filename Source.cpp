#include <stdio.h>
#include <stdlib.h>
#include "glut.h"
#include "glm/glm.hpp"
#include "font.h"
#include "Rect.h"
#include "Ball.h"
#include <time.h>
#include "Paddle.h"
#include "audio.h"
#define PLAYER_MAX 2
#define SCORE_MAX 11

using namespace glm;
#define BALL_MAX 2
ivec2 windowSize = { 800, 600 };

bool keys[256];
Paddle paddle;
Paddle paddles[PLAYER_MAX];
Ball balls[BALL_MAX];
Ball ball = { 8 };
int scores[PLAYER_MAX];
int wait;
bool started;

//Rect rect1 = Rect(vec2(100, 100), vec2(100, 200));
//Rect rect2 = Rect(vec2(windowSize.x/2, windowSize.y / 2), vec2(200, 100));

void display(void) {
	
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);//(GLenum mode);
	glLoadIdentity();
	gluOrtho2D(
		0,//GLdouble left,
		windowSize.x,//GLdouble right,
		windowSize.y,	//GLdouble bottom,
		0);	//GLdouble top);
	glMatrixMode(GL_MODELVIEW);//GLenum mode
	glLoadIdentity();
	
	{
		GLfloat range[2];
		glGetFloatv(
			GL_LINE_WIDTH_RANGE,//GLenum pname, 
			range);//GLfloat * params
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glEnable(GL_LINE_STIPPLE);//GLenum cap)
		glLineStipple(
		range[1],      //GLint factor,;
		0x5555);	// GLushort pattern)
		
		glBegin(GL_LINES);
		{
			for (int i=0; i < 2; i++) {
				glVertex2f(windowSize.x/2,windowSize.y*i);//GLfloat x, GLfloat y);
			}
		}
		glEnd();
		glPopAttrib();
	}

	if(wait<=0)
		ball.draw();

	if(started)
	for (int i = 0; i < PLAYER_MAX; i++)
		paddles[i].draw();
	

	fontBegin();
	//fontSetColor(0, 0xff, 0);
	fontSetHeight(FONT_DEFAULT_HEIGHT);//FONT_DEFAULT_SIZE/40);
	fontSetWeight(fontGetWeightMax());
	float y = fontGetWeight();
	for (int i = 0; i < PLAYER_MAX; i++) {
		fontSetPosition(windowSize.x/4-80+windowSize.x/2*i, y);
		fontDraw("%2d",scores[i]);
}
	
	fontEnd();

	glutSwapBuffers();
};

void idle(void){
	audioUpdate();
	if (started) {
		if (wait > 0) {
			wait--;
			if ((wait <= 0) && (scores[0] >= SCORE_MAX)
				|| (scores[1] >= SCORE_MAX)) {
				started = false;
			}
		}
		float paddleSpeed = 8;
		if (keys['w']) paddles[0].m_position.y -= paddleSpeed;
		if (keys['s']) paddles[0].m_position.y += paddleSpeed;
		if (keys['i']) paddles[1].m_position.y -= paddleSpeed;
		if (keys['k']) paddles[1].m_position.y += paddleSpeed;

		for (int i = 0; i < PLAYER_MAX; i++) {
			paddles[i].m_position.y = max(paddles[i].m_position.y, 0.f);
			paddles[i].m_position.y = min(paddles[i].m_position.y, windowSize.y - paddles[i].m_height);

		}
	}
	if (wait <= 0) {
		ball.update();

		if ((ball.m_position.x < 0)
			|| (ball.m_position.x >= windowSize.x)) {
			if (started) {     //start game
				audioLength(1000);
				audioDecay(0);
				audioFreq(440/4);
				audioPlay();
				if (ball.m_position.x < 0)
					scores[1]++;
				else
					scores[0]++;

				wait = 60;

				ball.m_position.x = windowSize.x / 2;
				ball.m_lastPosition = ball.m_position;
			}
			else {   //�f����

				ball.m_position = ball.m_lastPosition;
				ball.m_speed.x *= -1;
			}
		}
		if ((ball.m_position.y < 0)||
			(ball.m_position.y >= windowSize.y) ){
			if (started) {
				audioDecay(.9);
				audioFreq(440/2);
				audioPlay();
			}
			ball.m_position = ball.m_lastPosition;
			ball.m_speed.y *=-1 ;
		}
		/*if (ball.m_position.y >= windowSize.y) {
			ball.m_position = ball.m_lastPosition;
			ball.m_speed.y = -fabs(ball.m_speed.y);
		}*/


		if (started) {
			for (int i = 0; i < PLAYER_MAX; i++) {
				if (paddles[i].intersectBall(ball)) {
					audioWaveform(AUDIO_WAVEFORM_PULSE_50);
					audioDecay(.9);
					audioFreq(440 * 1.5);
					audioPlay();
					ball.m_position = ball.m_lastPosition;
					ball.m_speed.x *= -1;

					float paddleCenterY = paddles[i].m_position.y + paddles[i].m_height / 2;
					float subMax = paddles[i].m_height;
					ball.m_speed.y = (ball.m_position.y - paddleCenterY) / subMax * 16;
				}
			}
		}
	}
	
	glutPostRedisplay();
}
void timer(int value) {
	glutPostRedisplay();
	glutTimerFunc(1000 / 60, timer, 0);
}
void reshape(int width, int height) {
	//printf("rehape:width:%d height:%d\n",width,height);
	glViewport(0,0,width,height);
	//GLint x, GLint y, GLsizei width, GLsizei height);
	windowSize = ivec2(width, height);
	
}
void keyboard(unsigned char key, int x, int y) {
	if (key == 0x1b)
		exit(0);
	if (!started) {
		for (int i = 0; i < PLAYER_MAX; i++) {
			scores[i] = 0;
			paddles[i].m_height = 64;
			paddles[i].m_position.y = (windowSize.y - paddles[i].m_height) / 2;
		}
		paddles[0].m_position.x = 100;
		paddles[1].m_position.x = windowSize.x - 100;
		
		ball.m_position=
			ball.m_lastPosition= vec2(windowSize.x, windowSize.y)/2.f;

		started = true;
	}
	//printf("keyboard:\'%c\'(%#x)\n", key, key);
	keys[key] = true;
}
void keyboardUp(unsigned char key, int x, int y) {
	//printf("keyboardUp:\'%c\'(%#x)\n", key, key);
	keys[key] = false;
}
void passiveMotion(int x, int y) {
	paddle.m_position = vec2(x, y);
}
int main(int argc, char* argv[]) {
	srand(time(NULL));
	ball.m_speed = vec2(1, 1)*4.f;
	audioInit();
	
	glutInit(&argc, argv);

	glutInitDisplayMode(GL_DOUBLE);
	glutInitWindowPosition(640,0);
	glutInitWindowSize(windowSize.x, windowSize.y);
	glutCreateWindow("a");
	glutDisplayFunc(display);
	//glutTimerFunc(0, timer, 0);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);//void (GLUTCALLBACK *func)(int width, int height));
	glutKeyboardFunc(keyboard);//GLUTCALLBACK *func)(unsigned char key, int x, int y));
	glutKeyboardUpFunc(keyboardUp);//void (GLUTCALLBACK *func)(unsigned char key, int x, int y));
	//glutPassiveMotionFunc(passiveMotion); //void (GLUTCALLBACK * func)(int x, int y));
    //glutMotionFunc(motion); void (GLUTCALLBACK * func)(int x, int y));
	glutIgnoreKeyRepeat(GL_TRUE);//int ignore
	glutMainLoop();
}