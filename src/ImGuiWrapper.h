#include "Window.h"

namespace ALZ {

	class ImGuiWrapper
	{
	public:
		static void Init();
		static void Terminate();
		static void NewFrame();
		static void Render();
	};
}