#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <string>
#include <cmath>

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

struct SceneObject
{
	Model model;
	Vector3 position;
	MyAABB aabb;
	bool isVisible;
};

bool IsAABBInFrustum(Frustum& frustum, MyAABB& aabb);
void UpdateFrustum(Frustum& frustum, Camera camera, float aspect, float nearDist, float farDist);
MyAABB CalculateLocalAABB(Mesh mesh);
MyAABB GetUpdatedAABB(MyAABB localBB, Matrix transform);
void DrawAABB(MyAABB aabb, Color color);

int main()
{
	const int screenWidth = 1600;
	const int screenHeight = 900;

	InitWindow(screenWidth, screenHeight, "Frustum Culling");
	SetWindowState(FLAG_WINDOW_RESIZABLE);

	Camera camera = { 0 };
	camera.position = { 20.0f, 15.0f, 20.0f };
	camera.target = { 0.0f, 0.0f, 0.0f };
	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	float nearPlane = 2.5f;
	float farPlane = 50.0f;

	std::vector<SceneObject> sceneObjects;
	Frustum cameraFrustum;

	SceneObject obj1;
	obj1.model = LoadModel("res/decahedron.obj");
	obj1.position = { 5.0f, 5.0f, 5.0f };
	obj1.aabb = CalculateLocalAABB(*obj1.model.meshes);
	sceneObjects.push_back(obj1);

	SceneObject obj2;
	obj2.model = LoadModel("res/dodecahedron.obj");
	obj2.position = { 3.0f, 2.0f, 0.0f };
	obj2.aabb = CalculateLocalAABB(*obj2.model.meshes);
	sceneObjects.push_back(obj2);

	for (int i = 0; i < 50; i++)
	{
		SceneObject objRand;
		objRand.model = LoadModel("res/tetrahedron.obj");
		objRand.position = { (float)(GetRandomValue(-50, 50)), 0.0f, (float)(GetRandomValue(-50, 50)) };
		objRand.aabb = CalculateLocalAABB(*objRand.model.meshes);
		sceneObjects.push_back(objRand);
	}

	DisableCursor();

	while (!WindowShouldClose())
	{
		UpdateCamera(&camera, CAMERA_FREE);

		if (IsKeyPressed(KEY_Q))
		{
			camera.fovy += 2.0f;
		}
		if (IsKeyPressed(KEY_E))
		{
			camera.fovy -= 2.0f;
		}
		camera.fovy = Clamp(camera.fovy, 1.0f, 180.0f);

		if (IsKeyPressed(KEY_R))
		{
			nearPlane++;
		}
		if (IsKeyPressed(KEY_F))
		{
			nearPlane--;
		}
		nearPlane = Clamp(nearPlane, 0.0f, farPlane - 1.0f);

		if (IsKeyPressed(KEY_T))
		{
			farPlane++;
		}
		if (IsKeyPressed(KEY_G))
		{
			farPlane--;
		}
		farPlane = Clamp(farPlane, nearPlane + 1.0f, farPlane);

		float aspectRatio = (float)GetScreenWidth() / (float)GetScreenHeight();

		UpdateFrustum(cameraFrustum, camera, aspectRatio, nearPlane, farPlane);

		int visibleCount = 0;

		for (int i = 0; i < sceneObjects.size(); i++)
		{
			Matrix matTranslate = MatrixTranslate(sceneObjects[i].position.x, sceneObjects[i].position.y, sceneObjects[i].position.z);
			MyAABB worldAABB = GetUpdatedAABB(sceneObjects[i].aabb, matTranslate);

			sceneObjects[i].isVisible = IsAABBInFrustum(cameraFrustum, worldAABB);

			if (sceneObjects[i].isVisible)
			{
				visibleCount++;
			}
		}

		BeginDrawing();
		ClearBackground(RAYWHITE);

		BeginMode3D(camera);

		DrawGrid(100, 1.0f);

		for (int i = 0; i < sceneObjects.size(); i++)
		{
			Matrix matTranslate = MatrixTranslate(sceneObjects[i].position.x, sceneObjects[i].position.y, sceneObjects[i].position.z);
			MyAABB worldAABB = GetUpdatedAABB(sceneObjects[i].aabb, matTranslate);

			if (sceneObjects[i].isVisible)
			{
				DrawModel(sceneObjects[i].model, sceneObjects[i].position, 1.0f, RED);
				DrawModelWires(sceneObjects[i].model, sceneObjects[i].position, 1.0f, MAROON);
				DrawAABB(worldAABB, BLUE);
			}
			else
			{
				DrawModelWires(sceneObjects[i].model, sceneObjects[i].position, 1.0f, LIGHTGRAY);
				DrawAABB(worldAABB, GRAY);
			}
		}

		EndMode3D();

		DrawText(TextFormat("Visible objects: %d/%d", visibleCount, sceneObjects.size()), 10, 10, 20, BLACK);
		DrawText("WASD to Move, Mouse to Look, Q/E to Change FOV", 10, 35, 15, DARKGRAY);
		DrawText("R/F to Change NearPlane, T/G to Change FarPlane", 10, 55, 15, DARKGRAY);

		DrawText("FOV:", 10, 80, 15, DARKGRAY);
		DrawText(TextFormat("%s", std::to_string((int)camera.fovy).c_str()), 50, 80, 15, BLACK);
		DrawText("Planes:", 10, 95, 15, DARKGRAY);
		DrawText(TextFormat("%s", std::to_string((int)nearPlane).c_str()), 10, 125, 15, BLACK);
		DrawText(TextFormat("%s", std::to_string((int)farPlane).c_str()), 10, 150, 15, BLACK);

		EndDrawing();
	}

	for (int i = 0; i < sceneObjects.size() - 1; i++)
	{
		SceneObject& obj = sceneObjects[i];
		if (IsModelValid(obj.model)) 
		{
			UnloadModel(obj.model);
		}
	}

	CloseWindow();

	return 0;
}

