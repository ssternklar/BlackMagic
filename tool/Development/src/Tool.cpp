#include <sstream>

#include "Tool.h"
#include "Input.h"
#include "Assets.h"

using namespace DirectX;

Tool* Tool::DXCoreInstance = 0;

LRESULT Tool::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (DXCoreInstance)
		return DXCoreInstance->ProcessMessage(hWnd, uMsg, wParam, lParam);
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

Tool::Tool()
{
	DXCoreInstance = this;
	resizing = false;

	CreateConsoleWindow(500, 120, 32, 120);
}

Tool::~Tool()
{
	ImGui_ImplDX11_Shutdown();
	delete camera;
	delete graphics;
}

HRESULT Tool::Run(HINSTANCE hInstance, unsigned int windowWidth, unsigned int windowHeight)
{
	camera = new Camera();

	graphics = new Graphics(windowWidth, windowHeight);
	HRESULT hr = graphics->Init(hInstance);
	if (FAILED(hr)) return hr;

	MeshData::Instance().Init(graphics->GetDevice());

	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x02;
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = graphics->GetHandle();
	RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

	ImGui_ImplDX11_Init(graphics->GetHandle(), graphics->GetDevice(), graphics->GetContext());

	Input::BindToControl("Quit", VK_ESCAPE);

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (Input::WasControlPressed("Quit"))
				Quit();

			ImGui_ImplDX11_NewFrame();
			
			if (AssetManager::Instance().IsReady())
			{
				float delta = ImGui::GetIO().DeltaTime;
				InvokeGUI();

				ImGuiIO& io = ImGui::GetIO();
				if (!io.WantCaptureKeyboard && !io.WantCaptureMouse && !io.WantTextInput)
				{
					camera->Update(delta);
					if (io.MouseClicked[0])
						ScanEntities(io.MousePos.x, io.MousePos.y);
				}

				TransformData::Instance().UpdateTransforms();
				graphics->Draw(camera, delta);
			}
			else
			{
				graphics->Clear();
				HelloGUI();
			}

			ImGui::Render();
			Input::UpdateControlStates();
			graphics->Present();
		}
	}

	return (HRESULT)msg.wParam;
}

void Tool::Quit()
{
	DestroyWindow(graphics->GetHandle());
}

void Tool::ScanEntities(float x, float y)
{
	BoundingFrustum camFrustum;
	camera->frustum.Transform(camFrustum, XMMatrixTranspose(XMLoadFloat4x4(&camera->transform->matrix)));

	XMFLOAT4X4 proj;
	XMStoreFloat4x4(&proj, XMMatrixTranspose(XMLoadFloat4x4(&camera->ProjectionMatrix())));

	XMFLOAT3 tempRayDir;
	tempRayDir.x = (((2 * x) / graphics->GetWidth()) - 1) / proj._11;
	tempRayDir.y = -(((2 * y) / graphics->GetHeight()) - 1) / proj._22;
	tempRayDir.z = 1.0f;

	XMMATRIX view = XMMatrixInverse(NULL, XMMatrixTranspose(XMLoadFloat4x4(&camera->ViewMatrix())));
	XMVECTOR rayDir = XMVector3TransformNormal(XMVector3Normalize(XMLoadFloat3(&tempRayDir)), view);
	XMVECTOR rayPos = XMLoadFloat3(&camera->transform->pos);

	Entity* entities = EntityData::Instance().Entities();
	size_t count = EntityData::Instance().Size();
	BoundingOrientedBox entBox;
	float distance;
	std::vector<EntityData::Handle> entityQueue;

	for (size_t i = 0; i < count; ++i)
	{
		entities[i].mesh->obb.Transform(entBox, XMMatrixTranspose(XMLoadFloat4x4(&entities[i].transform->matrix)));
		if (camFrustum.Contains(entBox) != ContainmentType::DISJOINT)
			if (entBox.Intersects(rayPos, rayDir, distance))
				entityQueue.push_back(EntityData::Instance().Recover(entities + i));
	}
	
	EntityData::Handle nearestEntity;
	float filterDistance = FLT_MAX;

	for (size_t i = 0; i < entityQueue.size(); ++i)
	{
		EntityData::Handle entity = entityQueue[i];

		XMMATRIX entMatrix = XMMatrixInverse(NULL, XMMatrixTranspose(XMLoadFloat4x4(&entity->transform->matrix)));
		XMVECTOR rayPosLocal = XMVector3TransformCoord(rayPos, entMatrix);
		XMVECTOR rayDirLocal = XMVector3Normalize(XMVector3TransformNormal(rayDir, entMatrix));

		Vertex* verts = entity->mesh->verts;
		UINT* faces = entity->mesh->faces;

		for (size_t j = 0; j < entity->mesh->faceCount; j += 3)
		{
			if (TriangleTests::Intersects(rayPosLocal, rayDirLocal, XMLoadFloat3(&verts[faces[j]].Position), XMLoadFloat3(&verts[faces[j + 1]].Position), XMLoadFloat3(&verts[faces[j + 2]].Position), distance))
				if (distance < filterDistance)
				{
					filterDistance = distance;
					nearestEntity = entity;
				}
		}
	}

	SelectEntity(nearestEntity);
}

