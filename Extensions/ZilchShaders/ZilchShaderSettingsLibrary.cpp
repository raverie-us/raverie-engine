///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "ZilchShaderSettingsLibrary.hpp"
#include "ZilchShaderSharedSettings.hpp"
#include "ShaderErrors.hpp"
#include "ShaderIntrinsicTypes.hpp"

// Bind all of the classes needed for handling system values (add more settings later).
// Note: the system value settings are not safe as they are internal (adding new values could invalidate old pointers).
namespace Zero
{
// Bind the fragment type enum
ZilchDefineExternalBaseType(Zero::ShaderStageType::Enum, Zilch::TypeCopyMode::ValueType, builder, type)
{
  ZilchFullBindEnum(builder, type, Zilch::SpecialType::Enumeration);
  ZilchFullBindEnumValue(builder, type, Zero::ShaderStageType::Vertex, "Vertex");
  ZilchFullBindEnumValue(builder, type, Zero::ShaderStageType::GeometryPrimitive, "GeometryPrimitive");
  ZilchFullBindEnumValue(builder, type, Zero::ShaderStageType::GeometryVertex, "GeometryVertex");
  ZilchFullBindEnumValue(builder, type, Zero::ShaderStageType::Pixel, "Pixel");
}

ZilchDefineType(LanguageSystemValue, builder, type)
{
  type->HandleManager = ZilchManagerId(Zilch::PointerManager);
  ZilchBindField(mMinVersion);
  ZilchBindField(mMaxVersion);
  ZilchFullBindMethod(builder, type, &LanguageSystemValue::SetInput, ZilchNoOverload, "SetInput", "type, name, attribute");
  ZilchFullBindMethod(builder, type, &LanguageSystemValue::SetOutput, ZilchNoOverload, "SetOutput", "type, name, attribute");
  ZilchFullBindMethod(builder, type, &LanguageSystemValue::SetInputAndOutput, ZilchNoOverload, "SetInputAndOutput", "type, name, attribute");
  ZilchBindGetterSetter(Forced);
}

ZilchDefineType(ShaderSystemValue, builder, type)
{
  type->HandleManager = ZilchManagerId(Zilch::PointerManager);
  ZilchFullBindMethod(builder, type, &ShaderSystemValue::AddLanguageSystemValue, (LanguageSystemValue*(ShaderSystemValue::*)(StringParam)), "AddLanguageSystemValue", "language");
}

ZilchDefineType(ShaderSystemValueSettings, builder, type)
{
  type->HandleManager = ZilchManagerId(Zilch::PointerManager);
  ZilchFullBindMethod(builder, type, &ShaderSystemValueSettings::AddSystemValue, ZilchNoOverload, "AddSystemValue", "fragmentStage, type, name");
}

ZilchDefineStaticLibrary(ShaderSettingsLibrary)
{
  builder.CreatableInScriptDefault = false;
  
  ZilchInitializeExternalTypeAs(ShaderStageType::Enum, "ShaderStageType");
  ZilchInitializeType(LanguageSystemValue);
  ZilchInitializeType(ShaderSystemValue);
  ZilchInitializeType(ShaderSystemValueSettings);
  ZilchInitializeType(TranslationErrorEvent);
}

//-------------------------------------------------------------------ZilchShaderSettingsLoader
void ZilchShaderSettingsLoader::LoadSettings(ZilchShaderSettings* settings, StringParam settingsDirectoryPath)
{
  Zilch::LibraryRef coreLibrary = Zilch::Core::GetInstance().GetLibrary();
  Zilch::LibraryRef shaderSettingsLibrary = ShaderSettingsLibrary::GetInstance().GetLibrary();

  Zilch::Module zilchDependencies;
  zilchDependencies.PushBack(shaderSettingsLibrary);

  // Load all of the zilch files in the settings directory
  Zilch::Project settingsProject;
  FileRange fileRange(settingsDirectoryPath);
  for(; !fileRange.Empty(); fileRange.PopFront())
  {
    FileEntry entry = fileRange.frontEntry();
    String filePath = entry.GetFullPath();
    String fileExt = FilePath::GetExtension(filePath);
    settingsProject.AddCodeFromFile(filePath, nullptr);
  }
  EventConnect(&settingsProject, Events::TranslationError, &ZilchShaderSettingsLoader::OnForwardEvent, this);
  EventConnect(&settingsProject, Zilch::Events::CompilationError, &ZilchShaderSettingsLoader::OnForwardEvent, this);

  // Compile the syntax tree
  Zilch::Array<Zilch::UserToken> tokensOut;
  Zilch::SyntaxTree syntaxTree;
  Zilch::LibraryBuilder libraryBuilder("ShaderSettings");
  bool success = settingsProject.CompileCheckedSyntaxTree(syntaxTree, libraryBuilder, tokensOut, zilchDependencies, Zilch::EvaluationMode::Project);
  if(!success)
  {
    Error("Failed to compile settings project");
    return;
  }

  // Generate code into a library
  Zilch::CodeGenerator codeGenerator;
  Zilch::LibraryRef library = codeGenerator.Generate(syntaxTree, libraryBuilder);
  // Compile the library and dependencies into an executable state
  zilchDependencies.PushBack(library);
  Zilch::ExecutableState* state = zilchDependencies.Link();

  // Load the system value settings
  LoadSystemValueSettings(settings, library, state);

  delete state;
}

void ZilchShaderSettingsLoader::LoadSystemValueSettings(ZilchShaderSettings* settings, Zilch::LibraryRef& library, Zilch::ExecutableState* state)
{
  Zilch::Core& core = Zilch::Core::GetInstance();

  // Find the SystemValueSettings type
  Zilch::BoundType* zilchType = library->BoundTypes.FindValue("SystemValueSettings", nullptr);
  ReturnIf(zilchType == nullptr, ,"Settings library must contain a type called SystemValueSettings");

  // Find the main function with the correct signature
  Zilch::BoundType* settingsType = ZilchTypeId(ShaderSystemValueSettings);
  Array<Zilch::Type*> mainParameters(ZeroInit, settingsType);
  Zilch::Function* mainFunction = zilchType->FindFunction("Main", mainParameters, core.VoidType, Zilch::FindMemberOptions::None);
  ReturnIf(mainFunction == nullptr, , "SystemValueSettings must have a function with the signature: 'Main(settings : ShaderSystemValueSettings)'");

  Zilch::ExceptionReport report;
  // Allocate the SystemValueSettings struct
  Zilch::Handle preconstructedObject = state->AllocateDefaultConstructedHeapObject(zilchType, report, Zilch::HeapFlags::NonReferenceCounted);
  // Set the ShaderSystemValueSettings class as the first parameter and call the main function
  Zilch::Call call(mainFunction, state);
  call.Set(0, &settings->mSystemValueSettings);
  call.SetHandle(Zilch::Call::This, preconstructedObject);
  call.Invoke(report);

  preconstructedObject.Delete();
}

void ZilchShaderSettingsLoader::OnForwardEvent(Zilch::EventData* e)
{
  EventSend(this, e->EventName, e);
}

}//namespace Zero
