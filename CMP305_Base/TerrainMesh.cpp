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

	emitter_ = new Emitter(GetRandomPos()); // create emitter and set it in a random pos
}

//Cleanup the heightMap
TerrainMesh::~TerrainMesh()
{
	delete[] heightMap;
	heightMap = 0;

	delete emitter_;
	emitter_ = nullptr;

}


//Fill an array of floats that represent the height values at each grid point.
//Here we are producing a Sine wave along the X-axis
void TerrainMesh::BuildCustomHeightMap() {
	
	float height = 0.0f;

	//Scale everything so that the look is consistent across terrain resolutions
	const float scale =  terrainSize / (float)resolution;

	//xOffset += 0.1f; // moving the wave

	//TODO: Give some meaning to these magic numbers! What effect does changing them have on terrain?
	// The number inside the sin or cos modify the amplitude and the number outside is the Amplitude
	for( int k = 0; k < ( resolution ); k++ ) {
		for( int i = 0; i < ( resolution ); i++ ) {
			height = ( sin( (float)i * xFrequency * scale + xOffset) ) * xAmplitude; // Waves 1 On x
			height += (sin((float)i * xFrequency*2.0f * scale + xOffset)) * xAmplitude*0.5f; // Waves 2 On x (it is half of amplitud1 and double of frequency1)
			height += (sin((float)i * xFrequency*4.0f * scale + xOffset)) * xAmplitude*0.25f; // Waves 3 On x (it is half of amplitud2 and double of frequency2)
			height += (cos( (float)k * zFrequency * scale + zOffset) ) * zAmplitude; // Waves On z
			height += (cos((float)k * zFrequency*2.0f * scale + zOffset)) * zAmplitude*0.5f; // Waves On z
			height += (cos((float)k * zFrequency*4.0f * scale + zOffset)) * zAmplitude*0.25f; // Waves On z
			heightMap[( k * resolution ) + i] = height;
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
			heightMap[(k * resolution) + i] = (float)Utils::GetRandom(min, max);; // random number in the range [min, max]
		}
	}
}

void TerrainMesh::Flatten()
{
	for (int k = 0; k < (resolution); k++)
	{
		for (int i = 0; i < (resolution); i++)
		{
			heightMap[(k * resolution) + i] = 0.0f;
		}
	}
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
				heightMap[(k * resolution) + i] += maxHeight;
			}
			else // right side
			{
				// move down
				heightMap[(k * resolution) + i] -= maxHeight;
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
			smoothedHeightMap[(k * resolution) + i] = NeighboursAverage(i, k);;
		}
	}

	// replace the old height map with the filtered one
	heightMap = smoothedHeightMap;
}

void TerrainMesh::ParticleDeposition()
{
	// call to the emitter to drop a particle
	Particle particle = emitter_->dropParticle();

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
			if (InBounds(n, m) && heightMap[(m * resolution) + n] < heightMap[((int)lowest_height_point.z * resolution) + (int)lowest_height_point.x]) // avoid to check the center cell again and check if it is in bounds
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
	Particle particle = emitter_->dropParticle();

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
			if (InBounds(n, m) && heightMap[(m * resolution) + n] > heightMap[((int)highest_height_point.z * resolution) + (int)highest_height_point.x]) // avoid to check the center cell again and check if it is in bounds
			{
				highest_height_point = XMFLOAT3(n, 0.0f, m);// save the new lowest height point
			}
		}
	}

	// substract height to the map
	heightMap[((int)highest_height_point.z * resolution) + (int)highest_height_point.x] -= particle.height;
}

XMFLOAT3 TerrainMesh::GetRandomPos()
{
	return XMFLOAT3(Utils::GetRandom(0.0f, (float)resolution), 0.0f, Utils::GetRandom(0.0f, (float)resolution));
}

float TerrainMesh::NeighboursAverage(int i, int k)
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
			if (InBounds(n, m))
			{
				totalHeight += heightMap[(m * resolution) + n];
				numPoints++; // count this point
			}
		}
	}

	return totalHeight / (float)numPoints; // return the average
}


bool TerrainMesh::InBounds(int i, int k)
{
	// True if ik are valid indices; false otherwise
	return (i >= 0 && i < resolution&& k >= 0 && k < resolution);
}

void TerrainMesh::Resize( int newResolution ) {
	resolution = newResolution;
	heightMap = new float[resolution * resolution];
	if( vertexBuffer != NULL ) {
		vertexBuffer->Release();
	}
	vertexBuffer = NULL;
}

