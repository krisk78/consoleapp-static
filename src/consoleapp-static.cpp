// \file consoleapp-static.cpp : Defines the functions for the static library.
//

#ifdef _WIN32
#define UNICODE
#include <Windows.h>
#endif

#include <ios>
#include <iostream>
#include <sstream>
#include <cassert>
#include <system_error>

#include <consoleapp-static.hpp>
#include <file-utils-static.hpp>
#include <str-utils-static.hpp>

std::string ConsoleApp::ConsoleApp::Arguments(int argc, char* argv[])
{
	assert(!m_argschecked && "Arguments checks were already performed.");	// Arguments function should be called once
	SetUsage();
	if (get_name() == "undefined")
	{
		std::string appName = argv[0];
		auto itr = appName.find_last_of("/\\");
		if (itr != std::string::npos)
			appName = appName.substr(itr + 1, appName.size() - itr);
		name(appName);
	}
	try
	{
		parse(argc, argv);
	}
	catch (const CLI::ParseError& e)
	{
		if (e.get_exit_code() == 0)		// help is requested
		{
			if (m_windowsmode)
#ifdef _WIN32
				MessageBox(NULL, str_utils::str_to_wstr(help(), CP_ACP).c_str(), str_utils::str_to_wstr(get_name(), CP_ACP).c_str(), MB_ICONINFORMATION | MB_OK);
#endif
			else
				std::cout << help();
			return "?";
		}
		else
		{
			if (m_windowsmode)
#ifdef _WIN32
				MessageBox(NULL, str_utils::str_to_wstr(e.what(), CP_ACP).c_str(), str_utils::str_to_wstr(get_name(), CP_ACP).c_str(), MB_ICONERROR | MB_OK);
#endif
			else
				std::cerr << e.what();
		}
		return e.what();
	}
	auto msg = CheckArguments();
	if (msg.size() != 0)
	{
#ifdef _WIN32
		if (m_windowsmode)
			MessageBox(NULL, str_utils::str_to_wstr(msg, CP_ACP).c_str(), str_utils::str_to_wstr(get_name(), CP_ACP).c_str(), MB_ICONERROR | MB_OK);
		else
#endif // _WIN32
			std::cout << msg;
	}
	m_argschecked = true;
	return msg;
}

std::unordered_map<std::string, std::vector<std::string>> ConsoleApp::ConsoleApp::values() const
{
	assert(m_argschecked && "Attempt to get values before parsing command line arguments.");
	std::unordered_map<std::string, std::vector<std::string>> args_map;
	for (const auto& option : get_options())
		if (option->count() > 0)
			args_map[option->get_name()] = option->results();
	return args_map;
}

std::vector<std::string> ConsoleApp::ConsoleApp::values(const std::string& name) const
{
	assert(m_argschecked && "Attempt to get values before parsing command line arguments.");
	auto option = get_option(name);
	assert(option && "Unknown argument name.");
	return option->results();
}

int ConsoleApp::ConsoleApp::Run()
{
	assert(m_argschecked && "Arguments must be parsed and checked first.");
	int nbfiles{ 0 };
	try
	{
		PreProcess();
		nbfiles = ByFile();
		PostProcess();
	}
	catch (const std::exception&) {
		throw;
	}
	return nbfiles;
}

int ConsoleApp::ConsoleApp::ByFile()
{
	int nbfiles{ 0 };
	auto files = get_option("files");
	if (files == NULL || !files->required() && files->results().empty())
		return nbfiles;
	for (auto value : files->results())
	{
		auto filelist = file_utils::dir(value);
		for (auto file : filelist)
		{
			try {
				MainProcess(file); }
			catch (...) {
				throw; }
			nbfiles++;
		}
	}
	if (nbfiles == 0)
		throw std::filesystem::filesystem_error("No matching file.", std::make_error_code(std::errc::no_such_file_or_directory));
	return nbfiles;
}

std::filesystem::path ConsoleApp::ConsoleApp::getOutPath(const std::filesystem::path& inpath)
{
	auto extarg = get_option("extension");
	if (extarg == NULL || !Arguments_Checked())
		return std::filesystem::path(inpath);
	std::filesystem::path outpath = inpath;
	outpath.replace_extension(extarg->results().front());
	return outpath;
}
