#pragma once
#include<time.h>
extern double thoroughW, thoroughH;

class FFBRender {
public:
	FFBRender();
	~FFBRender();
	void RenderLines(float *x, float *y, int len);
	void PutXYT(float x, float y, time_t t);
	void RenderFFBIndicator();
private:
	const float xZeroPosition = 400, yZeroPosition = 500, \
		gridWidth = 100, tAxisLength = 200, timeScale = 10.0;
	const int nForceTrajectory=40;
	static const int nArray = 100;
	float dataX[nArray] = { 0 }, dataY[nArray] = { 0 };
	time_t dataT[nArray] = { 0 };
};
