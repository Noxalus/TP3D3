#include "stdafx.h"
#include "d3d9.h"
#include "d3dx9.h"
#include "3DTP3.h"
#include <math.h>
#include "InputManager.h"

// Global Variables:
HINSTANCE hInst;			// current instance
HWND	    hWnd;				// windows handle used in DirectX initialization

// Forward declarations
bool				CreateWindows(HINSTANCE, int);
bool				CreateDevice();
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

// Input Manager
InputManager* input;

// Height map
unsigned short m_sizeX;
unsigned short m_sizeZ;
float m_maxY = 50;
float* m_height;

// Structure d'un vertex
struct Vertex
{
	D3DXVECTOR3 Position;
	D3DCOLOR Color;
};

bool LoadRAW (const std::string& map)
{
	FILE  *file;
	fopen_s(&file, map.c_str (), "rb");
	if (!file)
		return false;
	fread(&m_sizeX, sizeof(unsigned short), 1, file);
	fread(&m_sizeZ, sizeof(unsigned short), 1, file);
	unsigned int size = m_sizeX * m_sizeZ;
	unsigned char *tmp = new unsigned char[size];
	m_height = new float[size];
	fread(tmp, sizeof(unsigned char), size, file);
	fclose(file);
	int i = 0;
	for (unsigned short z = 0; z < m_sizeZ; ++z)
		for (unsigned short x = 0; x < m_sizeX; ++x, ++i)
			m_height[i] = float ((m_maxY * tmp[i]) / 255.0f);
	delete[] tmp;
	return true;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG oMsg;

	if (!CreateWindows (hInstance, nCmdShow))
	{
		MessageBox(NULL, L"Erreur lors de la création de la fenêtre", L"Error", 0);
		return false;
	}
	if (!CreateDevice())
	{
		MessageBox(NULL, L"Erreur lors de la création du device DirectX 9", L"Error", 0);
		return false;
	}

	// Input manager
	input = new InputManager();
	input->init(hInstance, hWnd);

	D3DCOLOR backgroundColor = D3DCOLOR_RGBA(0, 0, 0, 0);

	D3DXMATRIX WorldViewProj;
	D3DXMatrixIdentity(&WorldViewProj);

	// World matrix
	D3DXMATRIX World;

	D3DXMATRIX Position;
	D3DXMATRIX Rotation;

	D3DXMatrixIdentity(&Rotation);
	D3DXMatrixIdentity(&Position);

	World = Position * Rotation;

	// View matrix
	D3DXMATRIX View;

	D3DXVECTOR3 Eye (0.f, 0.f, -1.f);
	D3DXVECTOR3 At (0.f, 0.f, 0.f);
	D3DXVECTOR3 Up (0.f, 1.f, 0.f);
	D3DXMatrixLookAtLH(&View, &Eye, &At, &Up);

	// Projection matrix
	D3DXMATRIX Projection;

	float fovy = D3DX_PI / 2; // pi / 2
	D3DXMatrixPerspectiveFovLH(&Projection, fovy, 1.3f, 0.1f, 1000.0f);



	// Création de l’interface DirectX 9
	LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	D3DDISPLAYMODE displayMode;
	D3DPRESENT_PARAMETERS pp;
	pD3D->GetAdapterDisplayMode (D3DADAPTER_DEFAULT, &displayMode);
	pp.Windowed = true; //Mode fenêtré ou pas
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.BackBufferWidth  = 800;// Taille en x du Back Buffer
	pp.BackBufferHeight  = 600; // Taille en y du Back Buffer
	pp.BackBufferFormat  =  displayMode.Format; // Format du Back Buffer
	pp.BackBufferCount  = 1; // Nombre de Back Buffer
	pp.MultiSampleType = D3DMULTISAMPLE_NONE ; // Nombre de sample pour l’antialiasing
	pp.MultiSampleQuality  = 0; // Qualité pour l’antialiasing
	pp.hDeviceWindow = hWnd; //Handle de la fenêtre
	pp.EnableAutoDepthStencil = true; // True si on veut un depth-stencil buffer
	pp.AutoDepthStencilFormat = D3DFMT_D24S8; // Le format du deth-stencil buffer
	pp.Flags = 0; // Voir le man
	pp.FullScreen_RefreshRateInHz = 0; //Voir le man
	pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT; // Autrement dit 0, voir le man

	IDirect3DDevice9 *device;
	pD3D->CreateDevice(0, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &device);

	D3DDEVTYPE DeviceType = D3DDEVTYPE_HAL;
	int num_card = 0;
	// Look for 'NVIDIA PerfHUD' adapter
	// If it is present, override default settings
	for (unsigned int adapter = 0; adapter < pD3D->GetAdapterCount (); 
		adapter++)
	{
		D3DADAPTER_IDENTIFIER9 identifier;
		HRESULT Res;
		Res = pD3D->GetAdapterIdentifier (adapter, 0, &identifier);
		if (strstr (identifier.Description, "PerfHUD") != 0)
		{
			num_card = adapter;
			DeviceType = D3DDEVTYPE_REF;
			break;
		}
	}

	//Create and fill other DirectX Stuffs like Vertex/Index buffer, shaders  

	// Vertex declaration
	D3DVERTEXELEMENT9 dwDecl3[] = 
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		D3DDECL_END() 
	};

	IDirect3DVertexDeclaration9 *pDecl;
	device->CreateVertexDeclaration(dwDecl3, &pDecl );

	// Culling ?
	//device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// Wireframe ?
	//device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	/** Vertex & Index buffers (Triangle) **/
	// Vertex buffer
	int vertexCount = 3;
	IDirect3DVertexBuffer9* pVertexBuffer;
	device->CreateVertexBuffer(vertexCount * sizeof(Vertex), 0, 0, D3DPOOL_DEFAULT, &pVertexBuffer, NULL);

	Vertex* pVertexData;

	pVertexBuffer->Lock(0, 0, (void**) &pVertexData, 0);

	pVertexData[0].Position = D3DXVECTOR3(-0.5f, -0.5f, 0);
	pVertexData[0].Color = D3DCOLOR_RGBA(255, 0, 0, 0);

	pVertexData[1].Position = D3DXVECTOR3(0.f, 0.5f, 0);
	pVertexData[1].Color = D3DCOLOR_RGBA(0, 255, 0, 0);

	pVertexData[2].Position = D3DXVECTOR3(0.5f, -0.5f, 0);
	pVertexData[2].Color = D3DCOLOR_RGBA(0, 0, 255, 0);

	pVertexBuffer-> Unlock();

	// Index buffer
	IDirect3DIndexBuffer9* pIndexBuffer;
	device->CreateIndexBuffer(sizeof(int) * vertexCount, 0, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &pIndexBuffer, NULL);

	int* pIndexData;
	pIndexBuffer->Lock(0, 0, (void**) &pIndexData, 0);

	pIndexData[0] = 0;
	pIndexData[1] = 1;
	pIndexData[2] = 2;

	pIndexBuffer->Unlock();

	/** Vertex & Index buffers (Circle) **/
	// Vertex buffer
	int circleVertexCount = 100;
	IDirect3DVertexBuffer9* pCircleVertexBuffer;
	device->CreateVertexBuffer(circleVertexCount * sizeof(Vertex), 0, 0, D3DPOOL_DEFAULT, &pCircleVertexBuffer, NULL);

	Vertex* pCircleVertexData;

	pCircleVertexBuffer->Lock(0, 0, (void**) &pCircleVertexData, 0);

	// Center
	pCircleVertexData[0].Position = D3DXVECTOR3(0, 0, 0.5f);
	pCircleVertexData[0].Color = D3DCOLOR_RGBA(255, 0, 0, 0);

	double step = (2 * D3DX_PI) / (circleVertexCount - 1);
	double pos = 0;
	int blue = 255;
	int red = 0;
	int green = 0;
	for(int i = 1; i < circleVertexCount; i++)
	{
		pCircleVertexData[i].Position = D3DXVECTOR3(cos(pos), sin(pos), 0.5f);
		pCircleVertexData[i].Color = D3DCOLOR_RGBA(red, green, blue, 0);

		pos -= step;
	}

	pCircleVertexBuffer-> Unlock();

	// Index buffer
	IDirect3DIndexBuffer9* pCircleIndexBuffer;
	device->CreateIndexBuffer(sizeof(int) * (circleVertexCount + 1), 0, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &pCircleIndexBuffer, NULL);

	int* pCircleIndexData;
	pCircleIndexBuffer->Lock(0, 0, (void**) &pCircleIndexData, 0);

	for(int i = 0; i < circleVertexCount; i++)
	{
		pCircleIndexData[i] = i;
	}

	pCircleIndexData[circleVertexCount] = 1;

	pCircleIndexBuffer->Unlock();

	/** Vertex & Index buffers (Rectangle) **/
	// Vertex buffer
	int rectangleVertexCount = 4;
	IDirect3DVertexBuffer9* pRectangleVertexBuffer;
	device->CreateVertexBuffer(rectangleVertexCount * sizeof(Vertex), 0, 0, D3DPOOL_DEFAULT, &pRectangleVertexBuffer, NULL);

	Vertex* pRectangleVertexData;

	pRectangleVertexBuffer->Lock(0, 0, (void**) &pRectangleVertexData, 0);

	pRectangleVertexData[0].Position = D3DXVECTOR3(-2, 2, 1.5f);
	pRectangleVertexData[0].Color = D3DCOLOR_RGBA(255, 0, 0, 0);

	pRectangleVertexData[1].Position = D3DXVECTOR3(2, 2, 1.5f);
	pRectangleVertexData[1].Color = D3DCOLOR_RGBA(0, 255, 0, 0);

	pRectangleVertexData[2].Position = D3DXVECTOR3(-2, -2, 1.5f);
	pRectangleVertexData[2].Color = D3DCOLOR_RGBA(0, 0, 255, 0);

	pRectangleVertexData[3].Position = D3DXVECTOR3(2, -2, 1.5f);
	pRectangleVertexData[3].Color = D3DCOLOR_RGBA(255, 0, 0, 0);

	pRectangleVertexBuffer-> Unlock();

	// Index buffer
	IDirect3DIndexBuffer9* pRectangleIndexBuffer;
	device->CreateIndexBuffer(sizeof(int) * rectangleVertexCount, 0, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &pRectangleIndexBuffer, NULL);

	int* pRectangleIndexData;
	pRectangleIndexBuffer->Lock(0, 0, (void**) &pRectangleIndexData, 0);

	for(int i = 0; i < rectangleVertexCount; i++)
	{
		pRectangleIndexData[i] = i;
	}

	pRectangleIndexBuffer->Unlock();

	/** Vertex & Index buffers (Heightmap) **/
	// Load height map
	LoadRAW("../Resources/terrainheight.raw");

	/*
	m_sizeX = 10;
	m_sizeZ = 10;
	*/

	// Vertex buffer
	IDirect3DVertexBuffer9* pMapVertexBuffer;
	device->CreateVertexBuffer((m_sizeX * m_sizeZ) * sizeof(Vertex), 0, 0, D3DPOOL_DEFAULT, &pMapVertexBuffer, NULL);

	Vertex* pMapVertexData;

	pMapVertexBuffer->Lock(0, 0, (void**) &pMapVertexData, 0);

	for(int x = 0; x < m_sizeX; x++)
	{
		for(int y = 0; y < m_sizeZ; y++)
		{
			float heightValue = m_height[x + (m_sizeX * y)];

			pMapVertexData[x + (m_sizeX * y)].Position = D3DXVECTOR3(x, heightValue, y);

			// Blue
			if (heightValue < m_maxY / 3)
				pMapVertexData[x + (m_sizeX * y)].Color = D3DCOLOR_RGBA(0, 0, 255, 0);
			// Green
			else if (heightValue >= m_maxY / 3 && heightValue <= 2 * (m_maxY / 3))
				pMapVertexData[x + (m_sizeX * y)].Color = D3DCOLOR_RGBA(0, 255, 0, 0);
			// Red
			else
				pMapVertexData[x + (m_sizeX * y)].Color = D3DCOLOR_RGBA(255, 0, 0, 0);
		}
	}

	pMapVertexBuffer->Unlock();

	// Index buffer
	IDirect3DIndexBuffer9* pMapIndexBuffer;
	device->CreateIndexBuffer(6 * (m_sizeX * m_sizeZ) * sizeof(int), 0, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &pMapIndexBuffer, NULL);

	int* pMapIndexData;
	pMapIndexBuffer->Lock(0, 0, (void**) &pMapIndexData, 0);


	int counter = 0;
	for(int y = 0; y < m_sizeZ - 1; y++)
	{
		for(int x = 0; x < m_sizeX - 1; x++)
		{
			// First triangle
			pMapIndexData[counter] = x + (m_sizeX * (y + 1));
			counter++;
			pMapIndexData[counter] = (x + 1) + (m_sizeX * y);
			counter++;
			pMapIndexData[counter] = x + (m_sizeX * y);
			counter++;

			// Second triangle
			pMapIndexData[counter] = (x + 1) + (m_sizeX * (y + 1));
			counter++;
			pMapIndexData[counter] = (x + 1) + (m_sizeX * y);
			counter++;
			pMapIndexData[counter] = x + (m_sizeX * (y + 1));
			counter++;
		}
	}

	pMapIndexBuffer->Unlock();

	// Shader
	LPCWSTR pFxFile = L"../Resources/shader.fx";
	LPD3DXEFFECT pEffect;
	LPD3DXBUFFER CompilationErrors;

	if (D3D_OK != D3DXCreateEffectFromFile(device, pFxFile, NULL, NULL, 0, NULL, &pEffect, &CompilationErrors))
	{
		MessageBoxA (NULL, (char *) 
			CompilationErrors->GetBufferPointer(), "Error", 0);
	}

	D3DXHANDLE hWorldViewProj = pEffect->GetParameterByName (NULL, "WorldViewProj");

	PeekMessage( &oMsg, NULL, 0, 0, PM_NOREMOVE );
	while ( oMsg.message != WM_QUIT )
	{
		if (PeekMessage( &oMsg, NULL, 0, 0, PM_REMOVE )) 
		{
			TranslateMessage( &oMsg );
			DispatchMessage( &oMsg );
		}
		else
		{
			/*
			// Update the input state
			if (input->keyDown(DIK_LEFT)){
			D3DXMatrixTranslation(&Position, -1, 0, 0);
			}
			if (input->keyDown(DIK_RIGHT)){
			D3DXMatrixTranslation(&Position, 1, 0, 0);
			}
			if (input->keyDown(DIK_UP)){
			}
			if (input->keyDown(DIK_DOWN)){
			}

			//D3DXMatrixRotationX(&Rotation, -D3DX_PI / 4);
			*/

			// Top view
			/*
			D3DXMatrixRotationX(&Rotation, -D3DX_PI / 2);
			D3DXMatrixTranslation(&Position, -((m_sizeX - 1) / 2), -((m_sizeZ + 5) / 2), -4);
			*/


			D3DXMatrixRotationX(&Rotation, -D3DX_PI / 10);
			D3DXMatrixRotationY(&Rotation, D3DX_PI / 5);
			D3DXMatrixTranslation(&Position, -((m_sizeX - 1) / 2), -((m_sizeZ - 1) / 2) + 100, 10);

			World =  Rotation * Position;

			WorldViewProj = World * View * Projection;

			// Do a lot of thing like draw triangles with DirectX
			device->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, backgroundColor, 1.0f, 0);
			device->BeginScene();

			// Set device vertex declaration
			device->SetVertexDeclaration(pDecl);

			// "Send" WorldViewProj matrix to shader
			pEffect->SetMatrix(hWorldViewProj, &WorldViewProj);

			unsigned int cPasses, iPass;

			// Draw triangle
			/*
			device->SetStreamSource(0, pVertexBuffer, 0, sizeof(Vertex));
			device->SetIndices(pIndexBuffer);

			cPasses = 0, iPass = 0;
			pEffect->Begin(&cPasses, 0);
			for (iPass= 0; iPass< cPasses; ++iPass)
			{
			pEffect->BeginPass(iPass);
			pEffect->CommitChanges(); // que si on a changé des états après le BeginPass

			device->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, vertexCount, 0, 1);

			pEffect->EndPass();
			}

			pEffect->End();
			*/
			/*
			// Draw circle
			device->SetStreamSource(0, pCircleVertexBuffer, 0, sizeof(Vertex));
			device->SetIndices(pCircleIndexBuffer);

			cPasses = 0, iPass = 0;
			pEffect->Begin(&cPasses, 0);
			for (iPass= 0; iPass< cPasses; ++iPass)
			{
			pEffect->BeginPass(iPass);
			pEffect->CommitChanges(); // que si on a changé des états après le BeginPass

			device->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, circleVertexCount, 0, circleVertexCount - 1);

			pEffect->EndPass();
			}

			pEffect->End();

			// Draw rectangle
			device->SetStreamSource(0, pRectangleVertexBuffer, 0, sizeof(Vertex));
			device->SetIndices(pRectangleIndexBuffer);

			cPasses = 0, iPass = 0;
			pEffect->Begin(&cPasses, 0);
			for (iPass= 0; iPass< cPasses; ++iPass)
			{
			pEffect->BeginPass(iPass);
			pEffect->CommitChanges(); // que si on a changé des états après le BeginPass

			device->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, rectangleVertexCount, 0, 2);

			pEffect->EndPass();
			}

			pEffect->End();
			*/

			// Draw map
			device->SetStreamSource(0, pMapVertexBuffer, 0, sizeof(Vertex));
			device->SetIndices(pMapIndexBuffer);

			cPasses = 0, iPass = 0;
			pEffect->Begin(&cPasses, 0);
			for (iPass= 0; iPass< cPasses; ++iPass)
			{
				pEffect->BeginPass(iPass);
				pEffect->CommitChanges(); // que si on a changé des états après le BeginPass

				device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 2 * (m_sizeX * m_sizeZ), 0, 2 * ((m_sizeX * m_sizeZ) - 1));

				pEffect->EndPass();
			}

			pEffect->End();


			device->EndScene();
			device->Present(NULL, NULL, NULL, NULL); 
		}
	}

	//Release D3D objectssss
	pD3D->Release();
	device->Release();
	pDecl->Release();
	pVertexBuffer->Release();
	pIndexBuffer->Release();
	pCircleVertexBuffer->Release();
	pCircleIndexBuffer->Release();
	pRectangleVertexBuffer->Release();
	pRectangleIndexBuffer->Release();
	pEffect->Release();

	return (int) oMsg.wParam;
}


bool CreateWindows(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	//
	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style					= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc		= WndProc;
	wcex.hInstance			= hInstance;
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszClassName	= L"3DTPClassName";

	if ( RegisterClassEx(&wcex) == 0 )
		return false;

	hWnd = CreateWindow(L"3DTPClassName", L"This course is awesome", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return false;

	SetWindowPos(hWnd, NULL, 0, 0, 800, 600, 0);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return true;
}

bool CreateDevice()
{
	return true;
}

//
//  PURPOSE:  Processes messages for the main window.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		break;
	case WM_KEYUP:
		{
			switch(wParam)
			{
			case VK_ESCAPE:
				{
					PostQuitMessage(0);
					break;
				}
			}
			break;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}