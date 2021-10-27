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
	m_Terrain = new TerrainMesh(renderer->getDevice(), renderer->getDeviceContext(), 5);
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

	// Camera Position
	XMFLOAT3 cameraPos = camera->getPosition();
	ImGui::Text("Camera Pos: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);

	// Title for terrain general settings
	ImGui::Text("\nTerrain General Settings:");
	// Wireframe mode
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	// Resolution
	int resolution = m_Terrain->GetResolution();
	ImGui::Text("(2^n)+1: 3, 5, 9, 17, 33, 65, 129, 257, 513, 1025");
	ImGui::SliderInt("Resolution", &resolution, 2, 1025);
	if (resolution != m_Terrain->GetResolution()) {
		m_Terrain->Resize(resolution);
		m_Terrain->Flatten();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}
	// Set Height Offset Range
	Range heightOffsetRange = m_Terrain->GetHeightOffsetRange();
	float range[2] = { heightOffsetRange.min, heightOffsetRange.max };
	ImGui::SliderFloat2("Height Offset Range (min-max)", range, 0.0f, 50.0f);
	if (range[0] != heightOffsetRange.min || range[1] != heightOffsetRange.max)
	{
		heightOffsetRange.min = range[0];
		heightOffsetRange.max = range[1];
		m_Terrain->SetHeightOffsetRange(heightOffsetRange);
	}

	// Regenerate completely the height map 
	ImGui::Text("\n\nRebuild Height Map Functions:\n");

	// Waves
	ImGui::Text("\nTerrain Sin and Cos Waves:");
	WavesData wavesData = m_Terrain->GetWavesData();

	float frequency[2] = { wavesData.frequency.x,  wavesData.frequency.z };
	ImGui::SliderFloat2("Frequency (x,z)", frequency, 0.033f, 1.2f);
	float amplitude[2] = { wavesData.amplitude.x,  wavesData.amplitude.z };
	ImGui::SliderFloat2("Amplitud (x,z)", amplitude, 0.0f, 20.0f);
	if (frequency[0] != wavesData.frequency.x || frequency[1] != wavesData.frequency.z ||
		amplitude[0] != wavesData.amplitude.x || amplitude[1] != wavesData.amplitude.z)
	{
		wavesData.frequency.x = frequency[0];
		wavesData.frequency.z = frequency[1];
		wavesData.amplitude.x = amplitude[0];
		wavesData.amplitude.z = amplitude[1];

		m_Terrain->SetWavesData(wavesData);
	}
	if (ImGui::Button("Create Waves"))
	{
		m_Terrain->BuildCustomHeightMap();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	// Apply Random Height
	if (ImGui::Button("Random Height")) {
		// build map height
		m_Terrain->BuildRandomHeightMap();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	// Diamond
	if (ImGui::Button("Diamond-Square Algorithm")) {
		m_Terrain->DiamondSquareAlgorithm();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	// Regenerate completely the height map 
	ImGui::Text("\n\nModify Height Map functions:\n");

	// Smooth
	if (ImGui::Button("Smooth")) {
		m_Terrain->Smooth();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}
	// Fault
	if (ImGui::Button("Fault")) {
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

