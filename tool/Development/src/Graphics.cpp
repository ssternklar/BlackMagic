#include "Graphics.h"
#include "Tool.h"
#include "Assets.h"

using namespace DirectX;

Graphics::Graphics()
{
	device = nullptr;
	context = nullptr;
	swapChain = nullptr;
	backBuffer = nullptr;
	depthStencil = nullptr;

	depthStencilTexture = nullptr;
	gBufferSampler = nullptr;
	quad = nullptr;
	blendState = nullptr;

	shadowRS = nullptr;
	shadowSampler = nullptr;
	for (size_t i = 0; i < NUM_SHADOW_CASCADES; ++i)
		shadowMapDSVs[i] = nullptr;
	shadowMapSRV = nullptr;

	skyboxSampler = nullptr;
	envSampler = nullptr;
	skyboxDS = nullptr;
	skyboxRS = nullptr;
}

Graphics::~Graphics()
{
	DX_RELEASE(depthStencil);
	DX_RELEASE(backBuffer);
	DX_RELEASE(swapChain);

	DX_RELEASE(depthStencilTexture);
	DX_RELEASE(gBufferSampler);
	DX_RELEASE(quad);
	DX_RELEASE(blendState);

	DX_RELEASE(shadowRS);
	DX_RELEASE(shadowSampler);
	for (size_t i = 0; i < NUM_SHADOW_CASCADES; ++i)
		DX_RELEASE(shadowMapDSVs[i]);
	DX_RELEASE(shadowMapSRV);

	DX_RELEASE(skyboxSampler);
	DX_RELEASE(envSampler);
	DX_RELEASE(skyboxDS );
	DX_RELEASE(skyboxRS);
	DX_RELEASE(context);

	//ID3D11Debug* debug;
	//device->QueryInterface<ID3D11Debug>(&debug);
	//debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	//debug->Release();

	DX_RELEASE(device);
}

