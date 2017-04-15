#include "DX11Renderer.h"
#include "Vertex.h"
#include "GraphicsTypes.h"
#include "WICTextureLoader.h"
#include "DirectXMath.h"
#include "DirectXCollision.h"
#include "DDSTextureLoader.h"

using namespace BlackMagic;
using DirectX::XMFLOAT2;
using DirectX::XMFLOAT4;
using DirectX::XMFLOAT4X4;

DX11Renderer::DX11Renderer()
{}

DX11Renderer::~DX11Renderer()
{
	if (_albedoMap)
	{
		delete _albedoMap;
	}

	if (_roughnessMap)
	{
		delete _roughnessMap;
	}

	if (_metalMap)
	{
		delete _metalMap;
	}

	if (_cavityMap)
	{
		delete _cavityMap;
	}

	if (_normalMap)
	{
		delete _normalMap;
	}

	if (_positionMap)
	{
		delete _positionMap;
	}

	if (_lightMap)
	{
		delete _lightMap;
	}
}


ComPtr<ID3D11Device> DX11Renderer::Device() const
{
	return _device;
}

ComPtr<ID3D11DeviceContext> DX11Renderer::Context() const
{
	return _context;
}

D3D_FEATURE_LEVEL DX11Renderer::FeatureLevel() const
{
	return _featureLevel;
}

