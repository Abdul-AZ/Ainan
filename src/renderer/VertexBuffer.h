#pragma once

namespace Ainan {

	enum class ShaderVariableType
	{
		Int, UnsignedInt, Float, Vec2, Vec3, Vec4, Mat3, Mat4
	};

	struct VertexLayoutPart
	{
		std::string Name;
		ShaderVariableType Type;
	};

	using VertexLayout = std::vector<VertexLayoutPart>;
	class ShaderProgram;

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {};

		//virtual void SetLayout(const VertexLayout& layout, const std::shared_ptr<ShaderProgram>& shaderProgram) = 0;

		//NOTE: offset and size are in bytes
		//offset is the start of the memory location you want to update
		virtual void UpdateData(const int& offset, const int& size, void* data) = 0;

		virtual unsigned int GetRendererID() = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};

	//returns size in bytes
	constexpr int GetShaderVariableSize(const ShaderVariableType& type)
	{
		switch (type)
		{
		case ShaderVariableType::Int:
			return sizeof(int);

		case ShaderVariableType::UnsignedInt:
			return sizeof(unsigned int);

		case ShaderVariableType::Float:
			return sizeof(float);

		case ShaderVariableType::Vec2:
			return sizeof(float) * 2;

		case ShaderVariableType::Vec3:
			return sizeof(float) * 3;

		case ShaderVariableType::Vec4:
			return sizeof(float) * 4;

		case ShaderVariableType::Mat3:
			return sizeof(float) * 9;

		case ShaderVariableType::Mat4:
			return sizeof(float) * 16;

		default:
			assert(false);
			return 0;
		}
	}

	//returns size in bytes
	constexpr int GetShaderVariableComponentCount(const ShaderVariableType& type)
	{
		switch (type)
		{
		case ShaderVariableType::Int:
			return 1;

		case ShaderVariableType::UnsignedInt:
			return 1;

		case ShaderVariableType::Float:
			return 1;

		case ShaderVariableType::Vec2:
			return 2;

		case ShaderVariableType::Vec3:
			return 3;

		case ShaderVariableType::Vec4:
			return 4;

		case ShaderVariableType::Mat3:
			return 9;

		case ShaderVariableType::Mat4:
			return 16;

		default:
			assert(false);
			return 0;
		}
	}

}