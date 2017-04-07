#pragma once

namespace BlackMagic
{
	class Vector3;
	class Vector4;
	class Matrix4;
	class Quaternion;

	Vector3& CreateVector3(float x, float y, float z);
	Vector3& CreateVector3(Vector3& other);
	Vector3& CreateVector3(Vector4& other);
	Vector3& CreateVector3Zero()
	{
		return CreateVector3(0, 0, 0);
	}
	float Dot(Vector3& left, Vector3& right);
	Vector3& Cross(Vector3& left, Vector3& right);
	Vector3& Normalize(Vector3& vec);
	Vector3& Lerp(Vector3& t0, Vector3& t1, float t);
	Vector3& operator*(Vector3& left, float scalar);
	Vector3& operator/(Vector3& left, float scalar);
	Vector3& operator+(Vector3& left, Vector3& right);
	Vector3& operator-(Vector3& left, Vector3& right);
	float GetX(Vector3& vec);
	float GetY(Vector3& vec);
	float GetZ(Vector3& vec);


	Vector4& CreateVector4(float x, float y, float z, float w);
	Vector4& CreateVector4(Vector4& other);
	Vector4& CreateVector4(Vector3& other);
	Vector4& CreateVector4Zero()
	{
		return CreateVector4(0, 0, 0, 0);
	}
	float Dot(Vector4& left, Vector4& right);
	Vector4& Normalize(Vector4& vec);
	Vector3& Lerp(Vector4& t0, Vector4& t1, float t);
	Vector4& operator*(Vector4& left, float scalar);
	Vector4& operator/(Vector4& left, float scalar);
	Vector4& operator+(Vector4& left, Vector4& right);
	Vector4& operator-(Vector4& left, Vector4& right);
	float GetX(Vector4& vec);
	float GetY(Vector4& vec);
	float GetZ(Vector4& vec);
	float GetW(Vector4& vec);


	Matrix4& CreateMatrix4(float* data);
	Matrix4& CreateMatrix4(Vector4& a, Vector4& b, Vector4& c, Vector4& d);
	Matrix4& CreateMatrix4(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44);
	Matrix4& Transpose(Matrix4& mat4);
	Matrix4& Inverse(Matrix4& mat4);
	Matrix4& CreateMatrix4Identity()
	{
		return CreateMatrix4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		);
	}
	Matrix4& CreateMatrix4Zero()
	{
		return CreateMatrix4(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	}
	float Matrix4GetData(Matrix4& mat4, int index);
	Vector4& operator*(Vector4& vec, Matrix4& matrix);
	Vector4& operator*(Matrix4& matrix, Vector4& vector);
	Matrix4& operator*(Matrix4& left, Matrix4& right);

	Quaternion& CreateQuaternion(float x, float y, float z, float w);
	Quaternion& CreateQuaternion(Vector4& forward, Vector4& up);
	Quaternion& CreateQuaternionIdentity()
	{
		return CreateQuaternion(0, 0, 0, 1);
	}
	Quaternion& Slerp(Quaternion& t0, Quaternion& t1, float t);
	float GetX(Quaternion& vec);
	float GetY(Quaternion& vec);
	float GetZ(Quaternion& vec);
	float GetW(Quaternion& vec);

	Vector3& operator*(Quaternion& quat, Vector3& vec);
	Vector4& operator*(Quaternion& quat, Vector4& vec);
	Matrix4& QuaternionToMatrix(Quaternion& quat);

}