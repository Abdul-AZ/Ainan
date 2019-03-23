
class FileBrowser 
{
public:
	FileBrowser(const std::string& startingFolder, const std::string& windowName = "File Browser");

	void DisplayGUI();

private:
	std::string m_CurrentFolder;
	std::string m_WindowName;
	std::string m_CurrentselectedFilePath;
	std::string m_InputFolder;
};