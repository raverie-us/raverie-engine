// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
/// System Memory Information
struct ZeroShared MemoryInfo
{
  uint Reserve;
  uint Commit;
  uint Free;
};

namespace Os
{

// Sleep the current thread for ms milliseconds.
ZeroShared void Sleep(uint ms);

// Set the Timer Frequency (How often the OS checks threads for sleep, etc)
ZeroShared void SetTimerFrequency(uint ms);

// Get the user name for the current profile
ZeroShared String UserName();

// Get the computer name
ZeroShared String ComputerName();

// Get computer Mac Address of adapter 0
ZeroShared u64 GetMacAddress();

// Check if a debugger is attached
ZeroShared bool IsDebuggerAttached();

// Output a message to any attached debuggers
ZeroShared void DebuggerOutput(const char* message);

// Debug break (only if a debugger is attached)
ZeroShared bool DebugBreak();

// Attempts to enable memory leak checking (break on
ZeroShared void EnableMemoryLeakChecking(int breakOnAllocation = -1);

// When a diagnostic error occurs, this is the default response
ZeroShared bool ErrorProcessHandler(ErrorSignaler::ErrorData& errorData);

// Tells the shell to open or show a directory.
ZeroShared bool ShellOpenDirectory(StringParam directory);

// Tells the shell to open or show a file.
ZeroShared bool ShellOpenFile(StringParam file);

// Tells the shell to edit a file.
ZeroShared bool ShellEditFile(StringParam file);

// Open the application with parameters.
ZeroShared bool ShellOpenApplication(StringParam file, StringParam parameters = String(), StringParam workingDirectory = String());

// On browser based platforms, we can't access the user's file-system so we need to download files instead.
ZeroShared bool SupportsDownloadingFiles();

// Download a single file to the user's file system (on supported browser platforms).
ZeroShared void DownloadFile(cstr fileName, const DataBlock& data);

// Open's a url in a browser or tab.
ZeroShared void OpenUrl(cstr url);

// Mark a file as executable.
ZeroShared void MarkAsExecutable(cstr fileName);

// Get the time in milliseconds for a double click.
ZeroShared unsigned int GetDoubleClickTimeMs();

// Get the memory status of the Os.
ZeroShared void GetMemoryStatus(MemoryInfo& memoryInfo);

// Get an Environmental variable
ZeroShared String GetEnvironmentalVariable(StringParam variable);

// Get a string describing the current operating system version.
ZeroShared String GetVersionString();

// Get the path to an installed executable (may use similar logic to discover the executable name as in
// BuildVersion.cpp).
ZeroShared String GetInstalledExecutable(StringParam organization, StringParam name, StringParam guid);
} // namespace Os

// Generate a 64 bit unique Id. Uses system timer and mac
// address to generate the id.
ZeroShared u64 GenerateUniqueId64();

// Waits for expression to evaluate to true, checking approximately every
// pollPeriod (in milliseconds)
#define WaitUntil(expression, pollPeriod)                                                                              \
  do                                                                                                                   \
  {                                                                                                                    \
    while (!(expression))                                                                                              \
    {                                                                                                                  \
      Os::Sleep(pollPeriod);                                                                                           \
    }                                                                                                                  \
  } while (gConditionalFalseConstant)

} // namespace Zero
