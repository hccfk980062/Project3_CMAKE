#pragma once

#include <array>
#include <string>
#include <map>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Component/Camera.h>
#include <Component/TriMesh.h>

constexpr auto PARTSNUM = 18;

namespace CG
{
	struct StickerState
	{
		bool      enabled    = true;
		GLuint    textureID  = 0;
		int       texW       = 0;
		int       texH       = 0;
		char      texPath[512] = "";

		// Projection plane
		int       projAxis   = 0;    // 0=Camera, 1=Front(+Z), 2=Back(-Z), 3=Top(+Y), 4=Bottom(-Y), 5=Right(+X), 6=Left(-X)
		glm::vec3 center     = glm::vec3(0.0f);

		// Sticker transform
		float     scale      = 0.3f; // half-size in world units
		float     rotation   = 0.0f; // degrees
		glm::vec2 offset     = glm::vec2(0.0f);
		glm::vec2 repeat     = glm::vec2(1.0f);
		float     blend      = 1.0f;
	};

	class MainScene
	{
	public:
		MainScene();
		~MainScene();

		auto Initialize(int width, int height) -> bool;
		void Update(double dt);
		void Render(int screenWidth, int screenHeight);

		void OnResize(int width, int height);
		void RayCastTest(glm::vec2 mousePosRel, int display_w, int display_h);

		bool LoadStickerTexture(const char* path);

		Camera* camera;

		// Public sticker state (read/written by ControlWindow UI)
		StickerState sticker;

		// Last left-click hit point on mesh surface
		glm::vec3 lastHitWorldPos = glm::vec3(0.0f);
		bool      hasHitPoint     = false;

	private:
		auto LoadScene() -> bool;
		void GetStickerProjectionVectors(glm::vec3& right, glm::vec3& up) const;

	private:
		TriMesh* mesh;
		bool isFaceSelected = false;

		OpenMesh::SmartFaceHandle   selectedFace;
		OpenMesh::SmartVertexHandle selectedVertex;
	};
}
