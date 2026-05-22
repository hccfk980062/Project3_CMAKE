#include "MainScene.h"

namespace CG
{
	MainScene::MainScene()
	{
		camera = nullptr;
		mesh = nullptr;
	}

	MainScene::~MainScene()
	{}

	auto MainScene::Initialize(int width, int height) -> bool
	{
		camera = new Camera(glm::vec3(0, 0, 1));
		camera->configureLookAt(glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
		camera->SetProjectionMatrix(width, height);

		return LoadScene();
	}

	void MainScene::Update(double dt)
	{

	}

	void MainScene::Render(int screenWidth, int screenHeight)
	{
		glClearColor(0.0, 0.0, 0.0, 1); //black screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mesh->Render(camera->GetProjectionMatrix(), camera->GetViewMatrix());

		if (isFaceSelected)
		{
			std::vector<glm::vec3> triangleCoords;
			glm::vec3 trianglePointCoord;

			for (auto vh : selectedFace.vertices())
			{
				auto point = mesh->point(vh);
				triangleCoords.push_back(glm::vec3(point[0], point[1], point[2]));
			}
			{
				auto point = mesh->point(selectedVertex);
				trianglePointCoord = glm::vec3(point[0], point[1], point[2]);
			}



			glm::mat4 view = camera->GetViewMatrix();
			glm::mat4 projection = camera->GetProjectionMatrix();
			glm::vec4 viewport = glm::vec4(0, 0, screenWidth, screenHeight);

			// 核心函數
			glm::vec3 screenPos0 = glm::project(triangleCoords[0], view, projection, viewport);
			glm::vec3 screenPos1 = glm::project(triangleCoords[1], view, projection, viewport);
			glm::vec3 screenPos2 = glm::project(triangleCoords[2], view, projection, viewport);

			glm::vec3 screenPos_vertex = glm::project(trianglePointCoord, view, projection, viewport);

			// 保存當前矩陣狀態
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0.0, screenWidth, screenHeight, 0.0, -1.0, 1.0);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			glDisable(GL_DEPTH_TEST);
			glColor3f(1.0f, 0.0f, 0.0f); // 紅色
			glLineWidth(2.0f);

			// Y 座標需要翻轉（glm::project 返回的 Y 從底部開始，螢幕座標從頂部開始）
			glBegin(GL_LINE_LOOP);
			glVertex2f(screenPos0[0], screenHeight - screenPos0[1]); // 頂點 1
			glVertex2f(screenPos1[0], screenHeight - screenPos1[1]); // 頂點 2
			glVertex2f(screenPos2[0], screenHeight - screenPos2[1]); // 頂點 3
			glEnd();

			glBegin(GL_LINE_LOOP);
			glVertex2f(screenPos_vertex[0] + 10, screenHeight - screenPos_vertex[1] + 10); // 頂點 1
			glVertex2f(screenPos_vertex[0] + 10, screenHeight - screenPos_vertex[1] - 10); // 頂點 2
			glVertex2f(screenPos_vertex[0] - 10, screenHeight - screenPos_vertex[1] - 10); // 頂點 3
			glVertex2f(screenPos_vertex[0] - 10, screenHeight - screenPos_vertex[1] + 10); // 頂點 4
			glEnd();

			glLineWidth(1.0f);
			glEnable(GL_DEPTH_TEST);

			// 恢復矩陣狀態
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		}
	}

	void MainScene::OnResize(int width, int height)
	{
		std::cout << "MainScene Resize: " << width << " " << height << std::endl;
		camera->SetProjectionMatrix(width, height);
	}

