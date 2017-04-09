/*
	This File implements the glut window
*/
#include "VisualFFB.h"
#include "FFBRender.h"
#include "FFBManager.h"
#include "deviceBridge.h"

#include<iostream>
#include<string>
#include<Windows.h>
#include<gl\GL.h>
#include<gl\GLU.h>
#include"GL\glut.h"
#include <time.h>
char line[MaxLineLength];
static FFBRender ffbRender;
double thoroughW, thoroughH;

int Visual_main() {
	cout << "FFBvisual Window/handle Start" << endl;
	HANDLE hThread = CreateThread(NULL, 0, Glut_Window, NULL, 0, NULL); //start render thread
	return 0;
}

DWORD WINAPI Glut_Window(LPVOID args) {
	thoroughH = 800;
	thoroughW = 600;

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(400, 300);
	glutCreateWindow("FFBviusal");
	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutKeyboardFunc(glKeyHandler);
	SetupRC();
	glutTimerFunc(30, glOnTimer, 1);
	glutMainLoop();
	return 0;
}
void RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void ChangeSize(GLsizei w, GLsizei h) {
	//if(h==0) h=1;
	printf("width:%d height:%d\n", w, h);
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w*thoroughH <= h*thoroughW) glOrtho(0.0f - Edgewidth, thoroughW - Edgewidth, \
		0.0f - Edgewidth, thoroughW*h / w - Edgewidth, 1.0f, -1.0f);
	else glOrtho(0.0f - Edgewidth, thoroughH*w / h - Edgewidth, 0.0f - Edgewidth, thoroughH - Edgewidth, 1.0f, -1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
void glKeyHandler(unsigned char key, int x, int y) {
	cout << "X " << x << " Y " << y << endl;
	cout << "Key " << key << endl;
}
void SetupRC(void) {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void glOnTimer(int id) {
	//Get gl runTime in ms
	static time_t currentTime, previousTime=clock(), timeSpan, glRunTime=0;
	currentTime=clock();
	timeSpan=currentTime-previousTime;
	glRunTime+=timeSpan; 

	int32_t Tx,Ty;
	const int maxForce = 10000; //gridWidth = 100
	static BYTE output[6];

	FFBMngrEffRun(timeSpan, Tx, Ty); //max Force = 10000
	PInputReport posHandle = DeviceGetPos();
	if(Tx > maxForce) Tx=maxForce;
	if(Tx < -maxForce) Tx=-maxForce;
	if(Ty > maxForce) Ty = maxForce;
	if(Ty < -maxForce) Ty = -maxForce;	

	FormOutReport(output, Tx, Ty);
	DeviceWrite(output, 6);
	ffbRender.PutXYT(Tx / 100.0, -Ty / 100.0, glRunTime); //rende force
	ffbRender.RenderFFBIndicator();
	ffbRender.RenderStickXY(posHandle->x * 100 / 2048.0, -posHandle->y * 100 / 2048.0);

	//prepare for next action
	glutPostRedisplay(); 
	previousTime=currentTime;
	glutTimerFunc(30, glOnTimer, 1);
}