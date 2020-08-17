#pragma once

namespace Ainan {

	class Image;


	class Texture {
	public:
		virtual ~Texture() {};
		virtual void SetImage(std::shared_ptr<Image> image) = 0;
		virtual void SetImageUnsafe(std::shared_ptr<Image> image) = 0;

		virtual uint32_t GetMemorySize() const = 0; //in bytes

		//used by ImGui
		virtual void* GetTextureID() = 0;
	};
}