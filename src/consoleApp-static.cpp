// consoleapp.cpp : Defines the functions for the static library.
//

#include "pch.h"

#include "consoleapp.hpp"
#include "../utils/utils.hpp"

std::string ConsoleApp::Arguments(int argc, char* argv[])
{
	assert(!m_argschecked && "Arguments checks were already performed.");	// Arguments function should be called once
	SetUsage();
	if (us.program_name == "undefined")
	{
		us.program_name = argv[0];
		auto itr = us.program_name.find_last_of("/\\");
		if (itr != std::string::npos)
			us.program_name = us.program_name.substr(itr + 1, us.program_name.size() - itr);
	}
	auto msg = us.set_parameters(argc, argv);
	if (msg == "?")
	{
#ifdef _WIN32
		if (m_windowsmode)
		{
			std::ostringstream os;
			os << us;
			MessageBox(NULL, str_to_wstr(os.str(), CP_ACP).c_str(), str_to_wstr(us.program_name, CP_ACP).c_str(), MB_ICONINFORMATION | MB_OK);
		}
		else
#endif // _WIN32
			std::cout << us;
		return msg;				// m_argschecked is not set to true because in this case there is nothing to run
	}
	if (msg.size() == 0)
		msg = CheckArguments();
	if (msg.size() != 0)
	{
#ifdef _WIN32
		if (m_windowsmode)
			MessageBox(NULL, str_to_wstr(msg, CP_ACP).c_str(), str_to_wstr(us.program_name, CP_ACP).c_str(), MB_ICONERROR | MB_OK);
		else
#endif // _WIN32
			std::cout << msg;
	}
	m_argschecked = true;
	return msg;
}

std::unordered_map<std::string, std::vector<std::string>> ConsoleApp::values() const
{
	assert(m_argschecked && "Attempt to get values before parsing command line arguments.");
	return us.get_values();
}

std::vector<std::string> ConsoleApp::values(const std::string& name) const
{
	assert(m_argschecked && "Attempt to get values before parsing command line arguments.");
	return us.get_values(name);
}

int ConsoleApp::Run()
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

int ConsoleApp::ByFile()
{
	int nbfiles{ 0 };
	auto files = us.get_Argument("file");
	if (files == NULL || !files->required() && files->value.empty())
		return nbfiles;
	for (auto value : files->value)
	{
		auto filelist = dir(value);
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

std::filesystem::path ConsoleApp::getOutPath(const std::filesystem::path& inpath)
{
	std::string outname{ inpath.generic_string() };
	auto extarg = us.get_Argument("extension");
	if (extarg == NULL || !Arguments_Checked())
		return std::filesystem::path(outname);
	auto ext = outname.find_last_of('.');
	if (ext != std::string::npos)
		outname.erase(ext, outname.size() - ext);
	outname.append(extarg->value.front());
	return std::filesystem::path(outname);
}
