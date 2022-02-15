// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
// Provide a custom stack walker
class CustomStackWalker : public StackWalker
{
public:
  StringBuilder Stack;
  StringBuilder Modules;
  StringBuilder Other;

  void SetExtraSymbolPath(LPCSTR symPath)
  {
    m_szSymPath = _strdup(symPath);
  }

  void OnOutput(LPCSTR szText) override
  {
    // If it's a stack item...
    if (strstr(szText, STACK_TEXT) != NULL)
    {
      Stack.Append(szText + strlen(STACK_TEXT));
    }
#if OUTPUT_MODULES_AND_OTHER
    // If it's a module item...
    else if (strstr(szText, MODULE_TEXT) != NULL)
    {
      Modules.Append(szText + strlen(MODULE_TEXT);
    }
    // It's something else...
    else
    {
      Other.Append(szText);
    }
#endif
  }

  // Get the final output result
  String GetFinalOutput()
  {
    StringBuilder final;
    final.Append(Stack.ToString());
    final.Append("\r\n");
    final.Append(Other.ToString());
    final.Append("\r\n");
    final.Append(Modules.ToString());
    return final.ToString();
  }
};

String GetCallStack(StringParam extraSymbolPath, OsHandle exceptionContext)
{
  // Create a stack walker that will print out our stack for us
  CustomStackWalker stackWalker;
  if (!extraSymbolPath.Empty())
    stackWalker.SetExtraSymbolPath(extraSymbolPath.c_str());

  EXCEPTION_POINTERS* pException = (EXCEPTION_POINTERS*)exceptionContext;

  if (pException && pException->ContextRecord)
    stackWalker.ShowCallstack(GetCurrentThread(), pException->ContextRecord);
  else
    stackWalker.ShowCallstack(GetCurrentThread());

  String stack = stackWalker.GetFinalOutput();
  return stack;
}

} // namespace Zero
