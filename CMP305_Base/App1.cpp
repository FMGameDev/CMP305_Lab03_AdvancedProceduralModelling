#include "App1.h"

App1::App1()
{
	m_Terrain = nullptr;
	shader = nullptr;
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Load textures
	textureMgr->loadTexture(L"grass", L"res/grass.png");
	textureMgr->loadTexture(L"white", L"res/DefaultDiffuse.png");

	// Create Mesh object and shader object
	m_Terrain = new TerrainMesh(renderer->getDevice(), renderer->getDeviceContext());
	shader = new LightShader(renderer->getDevice(), hwnd);
	
	// Initialise light
	light = new Light();
	light->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	light->setDirection(1.0f, -1.0f, 0.0f);

}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.
	if (m_Terrain)
	{
		delete m_Terrain;
		m_Terrain = 0;
	}

	if (shader)
	{
		delete shader;
		shader = 0;
	}
}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	// Send geometry data, set shader parameters, render object with shader
	m_Terrain->sendData(renderer->getDeviceContext());
	shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"), light);
	shader->render(renderer->getDeviceContext(), m_Terrain->getIndexCount());

	// Render GUI
	gui();

	// Swap the buffers
	renderer->endScene();

	return true;
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI //
	// FPS
	ImGui::Text("FPS: %.2f", timer->getFPS());

	// Title for terrain settings
	ImGui::Text("\nTerrain Settings:");
	// Wireframe mode
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	// Resolution
	ImGui::SliderInt("Resolution", &terrainResolution, 2, 1024);
	// Regenerations
	ImGui::Text("Frecuency:");
	ImGui::SliderFloat("X-Frequency", &terrainXFrequency, 0.033f, 1.2f);
	ImGui::SliderFloat("Z-Frequency", &terrainZFrequency, 0.033f, 1.2f);

	ImGui::Text("Amplitude:");
	ImGui::SliderFloat("X-Amplitud", &terrainXAmplitude, 0.0f, 20.0f);
	ImGui::SliderFloat("Z-Amplitud", &terrainZAmplitude, 0.0f, 20.0f);

	// button to apply the changes
	if (ImGui::Button("Apply resolution, frecuency and amplitude")) {
		if (terrainResolution != m_Terrain->GetResolution()) {
			m_Terrain->Resize(terrainResolution);
		}
		if (terrainXFrequency != m_Terrain->GetXFrequency() || terrainZFrequency != m_Terrain->GetZFrequency())
			m_Terrain->SetWavesFrequency(terrainXFrequency, terrainZFrequency);
		if (terrainXAmplitude != m_Terrain->GetXAmplitude() || terrainZAmplitude != m_Terrain->GetZAmplitude())
			m_Terrain->SetWavesAmplitude(terrainXAmplitude, terrainZAmplitude);

		// build map height
		m_Terrain->BuildCustomHeightMap();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	// Random height field
	ImGui::Text("\n");
	ImGui::Text("\nRandom Height:\n");

	// Max height field
	ImGui::SliderInt("Max height", &terrainMaxHeight, 0.0f, 100.0f);
	if (ImGui::Button("Randomise")) {
		if (terrainResolution != m_Terrain->GetResolution()) {
			m_Terrain->Resize(terrainResolution);
		}
		if (terrainMaxHeight != m_Terrain->GetMaxHeight())
			m_Terrain->SetMaxHeight(terrainMaxHeight);

		// build map height
		m_Terrain->BuildRandomHeightMap(0, terrainMaxHeight);
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	// Apply White noise (random numbers between 0 and 1
	if (ImGui::Button("White Noise")) {
		// build map height
		m_Terrain->BuildRandomHeightMap(0, 1);
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	// Smooth
	if (ImGui::Button("Smooth")) {
		m_Terrain->Smooth();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}
	// Fault
	if (ImGui::Button("Fault")) {
		if (terrainMaxHeight != m_Terrain->GetMaxHeight())
			m_Terrain->SetMaxHeight(terrainMaxHeight);
		m_Terrain->Fault();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}
	// Flatten
	if (ImGui::Button("Flatten")) {
		m_Terrain->Flatten();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}
	// Particle Deposition
	if (ImGui::Button("Particle Deposition")) {
		m_Terrain->ParticleDeposition();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	// Anti-Particle Deposition
	if (ImGui::Button("Anti-Particle Deposition")) {
		m_Terrain->AntiParticleDeposition();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

