#pragma once
#include "PlaneMesh.h"
#include "Emitter.h"
#include "Utils.h"

class TerrainMesh :
	public PlaneMesh {
public:
	TerrainMesh( ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 128 );
	~TerrainMesh();

	void Resize( int newResolution );
	void SetWavesFrequency(float newXFrequency, float newZFrequency) { xFrequency = newXFrequency; zFrequency = newZFrequency; };
	void SetWavesAmplitude(float newXAmplitude, float newZAmplitude) { xAmplitude = newXAmplitude; zAmplitude = newZAmplitude; };
	void SetMaxHeight(int newMaxHeight) { maxHeight = newMaxHeight; }
	void Regenerate( ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	// set to 0 the height of every point
	void Flatten();
	// Fault is made adding or subtracting the max height value
	void Fault();
	// Algorithm from 3D Game Programming with Directx11 by Frank D. Luna (Page 603)
	void Smooth();
	// Randomly distribute, or emit, particles across the surface of our terrain.
	// Each time a particle "lands", raise the terrain a little
	// As the particles stack up, you get natural raises in the terrain and organic features
	// if there is a lower point to the left, right, up, down to the particle deposition then it is placed there.
	void ParticleDeposition();
	// susbtract height to the highest point surrounding the particle position
	void AntiParticleDeposition();
	void BuildCustomHeightMap(); // build height map for customised frequency and amplitud
	void BuildRandomHeightMap(int min, int max); // build height map with random numbers

	const inline int GetResolution(){ return resolution; }
	const inline float GetXFrequency() { return xFrequency; }
	const inline float GetZFrequency() { return xFrequency; }
	const inline float GetXAmplitude() { return xAmplitude; }
	const inline float GetZAmplitude() { return zAmplitude; }
	const inline float GetMaxHeight() { return maxHeight; }

private:
	void CreateBuffers( ID3D11Device* device, VertexType* vertices, unsigned long* indices );

	// check if a point is in the map/terrain
	bool InBounds(int i, int k);
	// return the height average of the neighbours to that point (inluding that point too)
	float NeighboursAverage(int i, int k);
	// return a random position from the map
	XMFLOAT3 GetRandomPos();

	Emitter* emitter_;
	
	const float m_UVscale = 10.0f;			//Tile the UV map 10 times across the plane
	const float terrainSize = 100.0f;		//What is the width and height of our terrain
	float* heightMap;

	// Frecuency and Amplitude for Waves
	float xFrequency = 0.1f;
	float zFrequency = 0.033f;

	float xAmplitude = 10.0f;
	float zAmplitude = 10.0f;

	float xOffset = 0.0f;
	float zOffset = 0.0f;

	int maxHeight = 20;  // max random height
};
