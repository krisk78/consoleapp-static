#include "pch.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include "../ConsoleAppFW/consoleapp.hpp"
#include "../utils/utils.hpp"

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
	std::vector<char*> argv{ "program.exe", "/t" };
	EXPECT_STREQ(cons0.Arguments((int)argv.size(), &argv[0]).c_str(), "Unknown argument '/t' - see program.exe /? for help.");
	EXPECT_STREQ(cons1.Arguments((int)argv.size(), &argv[0]).c_str(), "Unknown argument '/t' - see program.exe /? for help.");
}

TEST_F(MyAppTestDeath, Parse_More_Than_One_Time)
{
	std::vector<char*> argv{ "program.exe", "/t" };
	cons0.Arguments((int)argv.size(), &argv[0]);
	EXPECT_DEATH(cons0.Arguments((int)argv.size(), &argv[0]), "");
}

TEST_F(MyAppTest, Help)
{
	std::vector<char*> argv{ "program.exe", "/?" };
	EXPECT_STREQ(cons0.Arguments((int)argv.size(), &argv[0]).c_str(), "?");
	EXPECT_STREQ(cons1.Arguments((int)argv.size(), &argv[0]).c_str(), "?");
}

TEST_F(MyAppTestDeath, Run_Without_Parsing_Arguments)
{
	EXPECT_DEATH(cons0.Run(), "");
}

TEST_F(MyAppTest, No_Matching_File)
{
	std::vector<char*> argv{ "program.exe", "H:/Windows/System32/msxml2.*" };
	EXPECT_STREQ(cons0.Arguments((int)argv.size(), &argv[0]).c_str(), "Arguments are checked.");
	EXPECT_ANY_THROW(cons0.Run());
}

TEST_F(MyAppTest, Matching_Files)
{
	std::vector<char*> argv{ "program.exe", "H:/Windows/System32/msxml?.*" };
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
	std::vector<char*> argv{ "program.exe", "H:/Windows/System32/msxml?.*" };
	EXPECT_STREQ(cons0.Arguments((int)argv.size(), &argv[0]).c_str(), "Arguments are checked.");
	EXPECT_STREQ(cons1.Arguments((int)argv.size(), &argv[0]).c_str(), "Arguments are checked.");
	EXPECT_EQ(cons0.Run(), 2);
	EXPECT_EQ(cons1.Run(), 2);
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