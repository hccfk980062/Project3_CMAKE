#include <stb_image.h>

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

		glm::mat4 proj = camera->GetProjectionMatrix();
		glm::mat4 view = camera->GetViewMatrix();

		mesh->Render(proj, view);

		// Sticker decal pass
		if (sticker.enabled && sticker.textureID != 0)
		{
			glm::vec3 right, up;
			GetStickerProjectionVectors(right, up);

			float aspect = (sticker.texW > 0 && sticker.texH > 0)
				? (float)sticker.texH / (float)sticker.texW
				: 1.0f;
			glm::vec2 halfSize(sticker.scale, sticker.scale * aspect);

			mesh->RenderSticker(
				proj, view,
				sticker.textureID,
				sticker.center, right, up,
				halfSize, glm::radians(sticker.rotation),
				sticker.offset, sticker.repeat, sticker.blend);
		}

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

			glm::vec4 viewport = glm::vec4(0, 0, screenWidth, screenHeight);

			glm::vec3 screenPos0 = glm::project(triangleCoords[0], view, proj, viewport);
			glm::vec3 screenPos1 = glm::project(triangleCoords[1], view, proj, viewport);
			glm::vec3 screenPos2 = glm::project(triangleCoords[2], view, proj, viewport);
			glm::vec3 screenPos_vertex = glm::project(trianglePointCoord, view, proj, viewport);

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0.0, screenWidth, screenHeight, 0.0, -1.0, 1.0);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			glDisable(GL_DEPTH_TEST);
			glColor3f(1.0f, 0.0f, 0.0f);
			glLineWidth(2.0f);

			glBegin(GL_LINE_LOOP);
			glVertex2f(screenPos0[0], screenHeight - screenPos0[1]);
			glVertex2f(screenPos1[0], screenHeight - screenPos1[1]);
			glVertex2f(screenPos2[0], screenHeight - screenPos2[1]);
			glEnd();

			glBegin(GL_LINE_LOOP);
			glVertex2f(screenPos_vertex[0] + 10, screenHeight - screenPos_vertex[1] + 10);
			glVertex2f(screenPos_vertex[0] + 10, screenHeight - screenPos_vertex[1] - 10);
			glVertex2f(screenPos_vertex[0] - 10, screenHeight - screenPos_vertex[1] - 10);
			glVertex2f(screenPos_vertex[0] - 10, screenHeight - screenPos_vertex[1] + 10);
			glEnd();

			glLineWidth(1.0f);
			glEnable(GL_DEPTH_TEST);

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
		glm::vec3 nearPointCoord = glm::unProject(
			glm::vec3(mousePosRel.x, display_h - mousePosRel.y, 0),
			camera->GetViewMatrix(),
			camera->GetProjectionMatrix(),
			glm::vec4(0, 0, display_w, display_h)
		);

		glm::vec3 farPointCoord = glm::unProject(
			glm::vec3(mousePosRel.x, display_h - mousePosRel.y, 1),
			camera->GetViewMatrix(),
			camera->GetProjectionMatrix(),
			glm::vec4(0, 0, display_w, display_h)
		);

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

			glm::vec3 triangleVec1 = triangleCoords[1] - triangleCoords[0];
			glm::vec3 triangleVec2 = triangleCoords[2] - triangleCoords[0];

			glm::vec3 h = glm::cross(rayDirection, triangleVec2);
			float det = glm::dot(triangleVec1, h);
			if (std::abs(det) < EPSILON) continue;

			float invDet = 1.0f / det;

			glm::vec3 s = nearPointCoord - triangleCoords[0];
			float u = glm::dot(s, h) * invDet;
			if (u < 0 || u > 1) continue;

			glm::vec3 q = glm::cross(s, triangleVec1);
			float v = glm::dot(rayDirection, q) * invDet;
			if (v < 0 || u + v > 1) continue;

			float t = glm::dot(triangleVec2, q) * invDet;
			if (t < 0) continue;

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
			// Record world-space hit point for sticker placement
			lastHitWorldPos = nearPointCoord + closestDistance * rayDirection;
			hasHitPoint = true;

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

	bool MainScene::LoadStickerTexture(const char* path)
	{
		stbi_set_flip_vertically_on_load(true);

		int w, h, channels;
		unsigned char* data = stbi_load(path, &w, &h, &channels, 4);
		if (!data)
		{
			std::cerr << "Failed to load sticker texture: " << path << "\n";
			return false;
		}

		if (sticker.textureID != 0)
			glDeleteTextures(1, &sticker.textureID);

		glGenTextures(1, &sticker.textureID);
		glBindTexture(GL_TEXTURE_2D, sticker.textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);

		sticker.texW = w;
		sticker.texH = h;
		glBindTexture(GL_TEXTURE_2D, 0);
		return true;
	}

	void MainScene::GetStickerProjectionVectors(glm::vec3& right, glm::vec3& up) const
	{
		switch (sticker.projAxis)
		{
		case 0:  // Camera-aligned (always faces current view)
			right = camera->Right;
			up    = camera->Up;
			break;
		case 1:  // Front  — project from +Z toward -Z
			right = { 1, 0,  0 };
			up    = { 0, 1,  0 };
			break;
		case 2:  // Back   — project from -Z toward +Z
			right = {-1, 0,  0 };
			up    = { 0, 1,  0 };
			break;
		case 3:  // Top    — project from +Y toward -Y (overhead)
			right = { 1, 0,  0 };
			up    = { 0, 0, -1 };
			break;
		case 4:  // Bottom — project from -Y toward +Y
			right = { 1, 0,  0 };
			up    = { 0, 0,  1 };
			break;
		case 5:  // Right side — project from +X toward -X
			right = { 0, 0, -1 };
			up    = { 0, 1,  0 };
			break;
		case 6:  // Left side — project from -X toward +X
			right = { 0, 0,  1 };
			up    = { 0, 1,  0 };
			break;
		default:
			right = { 1, 0, 0 };
			up    = { 0, 1, 0 };
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
		mesh->InitStickerShader();

		return true;
	}
}
