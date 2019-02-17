#include "VelocityCustomizer.h"
#include "ColorCustomizer.h"
ColorCustomizer::ColorCustomizer() :
	mt(std::random_device{}())
{}

void ColorCustomizer::DisplayGUI()
{
	if (ImGui::TreeNode("Starting Color")) {

		ImGui::ColorEdit4("Starting Color", &definedColor.x);

		ImGui::TreePop();
	}
}

glm::vec4 ColorCustomizer::GetColor()
{
	return definedColor;
}
