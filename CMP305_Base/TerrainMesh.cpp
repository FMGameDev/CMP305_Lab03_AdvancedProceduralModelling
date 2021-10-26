#include "TerrainMesh.h"

#define _USE_MATH_DEFINES // it has to be set the first thing before any include <>
#include <cmath>

#include <cstdlib>
#include <time.h>       /* time */


TerrainMesh::TerrainMesh( ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution ) :
	PlaneMesh( device, deviceContext, lresolution ) 
{
	/* initialize random seed: */
	srand(time(NULL));

	Resize( resolution );
	Flatten();
	Regenerate( device, deviceContext );

	emitter = new Emitter(GetRandomPos()); // create emitter and set it in a random pos
}

TerrainMesh::~TerrainMesh()
{
	delete[] heightMap;
	heightMap = 0;

	delete emitter;
	emitter = nullptr;
}


void TerrainMesh::CreateBuffers(ID3D11Device* device, VertexType* vertices, unsigned long* indices) {

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Set up the description of the dyanmic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
}

void TerrainMesh::Resize(int newResolution) {
	resolution = newResolution;
	heightMap = new float[resolution * resolution];
	if (vertexBuffer != NULL) {
		vertexBuffer->Release();
	}
	vertexBuffer = NULL;
}

void TerrainMesh::Regenerate(ID3D11Device* device, ID3D11DeviceContext* deviceContext) {

	VertexType* vertices;
	unsigned long* indices;
	int index, i, j;
	float positionX, height, positionZ, u, v, increment;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Calculate the number of vertices in the terrain mesh.
	// We share vertices in this mesh, so the vertex count is simply the terrain 'resolution'
	// and the index count is the number of resulting triangles * 3 OR the number of quads * 6
	vertexCount = resolution * resolution;

	indexCount = ((resolution - 1) * (resolution - 1)) * 6;
	vertices = new VertexType[vertexCount];
	indices = new unsigned long[indexCount];

	index = 0;

	// UV coords.
	u = 0;
	v = 0;
	increment = m_UVscale / resolution;

	//Scale everything so that the look is consistent across terrain resolutions
	const float scale = terrainSize / (float)resolution;

	//Set up vertices
	for (j = 0; j < (resolution); j++) {
		for (i = 0; i < (resolution); i++) {
			positionX = (float)i * scale;
			positionZ = (float)(j)*scale;

			height = heightMap[index];
			vertices[index].position = XMFLOAT3(positionX, height, positionZ);
			vertices[index].texture = XMFLOAT2(u, v);

			u += increment;
			index++;
		}
		u = 0;
		v += increment;
	}

	//Set up index list
	index = 0;
	for (j = 0; j < (resolution - 1); j++) {
		for (i = 0; i < (resolution - 1); i++) {

			//Build index array
			indices[index] = (j * resolution) + i;
			indices[index + 1] = ((j + 1) * resolution) + (i + 1);
			indices[index + 2] = ((j + 1) * resolution) + i;

			indices[index + 3] = (j * resolution) + i;
			indices[index + 4] = (j * resolution) + (i + 1);
			indices[index + 5] = ((j + 1) * resolution) + (i + 1);
			index += 6;
		}
	}

	//Set up normals
	for (j = 0; j < (resolution - 1); j++) {
		for (i = 0; i < (resolution - 1); i++) {
			//Calculate the plane normals
			XMFLOAT3 a, b, c;	//Three corner vertices
			a = vertices[j * resolution + i].position;
			b = vertices[j * resolution + i + 1].position;
			c = vertices[(j + 1) * resolution + i].position;

			//Two edges
			XMFLOAT3 ab(c.x - a.x, c.y - a.y, c.z - a.z);
			XMFLOAT3 ac(b.x - a.x, b.y - a.y, b.z - a.z);

			//Calculate the cross product
			XMFLOAT3 cross;
			cross.x = ab.y * ac.z - ab.z * ac.y;
			cross.y = ab.z * ac.x - ab.x * ac.z;
			cross.z = ab.x * ac.y - ab.y * ac.x;
			float mag = (cross.x * cross.x) + (cross.y * cross.y) + (cross.z * cross.z);
			mag = sqrtf(mag);
			cross.x /= mag;
			cross.y /= mag;
			cross.z /= mag;
			vertices[j * resolution + i].normal = cross;
		}
	}

	//Smooth the normals by averaging the normals from the surrounding planes
	XMFLOAT3 smoothedNormal(0, 1, 0);
	for (j = 0; j < resolution; j++) {
		for (i = 0; i < resolution; i++) {
			smoothedNormal.x = 0;
			smoothedNormal.y = 0;
			smoothedNormal.z = 0;
			float count = 0;
			//Left planes
			if ((i - 1) >= 0) {
				//Top planes
				if ((j) < (resolution - 1)) {
					smoothedNormal.x += vertices[j * resolution + (i - 1)].normal.x;
					smoothedNormal.y += vertices[j * resolution + (i - 1)].normal.y;
					smoothedNormal.z += vertices[j * resolution + (i - 1)].normal.z;
					count++;
				}
				//Bottom planes
				if ((j - 1) >= 0) {
					smoothedNormal.x += vertices[(j - 1) * resolution + (i - 1)].normal.x;
					smoothedNormal.y += vertices[(j - 1) * resolution + (i - 1)].normal.y;
					smoothedNormal.z += vertices[(j - 1) * resolution + (i - 1)].normal.z;
					count++;
				}
			}
			//right planes
			if ((i) < (resolution - 1)) {

				//Top planes
				if ((j) < (resolution - 1)) {
					smoothedNormal.x += vertices[j * resolution + i].normal.x;
					smoothedNormal.y += vertices[j * resolution + i].normal.y;
					smoothedNormal.z += vertices[j * resolution + i].normal.z;
					count++;
				}
				//Bottom planes
				if ((j - 1) >= 0) {
					smoothedNormal.x += vertices[(j - 1) * resolution + i].normal.x;
					smoothedNormal.y += vertices[(j - 1) * resolution + i].normal.y;
					smoothedNormal.z += vertices[(j - 1) * resolution + i].normal.z;
					count++;
				}
			}
			smoothedNormal.x /= count;
			smoothedNormal.y /= count;
			smoothedNormal.z /= count;

			float mag = sqrt((smoothedNormal.x * smoothedNormal.x) + (smoothedNormal.y * smoothedNormal.y) + (smoothedNormal.z * smoothedNormal.z));
			smoothedNormal.x /= mag;
			smoothedNormal.y /= mag;
			smoothedNormal.z /= mag;

			vertices[j * resolution + i].normal = smoothedNormal;
		}
	}
	//If we've not yet created our dyanmic Vertex and Index buffers, do that now
	if (vertexBuffer == NULL) {
		CreateBuffers(device, vertices, indices);
	}
	else {
		//If we've already made our buffers, update the information
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		//  Disable GPU access to the vertex buffer data.
		deviceContext->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		//  Update the vertex buffer here.
		memcpy(mappedResource.pData, vertices, sizeof(VertexType) * vertexCount);
		//  Reenable GPU access to the vertex buffer data.
		deviceContext->Unmap(vertexBuffer, 0);
	}

	// Release the arrays now that the buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;
}



