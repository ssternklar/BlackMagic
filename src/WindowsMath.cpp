#include "BMMath.h"

//If errors start occurring, uncomment this:
//#define _XM_NO_INTRINSICS_
#include <DirectXCollision.h>
#include <DirectXMath.h>

using namespace DirectX;

namespace BlackMagic
{

inline XMVECTOR C(Vector3& vec)
{
	return *reinterpret_cast<XMVECTOR*>(&vec);
}

inline XMVECTOR C(Vector4& vec)
{
	return *reinterpret_cast<XMVECTOR*>(&vec);
}

inline XMVECTOR C(Quaternion& vec)
{
	return *reinterpret_cast<XMVECTOR*>(&vec);
}

inline XMMATRIX C(Matrix4& mat)
{
	return *reinterpret_cast<XMMATRIX*>(&mat);
}

inline DirectX::BoundingFrustum C(BlackMagic::BoundingFrustum& fr)
{
	XMFLOAT3 origin;
	XMFLOAT4 orient; 
	XMStoreFloat3(&origin, C(fr.Origin));
	XMStoreFloat4(&orient, C(fr.Orientation));
	return DirectX::BoundingFrustum(origin, orient,
		fr.RightSlope, fr.LeftSlope, fr.TopSlope, fr.BottomSlope,
		fr.Near, fr.Far);
}

inline Vector3 CV3(XMVECTOR& vec)
{
	return *reinterpret_cast<Vector3*>(&vec);
}

inline Vector4 CV4(XMVECTOR& vec)
{
	return *reinterpret_cast<Vector4*>(&vec);
}

inline Quaternion CQ(XMVECTOR& vec)
{
	return *reinterpret_cast<Quaternion*>(&vec);
}

inline Matrix4 CM4(XMMATRIX& mat)
{
	return *reinterpret_cast<Matrix4*>(&mat);
}

inline BlackMagic::BoundingFrustum CBF(DirectX::BoundingFrustum& fr)
{
	return BlackMagic::BoundingFrustum(CV3(XMLoadFloat3(&fr.Origin)), 
		CQ(XMLoadFloat4(&fr.Orientation)), 
		fr.RightSlope, 
		fr.LeftSlope, 
		fr.TopSlope, 
		fr.BottomSlope, 
		fr.Near, 
		fr.Far
	);
}

BoundingFrustum::BoundingFrustum()
	: Origin(CreateVector3Zero()), Orientation(CreateQuaternionIdentity()),
	RightSlope(0.0f), LeftSlope(0.0f), TopSlope(0.0f), BottomSlope(0.0f),
	Near(0.0f), Far(1.0f)
{}

BoundingFrustum::BoundingFrustum(Vector3& origin, Quaternion& orient, float rSlope, float lSlope, float tSlope, float bSlope, float n, float f)
	: Origin(origin), Orientation(orient), 
	RightSlope(rSlope), LeftSlope(lSlope), TopSlope(tSlope), BottomSlope(bSlope),
	Near(n), Far(f)
{}

BoundingFrustum::BoundingFrustum( Mat4& proj)
{
	*this = CBF(DirectX::BoundingFrustum(C(proj)));
}

void BoundingFrustum::Transform(BlackMagic::BoundingFrustum& out, Mat4& m)
{
	auto dxbf = C(*this);
	dxbf.Transform(dxbf, C(m));
	out = CBF(dxbf);
}

Vector2 operator-(Vector2& left, Vector2& right)
{
	Vector2 v;
	v.data[0] = left.data[0] - right.data[0];
	v.data[1] = left.data[1] - right.data[1];
	return v;
}

Vector3 CreateVector3(float x, float y, float z)
{
	return CV3(XMVectorSet(x, y, z, 0));
}

Vector3 CreateVector3(Vector3& other)
{
	return other;
}

Vector3 CreateVector3(Vector4& other)
{
	return CV3(XMVectorSetW(C(other), 0));
}

Vector3 CreateVector3Zero()
{
	return CreateVector3(0, 0, 0);
}

float Dot(Vector3& left, Vector3& right)
{
	return XMVectorGetX(XMVector3Dot(C(left), C(right)));
}

Vector3 Cross(Vector3& left, Vector3& right)
{
	return CV3(XMVector3Cross(C(left), C(right)));
}

Vector3 Normalize(Vector3& vec)
{
	return CV3(XMVector3Normalize(C(vec)));
}

Vector3 Rotate(Vector3& v, Quaternion& q)
{
	return CV3(XMVector3Rotate(C(v), C(q)));
}

Vector3 Lerp(Vector3& t0, Vector3& t1, float t)
{
	return CV3(XMVectorLerp(C(t0), C(t1), t));
}

Vector3 operator*(Vector3& left, float scalar)
{
	return CV3(XMVectorScale(C(left), scalar));
}

Vector3 operator*(float scalar, Vector3& right)
{
	return right * scalar;
}

Vector3 operator/(Vector3& left, float scalar)
{
	return operator*(left, 1 / scalar);
}

Vector3 operator+(Vector3& left, Vector3& right)
{
	return CV3(XMVectorAdd(C(left), C(right)));
}

Vector3 operator-(Vector3& left, Vector3& right)
{
	return CV3(XMVectorSubtract(C(left), C(right)));
}

float GetX(Vector3& vec)
{
	return XMVectorGetX(C(vec));
}

float GetY(Vector3& vec)
{
	return XMVectorGetY(C(vec));
}

float GetZ(Vector3& vec)
{
	return XMVectorGetZ(C(vec));
}

Vector4 CreateVector4(float x, float y, float z, float w)
{
	return CV4(XMVectorSet(x, y, z, w));
}

Vector4 CreateVector4(Vector4& other)
{
	return other;
}

Vector4 CreateVector4(Vector3& other)
{
	return *reinterpret_cast<Vector4*>(&other);
}

Vector4 CreateVector4Zero()
{
	return CreateVector4(0, 0, 0, 0);
}

float Dot(Vector4& left, Vector4& right)
{
	return XMVectorGetX(XMVector4Dot(C(left), C(right)));
}

Vector4 Normalize(Vector4& vec)
{
	return CV4(XMVector4Normalize(C(vec)));
}

Vector4 Lerp(Vector4& t0, Vector4& t1, float t)
{
	return CV4(XMVectorLerp(C(t0), C(t1), t));
}

Vector4 operator*(Vector4& left, float scalar)
{
	return CV4(XMVectorScale(C(left), scalar));
}

Vector4 operator*(float scalar, Vector4& right)
{
	return right * scalar;
}

Vector4 operator/(Vector4& left, float scalar)
{
	return operator*(left, 1 / scalar);
}

Vector4 operator+(Vector4& left, Vector4& right)
{
	return CV4(XMVectorAdd(C(left), C(right)));
}

Vector4 operator-(Vector4& left, Vector4& right)
{
	return CV4(XMVectorSubtract(C(left), C(right)));
}

float GetX(Vector4& vec)
{
	return XMVectorGetX(C(vec));
}

float GetY(Vector4& vec)
{
	return XMVectorGetY(C(vec));
}

float GetZ(Vector4& vec)
{
	return XMVectorGetZ(C(vec));
}

float GetW(Vector4& vec)
{
	return XMVectorGetW(C(vec));
}

Matrix4 CreateMatrix4(float* data)
{
	return CM4(XMLoadFloat4x4((XMFLOAT4X4*)data));
}

Matrix4 CreateMatrix4(Vector4& a, Vector4& b, Vector4& c, Vector4& d)
{
	XMMATRIX matrix;
	matrix.r[0] = C(a);
	matrix.r[1] = C(b);
	matrix.r[2] = C(c);
	matrix.r[3] = C(d);
	return CM4(matrix);
}

Matrix4 CreateMatrix4(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44)
{
	return CM4(XMMatrixSet(m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44));
}

Matrix4 CreateMatrix4Identity()
{
	return CreateMatrix4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
}

Matrix4 CreateMatrix4Zero()
{
	return CreateMatrix4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

Matrix4 CreateMatrixLookToLH(Vector3& pos, Vector3& dir, Vector3& up)
{
	return CM4(XMMatrixLookToLH(C(pos), C(dir), C(up)));
}

Matrix4 CreateMatrixPerspectiveFovLH(float fov, float aspect, float near, float far)
{
	return CM4(XMMatrixPerspectiveFovLH(fov, aspect, near, far));
}

Matrix4 CreateAffineTransformation(Vector3& scale, Vector3& rotOffset, Quaternion& rot, Vector3& pos)
{
	return CM4(XMMatrixAffineTransformation(C(scale), C(rotOffset), C(rot), C(pos)));
}

Matrix4 Transpose(Matrix4& mat4)
{
	return CM4(XMMatrixTranspose(C(mat4)));
}

Matrix4 Inverse(Matrix4& mat4)
{
	return CM4(XMMatrixInverse(nullptr, C(mat4)));
}

float Matrix4GetData(Matrix4& mat4, int row, int col)
{
	reinterpret_cast<XMMATRIX*>(&mat4)->r;
	return XMVectorGetByIndex(reinterpret_cast<XMMATRIX*>(&mat4)->r[row], col);
}

Vector4 operator*(Vector4& vec, Matrix4& matrix)
{
	return CV4(XMVector4Transform(C(vec), C(matrix)));
}

Vector4 operator*(Matrix4& matrix, Vector4& vector)
{
	return CV4(XMVector4Transform(C(vector), XMMatrixTranspose(C(matrix))));
}

Matrix4 operator*(Matrix4& left, Matrix4& right)
{
	return CM4(XMMatrixMultiply(C(left), C(right)));
}

Quaternion CreateQuaternion(float x, float y, float z, float w)
{
	return CQ(XMVectorSet(x, y, z, w));
}

Quaternion CreateQuaternion(float roll, float pitch, float yaw)
{
	return CQ(XMQuaternionRotationRollPitchYaw(pitch, yaw, roll));
}

Quaternion CreateQuaternion(Vector3& forward, Vector3& up)
{
	XMVECTOR fwdV = XMVector3Normalize(C(forward));
	XMVECTOR upV = XMVector3Normalize(C(up));
	XMVECTOR rightV = XMVector3Normalize(XMVector3Cross(upV, fwdV));
	upV = XMVector3Normalize(XMVector3Cross(fwdV, rightV));
	XMMATRIX mat;
	mat.r[0] = rightV;
	mat.r[1] = upV;
	mat.r[2] = fwdV;
	mat.r[3] = XMVectorSet(0, 0, 0, 1);
	return CQ(XMQuaternionRotationMatrix(mat));

}

Quaternion CreateQuaternion(Vector3& axis, float angle)
{
	return CQ(XMQuaternionRotationAxis(C(axis), angle));
}

Quaternion CreateQuaternionIdentity()
{
	return CreateQuaternion(0, 0, 0, 1);
}

Quaternion Slerp(Quaternion& t0, Quaternion& t1, float t)
{
	return CQ(XMQuaternionSlerp(C(t0), C(t1), t));
}

float GetX(Quaternion& vec)
{
	return XMVectorGetX(C(vec));
}
float GetY(Quaternion& vec)
{
	return XMVectorGetY(C(vec));
}
float GetZ(Quaternion& vec)
{
	return XMVectorGetZ(C(vec));
}
float GetW(Quaternion& vec)
{
	return XMVectorGetW(C(vec));
}

Vector3 operator*(Quaternion& quat, Vector3& vec)
{
	return CV3(XMVector3Rotate(C(vec), C(quat)));
}

Quaternion operator*(Quaternion& left, Quaternion& right)
{
	return CQ(XMQuaternionMultiply(C(left), C(right)));
}

Matrix4 QuaternionToMatrix(Quaternion& quat)
{
	return CM4(XMMatrixRotationQuaternion(C(quat)));
}

}