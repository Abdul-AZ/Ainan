#pragma once

namespace ALZ {

#define KeyCase(x) case GLFW_KEY_##x :\
					     return #x;

	constexpr const char* GLFWKeyToString(int glfwKey) noexcept
	{
		switch (glfwKey)
		{
			KeyCase(0)
			KeyCase(1)
			KeyCase(2)
			KeyCase(3)
			KeyCase(4)
			KeyCase(5)
			KeyCase(6)
			KeyCase(7)
			KeyCase(8)
			KeyCase(9)
			KeyCase(F1)
			KeyCase(F2)
			KeyCase(F3)
			KeyCase(F4)
			KeyCase(F5)
			KeyCase(F6)
			KeyCase(F7)
			KeyCase(F8)
			KeyCase(F9)
			KeyCase(F10)
			KeyCase(F11)
			KeyCase(F12)
			KeyCase(A)
			KeyCase(B)
			KeyCase(C)
			KeyCase(D)
			KeyCase(E)
			KeyCase(F)
			KeyCase(G)
			KeyCase(H)
			KeyCase(I)
			KeyCase(J)
			KeyCase(K)
			KeyCase(L)
			KeyCase(M)
			KeyCase(N)
			KeyCase(O)
			KeyCase(P)
			KeyCase(Q)
			KeyCase(R)
			KeyCase(S)
			KeyCase(T)
			KeyCase(U)
			KeyCase(V)
			KeyCase(W)
			KeyCase(X)
			KeyCase(Y)
			KeyCase(Z)
			KeyCase(SPACE)
			KeyCase(DELETE)

		default:
			return "";
		}
	}

#undef KeyCase

	constexpr const char* GLFWMouseKeyToString(int glfwKey) noexcept
	{
		switch (glfwKey)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			return "Left Mouse Button";

		case GLFW_MOUSE_BUTTON_RIGHT:
			return "Left Mouse Button";

		case GLFW_MOUSE_BUTTON_MIDDLE:
			return "Middle Mouse Button";

		default:
			return "";
		}
	}
}