//////////////////////////////////////////////////////////////// TERRAIN MANIPULATION HEIGHT MAP FUNCTIONS ////////////////////////////////////////////////////////////////



//////////////////////////////// BUILD HEIGHT MAP FROM 0 FUNCTIONS ////////////////////////////////

void TerrainMesh::BuildCustomHeightMap() {

	float height = 0.0f;

	//Scale everything so that the look is consistent across terrain resolutions
	const float scale = terrainSize / (float)resolution;

	//xOffset += 0.1f; // moving the wave

	//TODO: Give some meaning to these magic numbers! What effect does changing them have on terrain?
	// The number inside the sin or cos modify the amplitude and the number outside is the Amplitude
	for (int k = 0; k < (resolution); k++) {
		for (int i = 0; i < (resolution); i++) {
			// Waves along x-axis
			height = (sin((float)i * wavesData.frequency.x * scale + wavesData.offset.x)) * wavesData.amplitude.x; // Waves 1 On x
			height += (sin((float)i * wavesData.frequency.x * 2.0f * scale + wavesData.offset.x)) * wavesData.amplitude.x * 0.5f; // Waves 2 On x (it is half of amplitud1 and double of frequency1)
			height += (sin((float)i * wavesData.frequency.x * 4.0f * scale + wavesData.offset.x)) * wavesData.amplitude.x * 0.25f; // Waves 3 On x (it is half of amplitud2 and double of frequency2)
			// Waves along z-axis
			height += (cos((float)k * wavesData.frequency.z * scale + wavesData.offset.z)) * wavesData.amplitude.z; // Waves On z
			height += (cos((float)k * wavesData.frequency.z * 2.0f * scale + wavesData.offset.z)) * wavesData.amplitude.z * 0.5f; // Waves On z
			height += (cos((float)k * wavesData.frequency.z * 4.0f * scale + wavesData.offset.z)) * wavesData.amplitude.z * 0.25f; // Waves On z
			heightMap[GetIndex(k, i)] = height;
		}
	}
}

