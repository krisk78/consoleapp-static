
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
	#define UNICODE
	#include <Windows.h>
#endif

#include <filesystem>

#include <gtest/gtest.h>
#include <consoleapp-static.hpp>
#include <str-utils-static.hpp>

class MyApp : public ConsoleApp::ConsoleApp
{
public:
	MyApp() : ConsoleApp() {};
#ifdef _WIN32
	MyApp(bool windowed) : ConsoleApp(windowed) {};
#endif // _WIN32

protected:
	virtual void SetUsage() override;
	virtual std::string CheckArguments() override;
	virtual void PreProcess() override;
	virtual void MainProcess(const std::filesystem::path& file) override;
	virtual void PostProcess() override;
};

class MyAppTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		std::vector<const char*> argv_const{ "program.exe", "any_file(s)" };
		std::vector<char*> argv;
		for (const char* arg : argv_const) { argv.push_back(const_cast<char*>(arg)); }
		myAppFriend.Arguments((int)argv.size(), &argv[0]);
		argv.clear();
		argv_const = { "program.exe", "any_file(s)", "-e\".\"" };
		for (const char* arg : argv_const) { argv.push_back(const_cast<char*>(arg)); }
		myAppFriend2.Arguments((int)argv.size(), &argv[0]);
	}

	// void TearDown() override {}

	class MyAppFriend : public MyApp
	{
	public:
		using MyApp::getOutPath;
	};

	MyApp cons0;
#ifdef _WIN32
	MyApp cons1{ true };
#endif // _WIN32
	MyAppFriend myAppFriend;
	MyAppFriend myAppFriend2;
};

using MyAppTestDeath = MyAppTest;

TEST_F(MyAppTest, Wrong_Syntax)
{
	std::string expected_str{ "The following argument was not expected: -t" };
	std::vector<const char*> argv_const{ "program.exe", "any_file(s)", "-t" };
	std::vector<char*> argv;
	for (const char* arg : argv_const) { argv.push_back(const_cast<char*>(arg)); }
	EXPECT_STREQ(cons0.Arguments((int)argv.size(), &argv[0]).c_str(), expected_str.c_str());
#ifdef _WIN32
	EXPECT_STREQ(cons1.Arguments((int)argv.size(), &argv[0]).c_str(), expected_str.c_str());
#endif
}

TEST_F(MyAppTestDeath, Parse_More_Than_One_Time)
{
	std::vector<const char*> argv_const{ "program.exe", "any_file(s)" };
	std::vector<char*> argv;
	for (const char* arg : argv_const) { argv.push_back(const_cast<char*>(arg)); }
	cons0.Arguments((int)argv.size(), &argv[0]);
	EXPECT_DEATH(cons0.Arguments((int)argv.size(), &argv[0]), "");
}

TEST_F(MyAppTest, Help)
{
	std::vector<const char*> argv_const{ "program.exe", "-h" };
	std::vector<char*> argv;
	for (const char* arg : argv_const) { argv.push_back(const_cast<char*>(arg)); }
	EXPECT_STREQ(cons0.Arguments((int)argv.size(), &argv[0]).c_str(), "?");
#ifdef _WIN32
	EXPECT_STREQ(cons1.Arguments((int)argv.size(), &argv[0]).c_str(), "?");
#endif
}

TEST_F(MyAppTestDeath, Run_Without_Parsing_Arguments)
{
	EXPECT_DEATH(cons0.Run(), "");
}

TEST_F(MyAppTest, No_Matching_File)
{
#ifdef _WIN32
	char* path = SOURCE_DIR "\\tests\\msxml2.*";
#elif __unix__
	char* path = SOURCE_DIR "/tests/msxml2.*";
#endif
	std::vector<const char*> argv_const{ "program.exe", path };
	std::vector<char*> argv;
	for (const char* arg : argv_const) { argv.push_back(const_cast<char*>(arg)); }
	EXPECT_STREQ(cons0.Arguments((int)argv.size(), &argv[0]).c_str(), "Arguments are checked.");
	EXPECT_ANY_THROW(cons0.Run());
}

TEST_F(MyAppTest, Matching_Files)
{
#ifdef _WIN32
	std::vector<const char*> argv_const{ "program.exe", SOURCE_DIR "\\tests\\msxml?.*" };
#elif __unix__
	std::vector<const char*> argv_const{ "program.exe", SOURCE_DIR "/tests/msxml?.*" };
#endif
	std::vector<char*> argv;
	for (const char* arg : argv_const) { argv.push_back(const_cast<char*>(arg)); }
	EXPECT_STREQ(cons0.Arguments((int)argv.size(), &argv[0]).c_str(), "Arguments are checked.");
	auto files = cons0.values("files");
	EXPECT_EQ(files.size(), 1);
}

TEST_F(MyAppTestDeath, Get_Values_Before_Parsing_Arguments)
{
	EXPECT_DEATH(auto args = cons0.values(), "");
}

