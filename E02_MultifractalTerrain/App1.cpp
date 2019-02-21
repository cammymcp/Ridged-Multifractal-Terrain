#include "App1.h"

App1::App1()
{
	m_Terrain = nullptr;
	m_TessellationShader = nullptr;
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in)
{
	// Call super init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in);

	// Store screen width and height
	s_Width = screenWidth; s_Height = screenHeight;

	// Create Mesh object
	m_Terrain = new Terrain(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), TERRAIN_PATCHES_X, TERRAIN_PATCHES_Z);
	int totalPatches = TERRAIN_PATCHES_X * TERRAIN_PATCHES_Z;

	// Set up occlusion query
	occlusionDesc.Query = D3D11_QUERY_OCCLUSION;
	m_Direct3D->GetDevice()->CreateQuery(&occlusionDesc, &occlusionQuery);

	// Set up shaders
	m_TessellationShader = new TessellationShader(m_Direct3D->GetDevice(), hwnd);
	m_DepthShader = new DepthShader(m_Direct3D->GetDevice(), hwnd);
	m_TextureShader = new TextureShader(m_Direct3D->GetDevice(), hwnd);

	// Set up render texture
	m_DepthMap = new RenderTexture(m_Direct3D->GetDevice(), 2048, 2048, SCREEN_NEAR, SCREEN_DEPTH);

	// Set up camera
	m_Camera->SetPosition(347.0f, 88.0f, -182.0f);
	m_Camera->SetRotation(11.5f, 262.0f, 0.0f);
	m_CameraFrustum = new CameraFrustum();
	m_CameraFrustum->Initialise(SCREEN_DEPTH);

	// Set up skybox
	m_Skybox = new Skybox(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), L"../res/sky.png");

	// Set up sunlight
	m_Sunlight = new Light;
	m_Sunlight->SetDiffuseColour(1.0f, 0.6f, 0.3f, 1.0f);
	m_Sunlight->SetAmbientColour(0.4f, 0.3f, 0.3f, 1.0f);
	m_Sunlight->SetPosition(470.0f, 120.0f, -500.0f);
	m_Sunlight->SetLookAt(250.0f, 50.0f, -150.0f);
	m_LightSphere = new SphereMesh(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), L"../res/snow.jpg");

	m_Sunlight->GenerateViewMatrix(); // This only needs to happen every frame if the light will be moving!!!!
	m_Sunlight->GenerateProjectionMatrix(1.0f, 500.0f);

	// Ortho mesh for debugging
	m_OrthoMesh = new OrthoMesh(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), screenWidth / 4, screenHeight / 4, screenWidth / 8 - (screenWidth / 2), -screenHeight / 8 + (screenHeight / 2));

	// Init data for multifractal terrain
	m_MultifractalData.H = 1.6f;
	m_MultifractalData.lacunarity = 2.0f;
	m_MultifractalData.octaves = 8.0f;
	m_MultifractalData.offset = 1.0f;
	m_MultifractalData.gain = 2.0f;
	
	// GUI
	showDepthMap = false;
	tessellationFactor = 1.0f;
	wireframe = false;
	atmosphericEffects = true;
	tessDistNear = 150.0f;
	tessDistFar = 300.0f;
	rendering = 0; culling = 0;
}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D objects.
	if (m_Skybox)
	{
		delete m_Skybox;
		m_Skybox = 0;
	}

	if (m_OrthoMesh)
	{
		delete m_OrthoMesh;
		m_OrthoMesh = 0;
	}

	if (m_DepthMap)
	{
		delete m_DepthMap;
		m_DepthMap = 0;
	}


	if (m_Terrain)
	{
		delete m_Terrain;
		m_Terrain = 0;
	}

	if (m_TessellationShader)
	{
		delete m_TessellationShader;
		m_TessellationShader = 0;
	}

	if (m_DepthShader)
	{
		delete m_DepthShader;
		m_DepthShader = 0;
	}

	if (m_TextureShader)
	{
		delete m_TextureShader;
		m_TextureShader = 0;
	}

	if (m_Sunlight)
	{
		delete m_Sunlight;
		m_Sunlight = 0;
	}

	if (m_LightSphere)
	{
		delete m_LightSphere;
		m_LightSphere = 0;
	}

	if (m_CameraFrustum)
	{
		delete m_CameraFrustum;
		m_CameraFrustum = 0;
	}
}


