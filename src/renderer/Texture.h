#pragma once

namespace Ainan {

	class Image;


	class Texture {
	public:
		virtual ~Texture() {};
		virtual void SetImage(const Image& image) = 0;

		//TODO improve this with flags and stuff
		virtual void SetDefaultTextureSettings() = 0;

		virtual glm::vec2 GetSize() const = 0;

		//used by ImGui
		virtual void* GetTextureID() = 0;
	};
}