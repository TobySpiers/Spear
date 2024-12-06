#pragma once
#include <cmath>
#include <fstream>

template <typename T>
struct Vector2
{
	static_assert( std::is_same<T, int>::value
				|| std::is_same<T, float>::value
				|| std::is_same<T, double>::value,
				"Vector2 only supports the following types: int, float, double");

	friend std::ofstream& operator<<(std::ofstream& stream, const Vector2<T>& obj)
	{
		stream << obj.x << obj.y;
		return stream;
	}
	friend std::ifstream& operator>>(std::ifstream& stream, Vector2<T>& obj)
	{
		stream >> obj.x;
		stream >> obj.y;
		return stream;
	};

	Vector2(){};
	Vector2(T inVal) : x{ inVal }, y{ inVal }{};
	Vector2(T inX, T inY) : x{inX}, y{inY}{};

	T x{0};
	T y{0};

	float Angle() const {return atan2(x, y); };
	float Length() const { return sqrt((x * x) + (y * y)); };
	float LengthSqr() const {return (x * x) + (y * y); };
	Vector2<T> Normal() const {return Vector2<T>(-y, x); };
	Vector2<T> Rotate(float angle) const { return Vector2<T>(x * cos(angle) - y * sin(angle), y * cos(angle) + x * sin(angle));};

	// scalar operators
	Vector2<T> operator*(const float& scalar) const {return Vector2<T>(x * scalar, y * scalar); };
	Vector2<T> operator*=(const float& scalar) {*this = *this * scalar; return *this;};
	Vector2<T> operator/(const float& scalar) const { return Vector2<T>(x / scalar, y / scalar); };
	Vector2<T> operator/=(const float& scalar) { *this = *this / scalar; return *this; };

	// vector operators
	Vector2<T> operator+(const Vector2<T>& other) const { return Vector2<T>(x + other.x, y + other.y); };
	Vector2<T> operator+=(const Vector2<T>& other) { *this = *this + other; return *this; };
	Vector2<T> operator-(const Vector2<T>& other) const { return Vector2<T>(x - other.x, y - other.y); };
	Vector2<T> operator-=(const Vector2<T>& other) { *this = *this - other; return *this; };

	// Explicit Conversions Only
	Vector2<int> ToInt() const {return Vector2<int>(static_cast<int>(x), static_cast<int>(y)); };
	Vector2<float> ToFloat() const {return Vector2<float>(static_cast<float>(x), static_cast<float>(y)); };
	Vector2<double> ToDouble() const {return Vector2<double>(static_cast<double>(x), static_cast<double>(y)); };
};

template <typename T>
struct Vector3
{
	static_assert(std::is_same<T, int>::value
		|| std::is_same<T, float>::value
		|| std::is_same<T, double>::value,
		"Vector3 only supports the following types: int, float, double");

	friend std::ofstream& operator<<(std::ofstream& stream, const Vector3<T>& obj)
	{
		stream << obj.x << obj.y << obj.z;
		return stream;
	}
	friend std::ifstream& operator>>(std::ifstream& stream, Vector3<T>& obj)
	{
		stream >> obj.x;
		stream >> obj.y;
		stream >> obj.z;
		return stream;
	};

	Vector3() {};
	Vector3(T inVal) : x{ inVal }, y{ inVal }, z{ inVal } {};
	Vector3(T inX, T inY, T inZ) : x{ inX }, y{ inY }, z{ inZ } {};

	T x{ 0 };
	T y{ 0 };
	T z{ 0 };

	float Length() const { return sqrt((x * x) + (y * y) + (z * z)); };
	float LengthSqr() const { return (x * x) + (y * y) + (z * z); };

	// scalar operators
	Vector3<T> operator*(const float& scalar) const { return Vector2<T>(x * scalar, y * scalar, z * scalar); };
	Vector3<T> operator*=(const float& scalar) { *this = *this * scalar; return *this; };
	Vector3<T> operator/(const float& scalar) const { return Vector2<T>(x / scalar, y / scalar, z / scalar); };
	Vector3<T> operator/=(const float& scalar) { *this = *this / scalar; return *this; };