bool App1::Frame()
{
	bool result;

	// Wireframe Toggle
	if (m_Input->isKeyDown(VK_SPACE))
	{
		if (!wireframe)
		{
			m_Direct3D->TurnOnWireframe();
			wireframe = true;
			m_Input->SetKeyUp(VK_SPACE);
		}
		else
		{
			m_Direct3D->TurnOffWireframe();
			wireframe = false;
			m_Input->SetKeyUp(VK_SPACE);
		}
	}

	result = BaseApplication::Frame();
	if (!result)
	{
		return false;
	}
	// Render the graphics.
	result = Render();
	if (!result)
	{
		return false;
	}

	return true;
}


bool App1::Render()
{
	// First get the depth map for lighting and shadows
	RenderShadowMap();

	//// Begin drawing scene. (Clear to default blue colour)
	m_Direct3D->BeginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Test object occlusion for culling
	//OcclusionTest();
	
	// Final frame render
	RenderScene();	

	// Update frame time
	frameRate = 1 / m_Timer->GetTime();

	// Check for automatic optimisation
	if (optimisationMode == AUTO)
	{
		// If frame rate isn't equal to target (~30fps) then adjust tessellation values
		if (frameRate < 29.0f)
		{
			// Reduce detail range (within limits)
			if (tessDistFar > tessDistNear + 0.5)
				tessDistFar -= 0.5;
			if (tessDistNear > MIN_TESS + 0.1)
				tessDistNear -= 0.5;
		}
		else if (frameRate > 31.0f)
		{
			// Increase detail range (within limits)
			if (tessDistFar < MAX_TESS - 0.5)
				tessDistFar += 0.5;
			if (tessDistNear < tessDistFar - 0.1)
				tessDistNear += 0.5;
		}
	}

	// Round frame rate for display
	frameRate = roundf(frameRate);

	// Draw GUI
	gui();

	//// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}

void App1::RenderScene()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, baseViewMatrix, orthoMatrix;

	//// Generate the view matrix based on the camera's position.
	m_Camera->Update();

	//// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);
	
	// Reconstruct camera frustum
	m_CameraFrustum->ConstructFrustum(viewMatrix, projectionMatrix);

	// Render skybox, turn off depth test
	m_Direct3D->TurnZBufferOff();
	m_Direct3D->TurnOffWireframe();

		// Translate skybox to camera position
		worldMatrix = XMMatrixTranslation(m_Camera->GetPosition().x, m_Camera->GetPosition().y, m_Camera->GetPosition().z + 2.0f);
		
		//// Send geometry data (from mesh)
		m_Skybox->SendData(m_Direct3D->GetDeviceContext());
		//// Set shader parameters (matrices and texture)
		m_TextureShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, m_Skybox->GetTexture());
		//// Render object (combination of mesh geometry and shader process
		m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_Skybox->GetIndexCount());
		
		// Reset world matrix
		m_Direct3D->GetWorldMatrix(worldMatrix);

	if (wireframe) { m_Direct3D->TurnOnWireframe(); }
	m_Direct3D->TurnZBufferOn();

	//// Send geometry data (from terrain)
	rendering = 0; culling = 0;
	const std::vector<Patch*> localVec = m_Terrain->Patches();
	std::vector<Patch*>::const_iterator i = localVec.begin();
	while (i != localVec.end())
	{
		// Check if patch is within camera frustum before sending to render
		if (m_CameraFrustum->CheckRectangle((*i)->boundingBox.maxX, (*i)->boundingBox.maxY, (*i)->boundingBox.maxZ, (*i)->boundingBox.minX, (*i)->boundingBox.minY, (*i)->boundingBox.minZ) && (*i)->toRender)
		{
			(*i)->SendData(m_Direct3D->GetDeviceContext());
			//// Set shader parameters (matrices and texture)
			m_TessellationShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, // Matrices
														 m_Terrain->GetTerrainRockTexture()->GetTexture(), m_Terrain->GetTerrainSnowTexture()->GetTexture(), m_DepthMap->GetShaderResourceView(), // Textures and maps
														 tessDistNear, tessDistFar, m_Camera, m_Sunlight, m_MultifractalData, atmosphericEffects); // Other
			//// Render object (combination of mesh geometry and shader process
			m_TessellationShader->Render(m_Direct3D->GetDeviceContext(), (*i)->GetIndexCount());

			// Increment rendering counter
			rendering++;
		}
		else
		{
			// Increment culling counter
			culling++;
		}
		i++;
	}

	// Translate to light position
	worldMatrix = XMMatrixTranslation(m_Sunlight->GetPosition().x, m_Sunlight->GetPosition().y, m_Sunlight->GetPosition().z);
		// Send light sphere data 
		m_LightSphere->SendData(m_Direct3D->GetDeviceContext());
		//// Set shader parameters (matrices and texture)
		m_TextureShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, m_LightSphere->GetTexture());
		//// Render object (combination of mesh geometry and shader process
		m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_LightSphere->GetIndexCount());
	// Reset world matrix
	m_Direct3D->GetWorldMatrix(worldMatrix);

	// To render ortho mesh
	// Turn off the Z buffer to begin all 2D rendering.
	if (showDepthMap)
	{
		m_Direct3D->TurnZBufferOff();

			m_Direct3D->GetOrthoMatrix(orthoMatrix);// ortho matrix for 2D rendering
			m_Camera->GetBaseViewMatrix(baseViewMatrix);

			m_OrthoMesh->SendData(m_Direct3D->GetDeviceContext());
			m_TextureShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, m_DepthMap->GetShaderResourceView());
			m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_OrthoMesh->GetIndexCount());

		m_Direct3D->TurnZBufferOn();
	}
}

