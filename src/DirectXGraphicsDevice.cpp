#include "DirectXGraphicsDevice.h"
#include "Vertex.h"
#include "GraphicsTypes.h"
#include "WICTextureLoader.h"
#include "DirectXMath.h"
#include "DirectXCollision.h"

using namespace BlackMagic;
using DirectX::XMFLOAT2;
using DirectX::XMFLOAT4;
using DirectX::XMFLOAT4X4;

DirectXGraphicsDevice::DirectXGraphicsDevice()
{}

DirectXGraphicsDevice::~DirectXGraphicsDevice()
{
	if (_device)
	{
		_device->Release();
	}

	if (_context)
	{
		_context->Release();
	}

	if (_swapChain)
	{
		_swapChain->Release();
	}

	if (_backBuffer)
	{
		_backBuffer->Release();
	}

	if (_depthStencil)
	{
		_depthStencil->Release();
		_depthStencilTexture->Release();
	}

	if (_diffuseMap)
	{
		delete _diffuseMap;
	}

	if (_specularMap)
	{
		delete _specularMap;
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

	if (_quad)
	{
		_quad->Release();
	}

	for (size_t i = 0; i < NUM_SHADOW_CASCADES; i++)
	{
		if (_shadowMapDSVs[i])
			_shadowMapDSVs[i]->Release();
	}
	if (_shadowMapSRV)
		_shadowMapSRV->Release();

	if (_shadowRS)
	{
		_shadowRS->Release();
	}

	if (_skyboxDS)
	{
		_skyboxDS->Release();
	}

	if (_skyboxRS)
	{
		_skyboxRS->Release();
	}

	if (_projectionBlend)
	{
		_projectionBlend->Release();
	}
#if defined(DEBUG) || defined(_DEBUG)
	ID3D11Debug* debugDevice = nullptr;
	auto r = _device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debugDevice));
	r = debugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	debugDevice->Release();
#endif

}


ID3D11Device* DirectXGraphicsDevice::Device() const
{
	return _device;
}

ID3D11DeviceContext* DirectXGraphicsDevice::Context() const
{
	return _context;
}

D3D_FEATURE_LEVEL DirectXGraphicsDevice::FeatureLevel() const
{
	return _featureLevel;
}

