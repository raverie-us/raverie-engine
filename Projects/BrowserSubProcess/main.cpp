#include <include/cef_app.h>
#include <include/cef_browser.h>
#pragma comment(lib, "libcef.lib")

#if defined(NDEBUG)
#pragma comment(lib, "libcef_dll_wrapper_release.lib")
#else
#pragma comment(lib, "libcef_dll_wrapper_debug.lib")
#endif

class ZeroApp : public CefApp
{
public:
  void OnBeforeCommandLineProcessing(const CefString& process, CefRefPtr<CefCommandLine> commands) override
  {
    // These options are set for best performance
    commands->AppendSwitchWithValue("disable-surfaces", "1");
    commands->AppendSwitchWithValue("disable-gpu", "1");
    commands->AppendSwitchWithValue("disable-gpu-vsync", "1");
    commands->AppendSwitchWithValue("disable-gpu-async-worker-context", "1");
    commands->AppendSwitchWithValue("disable-webgl", "1");
    commands->AppendSwitchWithValue("disable-extensions", "1");
    //commands->AppendSwitchWithValue("disable-threaded-compositing", "1");
    commands->AppendSwitchWithValue("disable-threaded-scrolling", "1");
    commands->AppendSwitchWithValue("enable-begin-frame-scheduling", "1");
  }

  IMPLEMENT_REFCOUNTING(ZeroApp);
};

int RunChrome()
{
  // Execute the sub-process logic. This will block until the sub-process should exit
  // Note: We purposefully do not care about main arguments
  CefMainArgs main_args;
  CefRefPtr<ZeroApp> app(new ZeroApp());
  int result = CefExecuteProcess(main_args, app, nullptr);
  return result;
}

#if defined(_MSC_VER)
int __stdcall WinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
  return RunChrome();
}
#else
int main(void)
{
  return RunChrome();
}
#endif