HRESULT Graphics::Init(HINSTANCE hInstance, unsigned int windowWidth, unsigned int windowHeight)
{
	width = windowWidth;
	height = windowHeight;

	HRESULT hr = S_OK;
	hr = InitWindow(hInstance);
	if (FAILED(hr)) return hr;
	hr = InitDirectX();
	if (FAILED(hr)) return hr;

	MeshData::Instance().Init(device);
	TextureData::Instance().Init(device, context);
	VertexShaderData::Instance().Init(device, context);
	PixelShaderData::Instance().Init(device, context);
	MaterialData::Instance().Init(device);
	
	color = { 0.4f, 0.6f, 0.75f, 0.0f };

	InitBuffers();

	light = {
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 1, -1, 1 },
		0,
		{ 0, 1, 1 }
	};

	XMFLOAT2 quadVerts[6] = {
		{ -1, 1 },
		{ 1, 1 },
		{ 1, -1 },
		{ -1, 1 },
		{ 1, -1 },
		{ -1, -1 }
	};

	D3D11_BUFFER_DESC vbDesc = { 0 };
	vbDesc.ByteWidth = sizeof(quadVerts);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA vbData = { 0 };
	vbData.pSysMem = quadVerts;

	device->CreateBuffer(&vbDesc, &vbData, &quad);

	skybox = MeshData::Instance().Load("engine/skybox.obj");
	skyboxTex = TextureData::Instance().Load("engine/park_skybox_env.dds", TextureDesc::CUBEMAP, TextureDesc::READ);
	skyboxRadiance = TextureData::Instance().Load("engine/park_skybox_radiance.dds", TextureDesc::CUBEMAP, TextureDesc::READ);
	skyboxIrradiance = TextureData::Instance().Load("engine/park_skybox_irradiance.dds", TextureDesc::CUBEMAP, TextureDesc::READ);
	cosLookup = TextureData::Instance().Load("engine/cosLUT.png", TextureDesc::FLAT, TextureDesc::READ);

	//Load device-specific shaders
	lightPassVS = VertexShaderData::Instance().Load("engine/QuadVS.hlsl");
	lightPassPS = PixelShaderData::Instance().Load("engine/LightPassPS.hlsl");
	shadowMapVS = VertexShaderData::Instance().Load("engine/ShadowMapVS.hlsl");
	skyboxVS = VertexShaderData::Instance().Load("engine/SkyboxVS.hlsl");
	skyboxPS = PixelShaderData::Instance().Load("engine/SkyboxPS.hlsl");
	fxaaVS = VertexShaderData::Instance().Load("engine/FXAA_VS.hlsl");
	fxaaPS = PixelShaderData::Instance().Load("engine/FXAA_PS.hlsl");
	mergePS = PixelShaderData::Instance().Load("engine/FinalMerge.hlsl");

	//Set up g-buffer sampler
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampDesc, &gBufferSampler);

	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	device->CreateSamplerState(&sampDesc, &envSampler);

	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	shadowSampDesc.BorderColor[0] = 0.0f;
	shadowSampDesc.BorderColor[1] = 0.0f;
	shadowSampDesc.BorderColor[2] = 0.0f;
	shadowSampDesc.BorderColor[3] = 0.0f;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	D3D11_RASTERIZER_DESC shadowRSDesc = {};
	shadowRSDesc.CullMode = D3D11_CULL_FRONT;
	shadowRSDesc.FillMode = D3D11_FILL_SOLID;
	shadowRSDesc.DepthClipEnable = true;
	shadowRSDesc.DepthBias = 1000;
	shadowRSDesc.DepthBiasClamp = 0.0f;
	shadowRSDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRSDesc, &shadowRS);
	device->CreateSamplerState(&sampDesc, &skyboxSampler);

	D3D11_DEPTH_STENCIL_DESC skyboxDSDesc = {};
	D3D11_DEPTH_STENCILOP_DESC defaultDesc = { D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };
	skyboxDSDesc.BackFace = defaultDesc;
	skyboxDSDesc.FrontFace = defaultDesc;
	skyboxDSDesc.DepthEnable = TRUE;
	skyboxDSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	skyboxDSDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	skyboxDSDesc.StencilEnable = FALSE;
	skyboxDSDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	skyboxDSDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	device->CreateDepthStencilState(&skyboxDSDesc, &skyboxDS);

	D3D11_RASTERIZER_DESC skyboxRSDesc = {};
	skyboxRSDesc.CullMode = D3D11_CULL_FRONT;
	skyboxRSDesc.FillMode = D3D11_FILL_SOLID;
	device->CreateRasterizerState(&skyboxRSDesc, &skyboxRS);

	return hr;
}

HRESULT Graphics::InitWindow(HINSTANCE hInstance)
{
	WNDCLASS wndClass = {};
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = Tool::WindowProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = "Direct3DWindowClass";

	if (!RegisterClass(&wndClass))
	{
		DWORD error = GetLastError();

		if (error != ERROR_CLASS_ALREADY_EXISTS)
			return HRESULT_FROM_WIN32(error);
	}

	RECT clientRect;
	SetRect(&clientRect, 0, 0, width, height);
	AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, false);

	RECT desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);
	int centeredX = (desktopRect.right / 2) - (clientRect.right / 2);
	int centeredY = (desktopRect.bottom / 2) - (clientRect.bottom / 2);

	hWnd = CreateWindow(
		wndClass.lpszClassName,
		"ChiroTool",
		WS_OVERLAPPEDWINDOW,
		centeredX,
		centeredY,
		clientRect.right - clientRect.left,
		clientRect.bottom - clientRect.top,
		0,
		0,
		hInstance,
		0);

	if (hWnd == NULL)
	{
		DWORD error = GetLastError();
		return HRESULT_FROM_WIN32(error);
	}

	ShowWindow(hWnd, SW_SHOW);

	return S_OK;
}

HRESULT Graphics::InitDirectX()
{
	unsigned int deviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Width = width;
	swapDesc.BufferDesc.Height = height;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.Flags = 0;
	swapDesc.OutputWindow = hWnd;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = true;

	HRESULT hr = S_OK;

	hr = D3D11CreateDeviceAndSwapChain(
		0,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		deviceFlags,
		0,
		0,
		D3D11_SDK_VERSION,
		&swapDesc,
		&swapChain,
		&device,
		&dxFeatureLevel,
		&context);

	if (!FAILED(hr))
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return hr;
}