bool IsAABBInFrustum(Frustum& frustum, MyAABB& aabb)
{
	for (int i = 0; i < 6; i++)
	{
		Vector3 vertex;

		if (frustum.planes[i].normal.x > 0)
		{
			vertex.x = aabb.max.x;
		}
		else 
		{
			vertex.x = aabb.min.x;
		}

		if (frustum.planes[i].normal.y > 0)
		{
			vertex.y = aabb.max.y;
		}
		else
		{
			vertex.y = aabb.min.y;
		}

		if (frustum.planes[i].normal.z > 0)
		{
			vertex.z = aabb.max.z;
		}
		else 
		{
			vertex.z = aabb.min.z;
		}

		if ((Vector3DotProduct(frustum.planes[i].normal, vertex) + frustum.planes[i].distance) < 0)
		{
			return false;
		}
	}
	return true;
}

void UpdateFrustum(Frustum& frustum, Camera camera, float aspect, float nearDist, float farDist)
{
	Vector3 forward = Vector3Subtract(camera.target, camera.position);
	forward = Vector3Normalize(forward);

	Vector3 right = Vector3CrossProduct(forward, camera.up);
	right = Vector3Normalize(right);

	Vector3 up = Vector3CrossProduct(right, forward);

	float halfHeight = tanf(camera.fovy * 0.5f * DEG2RAD);
	float halfWidth = halfHeight * aspect;

	Vector3 topSlope = Vector3Add(forward, Vector3Scale(up, halfHeight));
	frustum.planes[3].normal = Vector3Normalize(Vector3CrossProduct(topSlope, right)); // plano arriba

	Vector3 botSlope = Vector3Subtract(forward, Vector3Scale(up, halfHeight));
	frustum.planes[2].normal = Vector3Normalize(Vector3CrossProduct(right, botSlope));// plano abajo

	Vector3 leftSlope = Vector3Subtract(forward, Vector3Scale(right, halfWidth));
	frustum.planes[0].normal = Vector3Normalize(Vector3CrossProduct(leftSlope, up));// plano izquierda

	Vector3 rightSlope = Vector3Add(forward, Vector3Scale(right, halfWidth));
	frustum.planes[1].normal = Vector3Normalize(Vector3CrossProduct(up, rightSlope));// plano derecha

	frustum.planes[4].normal = forward;// plano near
	frustum.planes[5].normal = Vector3Negate(forward); // plano far

	frustum.planes[0].distance = -Vector3DotProduct(frustum.planes[0].normal, camera.position);
	frustum.planes[1].distance = -Vector3DotProduct(frustum.planes[1].normal, camera.position);
	frustum.planes[2].distance = -Vector3DotProduct(frustum.planes[2].normal, camera.position);
	frustum.planes[3].distance = -Vector3DotProduct(frustum.planes[3].normal, camera.position);

	Vector3 nearCenter = Vector3Add(camera.position, Vector3Scale(forward, nearDist));
	Vector3 farCenter = Vector3Add(camera.position, Vector3Scale(forward, farDist));

	frustum.planes[4].distance = -Vector3DotProduct(frustum.planes[4].normal, nearCenter);
	frustum.planes[5].distance = -Vector3DotProduct(frustum.planes[5].normal, farCenter);
}

