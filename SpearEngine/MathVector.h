#pragma once

struct Vector2D
{
	Vector2D(){};
	Vector2D(float inX, float inY);

	float x{0.f};
	float y{0.f};

	float Angle() const;
	float Length() const;
	float LengthSqr() const;
	Vector2D Normal() const;
	Vector2D Rotate(float angle) const;

	// scalar operators
	Vector2D operator*(const float& scalar) const;
	Vector2D operator/(const float& scalar) const;

	// vector operators
	Vector2D operator+(const Vector2D& other) const;
	Vector2D operator-(const Vector2D& other) const;
};

Vector2D operator*(double scalar, const Vector2D& vec);

float Dot(const Vector2D& a, const Vector2D& b);
float Cross(const Vector2D& a, const Vector2D& b);
Vector2D Normalize(const Vector2D& vec);

Vector2D Projection(const Vector2D& vecToProject, const Vector2D& vecTarget);
Vector2D Reflection(const Vector2D& vecToReflect, const Vector2D& vecSurface);

bool VectorIntersection(const Vector2D& posA, const Vector2D& vecA, const Vector2D& posB, const Vector2D& vecB, Vector2D& outIntersection);