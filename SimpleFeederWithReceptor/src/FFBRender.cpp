#include "FFBRender.h"

#include<iostream>
#include<windows.h>
#include<math.h>
#include<stdio.h>
#include<gl\GL.h>
#include<gl\GLU.h>
#include"GL\glut.h"
using namespace std;
FFBRender::FFBRender() {
	cout << "start rendering" << endl;
}
FFBRender::~FFBRender() {
	cout << "render dismissed" << endl;
}
void FFBRender::RenderLines(float *x, float *y, int len) {
	int i;
	glColor3f(0.0f, 0.0f, 0.3f);
	glBegin(GL_LINE_STRIP);
	for (i = 0; i < len; i++) {
		glVertex2f(x[i], y[i]);
	};
	glEnd();
	glFlush();
}
void FFBRender::PutXYT(float x, float y, time_t t){ //finite stack
	for (int i = nArray-1; i > 0; i--) {
		dataX[i] = dataX[i - 1];
		dataY[i] = dataY[i - 1];
		dataT[i] = dataT[i - 1];
	}
	dataX[0] = x;
	dataY[0] = y;
	dataT[0] = t;
}
void FFBRender::RenderFFBIndicator() {
	float temp;
	glColor3f(0.0f, 0.0f, 0.3f);
	glLineWidth(1.0);
	glBegin(GL_LINES); //Draw Force Vector
	glVertex2f(xZeroPosition, yZeroPosition);
	glVertex2f(dataX[0]+xZeroPosition, dataY[0]+yZeroPosition);
	glEnd();
	glFlush();

	glLineWidth(1.0);
	glBegin(GL_LINE_STRIP); //Draw Oscilloscope X-T
	for (int i = 0; i < nArray; i++) {
		temp = float(dataT[0]-dataT[i]);
		temp = temp / timeScale;
		glVertex2f(dataX[i]+xZeroPosition, yZeroPosition - gridWidth - temp);
	}
	glEnd();
	glFlush();
	glBegin(GL_LINE_STRIP); //Draw Oscilloscope Y-T
	for (int i = 0; i < nArray; i++) {
		temp = float(dataT[0] - dataT[i]);
		temp = temp / timeScale;
		glVertex2f(xZeroPosition + gridWidth + temp, dataY[i]+yZeroPosition);
	}
	glEnd();
	glFlush();

	glLineStipple(2, 0x5555); //Draw frame and Force trajectory
	glEnable(GL_LINE_STIPPLE);
	glBegin(GL_LINE_LOOP);
	glVertex2f(xZeroPosition - gridWidth, yZeroPosition - gridWidth);
	glVertex2f(xZeroPosition - gridWidth, yZeroPosition + gridWidth);
	glVertex2f(xZeroPosition + gridWidth, yZeroPosition + gridWidth);
	glVertex2f(xZeroPosition + gridWidth, yZeroPosition - gridWidth);
	glEnd();
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < nForceTrajectory; i++) {
		glVertex2f(dataX[i] + xZeroPosition, dataY[i] + yZeroPosition);
	}
	glEnd();
	glDisable(GL_LINE_STIPPLE);
	glFlush();
}