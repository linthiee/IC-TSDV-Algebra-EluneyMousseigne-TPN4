#include "raylib.h"
#include "raymath.h"

#include <vector>

struct Plane
{
	Vector3 normal;
	float distance;
};

struct Frustum
{
	Plane planes[6]; // izquierda, derecha, abajo, arriba, near, far
};

struct SceneObject
{
	Model model;
	Vector3 position;
	//BoundingBox AABB; cambiar por propio bounding box
	bool isVisible;
};

void main()
{
	SceneObject objects;

	Frustum cameraFrustum;

	Camera camera = { 0 };
	camera.position = { 10.0f, 5.0f, 10.0f };
	camera.target = { 0.0f, 0.0f, 0.0f };  
	camera.up = { 0.0f, 1.0f, 0.0f };        
	camera.fovy = 45.0f;                     
	camera.projection = CAMERA_PERSPECTIVE;   

	float nearPlane = 0.1f;
	float farPlane = 1000.0f; 

	std::vector<SceneObject> sceneObjects;

	InitWindow(800, 600, "Frustum Culling");

	objects.model = LoadModel("res/cube.obj");
	//objects.AABB = GetModelBoundingBox(objects.model); cambiar por propia implementacion de AABB
	objects.position = { 10.0f, 1.0f, 10.0f };
	objects.isVisible = true;

	while (!WindowShouldClose())
	{
		UpdateCamera(&camera, CAMERA_ORBITAL);

		float fov = camera.fovy * DEG2RAD;

		float aspectRatio = (float)GetScreenWidth() / (float)GetScreenHeight();

		Matrix view = GetCameraMatrix(camera);
		Matrix projection = MatrixPerspective(fov, aspectRatio, nearPlane, farPlane);
		Matrix viewProjection = MatrixMultiply(view, projection);

		BeginDrawing();
		ClearBackground(RAYWHITE);

		BeginMode3D(camera);

		DrawModel(objects.model, objects.position, 1.0f, WHITE);
		DrawModelWires(objects.model, objects.position, 1.0f, BLACK);

		DrawGrid(20, 1.0f);
		EndMode3D();

		EndDrawing();
	}
}
