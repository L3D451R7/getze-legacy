#pragma once

#include "core.hpp"

class Vector;

class QAngle
{
public:
	QAngle();
	QAngle( float x, float y, float z );
	QAngle( const QAngle& v );
	QAngle( const float* v );

public:
	void Set( float x = 0.0f, float y = 0.0f, float z = 0.0f );

	void Normalize();
	void Clamp();

	void clear()
	{
		x = 0.f;
		y = 0.f;
		z = 0.f;
	}

	bool IsZero( float tolerance = 0.01f );

	QAngle Normalized();
	QAngle Clamped();

	Vector ToVectors( Vector* side = nullptr, Vector* up = nullptr );
	Vector NiceCode();
	Vector ToVectorsTranspose( Vector* side = nullptr, Vector* up = nullptr );

public:
	float operator [] ( const std::uint32_t index );
	const float operator [] ( const std::uint32_t index ) const;

	QAngle& operator = ( const QAngle& v );
	QAngle& operator = ( const float* v );
	bool	operator == (const QAngle& v);
	bool	operator != (const QAngle& v);

	QAngle& operator += ( const QAngle& v );
	QAngle& operator -= ( const QAngle& v );
	QAngle& operator *= ( const QAngle& v );
	QAngle& operator /= ( const QAngle& v );

	QAngle& operator += ( float fl );
	QAngle& operator -= ( float fl );
	QAngle& operator *= ( float fl );
	QAngle& operator /= ( float fl );

	QAngle operator + ( const QAngle& v ) const;
	QAngle operator - ( const QAngle& v ) const;
	QAngle operator * ( const QAngle& v ) const;
	QAngle operator / ( const QAngle& v ) const;

	QAngle operator + ( float fl ) const;
	QAngle operator - ( float fl ) const;
	QAngle operator * ( float fl ) const;
	QAngle operator / ( float fl ) const;

public:
	static QAngle Zero;

public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};