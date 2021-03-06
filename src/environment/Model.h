#pragma once

#include "EnvironmentObjectInterface.h"
#include "renderer/Renderer.h"
#include "file/AssetManager.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Ainan {

	class Model : public EnvironmentObjectInterface
	{
	public:
		struct MeshVertex 
		{
			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec2 TexCoords;
		};

		struct MeshTexture 
		{
			Texture tex;
			std::string Type;
		};

	public:
		Model();
		~Model();

		void DisplayGuiControls() override;

		void LoadModel(std::filesystem::path path);
		void FreeModel();
		void Draw() override;
		std::vector<Model::MeshTexture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

	public:
		std::filesystem::path CurrentModelPath;
		bool FlipUVs = false;

	private:
		// mesh data
		struct Mesh
		{
			//call after setting all public members
			void SetupMesh();
			void DeleteMesh();

			std::vector<MeshVertex> Vertices;
			std::vector<uint32_t> Indices;
			std::vector<MeshTexture> Textures;
		private:
			VertexBuffer m_VertexBuffer;
			IndexBuffer m_IndexBuffer;

			friend class Model;
		};

		std::vector<Mesh> m_Meshes;
		UniformBuffer TransformUniformBuffer;

	private:
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	};

}