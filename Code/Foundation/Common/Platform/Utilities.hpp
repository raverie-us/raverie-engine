// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
/// System Memory Information
struct MemoryInfo
{
  uint Reserve;
  uint Commit;
  uint Free;
};

namespace Os
{

// Sleep the current thread for ms milliseconds.
void Sleep(uint ms);

// When a diagnostic error occurs, this is the default response
bool ErrorProcessHandler(ErrorSignaler::ErrorData& errorData);

// Open's a url in a browser or tab.
void OpenUrl(cstr url);

// Get the time in milliseconds for a double click.
unsigned int GetDoubleClickTimeMs();

} // namespace Os

// Generate a 64 bit unique Id. Uses system timer and mac
// address to generate the id.
u64 GenerateUniqueId64();

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
