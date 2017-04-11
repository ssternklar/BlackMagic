#include "Math.h"

//If errors start occurring, uncomment this:
#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>

using namespace DirectX;

namespace BlackMagic
{

class Vector3 : public XMVECTOR
{
public:
	Vector3() : XMVECTOR(CreateVector3Zero()){};
	Vector3(const XMVECTOR& other) : XMVECTOR(other) {}
};

class Vector4 : public XMVECTOR
{
public:
	Vector4() : XMVECTOR(CreateVector4Zero()) {};
	Vector4(const XMVECTOR& other) : XMVECTOR(other) {}
};

class Quaternion : public XMVECTOR
{
public:
	Quaternion() : XMVECTOR(CreateQuaternionIdentity()) {};
	Quaternion(const XMVECTOR& other) : XMVECTOR(other) {}
};

class Matrix4 : public XMMATRIX
{
public:
	Matrix4() : XMMATRIX(CreateMatrix4Zero()) {};
	Matrix4(const XMMATRIX& other) : XMMATRIX(other) {};
};

Vector3 CreateVector3(float x, float y, float z)
{
	return XMVectorSet(x, y, z, 0);
}

Vector3 CreateVector3(Vector3& other)
{
	XMVECTOR ret = other;
	return ret;
}

Vector3 CreateVector3(Vector4& other)
{
	XMVECTOR ret = other;
	return XMVectorSetW(other, 0);
}

Vector3 CreateVector3Zero()
{
	return CreateVector3(0, 0, 0);
}

float Dot(Vector3& left, Vector3& right)
{
	return XMVectorGetX(XMVector3Dot(left, right));
}

Vector3 Cross(Vector3& left, Vector3& right)
{
	return XMVector3Cross(left, right);
}

Vector3 Normalize(Vector3& vec)
{
	return XMVector3Normalize(vec);
}

Vector3 Lerp(Vector3& t0, Vector3& t1, float t)
{
	return XMVectorLerp(t0, t1, t);
}

Vector3 operator*(Vector3& left, float scalar)
{
	return XMVectorScale(left, scalar);
}

Vector3 operator/(Vector3& left, float scalar)
{
	return XMVectorScale(left, 1 / scalar);
}

Vector3 operator+(Vector3& left, Vector3& right)
{
	return XMVectorAdd(left, right);
}

Vector3 operator-(Vector3& left, Vector3& right)
{
	return XMVectorSubtract(left, right);
}

float GetX(Vector3& vec)
{
	return XMVectorGetX(vec);
}

float GetY(Vector3& vec)
{
	return XMVectorGetY(vec);
}

float GetZ(Vector3& vec)
{
	return XMVectorGetZ(vec);
}

Vector4 CreateVector4(float x, float y, float z, float w)
{
	return XMVectorSet(x, y, z, w);
}

Vector4 CreateVector4(Vector4& other)
{
	XMVECTOR vec = other;
	return vec;
}

Vector4 CreateVector4(Vector3& other)
{
	XMVECTOR vec = other;
	return vec;
}

Vector4 CreateVector4Zero()
{
	return CreateVector4(0, 0, 0, 0);
}

float Dot(Vector4& left, Vector4& right)
{
	return XMVectorGetX(XMVector4Dot(left, right));
}

Vector4 Normalize(Vector4& vec)
{
	return XMVector4Normalize(vec);
}

Vector3 Lerp(Vector4& t0, Vector4& t1, float t)
{
	return XMVectorLerp(t0, t1, t);
}

Vector4 operator*(Vector4& left, float scalar)
{
	return XMVectorScale(left, scalar);
}

Vector4 operator/(Vector4& left, float scalar)
{
	return XMVectorScale(left, 1 / scalar);
}

Vector4 operator+(Vector4& left, Vector4& right)
{
	return XMVectorAdd(left, right);
}

Vector4 operator-(Vector4& left, Vector4& right)
{
	return XMVectorSubtract(left, right);
}

float GetX(Vector4& vec)
{
	return XMVectorGetX(vec);
}

float GetY(Vector4& vec)
{
	return XMVectorGetY(vec);
}

float GetZ(Vector4& vec)
{
	return XMVectorGetZ(vec);
}

float GetW(Vector4& vec)
{
	return XMVectorGetW(vec);
}

Matrix4 CreateMatrix4(float* data)
{
	return XMLoadFloat4x4((XMFLOAT4X4*)data);
}

Matrix4 CreateMatrix4(Vector4& a, Vector4& b, Vector4& c, Vector4& d)
{
	XMMATRIX matrix;
	matrix.r[0] = a;
	matrix.r[1] = b;
	matrix.r[2] = c;
	matrix.r[3] = d;
	return matrix;
}

Matrix4 CreateMatrix4(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44)
{
	return XMMatrixSet(m11, m12, m13, m14, m21, m22, m23, 24, m31, m32, m33, m34, m41, m42, m43, m44);
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

Matrix4 Transpose(Matrix4& mat4)
{
	return XMMatrixTranspose(mat4);
}

Matrix4 Inverse(Matrix4& mat4)
{
	return XMMatrixInverse(nullptr, mat4);
}

float Matrix4GetData(Matrix4& mat4, int row, int col)
{
	return XMVectorGetByIndex(mat4.r[row], col);
}

Vector4 operator*(Vector4& vec, Matrix4& matrix)
{
	return XMVector4Transform(vec, matrix);
}

Vector4 operator*(Matrix4& matrix, Vector4& vector)
{
	return XMVector4Transform(vector, Transpose(matrix));
}

Matrix4 operator*(Matrix4& left, Matrix4& right)
{
	return XMMatrixMultiply(left, right);
}

Quaternion CreateQuaternion(float x, float y, float z, float w)
{
	return XMVectorSet(x, y, z, w);
}

Quaternion CreateQuaternion(float roll, float pitch, float yaw)
{
	return XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
}

Quaternion CreateQuaternion(Vector3& forward, Vector3& up)
{
	XMVECTOR fwdV = XMVector3Normalize(forward);
	XMVECTOR upV = XMVector3Normalize(up);
	XMVECTOR rightV = XMVector3Normalize(XMVector3Cross(upV, fwdV));
	upV = XMVector3Normalize(XMVector3Cross(fwdV, rightV));
	XMMATRIX mat;
	mat.r[0] = rightV;
	mat.r[1] = upV;
	mat.r[2] = fwdV;
	mat.r[3] = XMVectorSet(0, 0, 0, 1);
	return XMQuaternionRotationMatrix(mat);

}

Quaternion CreateQuaternionIdentity()
{
	return CreateQuaternion(0, 0, 0, 1);
}

Quaternion Slerp(Quaternion& t0, Quaternion& t1, float t)
{
	return XMQuaternionSlerp(t0, t1, t);
}

float GetX(Quaternion& vec)
{
	return XMVectorGetX(vec);
}
float GetY(Quaternion& vec)
{
	return XMVectorGetY(vec);
}
float GetZ(Quaternion& vec)
{
	return XMVectorGetZ(vec);
}
float GetW(Quaternion& vec)
{
	return XMVectorGetW(vec);
}

Vector3 operator*(Quaternion& quat, Vector3& vec)
{
	return XMVector3Rotate(vec, quat);
}

Quaternion operator*(Quaternion& left, Quaternion& right)
{
	return XMQuaternionMultiply(left, right);
}

Matrix4 QuaternionToMatrix(Quaternion& quat)
{
	return XMMatrixRotationQuaternion(quat);
}

}