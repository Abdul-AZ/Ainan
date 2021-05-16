
namespace Ainan
{
	//check if the directory is OK to use by checking the permissions and if it is hidden or a system directory
	bool WinEvaluateDirectory(std::filesystem::path path);
}