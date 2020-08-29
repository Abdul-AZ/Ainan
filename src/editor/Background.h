#pragma once

#include "Camera.h"
#include "renderer/ShaderProgram.h"
#include "renderer/Renderer.h"
#include "environment/RadialLight.h"
#include "environment/SpotLight.h"
#include "environment/Environment.h"

//This should be the same as the one in the Background shader (inside the fragment shader)
#define MAX_NUM_RADIAL_LIGHTS 10
#define MAX_NUM_SPOT_LIGHTS 10

namespace Ainan {

	class Background {
	public:
		Background();
		~Background();

		void DisplayGUI(Environment& env);
		void Draw(Environment& env);

	public:
		bool SettingsWindowOpen = true;

	private:
		std::shared_ptr<VertexBuffer> VBO;
		std::shared_ptr<UniformBuffer> TransformUniformBuffer;
		std::shared_ptr<UniformBuffer> LightingUniformBuffer;
		uint32_t LightDataPackingBufferSize;
		uint8_t* LightDataPackingBuffer;
	};
}