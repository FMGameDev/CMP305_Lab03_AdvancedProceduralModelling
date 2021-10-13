// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "LightShader.h"
#include "TerrainMesh.h"


class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void gui();

private:
	LightShader* shader;
	TerrainMesh* m_Terrain;

	Light* light;

	int terrainResolution = 128;

	// Frecuency and Amplitude for Waves
	float terrainXFrequency = 0.1f;
	float terrainZFrequency = 0.033f;

	float terrainXAmplitude = 10.0f;
	float terrainZAmplitude = 10.0f;

	int terrainMaxHeight = 20; // max random height
};

#endif