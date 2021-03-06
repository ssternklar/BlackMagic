#include <sstream>

#include "Tool.h"
#include "Input.h"
#include "Assets.h"
#include "FileUtil.h"
#include "../resource.h"

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

	gui.entityData.meshIndex = -1;
	gui.entityData.materialIndex = -1;
	gui.meshImporter = false;
	gui.textureImporter = false;
	gui.shaderImporter = false;
	gui.shaderType = 0;
	gui.miscImporter = false;
	gui.miscEdit = false;
	gui.sceneCreate = false;
	gui.sceneIndex = -1;
	gui.exitTool = false;
	gui.exportData.prompt = false;
	gui.exportData.sceneCount = 0;
	gui.exportData.dragIndex = -1;
	gui.materialData.create = false;
	gui.materialData.index = 0;
	gui.materialData.edit = false;
	gui.materialData.vertexShaderIndex = 0;
	gui.materialData.pixelShaderIndex = 0;

	CreateConsoleWindow(500, 120, 32, 120);
}

Tool::~Tool()
{
	ImGui_ImplDX11_Shutdown();
}

HRESULT Tool::Run(HINSTANCE hInstance, unsigned int windowWidth, unsigned int windowHeight)
{
	// dirty solution for now
	FileUtil::WriteResourceToDisk(IDR_MESH2, "mesh", "engine/skybox.obj");
	FileUtil::WriteResourceToDisk(IDR_CUBEMAP1, "cubemap", "engine/park_skybox_env.dds");
	FileUtil::WriteResourceToDisk(IDR_CUBEMAP2, "cubemap", "engine/park_skybox_radiance.dds");
	FileUtil::WriteResourceToDisk(IDR_CUBEMAP3, "cubemap", "engine/park_skybox_irradiance.dds");
	FileUtil::WriteResourceToDisk(IDB_PNG2, "png", "engine/cosLUT.png");
	FileUtil::WriteResourceToDisk(IDR_SHADER3, "shader", "engine/QuadVS.hlsl");
	FileUtil::WriteResourceToDisk(IDR_SHADER4, "shader", "engine/LightPassPS.hlsl");
	FileUtil::WriteResourceToDisk(IDR_SHADER5, "shader", "engine/ShadowMapVS.hlsl");
	FileUtil::WriteResourceToDisk(IDR_SHADER6, "shader", "engine/SkyboxVS.hlsl");
	FileUtil::WriteResourceToDisk(IDR_SHADER7, "shader", "engine/SkyboxPS.hlsl");
	FileUtil::WriteResourceToDisk(IDR_SHADER8, "shader", "engine/FXAA_VS.hlsl");
	FileUtil::WriteResourceToDisk(IDR_SHADER9, "shader", "engine/FXAA_PS.hlsl");
	FileUtil::WriteResourceToDisk(IDR_SHADER10, "shader", "engine/FinalMerge.hlsl");

	HRESULT hr = Graphics::Instance().Init(hInstance, windowWidth, windowHeight);
	if (FAILED(hr)) return hr;

	SceneData::Instance().Init(&gui.entityData);

	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x02;
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = Graphics::Instance().GetHandle();
	RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

	ImGui_ImplDX11_Init(Graphics::Instance().GetHandle(), Graphics::Instance().GetDevice(), Graphics::Instance().GetContext());

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
				PostMessage(Graphics::Instance().GetHandle(), WM_CLOSE, NULL, NULL);

			ImGui_ImplDX11_NewFrame();
			
			if (AssetManager::Instance().IsReady())
			{
				float delta = ImGui::GetIO().DeltaTime;
				InvokeGUI();

				ImGuiIO& io = ImGui::GetIO();
				if (!io.WantCaptureKeyboard && !io.WantCaptureMouse && !io.WantTextInput)
				{
					Camera::Instance().Update(delta);
					if (io.MouseClicked[0])
						ScanEntities(io.MousePos.x, io.MousePos.y);
				}

				TransformData::Instance().UpdateTransforms();
				Graphics::Instance().Render(delta);
			}
			else
			{
				Graphics::Instance().Clear();
				HelloGUI();
			}

			ImGui::Render();
			Input::UpdateControlStates();
			Graphics::Instance().Present();
		}
	}

	return (HRESULT)msg.wParam;
}

void Tool::Quit()
{
	DestroyWindow(Graphics::Instance().GetHandle());
}

void Tool::ScanEntities(float x, float y)
{
	SceneData::Handle scene = SceneData::Instance().CurrentScene();

	if (!scene.ptr())
		return;

	BoundingFrustum camFrustum;
	Camera::Instance().frustum.Transform(camFrustum, XMMatrixTranspose(XMLoadFloat4x4(&Camera::Instance().transform->matrix)));

	XMFLOAT4X4 proj;
	XMStoreFloat4x4(&proj, XMMatrixTranspose(XMLoadFloat4x4(&Camera::Instance().ProjectionMatrix())));

	XMFLOAT3 tempRayDir;
	tempRayDir.x = (((2 * x) / Graphics::Instance().GetWidth()) - 1) / proj._11;
	tempRayDir.y = -(((2 * y) / Graphics::Instance().GetHeight()) - 1) / proj._22;
	tempRayDir.z = 1.0f;

	XMMATRIX view = XMMatrixInverse(NULL, XMMatrixTranspose(XMLoadFloat4x4(&Camera::Instance().ViewMatrix())));
	XMVECTOR rayDir = XMVector3TransformNormal(XMVector3Normalize(XMLoadFloat3(&tempRayDir)), view);
	XMVECTOR rayPos = XMLoadFloat3(&Camera::Instance().transform->pos);

	BoundingOrientedBox entBox;
	float distance;
	std::vector<EntityData::Handle> entityQueue;

	for (size_t i = 0; i < scene->entities.size(); ++i)
	{
		scene->entities[i]->mesh->obb.Transform(entBox, XMMatrixTranspose(XMLoadFloat4x4(&scene->entities[i]->transform->matrix)));
		if (camFrustum.Contains(entBox) != ContainmentType::DISJOINT)
			if (entBox.Intersects(rayPos, rayDir, distance))
				entityQueue.push_back(scene->entities[i]);
	}
	
	EntityData::Handle nearestEntity;
	float filterDistance = FLT_MAX;

	for (size_t i = 0; i < entityQueue.size(); ++i)
	{
		EntityData::Handle entity = entityQueue[i];

		XMMATRIX entMatrix = XMMatrixInverse(NULL, XMMatrixTranspose(XMLoadFloat4x4(&entity->transform->matrix)));
		XMVECTOR rayPosLocal = XMVector3TransformCoord(rayPos, entMatrix);
		XMVECTOR rayDirLocal = XMVector3Normalize(XMVector3TransformNormal(rayDir, entMatrix));

		Mesh::Vertex* verts = entity->mesh->verts;
		UINT* faces = entity->mesh->faces;

		for (size_t j = 0; j < entity->mesh->faceCount; j += 3)
		{
			if (TriangleTests::Intersects(rayPosLocal, rayDirLocal, XMLoadFloat3(&verts[faces[j]].position), XMLoadFloat3(&verts[faces[j + 1]].position), XMLoadFloat3(&verts[faces[j + 2]].position), distance))
				if (distance < filterDistance)
				{
					filterDistance = distance;
					nearestEntity = entity;
				}
		}
	}

	SceneData::Instance().SelectEntity(nearestEntity);
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
	Camera::Instance().Resize(width, height);
	Graphics::Instance().Resize(width, height);
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
		if (AssetManager::Instance().IsReady())
		{
			gui.exitTool = true;
			return 0;
		}
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
