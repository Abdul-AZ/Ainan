#include "ScaleCustomizer.h"

ScaleCustomizer::ScaleCustomizer() :
	mt(std::random_device{}())
{}

void ScaleCustomizer::DisplayGUI()
{

	if (ImGui::TreeNode("Starting Scale")) {

		ImGui::Checkbox("Random Between 2 Numbers", &RandomScale);

		if (RandomScale)
		{
			if (maxScale < minScale)
				maxScale = minScale;

			ImGui::SliderFloat("Minimum Scale:", &minScale, 0.1f, 15.0f);
			ImGui::SliderFloat("Maximum Scale:", &maxScale, 0.1f, 15.0f);
		}
		else
		{
			ImGui::SliderFloat("Scale:", &definedScale, 0.1f, 15.0f);
		}
		ImGui::TreePop();
	}
}

float ScaleCustomizer::GetScale()
{
	if (RandomScale) {
		std::uniform_real_distribution<float> dist_scale(minScale, maxScale);
		return dist_scale(mt);
	}
	else
		return definedScale;
}
