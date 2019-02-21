// BaseApplication.cpp
// Base application functionality for inheritnace.
#include "BaseApplication.h"


BaseApplication::BaseApplication()
{
	
}


BaseApplication::~BaseApplication()
{
	// Shutdown imGUI
	//ImGui_ImplDX11_Shutdown();

	// Release the timer object.
	if (m_Timer)
	{
		delete m_Timer;
		m_Timer = 0;
	}

	// Release the camera object.
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Release the Direct3D object.
	if (m_Direct3D)
	{
		delete m_Direct3D;
		m_Direct3D = 0;
	}
}

void BaseApplication::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in)
{
	m_Input = in;
	wnd = hwnd;
	sWidth = screenWidth;
	sHeight = screenHeight;

	// Create the Direct3D object.
	m_Direct3D = new D3D(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	// Initialize the Direct3D object.
	if (!m_Direct3D)
	{
		MessageBox(hwnd, L"Could not initialize DirectX 11.", L"Error", MB_OK);
		exit(EXIT_FAILURE);
	}

	// Create the camera object.
	m_Camera = new Camera();
	// Initialize a base view matrix with the camera for 2D user interface rendering.
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera->Update();

	// Create the timer object.
	m_Timer = new Timer();

	//Initialise ImGUI
	ImGui_ImplDX11_Init(wnd, m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext());
}

bool BaseApplication::Frame()
{
	// Check if the user pressed escape and wants to exit the application.
	if (m_Input->isKeyDown(VK_ESCAPE) == true)
	{
		return false;
	}

	// Update the system stats.
	m_Timer->Frame();

	// Do the frame input processing.
	HandleInput(m_Timer->GetTime());

	// Start UI
	ImGui_ImplDX11_NewFrame();

	return true;
}


void BaseApplication::HandleInput(float frameTime)
{
	// Set the frame time for calculating the updated position.
	m_Camera->SetFrameTime(frameTime);

	// Handle the input.
	if (m_Input->isKeyDown('W'))
	{
		// forward
		m_Camera->MoveForward();
	}
	if (m_Input->isKeyDown('S'))
	{
		// back
		m_Camera->MoveBackward();
	}
	if (m_Input->isKeyDown('A'))
	{
		// Strafe Left
		m_Camera->StrafeLeft();
	}
	if (m_Input->isKeyDown('D'))
	{
		// Strafe Right
		m_Camera->StrafeRight();
	}
	if (m_Input->isKeyDown('Q'))
	{
		// Down
		m_Camera->MoveDownward();
	}
	if (m_Input->isKeyDown('E'))
	{
		// Up
		m_Camera->MoveUpward();
	}
	if (m_Input->isKeyDown(VK_UP))
	{
		// rotate up
		m_Camera->TurnUp();
	}
	if (m_Input->isKeyDown(VK_DOWN))
	{
		// rotate down
		m_Camera->TurnDown();
	}
	if (m_Input->isKeyDown(VK_LEFT))
	{
		// rotate left
		m_Camera->TurnLeft();
	}
	if (m_Input->isKeyDown(VK_RIGHT))
	{
		// rotate right
		m_Camera->TurnRight();
	}

	if (m_Input->isMouseActive())
	{
		// mouse look is on
		deltax = m_Input->getMouseX() - (sWidth / 2);
		deltay = m_Input->getMouseY() - (sHeight / 2);
		m_Camera->Turn(deltax, deltay);
		cursor.x = sWidth / 2;
		cursor.y = sHeight / 2;
		ClientToScreen(wnd, &cursor);
		SetCursorPos(cursor.x, cursor.y);
	}

	if (m_Input->isKeyDown(VK_SPACE))
	{
		// re-position cursor
		cursor.x = sWidth / 2;
		cursor.y = sHeight / 2;
		ClientToScreen(wnd, &cursor);
		SetCursorPos(cursor.x, cursor.y);
		m_Input->setMouseX(sWidth / 2);
		m_Input->setMouseY(sHeight / 2);
		m_Input->SetKeyUp(VK_SPACE);
		// if space pressed toggle mouse
		m_Input->setMouseActive(!m_Input->isMouseActive());
		if (!m_Input->isMouseActive())
		{
			ShowCursor(true);
		}
		else
		{
			ShowCursor(false);
		}
	}

	
}
