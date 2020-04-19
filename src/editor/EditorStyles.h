#pragma once

namespace Ainan 
{
	enum class EditorStyle 
	{
		Light,
		DarkTransparent,
		Dark_Gray,
		Dark,
		Classic
	};

	void SetEditorStyle(const EditorStyle& style);
}