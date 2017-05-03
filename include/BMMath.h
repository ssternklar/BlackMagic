#pragma once

namespace BlackMagic
{
	struct alignas(16) Vector2 { float data[4]; };
	struct alignas(16) Vector3 { float data[4]; };
	struct alignas(16) Vector4 { float data[4]; };
	struct alignas(16) Matrix4 { float data[16]; };
	struct alignas(16) Quaternion { float data[4]; };
	typedef Matrix4 Mat4;

	enum ContainmentType
	{
		Disjoint,
		Intersects,
		Contains
	};

	struct BoundingFrustum
	{
		Vector3 Origin;
		Quaternion Orientation;
		float RightSlope;
		float LeftSlope;
		float TopSlope;
		float BottomSlope;
		float Near, Far;

		BoundingFrustum();
		BoundingFrustum(Vector3 origin, Quaternion orient, float rSlope, float lSlopt, float tSlope, float bSlope, float n, float f);
		BoundingFrustum(Mat4 projection);
		BoundingFrustum(const BoundingFrustum&) = default;
		BoundingFrustum& operator=(const BoundingFrustum&) = default;
		
		void Transform(BoundingFrustum& out, Mat4 m);
	};

	Vector2 operator-(Vector2 left, Vector2 right);

	Vector3 CreateVector3(float x, float y, float z);
	Vector3 CreateVector3(Vector3 other);
	Vector3 CreateVector3(Vector4 other);
	Vector3 CreateVector3Zero();
	float Dot(Vector3 left, Vector3 right);
	Vector3 Cross(Vector3 left, Vector3 right);
	Vector3 Normalize(Vector3 vec);
	Vector3 Rotate(Vector3 v, Quaternion q);
	Vector3 Lerp(Vector3 t0, Vector3 t1, float t);
	Vector3 operator*(Vector3 left, float scalar);
	Vector3 operator*(float scalar, Vector3 right);
	Vector3 operator/(Vector3 left, float scalar);
	Vector3 operator+(Vector3 left, Vector3 right);
	Vector3 operator-(Vector3 left, Vector3 right);
	float GetX(Vector3 vec);
	float GetY(Vector3 vec);
	float GetZ(Vector3 vec);
	float Magnitude(Vector3 vec);
	float SquareMagnitude(Vector3 vec);

	Vector4 CreateVector4(float x, float y, float z, float w);
	Vector4 CreateVector4(Vector4 other);
	Vector4 CreateVector4(Vector3 other);
	Vector4 CreateVector4Zero();
	float Dot(Vector4 left, Vector4 right);
	Vector4 Floor(Vector4 v);
	Vector4 Normalize(Vector4 vec);
	Vector4 Lerp(Vector4 t0, Vector4 t1, float t);
	Vector4 operator*(Vector4 left, float scalar);
	Vector4 operator*(float scalar, Vector4 right);
	Vector4 operator/(Vector4 left, float scalar);
	Vector4 operator+(Vector4 left, Vector4 right);
	Vector4 operator-(Vector4 left, Vector4 right);
	float GetX(Vector4 vec);
	float GetY(Vector4 vec);
	float GetZ(Vector4 vec);
	float GetW(Vector4 vec);
	float Magnitude(Vector4 vec);
	float SquareMagnitude(Vector4 vec);

	Matrix4 CreateMatrix4(float* data);
	Matrix4 CreateMatrix4(Vector4 a, Vector4 b, Vector4 c, Vector4 d);
	Matrix4 CreateMatrix4(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44);
	Matrix4 Transpose(Matrix4 mat4);
	Matrix4 Inverse(Matrix4 mat4);
	Matrix4 CreateMatrix4Identity();
	Matrix4 CreateMatrix4Zero();
	Matrix4 CreateMatrixLookToLH(Vector3 pos, Vector3 dir, Vector3 up);
	Matrix4 CreateMatrixPerspectiveFovLH(float fov, float aspect, float near, float far);
	Matrix4 CreateAffineTransformation(Vector3 scale, Vector3 rotOffset, Quaternion rotation, Vector3 pos);
	float Matrix4GetData(Matrix4 mat4, int row, int col);
	Vector4 operator*(Vector4 vec, Matrix4 matrix);
	Vector4 operator*(Matrix4 matrix, Vector4 vector);
	Matrix4 operator*(Matrix4 left, Matrix4 right);

	Quaternion CreateQuaternion(float x, float y, float z, float w);
	Quaternion CreateQuaternion(float roll, float pitch, float yaw);
	Quaternion CreateQuaternion(Vector3 forward, Vector3 up);
	Quaternion CreateQuaternion(Vector3 axis, float angle);
	Quaternion CreateQuaternionIdentity();
	Quaternion Slerp(Quaternion t0, Quaternion t1, float t);
	float GetX(Quaternion vec);
	float GetY(Quaternion vec);
	float GetZ(Quaternion vec);
	float GetW(Quaternion vec);

	Vector3 operator*(Quaternion quat, Vector3 vec);
	Quaternion operator*(Quaternion left, Quaternion right);
	Matrix4 QuaternionToMatrix(Quaternion quat);
}