MyAABB CalculateLocalAABB(Mesh mesh)
{
	MyAABB aabb;
	if (mesh.vertexCount == 0)
	{
		aabb.min = Vector3Zero();
		aabb.max = Vector3Zero();
		return aabb;
	}

	aabb.min = { mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };
	aabb.max = aabb.min;

	for (int i = 1; i < mesh.vertexCount; i++)
	{
		Vector3 v = { mesh.vertices[i * 3 + 0], mesh.vertices[i * 3 + 1], mesh.vertices[i * 3 + 2] };

		aabb.min.x = fminf(aabb.min.x, v.x);
		aabb.min.y = fminf(aabb.min.y, v.y);
		aabb.min.z = fminf(aabb.min.z, v.z);

		aabb.max.x = fmaxf(aabb.max.x, v.x);
		aabb.max.y = fmaxf(aabb.max.y, v.y);
		aabb.max.z = fmaxf(aabb.max.z, v.z);
	}
	return aabb;
}

MyAABB GetUpdatedAABB(MyAABB localBB, Matrix transform)
{
	Vector3 corners[8];
	corners[0] = { localBB.min.x, localBB.min.y, localBB.min.z };
	corners[1] = { localBB.max.x, localBB.min.y, localBB.min.z };
	corners[2] = { localBB.min.x, localBB.max.y, localBB.min.z };
	corners[3] = { localBB.min.x, localBB.min.y, localBB.max.z };
	corners[4] = { localBB.max.x, localBB.max.y, localBB.max.z };
	corners[5] = { localBB.min.x, localBB.max.y, localBB.max.z };
	corners[6] = { localBB.max.x, localBB.min.y, localBB.max.z };
	corners[7] = { localBB.max.x, localBB.max.y, localBB.min.z };

	for (int i = 0; i < 8; i++)
	{
		corners[i] = Vector3Transform(corners[i], transform);
	}

	Vector3 min = corners[0];
	Vector3 max = corners[0];
	for (int i = 1; i < 8; i++)
	{
		min.x = fminf(min.x, corners[i].x);
		min.y = fminf(min.y, corners[i].y);
		min.z = fminf(min.z, corners[i].z);

		max.x = fmaxf(max.x, corners[i].x);
		max.y = fmaxf(max.y, corners[i].y);
		max.z = fmaxf(max.z, corners[i].z);
	}

	return { min, max };
}

void DrawAABB(MyAABB aabb, Color color)
{
	Vector3 size = { aabb.max.x - aabb.min.x, aabb.max.y - aabb.min.y, aabb.max.z - aabb.min.z };
	Vector3 center = { aabb.min.x + size.x * 0.5f, aabb.min.y + size.y * 0.5f, aabb.min.z + size.z * 0.5f };
	DrawCubeWiresV(center, size, color);
}