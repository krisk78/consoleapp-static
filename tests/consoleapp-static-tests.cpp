
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define UNICODE
#include <Windows.h>
#endif

#include <gtest/gtest.h>
#include <consoleapp-static.hpp>
#include <str-utils-static.hpp>

class MyApp : public ConsoleApp
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
	// void SetUp() override {}

	// void TearDown() override {}

	MyApp cons0;
#ifdef _WIN32
	MyApp cons1{ true };
#endif // _WIN32
};

using MyAppTestDeath = MyAppTest;

TEST_F(MyAppTest, Wrong_Syntax)
{
#ifdef _WIN32
	std::string expected_str{ "Unknown argument '/t' - see program.exe /? for help." };
	std::vector<char*> argv{ "program.exe", "/t" };
#elif __unix__
	std::string expected_str{ "Unknown argument '-t' - see program.exe -h for help." };
	std::vector<char*> argv{ "program.exe", "-t" };
#endif
	EXPECT_STREQ(cons0.Arguments((int)argv.size(), &argv[0]).c_str(), expected_str.c_str());
#ifdef _WIN32
	EXPECT_STREQ(cons1.Arguments((int)argv.size(), &argv[0]).c_str(), expected_str.c_str());
#endif
}

TEST_F(MyAppTestDeath, Parse_More_Than_One_Time)
{
#ifdef _WIN32
	std::vector<char*> argv{ "program.exe", "/t" };
#elif __unix__
	std::vector<char*> argv{ "program.exe", "-t" };
#endif
	cons0.Arguments((int)argv.size(), &argv[0]);
	EXPECT_DEATH(cons0.Arguments((int)argv.size(), &argv[0]), "");
}

TEST_F(MyAppTest, Help)
{
#ifdef _WIN32
	std::vector<char*> argv{ "program.exe", "/?" };
#elif __unix__
	std::vector<char*> argv{ "program.exe", "-h" };
#endif
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
	std::vector<char*> argv{ "program.exe", path };
	EXPECT_STREQ(cons0.Arguments((int)argv.size(), &argv[0]).c_str(), "Arguments are checked.");
	EXPECT_ANY_THROW(cons0.Run());
}

TEST_F(MyAppTest, Matching_Files)
{
#ifdef _WIN32
	std::vector<char*> argv{ "program.exe", SOURCE_DIR "\\tests\\msxml?.*" };
#elif __unix__
	std::vector<char*> argv{ "program.exe", SOURCE_DIR "/tests/msxml?.*" };
#endif
	EXPECT_STREQ(cons0.Arguments((int)argv.size(), &argv[0]).c_str(), "Arguments are checked.");
	auto files = cons0.values("file");
	EXPECT_EQ(files.size(), 1);
}

TEST_F(MyAppTestDeath, Get_Values_Before_Parsing_Arguments)
{
	EXPECT_DEATH(auto args = cons0.values(), "");
}

TEST_F(MyAppTest, Run_Test)
{
#ifdef _WIN32
	std::vector<char*> argv{ "program.exe", SOURCE_DIR "\\tests\\msxml?.*" };
#elif __unix__
	std::vector<char*> argv{ "program.exe", SOURCE_DIR "/tests/msxml?.*" };
#endif
	EXPECT_STREQ(cons0.Arguments((int)argv.size(), &argv[0]).c_str(), "Arguments are checked.");
#ifdef _WIN32
	EXPECT_STREQ(cons1.Arguments((int)argv.size(), &argv[0]).c_str(), "Arguments are checked.");
#endif
	EXPECT_EQ(cons0.Run(), 2);
#ifdef _WIN32
	EXPECT_EQ(cons1.Run(), 2);
#endif
}

void MyApp::SetUsage()
{
	us.set_syntax("program.exe arguments...");
	Unnamed_Arg f{ "file" };
	f.set_required(true);
	f.many = true;
	us.add_Argument(f);
	us.description = "Program description.";
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
		MessageBox(NULL, str_to_wstr(msg, CP_ACP).c_str(), str_to_wstr(us.program_name, CP_ACP).c_str(), MB_ICONINFORMATION | MB_OK);
	else
#endif // _WIN32
		std::cout << msg << std::endl;
}

void MyApp::MainProcess(const std::filesystem::path& file)
{
	const std::string msg{ "Processing file " + file.string() };
#ifdef _WIN32
	if (windows_mode())
		MessageBox(NULL, str_to_wstr(msg, CP_ACP).c_str(), str_to_wstr(us.program_name, CP_ACP).c_str(), MB_ICONINFORMATION | MB_OK);
	else
#endif // _WIN32
		std::cout << msg << std::endl;
}

void MyApp::PostProcess()
{
	const std::string msg{ "Postprocessing..." };
#ifdef _WIN32
	if (windows_mode())
		MessageBox(NULL, str_to_wstr(msg, CP_ACP).c_str(), str_to_wstr(us.program_name, CP_ACP).c_str(), MB_ICONINFORMATION | MB_OK);
	else
#endif // _WIN32
		std::cout << msg << std::endl;
}