void Graphics::Resize(unsigned int width, unsigned int height)
{
	if (!device)
		return;

	this->width = width;
	this->height = height;

	backBuffer->Release();
	swapChain->ResizeBuffers(
		1,
		width,
		height,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0);


	InitBuffers();
}

void Graphics::Clear()
{
	FLOAT black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->ClearRenderTargetView(backBuffer, &color.x);
	context->ClearRenderTargetView(albedoMap->rtv, black);
	context->ClearRenderTargetView(roughnessMap->rtv, black);
	context->ClearRenderTargetView(normalMap->rtv, black);
	context->ClearRenderTargetView(positionMap->rtv, black);
	context->ClearRenderTargetView(metalMap->rtv, black);
	context->ClearRenderTargetView(lightMap->rtv, black);
	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	for (size_t i = 0; i < NUM_SHADOW_CASCADES; i++)
		context->ClearDepthStencilView(shadowMapDSVs[i], D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Graphics::Render(float deltaTime)
{
	Clear();

	SceneData::Handle scene = SceneData::Instance().CurrentScene();

	if (!scene.ptr())
		return;
	
	static UINT stride = sizeof(Mesh::Vertex);
	static UINT quadStride = sizeof(XMFLOAT2);
	static UINT offset = 0;
	ID3D11ShaderResourceView* srvs[32] = { 0 };

	//RenderShadowMaps();

	ID3D11RenderTargetView* rts[] = {
		albedoMap->rtv,
		positionMap->rtv,
		roughnessMap->rtv,
		normalMap->rtv,
		cavityMap->rtv,
		metalMap->rtv
	};
	context->OMSetRenderTargets(sizeof(rts) / sizeof(ID3D11RenderTargetView*), rts, depthStencil);

	XMFLOAT4X4 view = Camera::Instance().ViewMatrix();
	XMFLOAT4X4 proj = Camera::Instance().ProjectionMatrix();

	EntityData::Handle entity;
	SimpleVertexShader* vertexShader;

	for (size_t i = 0; i < scene->entities.size(); ++i)
	{
		entity = scene->entities[i];
		vertexShader = entity->material->vertexShader->shader;

		vertexShader->SetMatrix4x4("world", scene->entities[i]->transform->matrix);
		vertexShader->SetMatrix4x4("view", view);
		vertexShader->SetMatrix4x4("projection", proj);

		MaterialData::Instance().Use(entity->material);

		context->IASetVertexBuffers(0, 1, &scene->entities[i]->mesh->vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(scene->entities[i]->mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		context->DrawIndexed(scene->entities[i]->mesh->faceCount, 0, 0);
	}

	context->OMSetRenderTargets(1, &lightMap->rtv, nullptr);
	XMFLOAT3 camPos = Camera::Instance().transform->pos;
	//size_t padding = (16 - (sizeof(DirectionalLight) % 16))*(lights.size() - 1);

	lightPassVS->shader->SetShader();
	lightPassPS->shader->SetShader();
	lightPassPS->shader->SetFloat3("cameraPosition", camPos);
	lightPassPS->shader->SetData("sceneLight", &light, sizeof(DirectionalLight));
	lightPassPS->shader->SetData("lightView", &shadowViews[0], sizeof(XMFLOAT4X4)*NUM_SHADOW_CASCADES);
	lightPassPS->shader->SetData("lightProjection", &shadowProjections[0], sizeof(XMFLOAT4X4)*NUM_SHADOW_CASCADES);
	lightPassPS->shader->SetSamplerState("mainSampler", gBufferSampler);
	lightPassPS->shader->SetSamplerState("shadowSampler", shadowSampler);
	lightPassPS->shader->SetSamplerState("envSampler", envSampler);
	lightPassPS->shader->SetShaderResourceView("albedoMap", albedoMap->srv);
	lightPassPS->shader->SetShaderResourceView("roughnessMap", roughnessMap->srv);
	lightPassPS->shader->SetShaderResourceView("positionMap", positionMap->srv);
	lightPassPS->shader->SetShaderResourceView("normalMap", normalMap->srv);
	lightPassPS->shader->SetShaderResourceView("metalnessMap", metalMap->srv);
	lightPassPS->shader->SetShaderResourceView("cavityMap", cavityMap->srv);
	lightPassPS->shader->SetShaderResourceView("shadowMap", shadowMapSRV);
	lightPassPS->shader->SetShaderResourceView("depth", depthStencilTexture);
	lightPassPS->shader->SetShaderResourceView("skyboxRadianceMap", skyboxRadiance->srv);
	lightPassPS->shader->SetShaderResourceView("skyboxIrradianceMap", skyboxIrradiance->srv);
	lightPassPS->shader->SetShaderResourceView("cosLookup", cosLookup->srv);
	lightPassPS->shader->CopyAllBufferData();

	context->IASetVertexBuffers(0, 1, &quad, &quadStride, &offset);
	context->Draw(6, 0);
	context->PSSetShaderResources(0, lightPassPS->shader->GetShaderResourceViewCount() + 1, srvs);
	
	ID3D11BlendState* oldBlendState = nullptr;
	float blendFac[4];
	UINT sampleMask = 0;
	context->OMSetRenderTargets(1, &backBuffer, nullptr);
	context->OMGetBlendState(&oldBlendState, blendFac, &sampleMask);
	mergePS->shader->SetShader();
	mergePS->shader->SetSamplerState("mainSampler", gBufferSampler);
	mergePS->shader->SetShaderResourceView("lightMap", lightMap->srv);
	mergePS->shader->CopyAllBufferData();
	context->Draw(6, 0);

	RenderSkybox();

	context->OMSetBlendState(oldBlendState, blendFac, sampleMask);
	context->PSSetShaderResources(0, mergePS->shader->GetShaderResourceViewCount() + 1, srvs);

	/*fxaaVS->SetShader();
	fxaaPS->SetShader();
	fxaaPS->SetInt("width", width);
	fxaaPS->SetInt("height", height);
	fxaaPS->SetSamplerState("mainSampler", gBufferSampler.As<SamplerHandle>());
	fxaaPS->SetShaderResourceView("inputMap", lightMap->GetShaderResource());
	fxaaPS->CopyAllBufferData();
	context->Draw(6, 0);*/
}

void Graphics::RenderShadowMaps()
{
	Camera* cam = &Camera::Instance();
	const float coeff = (cam->farPlane - cam->nearPlane) / NUM_SHADOW_CASCADES;
	DirectX::BoundingFrustum frustum = cam->frustum;
	XMFLOAT4X4 vT = cam->ViewMatrix();
	XMVECTOR dir = DirectX::XMVector3Normalize(XMLoadFloat3(&light.Direction));
	XMVECTOR up = XMLoadFloat3(&light.Up);

	XMFLOAT3 points[8];
	XMVECTOR pointsV[8];
	BoundingBox box;
	for (size_t thisCascade = 0; thisCascade < NUM_SHADOW_CASCADES; thisCascade++)
	{
		float zNear = cam->nearPlane;
		float zFar = (thisCascade + 1)*coeff;

		auto subfrustum = BoundingFrustum(XMMatrixPerspectiveFovLH(cam->fov, static_cast<float>(width) / height, zNear, zFar));
		subfrustum.Transform(subfrustum, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&vT))));
		subfrustum.GetCorners(points);

		XMVECTOR centroid = XMVectorZero();
		for (size_t i = 0; i < 8; i++)
		{
			pointsV[i] = XMLoadFloat3(&points[i]);
			centroid += pointsV[i];
		}

		centroid /= 8;
		centroid = XMVectorSetW(centroid, 1.0f);

		float len;
		XMStoreFloat(&len, XMVector3Length(pointsV[4] - pointsV[5]));
		float dist = max(zFar - zNear, len) + 50.0f;
		XMMATRIX shadowView = XMMatrixLookAtLH(centroid - dist*(dir), centroid, up);

		XMStoreFloat4x4(&shadowViews[thisCascade], XMMatrixTranspose(shadowView));
		subfrustum.Transform(subfrustum, shadowView);
		subfrustum.GetCorners(points);

		static float extent = 0.0f;
		XMVECTOR minV, maxV;
		XMFLOAT3 min, max;
		BoundingBox::CreateFromPoints(box, 8, points, sizeof(XMFLOAT3));
		XMVECTOR center = XMLoadFloat3(&box.Center), extents = XMLoadFloat3(&box.Extents);
		minV = center - extents;
		maxV = center + extents;
		extent = max(extent, max(XMVectorGetX(maxV), max(XMVectorGetY(maxV), XMVectorGetZ(maxV))));
		XMVECTOR unitsPerTexel = XMVectorSet(extent / SHADOWMAP_DIM, extent / SHADOWMAP_DIM, 1.0f, 1.0f);
		XMStoreFloat3(&min, XMVectorFloor(minV / unitsPerTexel)*unitsPerTexel);
		XMStoreFloat3(&max, XMVectorFloor(maxV / unitsPerTexel)*unitsPerTexel);

		XMMATRIX shadowProj = XMMatrixTranspose(XMMatrixOrthographicOffCenterLH(min.x, max.x, min.y, max.y, min.z, 1.1f*max.z));
		XMStoreFloat4x4(&shadowProjections[thisCascade], shadowProj);

		//Save old viewport for reapplying later
		D3D11_VIEWPORT old;
		UINT numViewports = 1;
		context->RSGetViewports(&numViewports, &old);

		D3D11_VIEWPORT shadowViewport;
		shadowViewport.Width = SHADOWMAP_DIM;
		shadowViewport.Height = SHADOWMAP_DIM;
		shadowViewport.MinDepth = 0.0f;
		shadowViewport.MaxDepth = 1.0f;
		shadowViewport.TopLeftX = 0;
		shadowViewport.TopLeftY = 0;
		context->RSSetViewports(1, &shadowViewport);
		context->RSSetState(shadowRS);
		context->OMSetRenderTargets(0, nullptr, shadowMapDSVs[thisCascade]);

		//Don't need a pixel shader since we just want the depth information
		shadowMapVS->shader->SetShader();
		context->PSSetShader(nullptr, nullptr, 0);
		shadowMapVS->shader->SetMatrix4x4("view", shadowViews[thisCascade]);
		shadowMapVS->shader->SetMatrix4x4("projection", shadowProjections[thisCascade]);
		shadowMapVS->shader->CopyBufferData("PerFrame");

		const UINT stride = sizeof(Mesh::Vertex);
		const UINT offset = 0;
		SceneData::Handle scene = SceneData::Instance().CurrentScene();
		EntityData::Handle entity;
		MeshData::Handle mesh;

		for (size_t i = 0; i < scene->entities.size(); ++i)
		{
			entity = scene->entities[i];
			mesh = entity->mesh;

			shadowMapVS->shader->SetMatrix4x4("model", entity->transform->matrix);
			shadowMapVS->shader->CopyBufferData("PerInstance");
			context->IASetVertexBuffers(0, 1, &mesh->vertexBuffer, &stride, &offset);
			context->IASetIndexBuffer(mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			context->DrawIndexed(mesh->faceCount, 0, 0);
		}

		//Reset to old viewport and default rasterizer state
		context->RSSetViewports(1, &old);
		context->RSSetState(nullptr);

		//Unbind the depth texture
		context->OMSetRenderTargets(0, nullptr, nullptr);
	}
}

void Graphics::RenderSkybox()
{
	ID3D11DepthStencilState* lastDepthState;
	UINT lastStencilRef;
	context->OMGetDepthStencilState(&lastDepthState, &lastStencilRef);

	ID3D11RasterizerState* lastRasterState;
	context->RSGetState(&lastRasterState);

	skyboxVS->shader->SetShader();
	skyboxPS->shader->SetShader();
	skyboxVS->shader->SetFloat3("camPos", Camera::Instance().transform->pos);
	skyboxVS->shader->SetMatrix4x4("view", Camera::Instance().ViewMatrix());
	skyboxVS->shader->SetMatrix4x4("proj", Camera::Instance().ProjectionMatrix());
	skyboxPS->shader->SetShaderResourceView("skyboxTex", skyboxTex->srv);
	skyboxPS->shader->SetSamplerState("mainSampler", skyboxSampler);
	skyboxVS->shader->CopyAllBufferData();
	skyboxPS->shader->CopyAllBufferData();
	context->RSSetState(skyboxRS);
	context->OMSetDepthStencilState(skyboxDS, lastStencilRef);
	context->OMSetRenderTargets(1, &backBuffer, depthStencil);

	const UINT stride = sizeof(Mesh::Vertex);
	const UINT offset = 0;

	context->IASetVertexBuffers(0, 1, &skybox->vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(skybox->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->DrawIndexed(skybox->faceCount, 0, 0);

	//Reset the DS and RS states
	context->OMSetDepthStencilState(lastDepthState, lastStencilRef);
	context->RSSetState(lastRasterState);
}

void Graphics::InitBuffers()
{
	ID3D11Texture2D* backBufferTexture;
	swapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&backBufferTexture);

	device->CreateRenderTargetView(
		backBufferTexture,
		0,
		&backBuffer);
	backBufferTexture->Release();

	D3D11_TEXTURE2D_DESC albedoMapDesc;
	albedoMapDesc.ArraySize = 1;
	albedoMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	albedoMapDesc.CPUAccessFlags = 0;
	albedoMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	albedoMapDesc.MipLevels = 0;
	albedoMapDesc.MiscFlags = 0;
	albedoMapDesc.SampleDesc.Count = 1;
	albedoMapDesc.SampleDesc.Quality = 0;
	albedoMapDesc.Usage = D3D11_USAGE_DEFAULT;
	albedoMapDesc.Height = height;
	albedoMapDesc.Width = width;

	D3D11_TEXTURE2D_DESC pbrAttributeDesc;
	pbrAttributeDesc.ArraySize = 1;
	pbrAttributeDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	pbrAttributeDesc.CPUAccessFlags = 0;
	pbrAttributeDesc.Format = DXGI_FORMAT_R8_UNORM;
	pbrAttributeDesc.MipLevels = 0;
	pbrAttributeDesc.MiscFlags = 0;
	pbrAttributeDesc.SampleDesc.Count = 1;
	pbrAttributeDesc.SampleDesc.Quality = 0;
	pbrAttributeDesc.Usage = D3D11_USAGE_DEFAULT;
	pbrAttributeDesc.Height = height;
	pbrAttributeDesc.Width = width;

	D3D11_TEXTURE2D_DESC normalMapDesc;
	normalMapDesc.ArraySize = 1;
	normalMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	normalMapDesc.CPUAccessFlags = 0;
	normalMapDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	normalMapDesc.MipLevels = 0;
	normalMapDesc.MiscFlags = 0;
	normalMapDesc.SampleDesc.Count = 1;
	normalMapDesc.SampleDesc.Quality = 0;
	normalMapDesc.Usage = D3D11_USAGE_DEFAULT;
	normalMapDesc.Height = height;
	normalMapDesc.Width = width;

	D3D11_TEXTURE2D_DESC posMapDesc;
	posMapDesc.ArraySize = 1;
	posMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	posMapDesc.CPUAccessFlags = 0;
	posMapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	posMapDesc.MipLevels = 1;
	posMapDesc.MiscFlags = 0;
	posMapDesc.SampleDesc.Count = 1;
	posMapDesc.SampleDesc.Quality = 0;
	posMapDesc.Usage = D3D11_USAGE_DEFAULT;
	posMapDesc.Height = height;
	posMapDesc.Width = width;


	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;

	D3D11_TEXTURE2D_DESC shadowMapDesc;
	shadowMapDesc.Width = SHADOWMAP_DIM;
	shadowMapDesc.Height = SHADOWMAP_DIM;
	shadowMapDesc.MipLevels = 0;
	shadowMapDesc.ArraySize = NUM_SHADOW_CASCADES;
	shadowMapDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowMapDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowMapDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowMapDesc.CPUAccessFlags = 0;
	shadowMapDesc.MiscFlags = 0;
	shadowMapDesc.SampleDesc.Count = 1;
	shadowMapDesc.SampleDesc.Quality = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsDesc;
	dsDesc.Flags = 0;
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsDesc.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC depthSRVDesc;
	depthSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	depthSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	depthSRVDesc.Texture2D.MostDetailedMip = 0;
	depthSRVDesc.Texture2D.MipLevels = -1;

	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDS;
	shadowDS.Flags = 0;
	shadowDS.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDS.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	shadowDS.Texture2DArray.MipSlice = 0;
	shadowDS.Texture2DArray.ArraySize = NUM_SHADOW_CASCADES;
	shadowDS.Texture2DArray.FirstArraySlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRV;
	shadowSRV.Format = DXGI_FORMAT_R32_FLOAT;
	shadowSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	shadowSRV.Texture2DArray.MostDetailedMip = 0;
	shadowSRV.Texture2DArray.MipLevels = -1;
	shadowSRV.Texture2DArray.ArraySize = NUM_SHADOW_CASCADES;
	shadowSRV.Texture2DArray.FirstArraySlice = 0;

	D3D11_TEXTURE2D_DESC lightMapDesc;
	lightMapDesc.ArraySize = 1;
	lightMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	lightMapDesc.CPUAccessFlags = 0;
	lightMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	lightMapDesc.MipLevels = 1;
	lightMapDesc.MiscFlags = 0;
	lightMapDesc.SampleDesc.Count = 1;
	lightMapDesc.SampleDesc.Quality = 0;
	lightMapDesc.Usage = D3D11_USAGE_DEFAULT;
	lightMapDesc.Height = height;
	lightMapDesc.Width = width;

	TextureData::Instance().Revoke(albedoMap, true);
	TextureData::Instance().Revoke(roughnessMap, true);
	TextureData::Instance().Revoke(metalMap, true);
	TextureData::Instance().Revoke(cavityMap, true);
	TextureData::Instance().Revoke(normalMap, true);
	TextureData::Instance().Revoke(positionMap, true);
	TextureData::Instance().Revoke(lightMap, true);

	albedoMap = TextureData::Instance().CreateEmpty(albedoMapDesc);
	roughnessMap = TextureData::Instance().CreateEmpty(pbrAttributeDesc);
	metalMap = TextureData::Instance().CreateEmpty(pbrAttributeDesc);
	cavityMap = TextureData::Instance().CreateEmpty(pbrAttributeDesc);
	normalMap = TextureData::Instance().CreateEmpty(normalMapDesc);
	positionMap = TextureData::Instance().CreateEmpty(posMapDesc);
	lightMap = TextureData::Instance().CreateEmpty(lightMapDesc);

	DX_RELEASE(depthStencilTexture);
	DX_RELEASE(depthStencil);
	ID3D11Texture2D* depth;
	device->CreateTexture2D(&depthStencilDesc, nullptr, &depth);
	device->CreateDepthStencilView(depth, &dsDesc, &depthStencil);
	device->CreateShaderResourceView(depth, &depthSRVDesc, &depthStencilTexture);
	depth->Release();

	DX_RELEASE(shadowMapSRV);
	ID3D11Texture2D* smDepth;
	device->CreateTexture2D(&shadowMapDesc, nullptr, &smDepth);
	device->CreateShaderResourceView(smDepth, &shadowSRV, &shadowMapSRV);
	smDepth->Release();

	for (size_t i = 0; i < NUM_SHADOW_CASCADES; i++)
	{
		DX_RELEASE(shadowMapDSVs[i]);

		shadowDS.Flags = 0;
		shadowDS.Format = DXGI_FORMAT_D32_FLOAT;
		shadowDS.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		shadowDS.Texture2DArray.MipSlice = 0;
		shadowDS.Texture2DArray.ArraySize = 1;
		shadowDS.Texture2DArray.FirstArraySlice = (UINT)i;
		device->CreateDepthStencilView(smDepth, &shadowDS, shadowMapDSVs + i);
	}

	context->OMSetRenderTargets(1, &backBuffer, depthStencil);

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);
}

void Graphics::Present()
{
	swapChain->Present(0, 0);
}

HWND Graphics::GetHandle()
{
	return hWnd;
}

ID3D11Device* Graphics::GetDevice()
{
	return device;
}

ID3D11DeviceContext* Graphics::GetContext()
{
	return context;
}

D3D_FEATURE_LEVEL Graphics::GetFeatureLevel()
{
	return dxFeatureLevel;
}

unsigned int Graphics::GetWidth()
{
	return width;
}

unsigned int Graphics::GetHeight()
{
	return height;
}