void TerrainMesh::BuildRandomHeightMap(int min, int max)
{
	// The number inside the sin or cos modify the amplitude and the number outside is the Amplitude
	for (int k = 0; k < (resolution); k++)
	{
		for (int i = 0; i < (resolution); i++)
		{
			heightMap[GetIndex(k, i)] = (float)Utils::GetRandom(min, max);; // random number in the range [min, max]
		}
	}
}


//////////////////////////////// MODIFY HEIGHT MAP FUNCTIONS ////////////////////////////////

void TerrainMesh::Flatten()
{
	for (int k = 0; k < (resolution); k++)
	{
		for (int i = 0; i < (resolution); i++)
		{
			heightMap[GetIndex(k, i)] = 0.0f;
		}
	}

	// Reset the waves data
	wavesData = WavesData();
}

void TerrainMesh::Fault()
{
	XMFLOAT3 point1, point2, currentVertex;
	XMVECTOR faultLine, linkedLine, crossResult;

	point1 = XMFLOAT3(rand() % resolution, 0.0f, rand() % resolution); // a random point in the map
	point2 = XMFLOAT3(point1.x, point1.y, point1.z + 1.0f); // point 2 = point 1 displaced in z-axis
	faultLine = XMVectorSet(point2.x - point1.x, point2.y - point1.y, point2.z - point1.z, 1.0f); // Line from point 1 to point 2
	faultLine = XMVector3Rotate(faultLine, XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), ((rand() % 360) * M_PI) / 180)); // Rotate that line randomly

	for (int k = 0; k < resolution; k++)
	{
		for (int i = 0; i < resolution; i++)
		{
			currentVertex = XMFLOAT3(i, 0.0f, k); // get the current vertex
			linkedLine = XMVectorSet(currentVertex.x - point1.x, currentVertex.y - point1.y, currentVertex.z - point1.z, 1.0f); // line from the current vertex to the a point in the fault line (point 1) 

			crossResult = XMVector3Cross(faultLine, linkedLine);

			// detect if the current point is in the left or right side of the fault line
			if (XMVectorGetY(crossResult) > 0) // left side
			{
				// move up
				heightMap[GetIndex(k, i)] += maxHeight;
			}
			else // right side
			{
				// move down
				heightMap[GetIndex(k, i)] -= maxHeight;
			}
		}
	}
}

void TerrainMesh::Smooth()
{
	float* smoothedHeightMap = new float[resolution * resolution];

	for (int k = 0; k < (resolution); k++)
	{
		for (int i = 0; i < (resolution); i++)
		{
			smoothedHeightMap[GetIndex(k, i)] = NeighboursAverage(k, i);;
		}
	}

	// replace the old height map with the filtered one
	heightMap = smoothedHeightMap;
}

void TerrainMesh::ParticleDeposition()
{
	// call to the emitter to drop a particle
	Particle particle = emitter->dropParticle();

	// get x,z position of the particle
	int i = (int)particle.position.x;
	int k = (int)particle.position.z;

	XMFLOAT3 lowest_height_point = particle.position;
	bool found = false;

	// look throught the possible neighbours of the current point
	for (int m = k - 1; m <= k + 1; m++)
	{
		for (int n = i - 1; n <= i + 1; n++)
		{
			if (InBounds(m, n) && heightMap[GetIndex(m, n)] < heightMap[((int)lowest_height_point.z * resolution) + (int)lowest_height_point.x]) // avoid to check the center cell again and check if it is in bounds
			{
				lowest_height_point = XMFLOAT3(n, 0.0f, m);// save the new lowest height point
			}
		}
	}

	// add height to the map
	heightMap[((int)lowest_height_point.z * resolution) + (int)lowest_height_point.x] += particle.height;
}