void DirectXGraphicsDevice::Clear(XMFLOAT4 color)
{
	FLOAT black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	_context->ClearRenderTargetView(_backBuffer, reinterpret_cast<const FLOAT*>(&color));
	_context->ClearRenderTargetView(_diffuseMap->GetGraphicsRenderTarget().GetAs<ID3D11RenderTargetView*>(), reinterpret_cast<const FLOAT*>(&color));
	_context->ClearRenderTargetView(_specularMap->GetGraphicsRenderTarget().GetAs<ID3D11RenderTargetView*>(), black);
	_context->ClearRenderTargetView(_normalMap->GetGraphicsRenderTarget().GetAs<ID3D11RenderTargetView*>(), black);
	_context->ClearRenderTargetView(_positionMap->GetGraphicsRenderTarget().GetAs<ID3D11RenderTargetView*>(), black);
	_context->ClearRenderTargetView(_lightMap->GetGraphicsRenderTarget().GetAs<ID3D11RenderTargetView*>(), black);
	_context->ClearDepthStencilView(_depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	for (size_t i = 0; i < NUM_SHADOW_CASCADES; i++)
	{
		_context->ClearDepthStencilView(_shadowMapDSVs[i], D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
}

std::shared_ptr<ID3D11SamplerState> DirectXGraphicsDevice::CreateSamplerState(D3D11_SAMPLER_DESC& desc)
{
	ID3D11SamplerState* tempSampler;
	_device->CreateSamplerState(&desc, &tempSampler);
	std::shared_ptr<ID3D11SamplerState> sampler{ tempSampler, [=](ID3D11SamplerState* sampler) {
		sampler->Release();
	} };

	return sampler;
}

GraphicsBuffer DirectXGraphicsDevice::CreateBuffer(GraphicsBuffer::BufferType bufferType, void* data, size_t bufferSize)
{
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = bufferType == GraphicsBuffer::BufferType::VERTEX_BUFFER ? D3D11_BIND_VERTEX_BUFFER : D3D11_BIND_INDEX_BUFFER;
	desc.ByteWidth = bufferSize;
	desc.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA dat = {};
	dat.pSysMem = data;

	GraphicsBuffer buffer;
	ID3D11Buffer* b;
	_device->CreateBuffer(&desc, &dat, &b);
	buffer.buffer = b;
	return buffer;
}

void DirectXGraphicsDevice::ModifyBuffer(GraphicsBuffer& buffer, GraphicsBuffer::BufferType bufferType, void* newData, size_t newBufferSize)
{
	buffer.GetAs<ID3D11Buffer*>()->Release();
	buffer = CreateBuffer(bufferType, newData, newBufferSize);
}

void BlackMagic::DirectXGraphicsDevice::CleanupBuffer(GraphicsBuffer buffer)
{
	if(buffer.buffer) buffer.GetAs<ID3D11Buffer*>()->Release();
}

void DirectXGraphicsDevice::OnResize(UINT width, UINT height)
{
	_width = width;
	_height = height;
	
	// Release existing DirectX views and buffers
	if (_depthStencil)
	{
		_depthStencil->Release();
		_depthStencilTexture->Release();
	}
	if (_backBuffer)
	{
		_backBuffer->Release();
	}
	if (_diffuseMap)
	{
		delete _diffuseMap;
	}
	if (_specularMap)
	{
		delete _specularMap;
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

	// Resize the underlying swap chain buffers
	_swapChain->ResizeBuffers(
		1,
		width,
		height,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0);

	InitBuffers();
}

void DirectXGraphicsDevice::Present(UINT interval, UINT flags)
{
	_swapChain->Present(interval, flags);
}

HRESULT DirectXGraphicsDevice::InitDx(HWND window, UINT width, UINT height)
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
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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
		&_swapChain, // Pointer to our Swap Chain pointer
		&_device, // Pointer to our Device pointer
		&_featureLevel, // This will hold the actual feature level the app will use
		&_context); // Pointer to our Device Context pointer
	
	if (!FAILED(hr))
		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return hr;
}

void DirectXGraphicsDevice::Init(ContentManager* content)
{
	contentManagerAllocator = content->GetAllocator();
	//Initialize the GBuffer and depth buffer
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

	_device->CreateBuffer(&vbDesc, &vbData, &_quad);

	_skybox = content->Load<Mesh>(L"/models/skybox.obj");
	_skyboxTex = content->Load<Cubemap>(L"/textures/skybox_tex.dds");

	//Load device-specific shaders
	_lightPassVS = content->Load<VertexShader>(L"/shaders/LightPassVS.cso");
	_lightPassPS = content->Load<PixelShader>(L"/shaders/LightPassPS.cso");
	_shadowMapVS = content->Load<VertexShader>(L"/shaders/ShadowMapVS.cso");
	_skyboxVS = content->Load<VertexShader>(L"/shaders/SkyboxVS.cso");
	_skyboxPS = content->Load<PixelShader>(L"/shaders/SkyboxPS.cso");
	_fxaaVS = content->Load<VertexShader>(L"/shaders/FXAA_VS.cso");
	_fxaaPS = content->Load<PixelShader>(L"/shaders/FXAA_PS.cso");
	_projectionPS = content->Load<PixelShader>(L"/shaders/ProjectorPS.cso");

	//Set up g-buffer sampler
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	_gBufferSampler = CreateSamplerState(sampDesc);

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
	_shadowSampler = CreateSamplerState(shadowSampDesc);

	D3D11_RASTERIZER_DESC shadowRSDesc = {};
	shadowRSDesc.CullMode = D3D11_CULL_FRONT;
	shadowRSDesc.FillMode = D3D11_FILL_SOLID;
	shadowRSDesc.DepthClipEnable = true;
	shadowRSDesc.DepthBias = 1000;
	shadowRSDesc.DepthBiasClamp = 0.0f;
	shadowRSDesc.SlopeScaledDepthBias = 1.0f;
	_device->CreateRasterizerState(&shadowRSDesc, &_shadowRS);

	//Just need a default sampler, nothing fancy
	_skyboxSampler = CreateSamplerState(sampDesc);

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
	_device->CreateDepthStencilState(&skyboxDSDesc, &_skyboxDS);

	D3D11_RASTERIZER_DESC skyboxRS = {};
	skyboxRS.CullMode = D3D11_CULL_FRONT;
	skyboxRS.FillMode = D3D11_FILL_SOLID;
	_device->CreateRasterizerState(&skyboxRS, &_skyboxRS);

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
	_projectionSampler = CreateSamplerState(projectorDesc);

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
	_device->CreateBlendState(&projectorBlend, &_projectionBlend);


}

#if defined(DEBUG) || defined(_DEBUG)
#define DRAW_EVERYTHING
#endif

//TODO: Fix BoundingFrustum generation issues (near&far planes are too close)
void DirectXGraphicsDevice::Cull(const Camera& cam, ECS::World* gameWorld, std::vector<ECS::Entity*>& objectsToDraw, bool debugDrawEverything)
{
#ifndef DRAW_EVERYTHING
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
#else
	for (auto* ent : gameWorld->each<Transform, Renderable>())
	{
		objectsToDraw.push_back(ent);
	}
#endif
}

void DirectXGraphicsDevice::RenderShadowMaps(const Camera& cam, const std::vector<ECS::Entity*>& objects, const DirectionalLight& sceneLight)
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
		_context->RSSetState(_shadowRS);
		_context->OMSetRenderTargets(0, nullptr, _shadowMapDSVs[thisCascade]);

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
			auto& mesh = o->get<Renderable>()->_mesh;
			auto vBuf = mesh->VertexBuffer().GetAs<ID3D11Buffer*>();
			auto iBuf = mesh->IndexBuffer().GetAs<ID3D11Buffer*>();
			_shadowMapVS->SetMatrix4x4("model", *o->get<Transform>()->Matrix());
			_shadowMapVS->CopyBufferData("PerInstance");
			_context->IASetVertexBuffers(0, 1, &vBuf, &stride, &offset);
			_context->IASetIndexBuffer(iBuf, DXGI_FORMAT_R32_UINT, 0);
			_context->DrawIndexed(mesh->IndexCount(), 0, 0);
		}

		//Reset to old viewport and default rasterizer state
		_context->RSSetViewports(1, &old);
		_context->RSSetState(nullptr);

		//Unbind the depth texture
		_context->OMSetRenderTargets(0, nullptr, nullptr);
	}
}

