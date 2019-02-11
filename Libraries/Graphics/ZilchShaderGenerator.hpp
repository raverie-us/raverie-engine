// MIT Licensed (see LICENSE.md).

#pragma once

namespace Zero
{

ZilchShaderGenerator* CreateZilchShaderGenerator();

DeclareEnum5(ZilchFragmentType,
             CoreVertex,
             RenderPass,
             PostProcess,
             Protected,
             Fragment);
typedef HashMap<String, ZilchFragmentType::Enum> ZilchFragmentTypeMap;

// User data written per shader type. Used to deal with efficient re-compiling
// of fragments/shaders.
class FragmentUserData
{
public:
  FragmentUserData(){};
  FragmentUserData(StringParam resourceName) : mResourceName(resourceName){};
  String mResourceName;
};

class ZilchShaderGenerator : public Zilch::EventHandler
{
public:
  typedef HashMap<LibraryRef, ZilchShaderIRLibraryRef>
      ExternalToInternalLibraryMap;
  typedef ExternalToInternalLibraryMap::valuerange InternalLibraryRange;
  typedef ZilchShaderIRCompositor::ShaderDefinition ShaderDefinition;
  typedef Zilch::Ref<ShaderTranslationPassResult> TranslationPassResultRef;
  typedef Zilch::Ref<SimplifiedShaderReflectionData> SimplifiedReflectionRef;
  typedef Zilch::Ref<ZilchSpirVFrontEnd> ZilchSpirVFrontEndRef;

  ZilchShaderGenerator();
  ~ZilchShaderGenerator();

  void Initialize();
  void InitializeSpirV();

  LibraryRef BuildFragmentsLibrary(Module& dependencies,
                                   Array<ZilchDocumentResource*>& fragments,
                                   StringParam libraryName = "Fragments");
  bool Commit(ZilchCompileEvent* e);
  void MapFragmentTypes();

  bool BuildShaders(ShaderSet& shaders,
                    HashMap<String, UniqueComposite>& composites,
                    Array<ShaderEntry>& shaderEntries,
                    Array<ShaderDefinition>* compositeShaderDefs = nullptr);
  bool CompilePipeline(ZilchShaderIRType* shaderType,
                       ShaderPipelineDescription& pipeline,
                       Array<TranslationPassResultRef>& pipelineResults);

  ShaderInput CreateShaderInput(StringParam fragmentName,
                                StringParam inputName,
                                ShaderInputType::Enum type,
                                AnyParam value);

  void OnZilchFragmentCompilationError(Zilch::ErrorEvent* event);
  void OnZilchFragmentTypeParsed(Zilch::ParseEvent* event);
  void OnZilchFragmentTranslationError(TranslationErrorEvent* event);
  void OnZilchFragmentValidationError(ValidationErrorEvent* event);

  ZilchShaderIRLibraryRef GetInternalLibrary(LibraryRef library);
  ZilchShaderIRLibraryRef GetCurrentInternalLibrary(LibraryRef library);
  ZilchShaderIRLibraryRef GetPendingInternalLibrary(LibraryRef library);
  InternalLibraryRange GetCurrentInternalLibraries();
  ZilchShaderIRLibraryRef GetCurrentInternalProjectLibrary();
  ZilchShaderIRLibraryRef GetPendingInternalProjectLibrary();

  // The shared settings to be used through all of the compositing/translation
  ZilchShaderSpirVSettingsRef mSpirVSettings;
  // The front-end translator used.
  ZilchSpirVFrontEndRef mFrontEndTranslator;

  // Core libraries built once.
  ZilchShaderIRLibraryRef mCoreLibrary;
  ZilchShaderIRLibraryRef mShaderIntrinsicsLibrary;

  ZilchShaderIRProject mFragmentsProject;

  Array<String> mCoreVertexFragments;
  Array<String> mRenderPassFragments;

  ZilchFragmentTypeMap mFragmentTypes;

  // Zero libraries to internal libraries.
  ExternalToInternalLibraryMap mCurrentToInternal;
  ExternalToInternalLibraryMap mPendingToPendingInternal;

  HashMap<Library*, ZilchFragmentTypeMap> mPendingFragmentTypes;

  HashMap<String, u32> mSamplerAttributeValues;
};

} // namespace Zero
