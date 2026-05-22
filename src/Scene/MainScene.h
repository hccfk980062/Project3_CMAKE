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

		Camera* camera;
	private:
		auto LoadScene() -> bool;

	private:


		TriMesh* mesh;
		bool isFaceSelected = false;


		OpenMesh::SmartFaceHandle selectedFace;
		OpenMesh::SmartVertexHandle selectedVertex;

	};
}