	// vector operators
	Vector3<T> operator+(const Vector2<T>& other) const { return Vector2<T>(x + other.x, y + other.y, z + other.z); };
	Vector3<T> operator+=(const Vector2<T>& other) { *this = *this + other; return *this; };
	Vector3<T> operator-(const Vector2<T>& other) const { return Vector2<T>(x - other.x, y - other.y, z - other.z); };
	Vector3<T> operator-=(const Vector2<T>& other) { *this = *this - other; return *this; };

	// Explicit Conversions Only
	Vector3<int> ToInt() const { return Vector2<int>(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z)); };
	Vector3<float> ToFloat() const { return Vector2<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)); };
	Vector3<double> ToDouble() const { return Vector2<double>(static_cast<double>(x), static_cast<double>(y), static_cast<double>(z)); };
};

template <typename T>
Vector2<T> operator*(double scalar, const Vector2<T>& vec) {return Vector2<T>(vec.x * scalar, vec.y * scalar); };
template <typename T>
Vector3<T> operator*(double scalar, const Vector3<T>& vec) { return Vector3<T>(vec.x * scalar, vec.y * scalar, vec.z * scalar); };

template <typename T>
float Dot(const Vector2<T>& a, const Vector2<T>& b) { return (a.x * b.x) + (a.y * b.y); };
template <typename T>
float Dot(const Vector3<T>& a, const Vector3<T>& b) { return (a.x * b.x) + (a.y * b.y) + (a.z * b.z); };

template <typename T>
float Cross(const Vector2<T>& a, const Vector2<T>& b) { return (a.x * b.y) - (a.y * b.x); };
template <typename T>
Vector3<T> Cross(const Vector3<T>& a, const Vector3<T>& b) { return Vector3<T>(	(a.y * b.z) - (a.z * b.y),
																				(a.x * b.z) - (a.z * b.x),
																				(a.x * b.y) - (a.y * b.x)); };

template <typename T>
Vector2<T> Normalize(const Vector2<T>& vec) { return vec / vec.Length(); };
template <typename T>
Vector3<T> Normalize(const Vector3<T>& vec) { return vec / vec.Length(); };

template <typename T>
Vector2<T> NormalizeNonZero(const Vector2<T>& vec) { return vec.Length() ? vec / vec.Length() : vec; };
template <typename T>
Vector3<T> NormalizeNonZero(const Vector3<T>& vec) { return vec.Length() ? vec / vec.Length() : vec; };

template <typename T>
Vector2<T> Projection(const Vector2<T>& vecToProject, const Vector2<T>& vecTarget) { return Normalize(vecTarget) * Dot(vecToProject, Normalize(vecTarget)); };
template <typename T>
Vector3<T> Projection(const Vector3<T>& vecToProject, const Vector3<T>& vecTarget) { return Normalize(vecTarget) * Dot(vecToProject, Normalize(vecTarget)); };

template <typename T>
Vector2<T> Reflection(const Vector2<T>& vecToReflect, const Vector2<T>& vecSurface) { return (vecToReflect - (2.f * Projection(vecToReflect, vecSurface.Normal()))); };

template <typename T>
bool VectorIntersection2D(const Vector2<T>& posA, const Vector2<T>& vecA, const Vector2<T>& posB, const Vector2<T>& vecB, Vector2<T>& outIntersection)
{
	float abCross{ Cross(vecA, vecB) };
	if (abCross == 0.f)
	{
		return false;
	}

	// calc scalars equal to intersection point on each line
	float intersectScalarA{ Cross(posB - posA, vecB) / abCross };
	float intersectScalarB{ Cross(posB - posA, vecA) / abCross };

	// output intersection position
	outIntersection = posA + (vecA * intersectScalarA);

	// if both scalars are within length of original vectors, return true
	return (intersectScalarA >= 0 && intersectScalarA <= 1
		&& intersectScalarB >= 0 && intersectScalarB <= 1);
};

// Define common vector types
using Vector2i = Vector2<int>;
using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;

using Vector3i = Vector3<int>;
using Vector3f = Vector3<float>;
using Vector3d = Vector3<double>;

template <typename T> int Sign(T val) 
{
	return (T(0) < val) - (val < T(0));
}

float Lerp(const float lerpKey, const Vector2f* keyValueArray, int arraySize);
float Lerp(const float lerpKey, const Vector2f& keyValueA, const Vector2f& keyValueB);