#pragma once

#include "EnvironmentObjectInterface.h"

namespace Ainan {

	class CameraObject : public EnvironmentObjectInterface
	{
	public:
		CameraObject();

		Camera GetCamera() const { return m_Camera; };
		virtual void DisplayGuiControls() override;
		virtual void OnTransform() override;
	private:
		Camera m_Camera;
	};
}
