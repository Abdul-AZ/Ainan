#include "Model.h"

namespace Ainan {

	Assimp::Importer importer;
	const aiScene* scene;

	Model::Model()
	{
		Type = ModelType;

		scene = importer.ReadFile("res/3D/Avocado.gltf", aiProcess_Triangulate);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			AINAN_LOG_ERROR("Couldn't Load Model");
			AINAN_LOG_ERROR(importer.GetErrorString());
			return;
		}

		ProcessNode(scene->mRootNode, scene);
		TransformUniformBuffer = Renderer::CreateUniformBuffer("ObjectTransform", 1,
			{ VertexLayoutElement("u_Model", 0, ShaderVariableType::Mat4) }
		);
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		// process all the node's meshes (if any)
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_Meshes.push_back(ProcessMesh(mesh, scene));
		}
		// then do the same for each of its children
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	std::vector<Model::MeshTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
	{
		std::vector<Model::MeshTexture> textures;
		for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			Model::MeshTexture texture;
			texture.tex = Renderer::CreateTexture(Image::LoadFromFile(std::string("res/3D/") + str.C_Str(), TextureFormat::RGBA));
			texture.Type = typeName;
			textures.push_back(texture);
		}
		return textures;
	}

	Model::Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		Mesh ainanMesh;

		// process vertex positions, normals and texture coordinates
		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			MeshVertex vertex;

			vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);

			ainanMesh.Vertices.push_back(vertex);
		}

		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
				ainanMesh.Indices.push_back(face.mIndices[j]);
		}

		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<MeshTexture> diffuseMaps = loadMaterialTextures(material,
				aiTextureType_DIFFUSE, "texture_diffuse");
			ainanMesh.Textures.insert(ainanMesh.Textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			std::vector<MeshTexture> specularMaps = loadMaterialTextures(material,
				aiTextureType_SPECULAR, "texture_specular");
			ainanMesh.Textures.insert(ainanMesh.Textures.end(), specularMaps.begin(), specularMaps.end());
		}

		ainanMesh.SetupMesh();
		return ainanMesh;
	}

	void Model::Mesh::SetupMesh()
	{
		VertexLayout layout(3);
		layout[0] = VertexLayoutElement("POSITION", 0, ShaderVariableType::Vec3);
		layout[1] = VertexLayoutElement("NORMAL", 0, ShaderVariableType::Vec3);
		layout[2] = VertexLayoutElement("TEXCOORD", 0, ShaderVariableType::Vec2);

		m_VertexBuffer = Renderer::CreateVertexBuffer(Vertices.data(), Vertices.size() * sizeof(MeshVertex), layout,
			Renderer::ShaderLibrary()["3DAmbientShader"], false);

		m_IndexBuffer = Renderer::CreateIndexBuffer(Indices.data(), Indices.size());
	}

	void Model::Mesh::DeleteMesh()
	{
		Renderer::DestroyVertexBuffer(m_VertexBuffer);
		Renderer::DestroyIndexBuffer(m_IndexBuffer);

		for (auto& tex : Textures)
		{
			Renderer::DestroyTexture(tex.tex);
		}
	}

	Model::~Model()
	{
		for (auto& mesh : m_Meshes)
			mesh.DeleteMesh();
		Renderer::DestroyUniformBuffer(TransformUniformBuffer);
	}

	void Model::Draw()
	{
		TransformUniformBuffer.UpdateData(&ModelMatrix, sizeof(glm::mat4));
		auto& shader = Renderer::ShaderLibrary()["3DAmbientShader"];
		shader.BindUniformBuffer(TransformUniformBuffer, 1, RenderingStage::VertexShader);

		for (auto& mesh : m_Meshes)
		{
			shader.BindTexture(mesh.Textures[0].tex, 0, RenderingStage::FragmentShader);
			Renderer::Draw(mesh.m_VertexBuffer, shader, Primitive::Triangles, mesh.m_IndexBuffer);
		}
	}

}