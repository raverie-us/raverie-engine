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

void TestRenderer(SimpleZilchShaderGenerator& shaderGenerator, BaseRenderer& renderer, ErrorReporter& reporter)
{
  // Test individual fragment compilation
  //TestShaderCompilationOfDirectory(shaderGenerator, renderer, "Tests", reporter);
  // Test composite compilation and linking
  //TestCompilationAndLinkingOfCompositesInDirectory(shaderGenerator, renderer, "CompositeTests", reporter);
  // Test composite running results (run the shader and compare against zilch)
  //TestRuntime(shaderGenerator, renderer, "RunningTests", reporter);
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


void RunTests()
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

  RendererPackage glslPackage;
  glslPackage.mErrorReporter = &glslReporter;
  glslPackage.mRenderer = &glslRenderer;
  glslPackage.mBackend = new ZilchShaderGlslBackend();

  UnitTestPackage unitTestPackage;
  unitTestPackage.mBackends.PushBack(new ZilchSpirVDisassemblerBackend());
  unitTestPackage.mRenderPackages.PushBack(glslPackage);

  String extensionsPath = FilePath::Combine(GetApplicationDirectory(), "FragmentExtensionsIR");
  String fragmentSettingsPath = FilePath::Combine(GetApplicationDirectory(), "ZilchFragmentSettings");

  SpirVNameSettings nameSettings;
  SimpleZilchShaderIRGenerator::LoadNameSettings(nameSettings);
  ZilchShaderSpirVSettings* settings = SimpleZilchShaderIRGenerator::CreateUnitTestSettings(nameSettings);
  SimpleZilchShaderIRGenerator irGenerator(new ZilchSpirVFrontEnd(), settings);
  Zilch::EventConnect(&irGenerator, Zilch::Events::CompilationError, ZilchCompilerErrorCallback);
  Zilch::EventConnect(&irGenerator, Zero::Events::TranslationError, &ErrorReporter::OnTranslationError, &glslReporter);
  Zilch::EventConnect(&irGenerator, Zilch::Events::CompilationError, &ErrorReporter::OnCompilationError, &glslReporter);
  Zilch::EventConnect(&irGenerator, Zero::Events::ValidationError, &ErrorReporter::OnValidationError, &glslReporter);

  irGenerator.SetupDependencies(extensionsPath);

  TestDirectory(irGenerator, unitTestPackage, "IrTests", glslReporter, true);
  TestDirectory(irGenerator, unitTestPackage, "IrCompositeTests", glslReporter, true);
  TestRunning(irGenerator, unitTestPackage, "RunningIrTests", true);
  
  glslReporter.mAssert = false;
  TestDirectory(irGenerator, unitTestPackage, "IrErrorTests", glslReporter, true);

  // Run all of the unit test directories with our different translators/renderers
  //for(size_t i = 0; i < shaderLanguages.Size(); ++i)
  //{
  //  ShaderLanguageTestData& shaderLanguage = *(shaderLanguages[i]);
  //  SimpleZilchShaderGenerator& shaderGenerator = shaderLanguage.mShaderGenerator;
  //  Zilch::EventConnect(&shaderGenerator, Zilch::Events::CompilationError, ZilchCompilerErrorCallback);
  //  Zilch::EventConnect(&shaderGenerator, Zero::Events::TranslationError, ZilchTranslationErrorCallback);
  //  shaderGenerator.LoadSettings(fragmentSettingsPath);
  //  shaderGenerator.SetupDependencies(extensionsPath);
  //
  //  ZPrint("Running %s tests\n", shaderGenerator.mTranslator->GetFullLanguageString().c_str());
  //  TestRenderer(shaderGenerator, *shaderLanguage.mRenderer, *shaderLanguage.mErrorReporter);
  //}
}

int main()
{
  Zilch::ZilchSetup zilchSetup;

  Zilch::Module module;
  ExecutableState::CallingState = module.Link();

  ShaderSettingsLibrary::InitializeInstance();
  ShaderSettingsLibrary::GetInstance().GetLibrary();
  
  RunTests();

  ShaderSettingsLibrary::GetInstance().Destroy();
  delete ExecutableState::CallingState;

  return 0;
}
