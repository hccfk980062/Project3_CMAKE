#include <OpenMesh/Core/IO/MeshIO.hh>

#include "TriMesh.h"
#include <Utilty/LoadShaders.h>

namespace CG
{
	TriMesh::TriMesh()
	{
		model = glm::mat4(1.0);

		colorAmbient = glm::vec3(0.2, 0.2, 0.2);
		colorDiffuse = glm::vec3(1.0, 1.0, 0.2);
		colorSpecular = glm::vec3(1.0, 1.0, 1.0);
		colorLine = glm::vec3(0.8, 0.8, 0.8);
	}

	TriMesh::~TriMesh()
	{

	}

	bool TriMesh::LoadFromFile(std::string filename)
	{
		OpenMesh::IO::Options opt = OpenMesh::IO::Options::VertexNormal;
		bool isRead = OpenMesh::IO::read_mesh(*this, filename, opt);

		if (isRead)
		{
			// If the file did not provide vertex normals and mesh has vertex normal, then calculate them
			if (!opt.check(OpenMesh::IO::Options::VertexNormal) && this->has_vertex_normals())
			{
				this->update_normals();
			}

			CreateBuffers();
		}

		return isRead;
	}

	void TriMesh::Render(const glm::mat4 proj, const glm::mat4 view, bool wireframe)
	{
#pragma region Solid Rendering
		glUseProgram(programPhong);
		glBindVertexArray(sVAO);

		glUniformMatrix4fv(pModelID, 1, GL_FALSE, &model[0][0]);
		glUniform3fv(pMatKaID, 1, &colorAmbient[0]);
		glUniform3fv(pMatKdID, 1, &colorDiffuse[0]);
		glUniform3fv(pMatKsID, 1, &colorSpecular[0]);

		// update data to UBO for MVP
		glBindBuffer(GL_UNIFORM_BUFFER, sUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &view);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &proj);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// Draw solid mesh
		glDrawArrays(GL_TRIANGLES, 0, this->n_faces() * 3);
#pragma endregion

#pragma region Wireframe Rendering
			glUseProgram(programLine);
			glBindVertexArray(wVAO);

			glUniformMatrix4fv(lModelID, 1, GL_FALSE, &model[0][0]);
			glUniform3fv(lMatKdID, 1, &colorLine[0]);

			// update data to UBO for MVP
			glBindBuffer(GL_UNIFORM_BUFFER, wUBO);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &view);
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &proj);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

		if (wireframe)
		{
			glLineWidth(1.5f);
			// Draw wireframe mesh
			glDrawArrays(GL_LINES, 0, this->n_edges() * 2);
		}