void TerrainMesh::AntiParticleDeposition()
{
	// call to the emitter to drop a particle
	Particle particle = emitter->dropParticle();

	// get x,z position of the particle
	int i = (int)particle.position.x;
	int k = (int)particle.position.z;

	XMFLOAT3 highest_height_point = particle.position;
	bool found = false;

	// look throught the possible neighbours of the current point
	for (int m = k - 1; m <= k + 1; m++)
	{
		for (int n = i - 1; n <= i + 1; n++)
		{
			if (InBounds(n, m) && heightMap[GetIndex(m, n)] > heightMap[((int)highest_height_point.z * resolution) + (int)highest_height_point.x]) // avoid to check the center cell again and check if it is in bounds
			{
				highest_height_point = XMFLOAT3(n, 0.0f, m);// save the new lowest height point
			}
		}
	}

	// substract height to the map
	heightMap[((int)highest_height_point.z * resolution) + (int)highest_height_point.x] -= particle.height;
}



void TerrainMesh::DiamondSquare(int row, int col, int size, float offset)
{
	// corners points [(m*n) localitation in the array heightMap]
	int topLeft, topRight, bottomLeft, bottomRight;
	int center;
	float heightOffset = 10.0f;
	float cornersAvg;

	int m = 0; 
	int n = 0; 

	// get the height map indices of the corners 
	topLeft = GetIndex(0, 0);
	topRight = GetIndex(0, resolution - 1);
	bottomLeft = GetIndex(resolution - 1, 0);
	bottomRight = GetIndex(resolution - 1, 0);

	// get the index of the height map center (middle point)
	center = GetIndex((int)(resolution/2) + 1, (int)(resolution/2) + 1);


	// Apply Square Step //

	// Asign a random height to each corner
	heightMap[topLeft] = (float)Utils::GetRandom(-heightOffset, +heightOffset);
	heightMap[topRight] = (float)Utils::GetRandom(-heightOffset, +heightOffset);
	heightMap[bottomLeft] = (float)Utils::GetRandom(-heightOffset, +heightOffset);
	heightMap[bottomRight] = (float)Utils::GetRandom(-heightOffset, +heightOffset);

	// calculate the average of the corners
	cornersAvg = (heightMap[topLeft] + heightMap[topRight] + heightMap[bottomLeft] + heightMap[bottomRight]) / 4.0f;

	// set the value to the middle point
	heightMap[center] = cornersAvg + Utils::GetRandom(-heightOffset, +heightOffset);

	// halves the random value for the next
	heightOffset /= 2.0f;


	// Apply Diamond Step //

	//  Superior Diamond
	//int 
	//heightMap[topRight - topLeft - 1]

}

void TerrainMesh::SquareStep(int row, int col)
{

}

void TerrainMesh::DiamondStep()
{

}



//////////////////////////////// TOOL FUNCTIONS FOR HEIGHT MAP MANIPULATION ////////////////////////////////

XMFLOAT3 TerrainMesh::GetRandomPos()
{
	return XMFLOAT3(Utils::GetRandom(0.0f, (float)resolution), 0.0f, Utils::GetRandom(0.0f, (float)resolution));
}

float TerrainMesh::NeighboursAverage(int k, int i)
{
	// Function computes the average height of the ik element.
	// It averages itself with its eight neighbour pixels.
	// Note that if a pixel is missing neighbour, we just don't include it
	// in the average--that is, edge pixel don't have a neighbour pixel.
	//      
	// ---------
	// | 1| 2| 3|
	// | 4|ik| 6|
	// | 7| 8| 9|
	// ---------

	float totalHeight = 0.0f; // sum of the neighbours height plus the current point itself
	int numPoints = 0; // number of points

	// look throught the possible neighbours of the current point
	for (int m = k - 1; m <= k + 1; m++)
	{
		for (int n = i - 1; n <= i + 1; n++)
		{
			if (InBounds(m, n))
			{
				totalHeight += heightMap[GetIndex(m, n)];
				numPoints++; // count this point
			}
		}
	}

	return totalHeight / (float)numPoints; // return the average
}


bool TerrainMesh::InBounds(int m, int n)
{
	// True if ik are valid indices; false otherwise
	return (m >= 0 && m < resolution && n >= 0 && n < resolution);
}

int TerrainMesh::GetIndex(int m, int n)
{
	// m(rows) == k == z
	// n(columns) == i == x
	return ((m * resolution) + n);
}