/*
void DirectXGraphicsDevice::RenderProjectors(const std::vector<Projector>& projectors)
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

void DirectXGraphicsDevice::Render(const Camera& cam, const std::vector<ECS::Entity*>& objects, const DirectionalLight& sceneLight)
{
	static UINT stride = sizeof(Vertex);
	static UINT quadStride = sizeof(XMFLOAT2);
	static UINT offset = 0;

	Material* currentMaterial = nullptr;

	RenderShadowMaps(cam, objects, sceneLight);

	//TODO: Sort renderables by material and texture to minimize state switches
	ID3D11RenderTargetView* rts[4] = {
		_diffuseMap->GetGraphicsRenderTarget().GetAs<ID3D11RenderTargetView*>(),
		_specularMap->GetGraphicsRenderTarget().GetAs<ID3D11RenderTargetView*>(),
		_positionMap->GetGraphicsRenderTarget().GetAs<ID3D11RenderTargetView*>(),
		_normalMap->GetGraphicsRenderTarget().GetAs<ID3D11RenderTargetView*>()
	};
	_context->OMSetRenderTargets(4, rts, _depthStencil);

	//Load object attributes into the g-buffer (geometry pass)
	for(auto* object : objects)
	{
		auto renderable = object->get<Renderable>();
		auto transform = object->get<Transform>();

		if (renderable->_material.get() != currentMaterial)
		{
			currentMaterial = renderable->_material.get();
			renderable->_material->Apply(cam.ViewMatrix(), cam.ProjectionMatrix());
		}
		
		//Update per-object constant buffer
		renderable->_material->VertexShader()->SetData("world", transform->Matrix(), sizeof(XMFLOAT4X4));
		
		//Upload buffers and draw
		renderable->_material->Upload();

		auto vBuf = renderable->_mesh->VertexBuffer().GetAs<ID3D11Buffer*>();
		_context->IASetVertexBuffers(0, 1, &vBuf, &stride, &offset);
		_context->IASetIndexBuffer(renderable->_mesh->IndexBuffer().GetAs<ID3D11Buffer*>(), DXGI_FORMAT_R32_UINT, 0);
		_context->DrawIndexed(static_cast<UINT>(renderable->_mesh->IndexCount()), 0, 0);
	}

	ID3D11RenderTargetView* lightMapTarget = _lightMap->GetGraphicsRenderTarget().GetAs<ID3D11RenderTargetView*>();
	_context->OMSetRenderTargets(1, &lightMapTarget, nullptr);
	auto cPos = cam.Position();
	//size_t padding = (16 - (sizeof(DirectionalLight) % 16))*(lights.size() - 1);

	_lightPassVS->SetShader();
	_lightPassPS->SetShader();
	_lightPassPS->SetFloat3("cameraPosition", cPos);
	_lightPassPS->SetData("sceneLight", &sceneLight, sizeof(DirectionalLight));
	_lightPassPS->SetData("lightView", &_shadowViews[0], sizeof(XMFLOAT4X4)*NUM_SHADOW_CASCADES);
	_lightPassPS->SetData("lightProjection", &_shadowProjections[0], sizeof(XMFLOAT4X4)*NUM_SHADOW_CASCADES);
	_lightPassPS->SetSamplerState("mainSampler", _gBufferSampler.get());
	_lightPassPS->SetSamplerState("shadowSampler", _shadowSampler.get());
	_lightPassPS->SetShaderResourceView("diffuseMap", _diffuseMap->GetGraphicsTexture().GetAs<ID3D11ShaderResourceView*>());
	_lightPassPS->SetShaderResourceView("specularMap", _specularMap->GetGraphicsTexture().GetAs<ID3D11ShaderResourceView*>());
	_lightPassPS->SetShaderResourceView("positionMap", _positionMap->GetGraphicsTexture().GetAs<ID3D11ShaderResourceView*>());
	_lightPassPS->SetShaderResourceView("normalMap", _normalMap->GetGraphicsTexture().GetAs<ID3D11ShaderResourceView*>());
	_lightPassPS->SetShaderResourceView("shadowMap", _shadowMapSRV);
	_lightPassPS->SetShaderResourceView("depth", _depthStencilTexture);
	_lightPassPS->CopyAllBufferData();

	_context->IASetVertexBuffers(0, 1, &_quad, &quadStride, &offset);
	_context->Draw(6, 0);

	//Can't have SRVs and RTVs that are pointing to the same texture bound at the same time, so unset them
	ID3D11ShaderResourceView* srvs[6] = { 0 };
	_context->PSSetShaderResources(0, 6, srvs);

	RenderSkybox(cam);

	_context->OMSetRenderTargets(1, &_backBuffer, nullptr);

	_fxaaVS->SetShader();
	_fxaaPS->SetShader();
	_fxaaPS->SetInt("width", _width);
	_fxaaPS->SetInt("height", _height);
	_fxaaPS->SetSamplerState("mainSampler", _gBufferSampler.get());
	_fxaaPS->SetShaderResourceView("inputMap", _lightMap->GetGraphicsTexture().GetAs<ID3D11ShaderResourceView*>());
	_fxaaPS->CopyAllBufferData();

	_context->IASetVertexBuffers(0, 1, &_quad, &quadStride, &offset);
	_context->Draw(6, 0);
}

void DirectXGraphicsDevice::RenderSkybox(const Camera& cam)
{
	ID3D11DepthStencilState* lastDepthState;
	UINT lastStencilRef;
	_context->OMGetDepthStencilState(&lastDepthState, &lastStencilRef);

	ID3D11RasterizerState* lastRasterState;
	_context->RSGetState(&lastRasterState);

	ID3D11RenderTargetView* lightMapTarget = _lightMap->GetGraphicsRenderTarget().GetAs<ID3D11RenderTargetView*>();

	_context->RSSetState(_skyboxRS);
	_context->OMSetDepthStencilState(_skyboxDS, lastStencilRef);
	_context->OMSetRenderTargets(1, &lightMapTarget, _depthStencil);
	_skyboxVS->SetShader();
	_skyboxPS->SetShader();
	_skyboxVS->SetFloat3("camPos", cam.Position());
	_skyboxVS->SetMatrix4x4("view", cam.ViewMatrix());
	_skyboxVS->SetMatrix4x4("proj", cam.ProjectionMatrix());
	_skyboxPS->SetShaderResourceView("skyboxTex", _skyboxTex->GetGraphicsTexture().GetAs<ID3D11ShaderResourceView*>());
	_skyboxPS->SetSamplerState("mainSampler", _skyboxSampler.get());
	_skyboxVS->CopyAllBufferData();
	_skyboxPS->CopyAllBufferData();

	const UINT stride = sizeof(Vertex);
	const UINT offset = 0;
	auto vBuf = _skybox->VertexBuffer().GetAs<ID3D11Buffer*>();
	_context->IASetVertexBuffers(0, 1, &vBuf, &stride, &offset);
	_context->IASetIndexBuffer(_skybox->IndexBuffer().GetAs<ID3D11Buffer*>(), DXGI_FORMAT_R32_UINT, 0);
	_context->DrawIndexed(_skybox->IndexCount(), 0, 0);

	//Reset the DS and RS states
	_context->OMSetDepthStencilState(lastDepthState, lastStencilRef);
	_context->RSSetState(lastRasterState);
}

GraphicsTexture DirectXGraphicsDevice::CreateTexture(const char * texturePath, GraphicsRenderTarget * outOptionalRenderTarget)
{
	ID3D11ShaderResourceView* srv;
	auto result = DirectX::CreateWICTextureFromFile(_device, (wchar_t*)texturePath, nullptr, &srv);
	return GraphicsTexture(srv);
}

void BlackMagic::DirectXGraphicsDevice::CleanupTexture(GraphicsTexture texture)
{
	if(texture.buffer) texture.GetAs<ID3D11ShaderResourceView*>()->Release();
}

void BlackMagic::DirectXGraphicsDevice::CleanupRenderTarget(GraphicsRenderTarget renderTarget)
{
	if(renderTarget.buffer) renderTarget.GetAs<ID3D11RenderTargetView*>()->Release();
}

void DirectXGraphicsDevice::InitBuffers()
{
	// The above function created the back buffer render target
	// for us, but we need a reference to it
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
		&_backBuffer);
	backBufferTexture->Release();

	D3D11_TEXTURE2D_DESC colorMapDesc;
	colorMapDesc.ArraySize = 1;
	colorMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	colorMapDesc.CPUAccessFlags = 0;
	colorMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	colorMapDesc.MipLevels = 1;
	colorMapDesc.MiscFlags = 0;
	colorMapDesc.SampleDesc.Count = 1;
	colorMapDesc.SampleDesc.Quality = 0;
	colorMapDesc.Usage = D3D11_USAGE_DEFAULT;
	colorMapDesc.Height = _height;
	colorMapDesc.Width = _width;

	D3D11_TEXTURE2D_DESC normalMapDesc;
	normalMapDesc.ArraySize = 1;
	normalMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	normalMapDesc.CPUAccessFlags = 0;
	normalMapDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	normalMapDesc.MipLevels = 1;
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

	_diffuseMap = createEmptyTexture(colorMapDesc);
	_specularMap = createEmptyTexture(colorMapDesc);
	_normalMap = createEmptyTexture(normalMapDesc);
	_positionMap = createEmptyTexture(posMapDesc);
	_lightMap = createEmptyTexture(lightMapDesc);

	ID3D11Texture2D* depth;
	_device->CreateTexture2D(&depthStencilDesc, nullptr, &depth);
	_device->CreateDepthStencilView(depth, &dsDesc, &_depthStencil);
	_device->CreateShaderResourceView(depth, &depthSRVDesc, &_depthStencilTexture);
	depth->Release();

	ID3D11Texture2D* smDepth;
	_device->CreateTexture2D(&shadowMapDesc, nullptr, &smDepth);
	_device->CreateShaderResourceView(smDepth, &shadowSRV, &_shadowMapSRV);
	smDepth->Release();

	for (size_t i = 0; i < NUM_SHADOW_CASCADES; i++)
	{
		shadowDS.Flags = 0;
		shadowDS.Format = DXGI_FORMAT_D32_FLOAT;
		shadowDS.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		shadowDS.Texture2DArray.MipSlice = 0;
		shadowDS.Texture2DArray.ArraySize = 1;
		shadowDS.Texture2DArray.FirstArraySlice = i;
		_device->CreateDepthStencilView(smDepth, &shadowDS, &_shadowMapDSVs[i]);
	}

	// Bind the views to the pipeline, so rendering properly 
	// uses their underlying textures
	_context->OMSetRenderTargets(1, &_backBuffer, _depthStencil);

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

Texture* DirectXGraphicsDevice::createEmptyTexture(D3D11_TEXTURE2D_DESC& desc)
{
	ID3D11Texture2D* tex;
	ID3D11RenderTargetView* rtv;
	ID3D11ShaderResourceView* srv;

	_device->CreateTexture2D(&desc, nullptr, &tex);
	_device->CreateRenderTargetView(tex, nullptr, &rtv);
	_device->CreateShaderResourceView(tex, nullptr, &srv);
	tex->Release();

	return new Texture(this, GraphicsTexture(srv), GraphicsRenderTarget(rtv));
}


