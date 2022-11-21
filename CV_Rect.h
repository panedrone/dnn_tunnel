#pragma once

class CV_Rect
{
public:

	CV_Rect()
	{
	}

	CV_Rect(int x, int y, int w, int h)
	{
		X = x;
		Y = y;
		Width = w;
		Height = h;
	}

	int X = 0;
	int Y = 0;
	int Width = 0;
	int Height = 0;
};