// Set up the heightmap and create or update the appropriate buffers
void TerrainMesh::Regenerate( ID3D11Device * device, ID3D11DeviceContext * deviceContext) {

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

	indexCount = ( ( resolution - 1 ) * ( resolution - 1 ) ) * 6;
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
	for( j = 0; j < ( resolution ); j++ ) {
		for( i = 0; i < ( resolution ); i++ ) {
			positionX = (float)i * scale;
			positionZ = (float)( j ) * scale;

			height = heightMap[index];
			vertices[index].position = XMFLOAT3( positionX, height, positionZ );
			vertices[index].texture = XMFLOAT2( u, v );

			u += increment;
			index++;
		}
		u = 0;
		v += increment;
	}

	//Set up index list
	index = 0;
	for( j = 0; j < ( resolution - 1 ); j++ ) {
		for( i = 0; i < ( resolution - 1 ); i++ ) {

			//Build index array
			indices[index] = ( j*resolution ) + i;
			indices[index + 1] = ( ( j + 1 ) * resolution ) + ( i + 1 );
			indices[index + 2] = ( ( j + 1 ) * resolution ) + i;

			indices[index + 3] = ( j * resolution ) + i;
			indices[index + 4] = ( j * resolution ) + ( i + 1 );
			indices[index + 5] = ( ( j + 1 ) * resolution ) + ( i + 1 );
			index += 6;
		}
	}

	//Set up normals
	for( j = 0; j < ( resolution - 1 ); j++ ) {
		for( i = 0; i < ( resolution - 1 ); i++ ) {
			//Calculate the plane normals
			XMFLOAT3 a, b, c;	//Three corner vertices
			a = vertices[j * resolution + i].position;
			b = vertices[j * resolution + i + 1].position;
			c = vertices[( j + 1 ) * resolution + i].position;

			//Two edges
			XMFLOAT3 ab( c.x - a.x, c.y - a.y, c.z - a.z );
			XMFLOAT3 ac( b.x - a.x, b.y - a.y, b.z - a.z );
			
			//Calculate the cross product
			XMFLOAT3 cross;
			cross.x = ab.y * ac.z - ab.z * ac.y;
			cross.y = ab.z * ac.x - ab.x * ac.z;
			cross.z = ab.x * ac.y - ab.y * ac.x;
			float mag = ( cross.x * cross.x ) + ( cross.y * cross.y ) + ( cross.z * cross.z );
			mag = sqrtf( mag );
			cross.x/= mag;
			cross.y /= mag;
			cross.z /= mag;
			vertices[j * resolution + i].normal = cross;
		}
	}

	//Smooth the normals by averaging the normals from the surrounding planes
	XMFLOAT3 smoothedNormal( 0, 1, 0 );
	for( j = 0; j < resolution; j++ ) {
		for( i = 0; i < resolution; i++ ) {
			smoothedNormal.x = 0;
			smoothedNormal.y = 0;
			smoothedNormal.z = 0;
			float count = 0;
			//Left planes
			if( ( i - 1 ) >= 0 ) {
				//Top planes
				if( ( j ) < ( resolution - 1 ) ) {
					smoothedNormal.x += vertices[j * resolution + ( i - 1 )].normal.x;
					smoothedNormal.y += vertices[j * resolution + ( i - 1 )].normal.y;
					smoothedNormal.z += vertices[j * resolution + ( i - 1 )].normal.z;
					count++;
				}
				//Bottom planes
				if( ( j - 1 ) >= 0 ) {
					smoothedNormal.x += vertices[( j - 1 ) * resolution + ( i - 1 )].normal.x;
					smoothedNormal.y += vertices[( j - 1 ) * resolution + ( i - 1 )].normal.y;
					smoothedNormal.z += vertices[( j - 1 ) * resolution + ( i - 1 )].normal.z;
					count++;
				}
			}
			//right planes
			if( ( i ) <( resolution - 1 ) ) {

				//Top planes
				if( ( j ) < ( resolution - 1 ) ) {
					smoothedNormal.x += vertices[j * resolution + i].normal.x;
					smoothedNormal.y += vertices[j * resolution + i].normal.y;
					smoothedNormal.z += vertices[j * resolution + i].normal.z;
					count++;
				}
				//Bottom planes
				if( ( j - 1 ) >= 0 ) {
					smoothedNormal.x += vertices[( j - 1 ) * resolution + i].normal.x;
					smoothedNormal.y += vertices[( j - 1 ) * resolution + i].normal.y;
					smoothedNormal.z += vertices[( j - 1 ) * resolution + i].normal.z;
					count++;
				}
			}
			smoothedNormal.x /= count;
			smoothedNormal.y /= count;
			smoothedNormal.z /= count;

			float mag = sqrt( ( smoothedNormal.x * smoothedNormal.x ) + ( smoothedNormal.y * smoothedNormal.y ) + ( smoothedNormal.z * smoothedNormal.z ) );
			smoothedNormal.x /= mag;
			smoothedNormal.y /= mag;
			smoothedNormal.z /= mag;

			vertices[j * resolution + i].normal = smoothedNormal;
		}
	}
	//If we've not yet created our dyanmic Vertex and Index buffers, do that now
	if( vertexBuffer == NULL ) {
		CreateBuffers( device, vertices, indices );
	}
	else {
		//If we've already made our buffers, update the information
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory( &mappedResource, sizeof( D3D11_MAPPED_SUBRESOURCE ) );

		//  Disable GPU access to the vertex buffer data.
		deviceContext->Map( vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
		//  Update the vertex buffer here.
		memcpy( mappedResource.pData, vertices, sizeof( VertexType ) * vertexCount );
		//  Reenable GPU access to the vertex buffer data.
		deviceContext->Unmap( vertexBuffer, 0 );
	}

	// Release the arrays now that the buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;
}

//Create the vertex and index buffers that will be passed along to the graphics card for rendering
//For CMP305, you don't need to worry so much about how or why yet, but notice the Vertex buffer is DYNAMIC here as we are changing the values often
void TerrainMesh::CreateBuffers( ID3D11Device* device, VertexType* vertices, unsigned long* indices ) {

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Set up the description of the dyanmic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof( VertexType ) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer( &vertexBufferDesc, &vertexData, &vertexBuffer );

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof( unsigned long ) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	device->CreateBuffer( &indexBufferDesc, &indexData, &indexBuffer );
}