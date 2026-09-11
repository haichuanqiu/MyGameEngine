#pragma once
#include "Reflection.h"
class Vector3
{
	public:
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	private:
	
};
REFLECT(
	Vector3,

	FIELD(Vector3, x),
	FIELD(Vector3, y),
	FIELD(Vector3, z)
)
class Vector2Int
{
	public:
		int x = 0;
		int y = 0;
};
REFLECT(
	Vector2Int,

	FIELD(Vector2Int, x),
	FIELD(Vector2Int, y),
)
class Quaternion
{
	public:
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float w = 1.0f;
};

REFLECT(
	Quaternion,

	FIELD(Quaternion, x),
	FIELD(Quaternion, y),
	FIELD(Quaternion, z),
	FIELD(Quaternion, w)
	)