TEST_F(MyAppTest, Run_Test)
{
#ifdef _WIN32
	std::vector<const char*> argv_const{ "program.exe", SOURCE_DIR "\\tests\\msxml?.*" };
#elif __unix__
	std::vector<const char*> argv_const{ "program.exe", SOURCE_DIR "/tests/msxml?.*" };
#endif
	std::vector<char*> argv;
	for (const char* arg : argv_const) { argv.push_back(const_cast<char*>(arg)); }
	EXPECT_STREQ(cons0.Arguments((int)argv.size(), &argv[0]).c_str(), "Arguments are checked.");
#ifdef _WIN32
	EXPECT_STREQ(cons1.Arguments((int)argv.size(), &argv[0]).c_str(), "Arguments are checked.");
#endif
	EXPECT_EQ(cons0.Run(), 2);
#ifdef _WIN32
	EXPECT_EQ(cons1.Run(), 2);
#endif
}

TEST_F(MyAppTest, GetOutPath_Classic)
{
#ifdef _WIN32
	std::filesystem::path testPath{ "C:\\classic_path\\my_file.txt" };
	std::filesystem::path expPath{ "C:\\classic_path\\my_file.ext" };
#elif __unix__
	std::filesystem::path testPath{ "/classic_path/my_file.txt" };
	std::filesystem::path expPath{ "/classic_path/my_file.ext" };
#endif
	std::filesystem::path result = myAppFriend.getOutPath(testPath);
	EXPECT_EQ(result, expPath);
}

TEST_F(MyAppTest, GetOutPath_NoExtension)
{
#ifdef _WIN32
	std::filesystem::path testPath{ "C:\\classic_path\\my_file" };
	std::filesystem::path expPath{ "C:\\classic_path\\my_file.ext" };
#elif __unix__
	std::filesystem::path testPath{ "/classic_path/my_file" };
	std::filesystem::path expPath{ "/classic_path/my_file.ext" };
#endif
	std::filesystem::path result = myAppFriend.getOutPath(testPath);
	EXPECT_EQ(result, expPath);
}

TEST_F(MyAppTest, GetOutPath_PathWithDot)
{
#ifdef _WIN32
	std::filesystem::path testPath{ "C:\\.dot_path\\my_file" };
	std::filesystem::path expPath{ "C:\\.dot_path\\my_file.ext" };
#elif __unix__
	std::filesystem::path testPath{ "/.dot_path/my_file" };
	std::filesystem::path expPath{ "/.dot_path/my_file.ext" };
#endif
	std::filesystem::path result = myAppFriend.getOutPath(testPath);
	EXPECT_EQ(result, expPath);
}

TEST_F(MyAppTest, GetOutPath_RemoveExtension)
{
#ifdef _WIN32
	std::filesystem::path testPath{ "C:\\classic_path\\my_file.txt" };
	std::filesystem::path expPath{ "C:\\classic_path\\my_file" };
#elif __unix__
	std::filesystem::path testPath{ "/classic_path/my_file.txt" };
	std::filesystem::path expPath{ "/classic_path/my_file" };
#endif
	std::filesystem::path result = myAppFriend2.getOutPath(testPath);
	EXPECT_EQ(result, expPath);
}

void MyApp::SetUsage()
{
	name("program.exe");
	add_option("files", "File(s) to convert")->expected(-1)->required();
	add_option("-e,--extension", "Ouput file extension")->default_val(".ext");
	description("Program description.");
}

std::string MyApp::CheckArguments()
{
	return "Arguments are checked.";
}

void MyApp::PreProcess()
{
	const std::string msg{ "Preprocessing..." };
#ifdef _WIN32
	if (windows_mode())
		MessageBox(NULL, str_utils::str_to_wstr(msg, CP_ACP).c_str(), str_utils::str_to_wstr(get_name(), CP_ACP).c_str(), MB_ICONINFORMATION | MB_OK);
	else
#endif // _WIN32
		std::cout << msg << std::endl;
}

void MyApp::MainProcess(const std::filesystem::path& file)
{
	const std::string msg{ "Processing file " + file.string() };
#ifdef _WIN32
	if (windows_mode())
		MessageBox(NULL, str_utils::str_to_wstr(msg, CP_ACP).c_str(), str_utils::str_to_wstr(get_name(), CP_ACP).c_str(), MB_ICONINFORMATION | MB_OK);
	else
#endif // _WIN32
		std::cout << msg << std::endl;
}

void MyApp::PostProcess()
{
	const std::string msg{ "Postprocessing..." };
#ifdef _WIN32
	if (windows_mode())
		MessageBox(NULL, str_utils::str_to_wstr(msg, CP_ACP).c_str(), str_utils::str_to_wstr(get_name(), CP_ACP).c_str(), MB_ICONINFORMATION | MB_OK);
	else
#endif // _WIN32
		std::cout << msg << std::endl;
}
