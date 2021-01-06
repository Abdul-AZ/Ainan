#pragma once

namespace Ainan {

	enum class ShaderVariableType
	{
		Int, UnsignedInt, Float, Vec2, Vec3, Vec4, Mat3, Mat4,
		IntArray, UnsignedIntArray, FloatArray, Vec2Array, Vec3Array, Vec4Array, Mat3Array, Mat4Array
	};

	struct VertexLayoutElement
	{
		VertexLayoutElement(const std::string& semanticName, uint32_t semanticIndex, ShaderVariableType type, int32_t elementCount = 1)
			:
			SemanticName(semanticName),
			SemanticIndex(semanticIndex),
			Type(type),
			Count(elementCount)
		{}
		VertexLayoutElement()
		{};

		std::string SemanticName;
		uint32_t SemanticIndex;
		ShaderVariableType Type;
		uint32_t Count; //this is the number of elements if Type is an array

		uint32_t GetSize() const
		{
			switch (Type)
			{
			case ShaderVariableType::Int:
				return sizeof(int32_t);

			case ShaderVariableType::UnsignedInt:
				return sizeof(uint32_t);

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

			case ShaderVariableType::IntArray:
				return sizeof(int32_t) * Count;

			case ShaderVariableType::UnsignedIntArray:
				return sizeof(uint32_t) * Count;

			case ShaderVariableType::FloatArray:
				return sizeof(float) * Count;

			case ShaderVariableType::Vec2Array:
				return sizeof(float) * 2 * Count;

			case ShaderVariableType::Vec3Array:
				return sizeof(float) * 3 * Count;

			case ShaderVariableType::Vec4Array:
				return sizeof(float) * 4 * Count;

			case ShaderVariableType::Mat3Array:
				return sizeof(float) * 9 * Count;

			case ShaderVariableType::Mat4Array:
				return sizeof(float) * 16 * Count;

			default:
				assert(false);
				return 0;
			}
		}
	};

	using VertexLayout = std::vector<VertexLayoutElement>;
	class ShaderProgram;

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {};

		//virtual void SetLayout(const VertexLayout& layout, const std::shared_ptr<ShaderProgram>& shaderProgram) = 0;

		//NOTE: offset and size are in bytes
		//offset is the start of the memory location you want to update
		virtual void UpdateData(int32_t offset, int32_t size, void* data) = 0;
		virtual uint32_t GetUsedMemory() const = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

	private:
		virtual void UpdateDataUnsafe(int32_t offset, int32_t size, void* data) = 0;

		friend class Renderer;
	};

	class VertexBufferNew
	{
	public:
		uint32_t Identifier;

		//NOTE: offset and size are in bytes
		//offset is the start of the memory location you want to update
		void UpdateData(int32_t offset, int32_t size, void* data);
		uint32_t GetUsedMemory() const;

		friend class Renderer;
	};

	struct VertexBufferDataView
	{
		uint64_t Identifier;
		uint32_t Size;
		uint64_t Layout;
		uint32_t Array; //Used only in OpenGL
		uint32_t Stride;
		bool Deleted = false;
	};

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