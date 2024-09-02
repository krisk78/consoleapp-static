#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

/*! \file consoleapp.hpp
*	\brief Implements the class ConsoleApp.
*   \author Christophe COUAILLET
*/

#include <filesystem>
#include <string>

#include "../usage/usage.hpp"

/*! \brief An abstract class that implements a framework for console applications that process files.

	It implements an Usage object to handle the argument definitions of the application and the help.
	The protected member functions SetUsage, CheckArguments, PreProcess, MainProcess and PostProcess must/should be overriden if needed to implement the logic of the application.
	Then, the control of arguments is done by calling the member function Arguments and the execution of the logic is performed by calling the member function Run.
	An option is available on Windows platforms to display the messages in a modal window instead of the console outputs.
*/
class ConsoleApp
{
public:
	/*! \brief Default constructor. */
	ConsoleApp() = default;
#ifdef _WIN32
	/*! \brief Windows constructor that sets the window mode.
	* 
	*	\warning This constructor is only available on Windows platforms. The display of a message in a modal window suspends the execution of the application.
	*/
	ConsoleApp(bool windowsmode)
		: m_windowsmode(windowsmode) {};								// if windowsmode is true then messages are displayed in a modal window else to console
#endif // _WIN32
	/*! \brief Returns true if messages are displayed in a modal window.
	* 
	*	This status should be considered in overriden functions if messages must be displayed.
	*/
	bool windows_mode() { return m_windowsmode; }

	/*! \brief Returns the message of the command line parsing.
	* 
	*	An assertion occurs if the function has been called yet.
	*	This function initializes the Usage program name with the value of argv[0], parses the arguments of the command line then launches the function CheckArguments.
	*	The returned string should be considered before launching the function Run:
	*	\li "": the command line parsing has been done, the extra checks and initialization was performed and the function Run can be launched,
	*	\li "?": the usage help was requested and displayed, the main program should exit without error,
	*	\li other messages: an error occurred during parsing, it has been displayed and the main program should exit with an error.
	*	\sa Usage::set_parameters()
	*/
	std::string Arguments(int argc, char* argv[]);						// Launches SetUsage and parses the command line performing standard usage controls, then launches Check_Arguments
	/*! \brief Returns true if the function Arguments has been called yet. */
	bool Arguments_Checked() { return m_argschecked; }					// In case of you need it, indicates if function Arguments was already launched
	/*! \brief Returns the list of values of each argument.
	* 
	*	An assertion occurs if the member function Arguments has not been called first.
	*/
	std::unordered_map<std::string, std::vector<std::string>> values() const;
																		// Returns the values read for each argument
	/*! \brief Returns the list of values of the given argument.
	* 
	*	An assertion occurs if the argument name is unknown or if the function Arguments has not been called first.
	*/
	std::vector<std::string> values(const std::string& name) const;			// Returns the values read for a single argument
	/*! \brief Launches the sequence PreProcess, MainProcess and PostProcess.
	* 
	*	The overriden member function MainProcess is called for each file found regarding values of the argument named 'file'.
	*	If this argument is not defined, then MainProcess is not called.
	*	This function returns the number of files that have been processed.
	*	An assertion occurs if the function Arguments has not been called first.
	*	This function should not be called if the return of the function Arguments was not an empty string.
	*	\throws A "No matching file" exception is thrown if the argument 'file' is defined but no files matching passed values are found.
	*/
	int Run();															// Runs the sequence PreProcess, ByFile and PostProcess and returns the number of files processed

protected:
	/*! \brief Implements an Usage object to handle arguments.
	*	\sa Usage
	*/
	Usage	us{ "undefined" };											// Contains args and values

	/*! \brief This function must be overriden to set the arguments list and rules and the help output.
	* 
	*	\sa Usage
	*/
	virtual void SetUsage() = 0;										// Defines expected arguments and help.
	/*!	\brief This function should be overriden to perform more accurate checks and initializations.
	* 
	*	If an error occurs it should return the reason of this error (it will be displayed by the caller function Arguments).
	*/
	virtual std::string CheckArguments() { return ""; }					// Performs more accurate checks and initializations if necessary
	/*! \brief This function should be overriden to perform global actions.
	* 
	*	In example, opening global files that are used to store counts.
	*/
	virtual void PreProcess() {};										// Launched before ByFile, do nothing by default
	/*! \brief This function should be overriden to process each file matching the argument 'file' values. */
	virtual void MainProcess(const std::filesystem::path& file) {};		// Launched by ByFile for each file matching argument 'file' values
	/*! \brief This function should be overriden to perform global ending actions.
	* 
	*	In example, closing the global files that were open by the function PreProcess.
	*/
	virtual void PostProcess() {};										// Launched after ByFile, do nothing by default
	/*! \brief This function is provided as an help to create a path matching the input file name passed to MainProcess
	*	and the extension argument if exists.
	* 
	*	It returns the name of the inPath file with the last extension replaced by the value of the extension argument.
	*	\warning If the extension argument does not exist or the arguments were not checked before then the returned path
	*	points to the same file as inPath.
	*/
	std::filesystem::path getOutPath(const std::filesystem::path & inPath);

private:
	bool m_argschecked{ false };
	bool m_windowsmode{ false };

	int ByFile();														// Calls MainProcess for each file matching argument 'file' values and returns the number of files processed
};