#pragma once
#include "sdk.hpp"

class matrix3x4a_t;
class RadianEuler;
class C_BasePlayer;
struct mstudiobbox_t;

namespace Math
{
	extern inline void SinCos(float radians, float *sine, float *cosine);
	extern float PickRandomAngle(float numangles, ...);
	extern float GuwopNormalize(float flAngle);
	extern bool IntersectBB(Vector& start, Vector& end, Vector& min, Vector& max);
	extern bool Intersect(Vector start, Vector end, Vector a, Vector b, float radius);
	extern matrix3x4_t AngleMatrix(const QAngle angles);
	extern Vector VectorRotate(const Vector& in1, const QAngle& in2);
	extern void VectorAngles(const Vector& forward, QAngle &angles);
	extern void VectorAngles(const Vector & vecForward, Vector & vecAngles);
	extern void VectorToAngles(const Vector & forward, const Vector & up, QAngle & angles);
	extern void AngleVectors(const QAngle& angles, Vector *forward);
	extern void AngleVectors(const QAngle &angles, Vector *forward, Vector *right, Vector *up);
	extern void TransformAABB( matrix3x4_t & transform, const Vector & vecMinsIn, const Vector & vecMaxsIn, Vector & vecMinsOut, Vector & vecMaxsOut);
	extern void VectorMin(const Vector & a, const Vector & b, Vector & result);
	extern void VectorMax(const Vector & a, const Vector & b, Vector & result);
	extern bool IntersectionBoundingBox(const Vector & src, const Vector & dir, const Vector & min, const Vector & max, Vector* hit_point = nullptr);
	extern bool IntersectSegmentSphere(const Vector & vecRayOrigin, const Vector & vecRayDelta, const Vector & vecSphereCenter, float flRadius);
	extern bool IntersectSegmentCapsule(const Vector & start, const Vector & end, const Vector & min, const Vector & max, float radius);
	extern void VectorMAInline(const Vector & start, float scale, const Vector & direction, Vector & dest);
	extern void VectorMA(const Vector & start, float scale, const Vector & direction, Vector & dest);
	extern void AngleVectors(const Vector & angles, Vector * forward, Vector * right = nullptr, Vector * up = nullptr);
	extern void VectorAngles(const Vector & forward, Vector & up, QAngle & angles);
	extern QAngle CalcAngle(Vector v1, Vector v2);
	extern void CalcAngle(const Vector & vecSource, const Vector & vecDestination, QAngle & qAngles);
	extern float GetFov(const QAngle& viewAngle, const QAngle& aimAngle);
	extern float GetFov(const Vector & viewAngle, const Vector & aimAngle);
	extern float VectorDistance(Vector v1, Vector v2);
	extern void VectorTransform(Vector& in1, matrix3x4a_t& in2, Vector &out);
	extern void VectorTransform(Vector & in1, matrix3x4_t & in2, Vector & out);
	extern void MatrixPosition(const matrix3x4_t & matrix, Vector & position);
	extern void VectorSubtract(const Vector & a, const Vector & b, Vector & c);
	extern void VectorAdd(const Vector & a, const Vector & b, Vector & c);
	extern void QAngleTransform(Vector & in1, matrix3x4a_t & in2, QAngle & out);
	extern float GetDelta(float hspeed, float maxspeed, float airaccelerate);
	extern float NormalizeFloat(float value);
	extern float NormalizeFloatInPlace(float f);
	extern void MatrixGetColumn(const matrix3x4_t & in, int column, Vector & out);
	extern void MatrixSetColumn(const Vector & in, int column, matrix3x4_t & out);
	extern void AngleMatrix(RadianEuler const & angles, const Vector & position, matrix3x4_t & matrix);
	extern void AngleMatrix(const RadianEuler & angles, matrix3x4_t & matrix);
	extern void AngleMatrix(const QAngle & angles, const Vector & position, matrix3x4_t & matrix);
	extern void MatrixCopy(const matrix3x4_t & in, matrix3x4_t & out);
	extern float AngleDiff(float destAngle, float srcAngle);
	extern void ConcatTransforms(const matrix3x4_t & in1, const matrix3x4_t & in2, matrix3x4_t & out);
	extern void AngleMatrix(const QAngle & angles, matrix3x4_t & matrix);
	extern void MatrixAngles(const matrix3x4_t & matrix, QAngle & angles);
	extern void MatrixAngles(const matrix3x4_t & matrix, float * angles);
	extern void MatrixAngles(const matrix3x4_t & matrix, Vector * angles, Vector * forward);
	extern inline float RandFloat(float M, float N);

	// sperg cried about the previous method, 
	//here's not only a faster one but inaccurate as well to trigger more people
	inline float FASTSQRT(float x)
	{
		unsigned int i = *(unsigned int*)&x;

		i += 127 << 23;
		// approximation of square root
		i >>= 1;
		return *(float*)&i;
	}

	inline float NormalizePitch(float pitch)
	{
		while (pitch > 89.f)
			pitch -= 180.f;
		while (pitch < -89.f)
			pitch += 180.f;

		return pitch;
	}

	inline float NormalizeYaw(float yaw)
	{
		/*if (yaw > 180)
			yaw -= (round(yaw / 360) * 360.f);
		else if (yaw < -180)
			yaw += (round(yaw / 360) * -360.f);

		return yaw;*/
		return remainderf(yaw, 360.0f);
	}

	inline float normalize_angle(float angle)
	{
		/*if (angle > 180.f || angle < -180.f)
		{
			auto revolutions = round(abs(angle / 360.f));

			if (angle < 0.f)
				angle = angle + 360.f * revolutions;
			else
				angle = angle - 360.f * revolutions;
		}

		return angle;*/

		return remainderf(angle, 360.0f);
	}

	/*template <class T>
	FORCEINLINE T Lerp(float flPercent, T const &A, T const &B)
	{
		return A + (B - A) * flPercent;
	}*/

	template < typename t >
	FORCEINLINE t Lerp(const t& t1, const t& t2, float progress)
	{
		return t1 + (t2 - t1) * progress;
	}

	template< class T, class Y >
	FORCEINLINE T clamp(T const &val, Y const &minVal, Y const &maxVal)
	{
		if (val < minVal)
			return minVal;
		else if (val > maxVal)
			return maxVal;
		else
			return val;
	}
}