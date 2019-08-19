#pragma once


namespace ALZ {

	class AppStatusWindow
	{
	public:
		void DisplayGUI(ImGuiID& viewportDockID);

	public:
		std::string Text;
	};
}