	void MainScene::RayCastTest(glm::vec2 mousePosRel, int display_w, int display_h)
	{
		// 取得近平面上的世界座標（z = 0.0）
		glm::vec3 nearPointCoord = glm::unProject(
			glm::vec3(mousePosRel.x, display_h - mousePosRel.y, 0),
			camera->GetViewMatrix(),
			camera->GetProjectionMatrix(),
			glm::vec4(0, 0, display_w, display_h)
		);

		// 取得遠平面上的世界座標（z = 1.0）
		glm::vec3 farPointCoord = glm::unProject(
			glm::vec3(mousePosRel.x, display_h - mousePosRel.y, 1),
			camera->GetViewMatrix(),
			camera->GetProjectionMatrix(),
			glm::vec4(0, 0, display_w, display_h)
		);


		// 射線方向(向量)
		glm::vec3 rayDirection = glm::normalize(farPointCoord - nearPointCoord);

		std::cout << "RayDirection: (" << rayDirection.x << ", " << rayDirection.y << ", " << rayDirection.z << ")\n";

		float closestDistance = std::numeric_limits<float>::max();

		for (auto& face : mesh->faces())
		{
			std::vector<glm::vec3> triangleCoords;

			for (auto vh : face.vertices())
			{
				auto point = mesh->point(vh);
				triangleCoords.push_back(glm::vec3(point[0], point[1], point[2]));
			}

			const float EPSILON = 1e-8;

			// 1. 計算兩條邊向量
			glm::vec3 triangleVec1 = triangleCoords[1] - triangleCoords[0];
			glm::vec3 triangleVec2 = triangleCoords[2] - triangleCoords[0];

			// 2. 計算行列式（判斷射線是否平行三角形面）
			glm::vec3 h = glm::cross(rayDirection, triangleVec2);
			float det = glm::dot(triangleVec1, h);
			if (std::abs(det) < EPSILON) continue; // 平行，不相交

			float invDet = 1.0f / det;

			// 3. 計算 u（重心座標第一分量）
			glm::vec3 s = nearPointCoord - triangleCoords[0];
			float u = glm::dot(s, h) * invDet;
			if (u < 0 || u > 1) continue;

			// 4. 計算 v（重心座標第二分量）
			glm::vec3 q = glm::cross(s, triangleVec1);
			float v = glm::dot(rayDirection, q) * invDet;
			if (v < 0 || u + v > 1) continue;

			// 5. 計算 t（射線上的距離參數）
			float t = glm::dot(triangleVec2, q) * invDet;
			if (t < 0) continue; // 交點在射線出發點後方

			isFaceSelected = true;
			if (t < closestDistance)
			{
				closestDistance = t;
				selectedFace = face;
				std::cout << "Collide with distance: " << t << "\n";
			}
		}

		if (isFaceSelected)
		{
			// 計算選中三角形中最接近鼠標點擊位置的頂點
			std::vector<glm::vec3> triangleCoords;
			std::vector<OpenMesh::SmartVertexHandle> vertices;

			for (auto vh : selectedFace.vertices())
			{
				auto point = mesh->point(vh);
				triangleCoords.push_back(glm::vec3(point[0], point[1], point[2]));
				vertices.push_back(vh);
			}

			glm::vec4 viewport = glm::vec4(0, 0, display_w, display_h);
			glm::mat4 view = camera->GetViewMatrix();
			glm::mat4 projection = camera->GetProjectionMatrix();

			float closestVertexDistance = std::numeric_limits<float>::max();
			OpenMesh::SmartVertexHandle closestVertex = vertices[0];

			// 計算每個頂點到鼠標點擊位置的距離
			for (size_t i = 0; i < triangleCoords.size(); ++i)
			{
				glm::vec3 screenPos = glm::project(triangleCoords[i], view, projection, viewport);
				glm::vec2 vertexScreenPos(screenPos.x, display_h - screenPos.y);

				float distance = glm::distance(vertexScreenPos, mousePosRel);

				if (distance < closestVertexDistance)
				{
					closestVertexDistance = distance;
					closestVertex = vertices[i];
				}
			}

			selectedVertex = closestVertex;
			std::cout << "Selected vertex with distance: " << closestVertexDistance << "\n";
		}
		else
		{
			std::cout << "No collision detected\n";
		}
	}

	auto MainScene::LoadScene() -> bool
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_MULTISAMPLE);
		glDepthMask(GL_TRUE);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		mesh = new TriMesh();
		mesh->LoadFromFile("./res/models/xyzrgb_dragon_100k.obj");

		return true;
	}
}