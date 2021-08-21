#pragma once

#include "core.hpp"

class Vector
{
public:
	Vector();
	Vector( float x, float y, float z );
	Vector( const Vector& v );
	Vector( const float* v );

public:
	void Set( float x = 0.0f, float y = 0.0f, float z = 0.0f );

	bool IsZero( float tolerance = 0.01f );

	void clear()
	{
		x = 0.f;
		y = 0.f;
		z = 0.f;
	}

	float Normalize();

	float Dot( const Vector& v ) const;

	float LengthSquared() const;
	float Length() const;
	float Length2D() const;
	float NormalizeInPlace() const;

	FORCEINLINE void ValidateVector()
	{
		if (std::isnan(this->x)
			|| std::isnan(this->y)
			|| std::isnan(this->z))
			this->Set();

		if (std::isinf(this->x)
			|| std::isinf(this->y)
			|| std::isinf(this->z))
			this->Set();
	}


	float DistanceSquared( const Vector& v ) const;
	float Distance( const Vector& v ) const;

	Vector Normalized();
	Vector Cross( const Vector& v );

	Vector2D ToVector2D();
	Vector4D ToVector4D( float w = 0.0f );
	void Clamp();
	Vector ToVectors(Vector * side, Vector * up);

	Vector ToVectorsTranspose(Vector * side, Vector * up);

	QAngle ToEulerAngles( Vector* pseudoup = nullptr );

public:
	float operator [] ( const std::uint32_t index ) const;
	float &operator [] ( const std::uint32_t index );

	Vector& operator = ( const Vector& v );
	Vector& operator = ( const float* v );

	bool operator ==(const Vector & src);
	bool operator != (const Vector& v);

	Vector& operator += ( const Vector& v );
	Vector& operator -= ( const Vector& v );
	Vector& operator *= ( const Vector& v );
	Vector& operator /= ( const Vector& v );

	Vector& operator += ( float fl );
	Vector& operator -= ( float fl );
	Vector& operator *= ( float fl );
	Vector& operator /= ( float fl );

	Vector operator + ( const Vector& v ) const;
	Vector operator - ( const Vector& v ) const;
	Vector operator * ( const Vector& v ) const;
	Vector operator / ( const Vector& v ) const;

	Vector operator + ( float fl ) const;
	Vector operator - ( float fl ) const;
	Vector operator * ( float fl ) const;
	Vector operator * (int v) const;
	Vector operator / ( float fl ) const;

public:
	static Vector Zero;

public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

//using QAngle = Vector;