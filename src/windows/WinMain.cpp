#include <Windows.h>

extern int main();

int WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd
)
{
#ifndef NDEBUG

	AllocConsole();
	FILE* file;
	freopen_s(&file, "CONOUT$", "w", stdout);

#endif // DEBUG

	return main();
}