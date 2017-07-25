///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

class VisualStudioListener : public ConsoleListener
{
public:
  void Print(FilterType filterType, cstr message) override
  {
    OutputDebugStringA(message);
  }
};

void ZilchCompilerErrorCallback(Zilch::ErrorEvent* e)
{
  ZPrint("%s", e->GetFormattedMessage(Zilch::MessageFormat::MsvcCpp).c_str());
  ZERO_DEBUG_BREAK;
}

void ZilchTranslationErrorCallback(TranslationErrorEvent* e)
{
  ZPrint("%s", e->mFullMessage.c_str());
  __debugbreak();
}

void TestRenderer(SimpleZilchShaderGenerator& shaderGenerator, BaseRenderer& renderer, ErrorReporter& report)
{
  // Test individual fragment compilation
  TestShaderCompilationOfDirectory(shaderGenerator, renderer, "Tests", report);
  // Test composite compilation and linking
  TestCompilationAndLinkingOfCompositesInDirectory(shaderGenerator, renderer, "CompositeTests", report);
  // Test composite running results (run the shader and compare against zilch)
  TestRuntime(shaderGenerator, renderer, "RunningTests", report);
}

// Simple helper to contain common data for a language test
class ShaderLanguageTestData
{
public:

  SimpleZilchShaderGenerator mShaderGenerator;
  BaseRenderer* mRenderer;
  ErrorReporter* mErrorReporter;

  ShaderLanguageTestData(BaseRenderer* renderer, BaseShaderTranslator* translator, ErrorReporter* reporter)
    : mShaderGenerator(translator)
  {
    mRenderer = renderer;
    mErrorReporter = reporter;
  }
};

int main()
{
  // Hook up a listener to print all output to the visual studio console
  VisualStudioListener visualStudioOutput;
  if(Os::IsDebuggerAttached())
    Zero::Console::Add(&visualStudioOutput);

  HWND hiddenWindow = CreateWindowA("STATIC", "", WS_POPUP | WS_DISABLED, 0, 0, mScreenWidth, mScreenHeight, NULL, NULL, GetModuleHandle(NULL), NULL);
  ShowWindow(hiddenWindow, SW_HIDE);
  HDC drawContext = GetDC(hiddenWindow);

  ErrorReporter glslReporter;
  GlslRenderer glslRenderer(drawContext);

  Zilch::ZilchSetup zilchSetup;
  ShaderSettingsLibrary::GetInstance().GetLibrary();
  String extensionsPath = FilePath::Combine(GetApplicationDirectory(), "FragmentCore");
  String fragmentSettingsPath = FilePath::Combine(GetApplicationDirectory(), "ZilchFragmentSettings");

  // Add all of the current languages for translation
  Array<ShaderLanguageTestData*> shaderLanguages;
  shaderLanguages.PushBack(new ShaderLanguageTestData(&glslRenderer, new Glsl130Translator(), &glslReporter));
  shaderLanguages.PushBack(new ShaderLanguageTestData(&glslRenderer, new Glsl150Translator(), &glslReporter));

  // Run all of the unit test directories with our different translators/renderers
  for(size_t i = 0; i < shaderLanguages.Size(); ++i)
  {
    ShaderLanguageTestData& shaderLanguage = *(shaderLanguages[i]);
    SimpleZilchShaderGenerator& shaderGenerator = shaderLanguage.mShaderGenerator;
    Zilch::EventConnect(&shaderGenerator, Zilch::Events::CompilationError, ZilchCompilerErrorCallback);
    Zilch::EventConnect(&shaderGenerator, Zero::Events::TranslationError, ZilchTranslationErrorCallback);
    shaderGenerator.LoadSettings(fragmentSettingsPath);
    shaderGenerator.SetupDependencies(extensionsPath);

    ZPrint("Running %s tests\n", shaderGenerator.mTranslator->GetFullLanguageString().c_str());
    TestRenderer(shaderGenerator, *shaderLanguage.mRenderer, *shaderLanguage.mErrorReporter);
  }

  DeleteObjectsInContainer(shaderLanguages);

  return 0;
}
