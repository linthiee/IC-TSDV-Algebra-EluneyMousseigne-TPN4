#pragma once

#include "raylib.h"
#include "raymath.h"

struct Plane
{
	Vector3 normal;
	float distance;
};

struct Frustum
{
	Plane planes[6];
};

struct MyAABB
{
	Vector3 min;
	Vector3 max;
};

bool IsAABBInFrustum(Frustum& frustum, MyAABB& aabb);
bool IsMeshInFrustum(Frustum& frustum, Mesh mesh, Matrix transform);
void UpdateFrustum(Frustum& frustum, Camera camera, float aspect, float nearDist, float farDist);
MyAABB CalculateLocalAABB(Mesh mesh);
MyAABB GetUpdatedAABB(MyAABB localBB, Matrix transform);