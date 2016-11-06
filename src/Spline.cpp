#include "Spline.h"
#include "Vertex.h"

void Spline::GetPoint(float t, SplineControlPoint& outPoint)
{
	float modifiedT = t * segmentCount;
	int segment = modifiedT;
	float indivT = modifiedT - segment;
	segments[segment % segmentCount].GetPoint(indivT, outPoint);
}

float Spline::GuessNearestPoint(DirectX::XMFLOAT3& point, float* outDistanceSquared)
{
	float closest = 0;
	float closestDistSquared = FLT_MAX;
	int segID = 0;
	for (int i = 0; i < segmentCount; i++)
	{
		float guessDsq;
		float guess = segments[i].GuessNearestPoint(point, guessDsq);
		if (closestDistSquared > guessDsq)
		{
			closest = guess;
			closestDistSquared = guessDsq;
			segID = i;
		}
	}
	closest = (closest + segID) / segmentCount;
	if (outDistanceSquared)
	{
		*outDistanceSquared = closestDistSquared;
	}
	return closest;
}

void Spline::GuessNearestPoint(DirectX::XMFLOAT3& point, SplineControlPoint& outPoint)
{
	return GetPoint(GuessNearestPoint(point), outPoint);
}

void Spline::GenerateMesh(GraphicsDevice* device, Mesh* mesh)
{
	const size_t numVerts = 100000;
	const size_t numIndices = numVerts * 3;
	Vertex* vertices = new Vertex[numVerts];
	unsigned int* indices = new unsigned int[numIndices];
	using namespace DirectX;
	float stepAmt = 1 / (numVerts / 2.0f);
	float step = 0;
	SplineControlPoint point;
	XMFLOAT3 binormal;
	int uvY = 0;
	for (int i = 0; i < numVerts; i += 2)
	{
		GetPoint(step, point);
		auto position = XMLoadFloat3(&point.position);
		XMStoreFloat3(&binormal, XMVector3Cross(XMLoadFloat3(&point.normal), XMLoadFloat3(&point.tangent)));
		auto localScale = XMVector3Rotate(XMVectorSet(point.scale.x, 0, 0, 0), XMLoadFloat4(&point.rotation));
		//left vertex
		XMStoreFloat3(&vertices[i].Position, position - localScale);
		vertices[i].Normal = point.normal;
		vertices[i].Tangent = point.tangent;
		vertices[i].Binormal = binormal;
		vertices[i].UV = { 0, 0 };
		
		//right vertex
		XMStoreFloat3(&vertices[i+1].Position, position + localScale);
		vertices[i+1].Normal = point.normal;
		vertices[i+1].Tangent = point.tangent;
		vertices[i+1].Binormal = binormal;
		vertices[i+1].UV = { 1, 0 };

		step += stepAmt;
	}

	for (int i = 0; i < numVerts; i += 2)
	{
		indices[i * 3] = i % numVerts;
		indices[(i * 3) + 1] = (i + 1) % numVerts;
		indices[(i * 3) + 2] = (i + 2) % numVerts;
		indices[(i * 3) + 3] = (i + 5) % numVerts;
		indices[(i * 3) + 4] = (i + 4) % numVerts;
		indices[(i * 3) + 5] = (i + 3) % numVerts;
		
	}

	D3D11_BUFFER_DESC vertDesc = {};
	vertDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertDesc.ByteWidth = static_cast<UINT>(numVerts * sizeof(Vertex));
	vertDesc.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA vertData = {};
	vertData.pSysMem = &vertices[0];

	D3D11_BUFFER_DESC indDesc = {};
	indDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indDesc.ByteWidth = static_cast<UINT>(numIndices * sizeof(UINT));
	indDesc.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA indData = {};
	indData.pSysMem = &indices[0];

	mesh->Set(device->CreateBuffer(vertDesc, vertData), device->CreateBuffer(indDesc, indData), numIndices);
	delete[] vertices;
	delete[] indices;
}