void Tool::SelectEntity(EntityData::Handle ent)
{
	selectedEntity = ent;

	if (ent.ptr())
	{
		gui.meshIndex = (int)AssetManager::Instance().GetIndex<MeshData>(ent->mesh);
	}
	else
	{
		gui.meshIndex = -1;
	}
}

void Tool::CreateConsoleWindow(int bufferLines, int bufferColumns, int windowLines, int windowColumns)
{
	CONSOLE_SCREEN_BUFFER_INFO coninfo;

	AllocConsole();
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = bufferLines;
	coninfo.dwSize.X = bufferColumns;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	SMALL_RECT rect;
	rect.Left = 0;
	rect.Top = 0;
	rect.Right = windowColumns;
	rect.Bottom = windowLines;
	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &rect);

	FILE *stream;
	freopen_s(&stream, "CONIN$", "r", stdin);
	freopen_s(&stream, "CONOUT$", "w", stdout);
	freopen_s(&stream, "CONOUT$", "w", stderr);

	HWND consoleHandle = GetConsoleWindow();
	HMENU hmenu = GetSystemMenu(consoleHandle, FALSE);
	EnableMenuItem(hmenu, SC_CLOSE, MF_GRAYED);
}

void Tool::OnResize(unsigned int width, unsigned int height)
{
	ImGui_ImplDX11_InvalidateDeviceObjects();
	camera->Resize(width, height);
	graphics->Resize(width, height);
	ImGui_ImplDX11_CreateDeviceObjects();
}

extern LRESULT ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT Tool::ProcessMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static unsigned int width = 1280;
	static unsigned int height = 720;

	ImGui_ImplDX11_WndProcHandler(hWnd, uMsg, wParam, lParam);

	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_MENUCHAR: 
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_SIZE:
		width = LOWORD(lParam);
		height = HIWORD(lParam);
		if (wParam == SIZE_MAXIMIZED || (wParam == SIZE_RESTORED && !resizing))
			OnResize(width, height);
		return 0;

	case WM_ENTERSIZEMOVE:
		resizing = true;
		return 0;

	case WM_EXITSIZEMOVE:
		resizing = false;
		OnResize(width, height);
		return 0;

	case WM_KEYDOWN:
		Input::OnKeyDown(wParam);
		return 0;

	case WM_KEYUP:
		Input::OnKeyUp(wParam);
		return 0;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu (dear Imgui)
			return 0;
		break;

	case WM_INPUT:
	{
		UINT dwSize = sizeof(RAWINPUT);
		static BYTE lpb[sizeof(RAWINPUT)];

		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

		RAWINPUT* raw = (RAWINPUT*)lpb;

		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			int x = raw->data.mouse.lLastX;
			int y = raw->data.mouse.lLastY;
			Input::OnMouseMove(x, y);
		}
		break;
	}

	case WM_CLOSE:
		gui.exitTool = true;
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