void DX11Renderer::Clear(XMFLOAT4 color)
{
	FLOAT black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	_context->ClearRenderTargetView(_backBuffer.Get(), reinterpret_cast<const FLOAT*>(&color));
	_context->ClearRenderTargetView(_albedoMap->GetRenderTarget(), black);
	_context->ClearRenderTargetView(_roughnessMap->GetRenderTarget(), black);
	_context->ClearRenderTargetView(_normalMap->GetRenderTarget(), black);
	_context->ClearRenderTargetView(_positionMap->GetRenderTarget(), black);
	_context->ClearRenderTargetView(_metalMap->GetRenderTarget(), black);
	_context->ClearRenderTargetView(_lightMap->GetRenderTarget(), black);
	_context->ClearDepthStencilView(_depthStencil.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	for (size_t i = 0; i < NUM_SHADOW_CASCADES; i++)
	{
		_context->ClearDepthStencilView(_shadowMapDSVs[i].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
}

Sampler DX11Renderer::CreateSampler()
{
	D3D11_SAMPLER_DESC desc = {};
	desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	desc.Filter = D3D11_FILTER_ANISOTROPIC;
	desc.MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY;
	desc.MaxLOD = D3D11_FLOAT32_MAX;
	return CreateSampler(desc);
}

Sampler DX11Renderer::CreateSampler(D3D11_SAMPLER_DESC desc)
{
	ID3D11SamplerState* sampler;
	_device->CreateSamplerState(&desc, &sampler);
	return Sampler(this, sampler);
}

Buffer DX11Renderer::CreateBuffer(Buffer::Type bufferType, void* data, size_t bufferSize)
{
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = bufferType == Buffer::Type::VERTEX_BUFFER ? D3D11_BIND_VERTEX_BUFFER : D3D11_BIND_INDEX_BUFFER;
	desc.ByteWidth = (UINT)bufferSize;
	desc.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA dat = {};
	dat.pSysMem = data;

	ID3D11Buffer* b;
	_device->CreateBuffer(&desc, &dat, &b);
	return Buffer(this, b);
}

void DX11Renderer::ModifyBuffer(Buffer& buffer, Buffer::Type bufferType, void* newData, size_t newBufferSize)
{
	buffer.~Buffer();
	buffer = CreateBuffer(bufferType, newData, newBufferSize);
}

void DX11Renderer::OnResize(UINT width, UINT height)
{
	_width = width;
	_height = height;
	
	/*if (_albedoMap)
	{
		delete _albedoMap;
	}

	if (_roughnessMap)
	{
		delete _roughnessMap;
	}

	if (_metalMap)
	{
		delete _metalMap;
	}

	if (_cavityMap)
	{
		delete _cavityMap;
	}

	if (_normalMap)
	{
		delete _normalMap;
	}

	if (_positionMap)
	{
		delete _positionMap;
	}

	if (_lightMap)
	{
		delete _lightMap;
	}*/

	// Resize the underlying swap chain buffers
	_swapChain->ResizeBuffers(
		1,
		width,
		height,
		DXGI_FORMAT_R10G10B10A2_UNORM,
		0);

	InitBuffers();
}

void DX11Renderer::Present(UINT interval, UINT flags)
{
	_swapChain->Present(interval, flags);
}

HRESULT DX11Renderer::InitDx(HWND window, UINT width, UINT height)
{
	unsigned int deviceFlags = 0;
	_width = width;
	_height = height;

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
	swapDesc.OutputWindow = window;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = true;

	// Result variable for below function calls
	HRESULT hr = S_OK;

	// Attempt to initialize DirectX
	hr = D3D11CreateDeviceAndSwapChain(
		0, // Video adapter (physical GPU) to use, or null for default
		D3D_DRIVER_TYPE_HARDWARE, // We want to use the hardware (GPU)
		0, // Used when doing software rendering
		deviceFlags, // Any special options
		0, // Optional array of possible verisons we want as fallbacks
		0, // The number of fallbacks in the above param
		D3D11_SDK_VERSION, // Current version of the SDK
		&swapDesc, // Address of swap chain options
		_swapChain.ReleaseAndGetAddressOf(), // Pointer to our Swap Chain pointer
		_device.ReleaseAndGetAddressOf(), // Pointer to our Device pointer
		&_featureLevel, // This will hold the actual feature level the app will use
		_context.ReleaseAndGetAddressOf()); // Pointer to our Device Context pointer


	if (!FAILED(hr))
		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return hr;
}

void DX11Renderer::Init(ContentManager* content)
{
	contentManagerAllocator = content->GetAllocator();

	//Initialize the GBuffer and depth buffer
	InitBuffers();

	//Quad vertex buffer for the second pass
	XMFLOAT2 quad[6] = {
		{ -1, 1 },
		{ 1, 1 },
		{ 1, -1 },
		{ -1, 1 },
		{ 1, -1 },
		{ -1, -1 }
	};

	D3D11_BUFFER_DESC vbDesc = { 0 };
	vbDesc.ByteWidth = sizeof(quad);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA vbData = { 0 };
	vbData.pSysMem = quad;

	_device->CreateBuffer(&vbDesc, &vbData, _quad.ReleaseAndGetAddressOf());

	_skybox = content->Load<Mesh>(std::string("/models/skybox.bmmesh"));
	_skyboxTex = content->Load<Cubemap>(std::string("/textures/park_skybox_env.dds"));
	_skyboxRadiance = content->Load<Cubemap>(std::string("/textures/park_skybox_radiance.dds"));
	_skyboxIrradiance = content->Load<Cubemap>(std::string("/textures/park_skybox_irradiance.dds"));
	_cosLookup = content->Load<Texture>(std::string("/textures/cosLUT.png"));

	//Load device-specific shaders
	_lightPassVS = content->Load<VertexShader>(std::string("/shaders/QuadVS.cso"));
	_lightPassPS = content->Load<PixelShader>(std::string("/shaders/LightPassPS.cso"));
	_shadowMapVS = content->Load<VertexShader>(std::string("/shaders/ShadowMapVS.cso"));
	_skyboxVS = content->Load<VertexShader>(std::string("/shaders/SkyboxVS.cso"));
	_skyboxPS = content->Load<PixelShader>(std::string("/shaders/SkyboxPS.cso"));
	_fxaaVS = content->Load<VertexShader>(std::string("/shaders/FXAA_VS.cso"));
	_fxaaPS = content->Load<PixelShader>(std::string("/shaders/FXAA_PS.cso"));
	_projectionPS = content->Load<PixelShader>(std::string("/shaders/ProjectorPS.cso"));
	_mergePS = content->Load<PixelShader>(std::string("/shaders/FinalMerge.cso"));
    //_tonemapPS = content->Load<PixelShader>(std::string("/shaders/ReinhardTonemapping.hlsl"));

	//Set up g-buffer sampler
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	_gBufferSampler = CreateSampler(sampDesc);

	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	_envSampler = CreateSampler(sampDesc);

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
	_shadowSampler = CreateSampler(shadowSampDesc);

	D3D11_RASTERIZER_DESC shadowRSDesc = {};
	shadowRSDesc.CullMode = D3D11_CULL_FRONT;
	shadowRSDesc.FillMode = D3D11_FILL_SOLID;
	shadowRSDesc.DepthClipEnable = true;
	shadowRSDesc.DepthBias = 1000;
	shadowRSDesc.DepthBiasClamp = 0.0f;
	shadowRSDesc.SlopeScaledDepthBias = 1.0f;
	_device->CreateRasterizerState(&shadowRSDesc, _shadowRS.ReleaseAndGetAddressOf());
	//Just need a default sampler, nothing fancy
	_skyboxSampler = CreateSampler(sampDesc);

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
	_device->CreateDepthStencilState(&skyboxDSDesc, _skyboxDS.ReleaseAndGetAddressOf());

	D3D11_RASTERIZER_DESC skyboxRS = {};
	skyboxRS.CullMode = D3D11_CULL_FRONT;
	skyboxRS.FillMode = D3D11_FILL_SOLID;
	_device->CreateRasterizerState(&skyboxRS, _skyboxRS.ReleaseAndGetAddressOf());

	D3D11_SAMPLER_DESC projectorDesc = {};
	projectorDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	projectorDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	projectorDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	projectorDesc.BorderColor[0] = 0.0f;
	projectorDesc.BorderColor[1] = 0.0f;
	projectorDesc.BorderColor[2] = 0.0f;
	projectorDesc.BorderColor[3] = 0.0f;
	projectorDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	projectorDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	projectorDesc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
	projectorDesc.MaxLOD = D3D11_FLOAT32_MAX;
	_projectionSampler = CreateSampler(projectorDesc);

	D3D11_BLEND_DESC projectorBlend = {};
	projectorBlend.AlphaToCoverageEnable = false;
	projectorBlend.IndependentBlendEnable = false;
	projectorBlend.RenderTarget->BlendEnable = true;
	projectorBlend.RenderTarget->SrcBlend = D3D11_BLEND_SRC_ALPHA;
	projectorBlend.RenderTarget->DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	projectorBlend.RenderTarget->BlendOp = D3D11_BLEND_OP_ADD;
	projectorBlend.RenderTarget->SrcBlendAlpha = D3D11_BLEND_ZERO;
	projectorBlend.RenderTarget->DestBlendAlpha = D3D11_BLEND_ONE;
	projectorBlend.RenderTarget->BlendOpAlpha = D3D11_BLEND_OP_ADD;
	projectorBlend.RenderTarget->RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	_device->CreateBlendState(&projectorBlend, _projectionBlend.ReleaseAndGetAddressOf());
}


//TODO: Fix BoundingFrustum generation issues (near&far planes are too close)
void DX11Renderer::Cull(const Camera& cam, const std::vector<Entity*> objects, std::vector<Entity*>& objectsToDraw, bool debugDrawEverything)
{
	/*
	//Collect objects with sphere colliders
	for (auto* ent : gameWorld->each<Transform, Renderable, DirectX::BoundingSphere>())
	{
		if (debugDrawEverything || cam.Frustum().Contains(ent->get<DirectX::BoundingSphere>().get()) != DirectX::ContainmentType::DISJOINT)
		{
			objectsToDraw.push_back(ent);
		}
	}

	//Collect objects with box colliders
	for (auto* ent : gameWorld->each<Transform, Renderable, DirectX::BoundingBox>())
	{
		if (debugDrawEverything || cam.Frustum().Intersects(ent->get<DirectX::BoundingBox>().get()))
		{
			objectsToDraw.push_back(ent);
		}
	}
	*/

	for (auto* ent : objects)
	{
		objectsToDraw.push_back(ent);
	}
}

void DX11Renderer::RenderShadowMaps(const Camera& cam, const std::vector<Entity*>& objects, const DirectionalLight& sceneLight)
{
	using namespace DirectX;

	const float coeff = (CAM_FAR_Z - CAM_NEAR_Z) / NUM_SHADOW_CASCADES;
	auto frustum = cam.Frustum();
	auto vT = cam.ViewMatrix();
	auto dir = DirectX::XMVector3Normalize(XMLoadFloat3(&sceneLight.Direction));
	auto up = XMLoadFloat3(&sceneLight.Up);

	XMFLOAT3 points[8];
	XMVECTOR pointsV[8];
	BoundingBox box;
	for (size_t thisCascade = 0; thisCascade < NUM_SHADOW_CASCADES; thisCascade++)
	{
		float zNear = CAM_NEAR_Z;
		float zFar = (thisCascade + 1)*coeff;

		auto subfrustum = BoundingFrustum(XMMatrixPerspectiveFovLH(CAM_FOV, static_cast<float>(_width) / _height, zNear, zFar));
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

		XMStoreFloat4x4(&_shadowViews[thisCascade], XMMatrixTranspose(shadowView));
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
		XMStoreFloat4x4(&_shadowProjections[thisCascade], shadowProj);

		//Save old viewport for reapplying later
		D3D11_VIEWPORT old;
		UINT numViewports = 1;
		_context->RSGetViewports(&numViewports, &old);

		D3D11_VIEWPORT shadowViewport;
		shadowViewport.Width = SHADOWMAP_DIM;
		shadowViewport.Height = SHADOWMAP_DIM;
		shadowViewport.MinDepth = 0.0f;
		shadowViewport.MaxDepth = 1.0f;
		shadowViewport.TopLeftX = 0;
		shadowViewport.TopLeftY = 0;
		_context->RSSetViewports(1, &shadowViewport);
		_context->RSSetState(_shadowRS.Get());
		_context->OMSetRenderTargets(0, nullptr, _shadowMapDSVs[thisCascade].Get());

		//Don't need a pixel shader since we just want the depth information
		_shadowMapVS->SetShader();
		_context->PSSetShader(nullptr, nullptr, 0);
		_shadowMapVS->SetMatrix4x4("view", _shadowViews[thisCascade]);
		_shadowMapVS->SetMatrix4x4("projection", _shadowProjections[thisCascade]);
		_shadowMapVS->CopyBufferData("PerFrame");

		const UINT stride = sizeof(Vertex);
		const UINT offset = 0;
		for (auto* o : objects)
		{
			auto& mesh = o->AsRenderable()->_mesh;
			auto vBuf = mesh->VertexBuffer().As<BufferHandle>();
			auto iBuf = mesh->IndexBuffer().As<BufferHandle>();
			_shadowMapVS->SetMatrix4x4("model", *o->GetTransform().Matrix());
			_shadowMapVS->CopyBufferData("PerInstance");
			_context->IASetVertexBuffers(0, 1, &vBuf, &stride, &offset);
			_context->IASetIndexBuffer(iBuf, DXGI_FORMAT_R32_UINT, 0);
			_context->DrawIndexed((UINT)mesh->IndexCount(), 0, 0);
		}

		//Reset to old viewport and default rasterizer state
		_context->RSSetViewports(1, &old);
		_context->RSSetState(nullptr);

		//Unbind the depth texture
		_context->OMSetRenderTargets(0, nullptr, nullptr);
	}
}

/*
void DX11Renderer::RenderProjectors(const std::vector<Projector>& projectors)
{
	const UINT stride = sizeof(XMFLOAT2);
	const UINT offset = 0;

	_context->OMSetRenderTargets(1, &_backBuffer, nullptr);

	ID3D11BlendState* oldState;
	float oldBlendColor[4], blendColor[4] = { 0,0,0,0 };
	UINT oldSampleMask, sampleMask = 0xFFFFFFFF;
	_context->OMGetBlendState(&oldState, oldBlendColor, &oldSampleMask);
	_context->OMSetBlendState(_projectionBlend, blendColor, sampleMask);

	_lightPassVS->SetShader();
	_projectionPS->SetShader();
	_projectionPS->SetShaderResourceView("positionMap", _positionMap->GetGraphicsTexture().GetAs<ID3D11ShaderResourceView*>());
	_projectionPS->SetSamplerState("mainSampler", _projectionSampler.get());

	for(const auto& p : projectors)
	{
		_projectionPS->SetShaderResourceView("projectedTex", p.Texture().GetAs<ID3D11ShaderResourceView*>());
		_projectionPS->SetMatrix4x4("vp", p.Matrix());
		_projectionPS->CopyAllBufferData();
		_context->IASetVertexBuffers(0, 1, &_quad, &stride, &offset);
		_context->Draw(6, 0);
	}

	ID3D11ShaderResourceView* zeros[1] = { 0 };
	_context->PSSetShaderResources(1, 1, zeros);
	_context->OMSetBlendState(oldState, oldBlendColor, oldSampleMask);
	_context->OMSetRenderTargets(1, &_backBuffer, _depthStencil);
}
*/

void DX11Renderer::Render(const Camera& cam, const std::vector<Entity*>& objects, const DirectionalLight& sceneLight)
{
	static UINT stride = sizeof(Vertex);
	static UINT quadStride = sizeof(XMFLOAT2);
	static UINT offset = 0;
	ID3D11ShaderResourceView* srvs[32] = { 0 };

	const Material* currentMaterial = nullptr;

	//RenderShadowMaps(cam, objects, sceneLight);
	

	//TODO: Sort renderables by material and texture to minimize state switches
	ID3D11RenderTargetView* rts[] = {
		_albedoMap->GetRenderTarget(),
		_positionMap->GetRenderTarget(),
		_roughnessMap->GetRenderTarget(),
		_normalMap->GetRenderTarget(),
		_cavityMap->GetRenderTarget(),
		_metalMap->GetRenderTarget()
	};
	_context->OMSetRenderTargets(sizeof(rts)/sizeof(ID3D11RenderTargetView*), rts, _depthStencil.Get());

	auto view = cam.ViewMatrix();
	auto proj = cam.ProjectionMatrix();

	//Load object attributes into the g-buffer (geometry pass)
	for(auto* object : objects)
	{
		auto renderable = object->AsRenderable();

		auto m = object->GetTransform().Matrix();

		//Update per-object constant buffer
		renderable->_material.SetResource("world", Material::VS, sizeof(XMFLOAT4X4),
			reinterpret_cast<void*>(m));
		renderable->_material.SetResource("view", Material::VS, sizeof(XMFLOAT4X4),
			reinterpret_cast<void*>(&view));
		renderable->_material.SetResource("projection", Material::VS, sizeof(XMFLOAT4X4),
			reinterpret_cast<void*>(&proj));

		//Upload buffers and draw
		bool freshMaterial = true;
		if (currentMaterial)
			freshMaterial = renderable->_material != *currentMaterial;

		renderable->_material.Use(freshMaterial);
		currentMaterial = &renderable->_material;

		auto vBuf = renderable->_mesh->VertexBuffer().As<BufferHandle>();
		_context->IASetVertexBuffers(0, 1, &vBuf, &stride, &offset);
		_context->IASetIndexBuffer(renderable->_mesh->IndexBuffer().As<BufferHandle>(), DXGI_FORMAT_R32_UINT, 0);
		_context->DrawIndexed(static_cast<UINT>(renderable->_mesh->IndexCount()), 0, 0);
	}
	
	auto lightMap = _lightMap->GetRenderTarget();
	_context->OMSetRenderTargets(1, &lightMap, nullptr);
	auto cPos = cam.Position();
	//size_t padding = (16 - (sizeof(DirectionalLight) % 16))*(lights.size() - 1);

	_lightPassVS->SetShader();
	_lightPassPS->SetShader();
	_lightPassPS->SetFloat3("cameraPosition", cPos);
	_lightPassPS->SetData("sceneLight", &sceneLight, sizeof(DirectionalLight));
	_lightPassPS->SetData("lightView", &_shadowViews[0], sizeof(XMFLOAT4X4)*NUM_SHADOW_CASCADES);
	_lightPassPS->SetData("lightProjection", &_shadowProjections[0], sizeof(XMFLOAT4X4)*NUM_SHADOW_CASCADES);
	_lightPassPS->SetSamplerState("mainSampler", _gBufferSampler.As<SamplerHandle>());
	_lightPassPS->SetSamplerState("shadowSampler", _shadowSampler.As<SamplerHandle>());
	_lightPassPS->SetSamplerState("envSampler", _envSampler.As<SamplerHandle>());
	_lightPassPS->SetShaderResourceView("albedoMap", _albedoMap->GetShaderResource());
	_lightPassPS->SetShaderResourceView("roughnessMap", _roughnessMap->GetShaderResource());
	_lightPassPS->SetShaderResourceView("positionMap", _positionMap->GetShaderResource());
	_lightPassPS->SetShaderResourceView("normalMap", _normalMap->GetShaderResource());
	_lightPassPS->SetShaderResourceView("metalnessMap", _metalMap->GetShaderResource());
	_lightPassPS->SetShaderResourceView("cavityMap", _cavityMap->GetShaderResource());
	_lightPassPS->SetShaderResourceView("shadowMap", _shadowMapSRV.Get());
	_lightPassPS->SetShaderResourceView("depth", _depthStencilTexture.Get());
	_lightPassPS->SetShaderResourceView("skyboxRadianceMap", _skyboxRadiance->GetShaderResource());
	_lightPassPS->SetShaderResourceView("skyboxIrradianceMap", _skyboxIrradiance->GetShaderResource());
	_lightPassPS->SetShaderResourceView("cosLookup", _cosLookup->GetShaderResource());
	_lightPassPS->CopyAllBufferData();

	_context->IASetVertexBuffers(0, 1, _quad.GetAddressOf(), &quadStride, &offset);
	_context->Draw(6, 0);
	_context->PSSetShaderResources(0, _lightPassPS->GetShaderResourceViewCount()+1, srvs);
	
	ID3D11BlendState* oldBlendState = nullptr;
	float blendFac[4];
	UINT sampleMask = 0;
	_context->OMSetRenderTargets(1, _backBuffer.GetAddressOf(), nullptr);
	_context->OMGetBlendState(&oldBlendState, blendFac, &sampleMask);
	_context->OMSetBlendState(_projectionBlend.Get(), blendFac, sampleMask);
	_mergePS->SetShader();
	_mergePS->SetSamplerState("mainSampler", _gBufferSampler.As<SamplerHandle>());
	_mergePS->SetShaderResourceView("lightMap", _lightMap->GetShaderResource());
	_mergePS->CopyAllBufferData();
	_context->Draw(6, 0);
	RenderSkybox(cam);
	_context->OMSetBlendState(oldBlendState, blendFac, sampleMask);
	_context->PSSetShaderResources(0, _mergePS->GetShaderResourceViewCount()+1, srvs);

	/*_fxaaVS->SetShader();
	_fxaaPS->SetShader();
	_fxaaPS->SetInt("width", _width);
	_fxaaPS->SetInt("height", _height);
	_fxaaPS->SetSamplerState("mainSampler", _gBufferSampler.As<SamplerHandle>());
	_fxaaPS->SetShaderResourceView("inputMap", _lightMap->GetShaderResource());
	_fxaaPS->CopyAllBufferData();
	_context->Draw(6, 0);*/
}

void DX11Renderer::RenderSkybox(const Camera& cam)
{
	ID3D11DepthStencilState* lastDepthState;
	UINT lastStencilRef;
	_context->OMGetDepthStencilState(&lastDepthState, &lastStencilRef);

	ID3D11RasterizerState* lastRasterState;
	_context->RSGetState(&lastRasterState);

	_skyboxVS->SetShader();
	_skyboxPS->SetShader();
	_skyboxVS->SetFloat3("camPos", cam.Position());
	_skyboxVS->SetMatrix4x4("view", cam.ViewMatrix());
	_skyboxVS->SetMatrix4x4("proj", cam.ProjectionMatrix());
	_skyboxPS->SetShaderResourceView("skyboxTex", _skyboxTex->GetShaderResource());
	_skyboxPS->SetSamplerState("mainSampler", _skyboxSampler.As<SamplerHandle>());
	_skyboxVS->CopyAllBufferData();
	_skyboxPS->CopyAllBufferData();
	_context->RSSetState(_skyboxRS.Get());
	_context->OMSetDepthStencilState(_skyboxDS.Get(), lastStencilRef);
	_context->OMSetRenderTargets(1, _backBuffer.GetAddressOf(), _depthStencil.Get());

	const UINT stride = sizeof(Vertex);
	const UINT offset = 0;
	auto vBuf = _skybox->VertexBuffer().As<BufferHandle>();
	_context->IASetVertexBuffers(0, 1, &vBuf, &stride, &offset);
	_context->IASetIndexBuffer(_skybox->IndexBuffer().As<BufferHandle>(), DXGI_FORMAT_R32_UINT, 0);
	_context->DrawIndexed((UINT)_skybox->IndexCount(), 0, 0);

	//Reset the DS and RS states
	_context->OMSetDepthStencilState(lastDepthState, lastStencilRef);
	_context->RSSetState(lastRasterState);
}

Texture DX11Renderer::CreateTexture(BlackMagic::byte* data, size_t size, Texture::Type type, Texture::Usage usage)
{
	ID3D11Resource* tex = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;
	HRESULT result;
	switch(type)
	{
	case Texture::Type::FLAT_2D:
		result = DirectX::CreateWICTextureFromMemory(_device.Get(), data, size, &tex, &srv);
		break;
	
	case Texture::Type::CUBEMAP:
		result = DirectX::CreateDDSTextureFromMemory(_device.Get(), data, size, &tex, &srv);
		break;
	}

	if (!(usage & Texture::Usage::READ))
	{
		srv->Release();
		srv = nullptr;
	}

	ID3D11RenderTargetView* rtv = nullptr;
	if (usage & Texture::Usage::WRITE)
		result = _device->CreateRenderTargetView(tex, nullptr, &rtv);

	return Texture(this, tex, srv, rtv);
}

DXGI_FORMAT TranslateTextureFormat(Texture::Format f)
{
	switch (f)
	{
		case Texture::Format::R32G32B32A32_FLOAT:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case Texture::Format::R32G32B32A32_UINT:
			return DXGI_FORMAT_R32G32B32A32_UINT;
		case Texture::Format::R32G32B32A32_SINT:
			return DXGI_FORMAT_R32G32B32A32_SINT;
		case Texture::Format::R32G32B32_FLOAT:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case Texture::Format::R32G32B32_UINT:
			return DXGI_FORMAT_R32G32B32_UINT;
		case Texture::Format::R32G32B32_SINT:
			return DXGI_FORMAT_R32G32B32_SINT;
		case Texture::Format::R16G16B16A16_FLOAT:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case Texture::Format::R16G16B16A16_UNORM:
			return DXGI_FORMAT_R16G16B16A16_UNORM;
		case Texture::Format::R16G16B16A16_UINT:
			return DXGI_FORMAT_R16G16B16A16_UINT;
		case Texture::Format::R16G16B16A16_SNORM:
			return DXGI_FORMAT_R16G16B16A16_SNORM;
		case Texture::Format::R16G16B16A16_SINT:
			return DXGI_FORMAT_R16G16B16A16_SINT;
		case Texture::Format::R32G32_FLOAT:
			return DXGI_FORMAT_R32G32_FLOAT;
		case Texture::Format::R32G32_UINT:
			return DXGI_FORMAT_R32G32_UINT;
		case Texture::Format::R32G32_SINT:
			return DXGI_FORMAT_R32G32_SINT;
		case Texture::Format::R10G10B10A2_UNORM:
			return DXGI_FORMAT_R10G10B10A2_UNORM;
		case Texture::Format::R10G10B10A2_UINT:
			return DXGI_FORMAT_R10G10B10A2_UINT;
		case Texture::Format::R11G11B10_FLOAT:
			return DXGI_FORMAT_R11G11B10_FLOAT;
		case Texture::Format::R8G8B8A8_UNORM:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case Texture::Format::R8G8B8A8_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case Texture::Format::R8G8B8A8_UINT:
			return DXGI_FORMAT_R8G8B8A8_UINT;
		case Texture::Format::R8G8B8A8_SNORM:
			return DXGI_FORMAT_R8G8B8A8_SNORM;
		case Texture::Format::R8G8B8A8_SINT:
			return DXGI_FORMAT_R8G8B8A8_SINT;
		case Texture::Format::R16G16_FLOAT:
			return DXGI_FORMAT_R16G16_FLOAT;
		case Texture::Format::R16G16_UNORM:
			return DXGI_FORMAT_R16G16_UNORM;
		case Texture::Format::R16G16_UINT:
			return DXGI_FORMAT_R16G16_UINT;
		case Texture::Format::R16G16_SNORM:
			return DXGI_FORMAT_R16G16_SNORM;
		case Texture::Format::R16G16_SINT:
			return DXGI_FORMAT_R16G16_SINT;
		case Texture::Format::R32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;
		case Texture::Format::R32_UINT:
			return DXGI_FORMAT_R32_UINT;
		case Texture::Format::R32_SINT:
			return DXGI_FORMAT_R32_SINT;
		case Texture::Format::R8G8_UNORM:
			return DXGI_FORMAT_R8G8_UNORM;
		case Texture::Format::R8G8_UINT:
			return DXGI_FORMAT_R8G8_UINT;
		case Texture::Format::R8G8_SNORM:
			return DXGI_FORMAT_R8G8_SNORM;
		case Texture::Format::R8G8_SINT:
			return DXGI_FORMAT_R8G8_SINT;
		case Texture::Format::R16_FLOAT:
			return DXGI_FORMAT_R16_FLOAT;
		case Texture::Format::R16_UNORM:
			return DXGI_FORMAT_R16_UNORM;
		case Texture::Format::R16_UINT:
			return DXGI_FORMAT_R16_UINT;
		case Texture::Format::R16_SNORM:
			return DXGI_FORMAT_R16_SNORM;
		case Texture::Format::R16_SINT:
			return DXGI_FORMAT_R16_SINT;
		case Texture::Format::R8_UNORM:
			return DXGI_FORMAT_R8_UNORM;
		case Texture::Format::R8_UINT:
			return DXGI_FORMAT_R8_UINT;
		case Texture::Format::R8_SNORM:
			return DXGI_FORMAT_R8_SNORM;
		case Texture::Format::R8_SINT:
			return DXGI_FORMAT_R8_SINT;
		case Texture::Format::A8_UNORM:
			return DXGI_FORMAT_A8_UNORM;
		case Texture::Format::R1_UNORM:
			return DXGI_FORMAT_R1_UNORM;
		default:
			assert(false, "Invalid texture format provided to TranslateTextureFormat\n");
			return DXGI_FORMAT_UNKNOWN;
	}
}

unsigned int BytesPerPixel(Texture::Format f)
{
	switch (f)
	{
		case Texture::Format::R32G32B32A32_FLOAT:
		case Texture::Format::R32G32B32A32_UINT:
		case Texture::Format::R32G32B32A32_SINT:
			return 16;

		case Texture::Format::R32G32B32_FLOAT:
		case Texture::Format::R32G32B32_UINT:
		case Texture::Format::R32G32B32_SINT:
			return 12;

		case Texture::Format::R16G16B16A16_FLOAT:
		case Texture::Format::R16G16B16A16_UNORM:
		case Texture::Format::R16G16B16A16_UINT:
		case Texture::Format::R16G16B16A16_SNORM:
		case Texture::Format::R16G16B16A16_SINT:
		case Texture::Format::R32G32_FLOAT:
		case Texture::Format::R32G32_UINT:
		case Texture::Format::R32G32_SINT:
			return 8;

		case Texture::Format::R10G10B10A2_UNORM:
		case Texture::Format::R10G10B10A2_UINT:
		case Texture::Format::R11G11B10_FLOAT:
		case Texture::Format::R16G16_FLOAT:
		case Texture::Format::R16G16_UNORM:
		case Texture::Format::R16G16_UINT:
		case Texture::Format::R16G16_SNORM:
		case Texture::Format::R16G16_SINT:
		case Texture::Format::R32_FLOAT:
		case Texture::Format::R32_UINT:
		case Texture::Format::R32_SINT:
		case Texture::Format::R8G8B8A8_UNORM:
		case Texture::Format::R8G8B8A8_UNORM_SRGB:
		case Texture::Format::R8G8B8A8_UINT:
		case Texture::Format::R8G8B8A8_SNORM:
		case Texture::Format::R8G8B8A8_SINT:
			return 4;
		
		case Texture::Format::R8G8_UNORM:
		case Texture::Format::R8G8_UINT:
		case Texture::Format::R8G8_SNORM:
		case Texture::Format::R8G8_SINT:
		case Texture::Format::R16_FLOAT:
		case Texture::Format::R16_UNORM:
		case Texture::Format::R16_UINT:
		case Texture::Format::R16_SNORM:
		case Texture::Format::R16_SINT:
			return 2;

		case Texture::Format::R8_UNORM:
		case Texture::Format::R8_UINT:
		case Texture::Format::R8_SNORM:
		case Texture::Format::R8_SINT:
		case Texture::Format::A8_UNORM:
		case Texture::Format::R1_UNORM:
			return 1;
		default:
			assert(false, "Invalid texture format provided to BytesPerPixel\n");
			return 0;
	}
}

Texture DX11Renderer::CreateTexture(const TextureDesc& desc)
{
	auto bpp = BytesPerPixel(desc.Format);

	switch (desc.Type)
	{
		case Texture::FLAT_1D:
		{
			D3D11_TEXTURE1D_DESC d3dDesc = { 0 };
			d3dDesc.Width = desc.Width;
			d3dDesc.ArraySize = 1;
			d3dDesc.BindFlags = (desc.GPUUsage & Texture::Usage::READ ? D3D11_BIND_SHADER_RESOURCE : 0)
				| (desc.GPUUsage & Texture::Usage::WRITE ? D3D11_BIND_RENDER_TARGET : 0);
			d3dDesc.Usage = D3D11_USAGE_DEFAULT;
			d3dDesc.Format = TranslateTextureFormat(desc.Format);
			d3dDesc.MiscFlags = (d3dDesc.BindFlags == (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET) ? 
				D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);

			D3D11_SUBRESOURCE_DATA subDat = { 0 };
			subDat.pSysMem = desc.InitialData;

			ID3D11Texture1D* tex;
			ID3D11ShaderResourceView* srv = nullptr;
			ID3D11RenderTargetView* rtv = nullptr;
			_device->CreateTexture1D(&d3dDesc, nullptr, &tex);
			_context->UpdateSubresource(tex, 0, nullptr, desc.InitialData, bpp * desc.Width, 0);

			if (desc.GPUUsage & Texture::Usage::READ)
			{
				_device->CreateShaderResourceView(tex, nullptr, &srv);

				if (d3dDesc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS)
					_context->GenerateMips(srv);
			}
			if (desc.GPUUsage & Texture::Usage::WRITE)
				_device->CreateRenderTargetView(tex, nullptr, &rtv);

			return Texture(this, tex, srv, rtv);
		}

		case Texture::FLAT_2D:
		case Texture::CUBEMAP:
		{
			D3D11_TEXTURE2D_DESC d3dDesc = { 0 };
			d3dDesc.Height = desc.Height;
			d3dDesc.Width = desc.Width;
			d3dDesc.BindFlags = (desc.GPUUsage & Texture::Usage::READ ? D3D11_BIND_SHADER_RESOURCE : 0)
				| (desc.GPUUsage & Texture::Usage::WRITE ? D3D11_BIND_RENDER_TARGET : 0);
			d3dDesc.Usage = D3D11_USAGE_DEFAULT;
			d3dDesc.ArraySize = (desc.Type == Texture::CUBEMAP ? 6 : 1);
			d3dDesc.Format = TranslateTextureFormat(desc.Format);
			d3dDesc.MiscFlags = (desc.Type == Texture::CUBEMAP ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0) 
				| (d3dDesc.BindFlags == (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET) ?
					D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);
			d3dDesc.SampleDesc.Count = 1;
			d3dDesc.SampleDesc.Quality = 0;

			D3D11_SUBRESOURCE_DATA subDat = { 0 };
			subDat.pSysMem = desc.InitialData;
			subDat.SysMemPitch = bpp * desc.Width;

			ID3D11Texture2D* tex;
			ID3D11ShaderResourceView* srv = nullptr;
			ID3D11RenderTargetView* rtv = nullptr;
			_device->CreateTexture2D(&d3dDesc, &subDat, &tex);
			//_context->UpdateSubresource(tex, 0, nullptr, desc.InitialData, bpp * desc.Width, bpp * desc.Width * desc.Height);

			if (desc.GPUUsage & Texture::Usage::READ)
			{
				_device->CreateShaderResourceView(tex, nullptr, &srv);

				if (d3dDesc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS)
					_context->GenerateMips(srv);
			}
			if (desc.GPUUsage & Texture::Usage::WRITE)
				_device->CreateRenderTargetView(tex, nullptr, &rtv);

			return Texture(this, tex, srv, rtv);
		}
		
		case Texture::FLAT_3D:
		{
			D3D11_TEXTURE3D_DESC d3dDesc = { 0 };
			d3dDesc.Height = desc.Height;
			d3dDesc.Width = desc.Width;
			d3dDesc.BindFlags = (desc.GPUUsage & Texture::Usage::READ ? D3D11_BIND_SHADER_RESOURCE : 0)
				| (desc.GPUUsage & Texture::Usage::WRITE ? D3D11_BIND_RENDER_TARGET : 0);
			d3dDesc.Usage = D3D11_USAGE_DEFAULT;
			d3dDesc.Depth = desc.Depth;
			d3dDesc.Format = TranslateTextureFormat(desc.Format);
			d3dDesc.MiscFlags = (d3dDesc.BindFlags == (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET) ?
				D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);

			D3D11_SUBRESOURCE_DATA subDat = { 0 };
			subDat.pSysMem = desc.InitialData;
			subDat.SysMemPitch = bpp * desc.Width;
			subDat.SysMemSlicePitch = bpp * desc.Width * desc.Height;

			ID3D11Texture3D* tex;
			ID3D11ShaderResourceView* srv = nullptr;
			ID3D11RenderTargetView* rtv = nullptr;
			_device->CreateTexture3D(&d3dDesc, nullptr, &tex);
			_context->UpdateSubresource(tex, 0, nullptr, desc.InitialData, bpp * desc.Width, bpp * desc.Width * desc.Height);

			if (desc.GPUUsage & Texture::Usage::READ)
			{
				_device->CreateShaderResourceView(tex, nullptr, &srv);

				if (d3dDesc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS)
					_context->GenerateMips(srv);
			}
			if (desc.GPUUsage & Texture::Usage::WRITE)
				_device->CreateRenderTargetView(tex, nullptr, &rtv);

			return Texture(this, tex, srv, rtv);
		}
		
		default:
			assert(false, "Invalid texture type provided to DX11Renderer::CreateTexture\n");
			return Texture(nullptr, nullptr, nullptr, nullptr);
		
	}
}

void DX11Renderer::AddResourceRef(void* resource)
{
	if (resource)
		reinterpret_cast<IUnknown*>(resource)->AddRef();
}

void DX11Renderer::ReleaseResource(void* resource)
{
	if (resource)
		reinterpret_cast<IUnknown*>(resource)->Release();
}

void DX11Renderer::InitBuffers()
{
	// The above function created the back buffer render target
	// for us: but we need a reference to it
	ID3D11Texture2D* backBufferTexture;
	_swapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&backBufferTexture);

	// Now that we have the texture, create a render target view
	// for the back buffer so we can render into it.  Then release
	// our local reference to the texture, since we have the view.
	_device->CreateRenderTargetView(
		backBufferTexture,
		0,
		_backBuffer.ReleaseAndGetAddressOf());
	backBufferTexture->Release();

	D3D11_TEXTURE2D_DESC albedoMapDesc;
	albedoMapDesc.ArraySize = 1;
	albedoMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	albedoMapDesc.CPUAccessFlags = 0;
	albedoMapDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
	albedoMapDesc.MipLevels = 0;
	albedoMapDesc.MiscFlags = 0;
	albedoMapDesc.SampleDesc.Count = 1;
	albedoMapDesc.SampleDesc.Quality = 0;
	albedoMapDesc.Usage = D3D11_USAGE_DEFAULT;
	albedoMapDesc.Height = _height;
	albedoMapDesc.Width = _width;

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
	pbrAttributeDesc.Height = _height;
	pbrAttributeDesc.Width = _width;

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
	normalMapDesc.Height = _height;
	normalMapDesc.Width = _width;

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
	posMapDesc.Height = _height;
	posMapDesc.Width = _width;


	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = _width;
	depthStencilDesc.Height = _height;
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
	lightMapDesc.Height = _height;
	lightMapDesc.Width = _width;

	delete _albedoMap;
	delete _roughnessMap;
	delete _metalMap;
	delete _cavityMap;
	delete _normalMap;
	delete _positionMap;
	delete _lightMap;

	_albedoMap = createEmptyTexture(albedoMapDesc);
	_roughnessMap = createEmptyTexture(pbrAttributeDesc);
	_metalMap = createEmptyTexture(pbrAttributeDesc);
	_cavityMap = createEmptyTexture(pbrAttributeDesc);
	_normalMap = createEmptyTexture(normalMapDesc);
	_positionMap = createEmptyTexture(posMapDesc);
	_lightMap = createEmptyTexture(lightMapDesc);

	ID3D11Texture2D* depth;
	_device->CreateTexture2D(&depthStencilDesc, nullptr, &depth);
	_device->CreateDepthStencilView(depth, &dsDesc, _depthStencil.ReleaseAndGetAddressOf());
	_device->CreateShaderResourceView(depth, &depthSRVDesc, _depthStencilTexture.ReleaseAndGetAddressOf());
	depth->Release();

	ID3D11Texture2D* smDepth;
	_device->CreateTexture2D(&shadowMapDesc, nullptr, &smDepth);
	_device->CreateShaderResourceView(smDepth, &shadowSRV, _shadowMapSRV.ReleaseAndGetAddressOf());
	smDepth->Release();

	for (size_t i = 0; i < NUM_SHADOW_CASCADES; i++)
	{
		shadowDS.Flags = 0;
		shadowDS.Format = DXGI_FORMAT_D32_FLOAT;
		shadowDS.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		shadowDS.Texture2DArray.MipSlice = 0;
		shadowDS.Texture2DArray.ArraySize = 1;
		shadowDS.Texture2DArray.FirstArraySlice = (UINT)i;
		_device->CreateDepthStencilView(smDepth, &shadowDS, _shadowMapDSVs[i].ReleaseAndGetAddressOf());
	}

	// Bind the views to the pipeline, so rendering properly 
	// uses their underlying textures
	_context->OMSetRenderTargets(1, _backBuffer.GetAddressOf(), _depthStencil.Get());

	// Lastly, set up a viewport so we render into
	// to correct portion of the window
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(_width);
	viewport.Height = static_cast<float>(_height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	_context->RSSetViewports(1, &viewport);

}

Texture* DX11Renderer::createEmptyTexture(D3D11_TEXTURE2D_DESC& desc)
{
	ID3D11Texture2D* tex;
	ID3D11RenderTargetView* rtv;
	ID3D11ShaderResourceView* srv;

	_device->CreateTexture2D(&desc, nullptr, &tex);
	_device->CreateRenderTargetView(tex, nullptr, &rtv);
	_device->CreateShaderResourceView(tex, nullptr, &srv);

	return new Texture(this, tex, srv, rtv);
}


