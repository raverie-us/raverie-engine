// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{

RaverieShaderGenerator* CreateRaverieShaderGenerator();

DeclareEnum5(RaverieFragmentType, CoreVertex, RenderPass, PostProcess, Protected, Fragment);
typedef HashMap<String, RaverieFragmentType::Enum> RaverieFragmentTypeMap;

// User data written per shader type. Used to deal with efficient re-compiling
// of fragments/shaders.
class FragmentUserData
{
public:
  FragmentUserData(){};
  FragmentUserData(StringParam resourceName) : mResourceName(resourceName){};
  String mResourceName;
};

class RaverieShaderGenerator : public Raverie::EventHandler
{
public:
  typedef HashMap<LibraryRef, RaverieShaderIRLibraryRef> ExternalToInternalLibraryMap;
  typedef ExternalToInternalLibraryMap::valuerange InternalLibraryRange;
  typedef RaverieShaderIRCompositor::ShaderDefinition ShaderDefinition;
  typedef Raverie::Ref<ShaderTranslationPassResult> TranslationPassResultRef;
  typedef Raverie::Ref<SimplifiedShaderReflectionData> SimplifiedReflectionRef;
  typedef Raverie::Ref<RaverieSpirVFrontEnd> RaverieSpirVFrontEndRef;

  RaverieShaderGenerator();
  ~RaverieShaderGenerator();

  void Initialize();
  void InitializeSpirV();

  LibraryRef BuildFragmentsLibrary(Module& dependencies,
                                   Array<RaverieDocumentResource*>& fragments,
                                   StringParam libraryName = "Fragments");
  bool Commit(RaverieCompileEvent* e);
  void MapFragmentTypes();

  bool BuildShaders(ShaderSet& shaders,
                    HashMap<String, UniqueComposite>& composites,
                    Array<ShaderEntry>& shaderEntries,
                    Array<ShaderDefinition>* compositeShaderDefs = nullptr);
  bool CompilePipeline(RaverieShaderIRType* shaderType,
                       ShaderPipelineDescription& pipeline,
                       Array<TranslationPassResultRef>& pipelineResults);

  ShaderInput
  CreateShaderInput(StringParam fragmentName, StringParam inputName, ShaderInputType::Enum type, AnyParam value);

  void OnRaverieFragmentCompilationError(Raverie::ErrorEvent* event);
  void OnRaverieFragmentTypeParsed(Raverie::ParseEvent* event);
  void OnRaverieFragmentTranslationError(TranslationErrorEvent* event);
  void OnRaverieFragmentValidationError(ValidationErrorEvent* event);

  RaverieShaderIRLibraryRef GetInternalLibrary(LibraryRef library);
  RaverieShaderIRLibraryRef GetCurrentInternalLibrary(LibraryRef library);
  RaverieShaderIRLibraryRef GetPendingInternalLibrary(LibraryRef library);
  InternalLibraryRange GetCurrentInternalLibraries();
  RaverieShaderIRLibraryRef GetCurrentInternalProjectLibrary();
  RaverieShaderIRLibraryRef GetPendingInternalProjectLibrary();

  // The shared settings to be used through all of the compositing/translation
  RaverieShaderSpirVSettingsRef mSpirVSettings;
  // The front-end translator used.
  RaverieSpirVFrontEndRef mFrontEndTranslator;

  // Core libraries built once.
  RaverieShaderIRLibraryRef mCoreLibrary;
  RaverieShaderIRLibraryRef mShaderIntrinsicsLibrary;

  RaverieShaderIRProject mFragmentsProject;

  Array<String> mCoreVertexFragments;
  Array<String> mRenderPassFragments;

  RaverieFragmentTypeMap mFragmentTypes;

  // Raverie libraries to internal libraries.
  ExternalToInternalLibraryMap mCurrentToInternal;
  ExternalToInternalLibraryMap mPendingToPendingInternal;

  HashMap<Library*, RaverieFragmentTypeMap> mPendingFragmentTypes;

  HashMap<String, u32> mSamplerAttributeValues;
};

} // namespace Raverie