#pragma endregion

		// Unbind shader and VAO
		glBindVertexArray(0);
		glUseProgram(0);
	}

	void TriMesh::CreateBuffers()
	{
#pragma region Phong Shader
		ShaderInfo shadersPhong[] = {
			{ GL_VERTEX_SHADER, "./res/shaders/DSPhong_Material.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "./res/shaders/DSPhong_Material.fp" },//fragment shader
			{ GL_NONE, NULL } };
		programPhong = LoadShaders(shadersPhong);

		glUseProgram(programPhong);

		pMatVPID = glGetUniformBlockIndex(programPhong, "MatVP");
		pModelID = glGetUniformLocation(programPhong, "Model");
		pMatKaID = glGetUniformLocation(programPhong, "Material.Ka");
		pMatKdID = glGetUniformLocation(programPhong, "Material.Kd");
		pMatKsID = glGetUniformLocation(programPhong, "Material.Ks");
#pragma endregion

#pragma region Line Shader
		ShaderInfo shadersLine[] = {
			{ GL_VERTEX_SHADER, "./res/shaders/line.vert" },//vertex shader
			{ GL_FRAGMENT_SHADER, "./res/shaders/line.frag" },//fragment shader
			{ GL_NONE, NULL } };
		programLine = LoadShaders(shadersLine);

		glUseProgram(programLine);

		lMatVPID = glGetUniformBlockIndex(programLine, "MatVP");
		lModelID = glGetUniformLocation(programLine, "Model");
		lMatKdID = glGetUniformLocation(programLine, "Material.Kd");
#pragma endregion

#pragma region Solid Rendering
		glGenVertexArrays(1, &sVAO);
		glBindVertexArray(sVAO);

		// UBO
		glGenBuffers(1, &sUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, sUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, NULL, GL_DYNAMIC_DRAW);
		// get uniform struct size
		int sUBOsize = 0;
		glGetActiveUniformBlockiv(programPhong, pMatVPID, GL_UNIFORM_BLOCK_DATA_SIZE, &sUBOsize);
		// bind UBO to its idx
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, sUBO, 0, sUBOsize);
		glUniformBlockBinding(programPhong, pMatVPID, 0);

		// triangle vertex index
		std::vector<glm::vec3> face_vertices;
		std::vector<glm::vec3> face_normals;
		for (FaceHandle f : this->faces())
		{
			// this is basically a triangle fan for any face valence
			TriMesh::ConstFaceVertexCCWIter it = this->cfv_ccwbegin(f);
			VertexHandle first = *it;
			++it;
			uint face_triangles = this->valence(f) - 2;
			for (uint j = 0; j < face_triangles; ++j)
			{
				face_vertices.push_back(d2f(point(first)));
				face_normals.push_back(d2f(normal(first)));

				face_vertices.push_back(d2f(point(*it)));
				face_normals.push_back(d2f(normal(*it)));
				++it;
				face_vertices.push_back(d2f(point(*it)));
				face_normals.push_back(d2f(normal(*it)));
			}
		}

		glGenBuffers(1, &sVBOp);
		glBindBuffer(GL_ARRAY_BUFFER, sVBOp);
		glBufferData(GL_ARRAY_BUFFER, face_vertices.size() * sizeof(glm::vec3), glm::value_ptr(face_vertices[0]), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &sVBOn);
		glBindBuffer(GL_ARRAY_BUFFER, sVBOn);
		glBufferData(GL_ARRAY_BUFFER, face_normals.size() * sizeof(glm::vec3), glm::value_ptr(face_normals[0]), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
#pragma endregion

#pragma region Wireframe Rendering
		glGenVertexArrays(1, &wVAO);
		glBindVertexArray(wVAO);

		// UBO
		glGenBuffers(1, &wUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, wUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, NULL, GL_DYNAMIC_DRAW);
		// get uniform struct size
		int wUBOsize = 0;
		glGetActiveUniformBlockiv(programLine, lMatVPID, GL_UNIFORM_BLOCK_DATA_SIZE, &wUBOsize);
		// bind UBO to its idx
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, wUBO, 0, wUBOsize);
		glUniformBlockBinding(programLine, lMatVPID, 0);

		// triangle vertex index
		std::vector<glm::vec3> edge_vertices;
		std::vector<glm::vec3> edge_normals;
		for (EdgeHandle e : this->edges())
		{
			HalfedgeHandle he = this->halfedge_handle(e, 0);
			edge_vertices.push_back(d2f(point(from_vertex_handle(he))));
			edge_vertices.push_back(d2f(point(to_vertex_handle(he))));
			edge_normals.push_back(d2f(normal(e)));
			edge_normals.push_back(d2f(normal(e)));
		}

		glGenBuffers(1, &wVBOp);
		glBindBuffer(GL_ARRAY_BUFFER, wVBOp);
		glBufferData(GL_ARRAY_BUFFER, edge_vertices.size() * sizeof(glm::vec3), glm::value_ptr(edge_vertices[0]), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &wVBOn);
		glBindBuffer(GL_ARRAY_BUFFER, wVBOn);
		glBufferData(GL_ARRAY_BUFFER, edge_normals.size() * sizeof(glm::vec3), glm::value_ptr(edge_normals[0]), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
#pragma endregion
	}

	void TriMesh::InitStickerShader()
	{
		ShaderInfo shaders[] = {
			{ GL_VERTEX_SHADER,   "./res/shaders/sticker.vp" },
			{ GL_FRAGMENT_SHADER, "./res/shaders/sticker.fp" },
			{ GL_NONE, NULL }
		};
		programSticker = LoadShaders(shaders);
		if (!programSticker) return;

		glUseProgram(programSticker);
		stViewID      = glGetUniformLocation(programSticker, "View");
		stProjID      = glGetUniformLocation(programSticker, "Projection");
		stModelID     = glGetUniformLocation(programSticker, "Model");
		stTexID       = glGetUniformLocation(programSticker, "stickerTex");
		stCenterID    = glGetUniformLocation(programSticker, "stickerCenter");
		stRightID     = glGetUniformLocation(programSticker, "stickerRight");
		stUpID        = glGetUniformLocation(programSticker, "stickerUp");
		stHalfSizeID  = glGetUniformLocation(programSticker, "stickerHalfSize");
		stRotationID  = glGetUniformLocation(programSticker, "stickerRotation");
		stOffsetID    = glGetUniformLocation(programSticker, "stickerOffset");
		stRepeatID    = glGetUniformLocation(programSticker, "stickerRepeat");
		stBlendID     = glGetUniformLocation(programSticker, "stickerBlend");
		stTintID      = glGetUniformLocation(programSticker, "stickerTint");
		glUseProgram(0);
	}

	void TriMesh::RenderSticker(
		const glm::mat4& proj, const glm::mat4& view,
		GLuint texID,
		const glm::vec3& center, const glm::vec3& right, const glm::vec3& up,
		const glm::vec2& halfSize, float rotation,
		const glm::vec2& offset, const glm::vec2& repeat, float blend,
		const glm::vec3& tintColor)
	{
		if (!programSticker || !texID) return;

		glUseProgram(programSticker);
		glBindVertexArray(sVAO);

		glUniformMatrix4fv(stViewID,  1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(stProjID,  1, GL_FALSE, &proj[0][0]);
		glUniformMatrix4fv(stModelID, 1, GL_FALSE, &model[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texID);
		glUniform1i(stTexID, 0);

		glUniform3fv(stCenterID,   1, &center[0]);
		glUniform3fv(stRightID,    1, &right[0]);
		glUniform3fv(stUpID,       1, &up[0]);
		glUniform2fv(stHalfSizeID, 1, &halfSize[0]);
		glUniform1f (stRotationID, rotation);
		glUniform2fv(stOffsetID,   1, &offset[0]);
		glUniform2fv(stRepeatID,   1, &repeat[0]);
		glUniform1f (stBlendID,    blend);
		glUniform3fv(stTintID,     1, &tintColor[0]);

		// Draw sticker on top of existing depth with alpha blending.
		// Polygon offset shifts sticker depth slightly toward the camera so
		// it reliably wins the depth test at every camera angle (prevents Z-fighting).
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-1.0f, -4.0f);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDrawArrays(GL_TRIANGLES, 0, this->n_faces() * 3);

		// Restore render state
		glDisable(GL_POLYGON_OFFSET_FILL);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
		glUseProgram(0);
	}

	OpenMesh::Vec3d TriMesh::normal(const HalfedgeHandle he) const
	{
		const FaceHandle f = face_handle(he);
		if (f.is_valid())
		{
			return normal(f);
		}
		else
		{
			return OpenMesh::Vec3d(0, 0, 0);
		}
	}

	OpenMesh::Vec3d TriMesh::normal(const EdgeHandle e) const
	{
		const HalfedgeHandle he0 = halfedge_handle(e, 0);
		const HalfedgeHandle he1 = halfedge_handle(e, 1);
		assert(!is_boundary(he0) || !is_boundary(he1)); // free edge, bad
		if (is_boundary(he0))
		{
			return normal(face_handle(he1));
		}
		else if (is_boundary(he1))
		{
			return normal(face_handle(he0));
		}
		else
		{
			return (normal(face_handle(he0)) + normal(face_handle(he1))).normalized();
		}
	}

	OpenMesh::Vec3d TriMesh::normal(const FaceHandle f) const
	{
		return OpenMesh::TriMesh_ArrayKernelT<MyTraits>::normal(f);
	}

	OpenMesh::Vec3d TriMesh::normal(const VertexHandle v) const
	{
		return OpenMesh::TriMesh_ArrayKernelT<MyTraits>::normal(v);
	}

	glm::vec3 TriMesh::d2f(OpenMesh::Vec3d v)
	{
		return glm::vec3(v[0], v[1], v[2]);
	}
}
