
//std c++
#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>
#include <chrono>
#include <ctime>
#include <random>
#include <iterator>
#include <filesystem>
#include <functional>
#include <array>
#include <mutex>
#include <queue>
#include <atomic>
#include <numeric>
#include <limits>

#define GLM_FORCE_LEFT_HANDED 1
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1

//dependencies
#include "Log.h" //includes spdlog
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

//imgui dropdown control macro because it causes a lot of repeated code
#define IMGUI_DROPDOWN_START(name, getCurrentValueStrFunc) \
		ImGui::Text(name);\
		ImGui::SameLine(); \
		if (ImGui::BeginCombo("##" name, getCurrentValueStrFunc)) \
		{
//dropdowns that have a column seperator
#define IMGUI_DROPDOWN_START_USING_COLUMNS(name, getCurrentValueStrFunc) \
		ImGui::Text(name);\
		ImGui::NextColumn(); \
		if (ImGui::BeginCombo("##" name, getCurrentValueStrFunc)) \
		{
#define IMGUI_DROPDOWN_SELECTABLE(current, selectable, getSelectableStrFunc) \
		{\
			bool is_active = current == selectable;\
			if (ImGui::Selectable(getSelectableStrFunc, &is_active)) \
				{\
				ImGui::SetItemDefaultFocus();\
				current = selectable;\
				}\
		}
#define IMGUI_DROPDOWN_END() \
 			ImGui::EndCombo(); \
		}

#define STARTING_BROWSER_DIRECTORY "C:\\"
