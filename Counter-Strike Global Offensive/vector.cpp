#include "vector.hpp"
#include "vector2d.hpp"
#include "vector4d.hpp"
#include "qangle.hpp"

#define CHECK_VALID( _v ) 0

Vector::Vector()
{
	this->Set();
}

Vector::Vector( float x, float y, float z )
{
	this->Set( x, y, z );
}

Vector::Vector( const Vector& v )
{
	this->Set( v.x, v.y, v.z );
}

Vector::Vector( const float* v )
{
	this->Set( v[0], v[1], v[2] );
}

void Vector::Set( float x /*= 0.0f*/, float y /*= 0.0f */, float z /*= 0.0f*/)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

bool Vector::IsZero( float tolerance /*= 0.01f */)
{
	return ( this->x > -tolerance && this->x < tolerance &&
			 this->y > -tolerance && this->y < tolerance &&
			 this->z > -tolerance && this->z < tolerance );
}

float Vector::Normalize()
{
	auto length = this->Length();

	( *this ) /= length;

	return length;
}

float Vector::Dot( const Vector& v ) const
{
	return ( this->x * v.x +
			 this->y * v.y +
			 this->z * v.z );
}

float Vector::LengthSquared() const
{
	return ( this->Dot( *this ) );
}

float Vector::Length() const
{
	return ( std::sqrt( this->LengthSquared() ) );
}

float Vector::Length2D() const
{
	CHECK_VALID(*this);

	float root = 0.0f;

	float sqst = x * x + y * y;

	__asm
	{
		sqrtss xmm0, sqst
		movss root, xmm0
	}

	return root;
}

float Vector::DistanceSquared( const Vector& v ) const
{
	return ( ( *this - v ).LengthSquared() );
}

float Vector::Distance( const Vector& v ) const
{
	return ( ( *this - v ).Length() );
}

Vector Vector::Normalized()
{
	Vector copy( *this );
	copy.Normalize();
	return copy;
}
 
float Vector::NormalizeInPlace() const
{
	Vector v = *this;

	float iradius = 1.f / (this->Length() + 1.192092896e-07F); //FLT_EPSILON

	v.x *= iradius;
	v.y *= iradius;
	v.z *= iradius;

	return iradius;
}

Vector Vector::Cross( const Vector& v )
{
	return { this->y * v.z - this->z * v.y,
			 this->z * v.x - this->x * v.z,
			 this->x * v.y - this->y * v.x };
}

Vector2D Vector::ToVector2D()
{
	return { this->x, this->y };
}

Vector4D Vector::ToVector4D( float w /*= 0.0f */)
{
	return { this->x, this->y, this->z, w };
}

QAngle Vector::ToEulerAngles( Vector* pseudoup /*= nullptr*/)
{
	auto pitch = 0.0f;
	auto yaw = 0.0f;
	auto roll = 0.0f;

	auto length = this->ToVector2D().Length();

	if( pseudoup )
	{
		auto left = pseudoup->Cross( *this );

		left.Normalize();

		pitch = ToDegrees( std::atan2( -this->z, length ) );

		if( pitch < 0.0f )
			pitch += 360.0f;

		if( length > 0.001f )
		{
			yaw = ToDegrees( std::atan2( this->y, this->x ) );

			if( yaw < 0.0f )
				yaw += 360.0f;

			auto up_z = ( this->x * left.y ) - ( this->y * left.x );

			roll = ToDegrees( std::atan2( left.z, up_z ) );

			if( roll < 0.0f )
				roll += 360.0f;
		}
		else
		{
			yaw = ToDegrees( std::atan2( -left.x, left.y ) );

			if( yaw < 0.0f )
				yaw += 360.0f;
		}
	}
	else
	{
		if( this->x == 0.0f && this->y == 0.0f )
		{
			if( this->z > 0.0f )
				pitch = 270.0f;
			else
				pitch = 90.0f;
		}
		else
		{
			pitch = ToDegrees( std::atan2( -this->z, length ) );

			if( pitch < 0.0f )
				pitch += 360.0f;

			yaw = ToDegrees( std::atan2( this->y, this->x ) );

			if( yaw < 0.0f )
				yaw += 360.0f;
		}
	}

	return { pitch, yaw, roll };
}

float Vector::operator [] ( const std::uint32_t index ) const
{
	return ( ( ( float* )this )[index] );
}

float &Vector::operator [] ( const std::uint32_t index )
{
	return ( ( ( float* )this )[index] );
}

Vector& Vector::operator = ( const Vector& v )
{
	this->Set( v.x, v.y, v.z );

	return ( *this );
}

Vector& Vector::operator = ( const float* v )
{
	this->Set( v[0], v[1], v[2] );

	return ( *this );
}

bool Vector::operator == (const Vector& src)
{
	return (src.x == x) && (src.y == y) && (src.z == z);
}

bool Vector::operator != (const Vector& src)
{
	return (src.x != x) && (src.y != y) && (src.z != z);
}

Vector& Vector::operator += ( const Vector& v )
{
	this->x += v.x;
	this->y += v.y;
	this->z += v.z;

	return ( *this );
}

Vector& Vector::operator -= ( const Vector& v )
{
	this->x -= v.x;
	this->y -= v.y;
	this->z -= v.z;

	return ( *this );
}

Vector& Vector::operator *= ( const Vector& v )
{
	this->x *= v.x;
	this->y *= v.y;
	this->z *= v.z;

	return ( *this );
}

Vector& Vector::operator /= ( const Vector& v )
{
	this->x /= v.x;
	this->y /= v.y;
	this->z /= v.z;

	return ( *this );
}

Vector& Vector::operator += ( float fl )
{
	this->x += fl;
	this->y += fl;
	this->z += fl;

	return ( *this );
}

Vector& Vector::operator -= ( float fl )
{
	this->x -= fl;
	this->y -= fl;
	this->z -= fl;

	return ( *this );
}

Vector& Vector::operator *= ( float fl )
{
	this->x *= fl;
	this->y *= fl;
	this->z *= fl;

	return ( *this );
}

Vector& Vector::operator /= ( float fl )
{
	this->x /= fl;
	this->y /= fl;
	this->z /= fl;

	return ( *this );
}

Vector Vector::operator + ( const Vector& v ) const
{
	return { this->x + v.x,
			 this->y + v.y,
			 this->z + v.z };
}

Vector Vector::operator - ( const Vector& v ) const
{
	return { this->x - v.x,
			 this->y - v.y,
			 this->z - v.z };
}

Vector Vector::operator * ( const Vector& v ) const
{
	return { this->x * v.x,
			 this->y * v.y,
			 this->z * v.z };
}

Vector Vector::operator * (int v) const
{
	return { this->x * v,
		this->y * v,
		this->z * v};
}

Vector Vector::operator / ( const Vector& v ) const
{
	return { this->x / v.x,
			 this->y / v.y,
			 this->z / v.z };
}

Vector Vector::operator + ( float fl ) const
{
	return { this->x + fl,
			 this->y + fl,
			 this->z + fl };
}

Vector Vector::operator - ( float fl ) const
{
	return { this->x - fl,
			 this->y - fl,
			 this->z - fl };
}

Vector Vector::operator * ( float fl ) const
{
	return { this->x * fl,
			 this->y * fl,
			 this->z * fl };
}

Vector Vector::operator / ( float fl ) const
{
	return { this->x / fl,
			 this->y / fl,
			 this->z / fl };
}

Vector Vector::Zero( 0.0f, 0.0f, 0.0f );