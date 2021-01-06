#pragma once

#include "Image.h"

#undef max

namespace Ainan {

	class Texture {
	public:
		virtual ~Texture() {};
		virtual void SetImage(std::shared_ptr<Image> image) = 0;

		virtual uint32_t GetMemorySize() const = 0; //in bytes

		//used by ImGui
		virtual void* GetTextureID() = 0;

	private:
		virtual void SetImageUnsafe(std::shared_ptr<Image> image) = 0;

		friend class Renderer;
	};

	struct TextureDataView
	{
		uint64_t Identifier = std::numeric_limits<uint64_t>::max();
		uint64_t View = std::numeric_limits<uint64_t>::max(); //used in D3D
		uint64_t Sampler = std::numeric_limits<uint64_t>::max(); //used in D3D
		glm::vec2 Size;
		TextureFormat Format;
		bool Deleted = false;
	};

	class TextureNew
	{
	public:
		uint32_t Identifier = std::numeric_limits<uint32_t>::max();

		bool IsValid() 
		{
			return Identifier != std::numeric_limits<uint32_t>::max();
		}

		void SetImage(std::shared_ptr<Image> image);

		uint32_t GetMemorySize() const; //in bytes

		//used by ImGui
		uint64_t GetTextureID();

		friend class Renderer;
	};
}