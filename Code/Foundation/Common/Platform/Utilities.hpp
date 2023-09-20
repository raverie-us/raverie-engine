// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
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

// Get the time in milliseconds for a double click.
unsigned int GetDoubleClickTimeMs();

} // namespace Os

// Generate a 64 bit unique Id. Uses system timer and mac
// address to generate the id.
u64 GenerateUniqueId64();

// Waits for expression to evaluate to true, checking approximately every
// pollPeriod (in milliseconds)
#define WaitUntil(expression, pollPeriod)                                                                                                                                                              \
  do                                                                                                                                                                                                   \
  {                                                                                                                                                                                                    \
    while (!(expression))                                                                                                                                                                              \
    {                                                                                                                                                                                                  \
      Os::Sleep(pollPeriod);                                                                                                                                                                           \
    }                                                                                                                                                                                                  \
  } while (gConditionalFalseConstant)

} // namespace Raverie