void App1::RenderShadowMap()
{
	// First render pass, to texture

	XMMATRIX worldMatrix, lightViewMatrix, lightProjectionMatrix;

	// Set the render target to be the render to texture.
	m_DepthMap->SetRenderTarget(m_Direct3D->GetDeviceContext());

	// Clear the render to texture.
	m_DepthMap->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Update();

	// Get the world, view, and projection matrices from the light and d3d objects.
	lightViewMatrix = m_Sunlight->GetViewMatrix();
	lightProjectionMatrix = m_Sunlight->GetProjectionMatrix();
	m_Direct3D->GetWorldMatrix(worldMatrix);

	//// Send geometry data (from terrain)
	const std::vector<Patch*> localVec = m_Terrain->Patches();
	std::vector<Patch*>::const_iterator i = localVec.begin();
	while (i != localVec.end())
	{
		(*i)->SendData(m_Direct3D->GetDeviceContext());
		//// Set shader parameters (matrices and texture)
		m_DepthShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, 
										   m_Camera, m_MultifractalData, tessDistNear, tessDistFar);
		//// Render object (combination of mesh geometry and shader process
		m_DepthShader->Render(m_Direct3D->GetDeviceContext(), (*i)->GetIndexCount());

		i++;
	}

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_Direct3D->SetBackBufferRenderTarget();

	// Reset viewport
	m_Direct3D->ResetViewport();
}


void App1::gui()
{
	// Force turn off on Geometry shader
	m_Direct3D->GetDeviceContext()->GSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::InputInt("Patches rendering", &rendering);
	ImGui::InputInt("Patches culling", &culling);
	ImGui::InputFloat("FPS", &frameRate, 0.0f, 0.0f, 0);
	if(optimisationMode == MANUAL)
		ImGui::Text("Optimisation Mode: (Manual)");
	else
		ImGui::Text("Optimisation Mode: (Automatic)");
	if (ImGui::Button("Manual"))
	{
		if (optimisationMode == AUTO)
		{
			optimisationMode = MANUAL;
		}
	}
	if(ImGui::Button("Automatic"))
	{
		if (optimisationMode == MANUAL)
		{
			optimisationMode = AUTO;
		}
	}
	ImGui::Text("Dynamic Tessellation Range:");
	ImGui::SliderFloat("Near", &tessDistNear, MIN_TESS, tessDistFar);
	ImGui::SliderFloat("Far", &tessDistFar, tessDistNear, MAX_TESS);

	if (ImGui::Button("Wireframe", ImVec2(70, 20)))
	{
		if (!wireframe)
		{
			m_Direct3D->TurnOnWireframe();
			wireframe = true;
		}
		else
		{
			m_Direct3D->TurnOffWireframe();
			wireframe = false;
		}
	}
	if (ImGui::Button("Show Depth Map", ImVec2(110, 20)))
	{
		if (!showDepthMap)
		{			
			showDepthMap = true;
		}
		else
		{
			showDepthMap = false;
		}
	}
	if (ImGui::Button("Atmospheric Effects", ImVec2(140, 20)))
	{
		if (!atmosphericEffects)
		{
			atmosphericEffects = true;
		}
		else
		{
			atmosphericEffects = false;
		}
	}

	ImGui::Text("Ridged Multifractal Terrain:");
	ImGui::SliderFloat("Highest Fractal Dimension", &m_MultifractalData.H, 0.0f, 10.0f);
	ImGui::SliderFloat("Lacunarity", &m_MultifractalData.lacunarity, 0.0f, 2.000f);
	ImGui::SliderFloat("Octaves", &m_MultifractalData.octaves, 0.0f, 8.0f);
	ImGui::SliderFloat("Offset", &m_MultifractalData.offset, 0.0f, 5.0f);
	ImGui::SliderFloat("Gain", &m_MultifractalData.gain, 0.0f, 10.0f);

	// Render UI
	ImGui::Render();
}