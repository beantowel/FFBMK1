#pragma once
#include<Windows.h>
#include<gl\GL.h>
using namespace std;

const int MaxLineLength=128;
extern double thoroughW, thoroughH;
const double Edgewidth = 0.4;

int Visual_main();
DWORD WINAPI Glut_Window(LPVOID args);
void RenderScene();
void ChangeSize(GLsizei w, GLsizei h);
void glKeyHandler(unsigned char key, int x, int y);
void SetupRC(void);
void glOnTimer(int id);