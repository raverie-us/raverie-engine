#pragma once

namespace Zero
{

ZilchShaderGenerator* CreateZilchShaderGenerator();

DeclareEnum5(ZilchFragmentType, CoreVertex, RenderPass, PostProcess, Protected, Fragment);
typedef HashMap<String, ZilchFragmentType::Enum> ZilchFragmentTypeMap;

// User data written per shader type. Used to deal with efficient re-compiling of fragments/shaders.
class FragmentUserData
{
public:
  FragmentUserData() {};
  FragmentUserData(StringParam resourceName) : mResourceName(resourceName) {};
  String mResourceName;
};

class ZilchShaderGenerator : public Zilch::EventHandler
{
public:
  typedef HashMap<LibraryRef, ZilchShaderLibraryRef> ExternalToInternalLibraryMap;
  typedef ExternalToInternalLibraryMap::valuerange InternalLibraryRange;

  ZilchShaderGenerator();
  ~ZilchShaderGenerator();

  void Initialize();

  LibraryRef BuildFragmentsLibrary(Module& dependencies, Array<ZilchDocumentResource*>& fragments, StringParam libraryName = "Fragments");
  bool Commit(ZilchCompileEvent* e);
  void MapFragmentTypes();

  bool BuildShaders(ShaderSet& shaders, HashMap<String, UniqueComposite>& composites, Array<ShaderEntry>& shaderEntries, Array<ZilchShaderDefinition>* compositeShaderDefs = nullptr);

  ShaderInput CreateShaderInput(StringParam fragmentName, StringParam inputName, ShaderInputType::Enum type, AnyParam value);

  void OnZilchFragmentCompilationError(Zilch::ErrorEvent* event);
  void OnZilchFragmentTranslationError(TranslationErrorEvent* event);
  static void OnFragmentProjectPostSyntaxer(ParseEvent* e);

  ZilchShaderLibraryRef GetInternalLibrary(LibraryRef library);
  ZilchShaderLibraryRef GetCurrentInternalLibrary(LibraryRef library);
  ZilchShaderLibraryRef GetPendingInternalLibrary(LibraryRef library);
  InternalLibraryRange GetCurrentInternalLibraries();
  ZilchShaderLibraryRef GetCurrentInternalProjectLibrary();
  ZilchShaderLibraryRef GetPendingInternalProjectLibrary();

  ZilchShaderSettingsRef mSettings;
  BaseShaderTranslatorRef mTranslator;

  ZilchShaderLibraryRef mZilchCoreLibrary;
  ZilchShaderLibraryRef mIntrinsicsLibrary;

  ZilchShaderProject mFragmentsProject;

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
