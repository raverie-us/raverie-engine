// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class ScriptEditor;
class SearchViewEvent;
class SearchView;
class FloatingSearchView;

// Helper to filter raverie fragments based upon an attribute
class FragmentSearchProvider : public SearchProvider
{
public:
  FragmentSearchProvider(StringParam attribute);
  void Search(SearchData& search) override;
  void Search(SearchData& search, RaverieShaderIRLibrary* shaderLibrary, HashSet<RaverieShaderIRLibrary*>& visitedLibraries);

  // The attribute to filter fragments by
  String mAttribute;
};

// Entry for a data source that represents a target translation language
class ShaderLanguageEntry
{
public:
  ShaderLanguageEntry();

  String ToString(bool shortFormat = false) const;

  String mName;

  typedef Raverie::Ref<RaverieShaderIRBackend> BackendPassRef;
  BackendPassRef mBackend;
};

// A result from debugging shader translation. This entry might not
// actually be a translation as it could also be the raverie composite.
class ShaderTranslationEntry
{
public:
  ShaderTranslationEntry()
  {
  }
  ShaderTranslationEntry(Lexer::Enum lexerType, StringParam name, StringParam value);
  String ToString(bool shortFormat = false) const;

  // The name of this entry (typically the fragment name or something like
  // "RaverieVertex").
  String mName;
  // The translation string
  String mValue;
  // The lexer type to use for display since multiple languages can be present
  // in the same list.
  Lexer::Enum mLexerType;
};

/// Widget to aid in debugging shader translation. Lets the user pick the
/// material along with the core vertex type and render pass and then see the
/// resulting composites and translations.
class ShaderTranslationDebugHelper : public Composite
{
public:
  typedef Raverie::Ref<ShaderTranslationPassResult> TranslationPassResultRef;
  typedef ShaderTranslationDebugHelper RaverieSelf;
  ShaderTranslationDebugHelper(Composite* parent);

private:
  // Helper to create a search view with tags
  SearchView* CreateSearchView(SearchProvider* searchProvider, Array<String>& hiddenTags);
  // Helper to create a search view for filtering fragments by an attribute
  SearchView* CreateFragmentSearchView(StringParam attribute);

  void CreateGlslShaderLanguageEntry(int version, bool es);

  void OnCoreVertexClicked(Event* e);
  void OnCoreVertexSelected(SearchViewEvent* e);
  void OnMaterialClicked(Event* e);
  void OnMaterialSelected(SearchViewEvent* e);
  void OnRenderPassClicked(Event* e);
  void OnRenderPassSelected(SearchViewEvent* e);
  void OnCompileRaverieFragments(RaverieCompileFragmentEvent* event);
  void OnScriptsCompiledPrePatch(RaverieCompileEvent* event);
  void OnScriptCompilationFailed(Event* event);
  // void ValidateComposition(RaverieShaderGenerator& generator,
  // RaverieFragmentInfo& info, FragmentType::Enum fragmentType);
  void OnRunTranslation(Event* e);
  void OnScriptDisplayChanged(Event* e);

  RaverieShaderIRLibraryRef BuildShaderLibrary(RaverieShaderGenerator& generator, RaverieShaderIRCompositor::ShaderDefinition& shaderDef);
  bool CompilePipeline(RaverieShaderIRType* shaderType, ShaderPipelineDescription& pipeline, Array<TranslationPassResultRef>& pipelineResults, Array<TranslationPassResultRef>& debugPipelineResults);

  TextBox* mCoreVertexTextBox;
  TextBox* mMaterialTextBox;
  TextBox* mRenderPassTextBox;
  ComboBox* mTranslationModeComboBox;
  ScriptEditor* mScriptEditor;
  ListBox* mAvailableScriptsListBox;
  TextCheckBox* mOptimizerCheckBox;
  FloatingSearchView* mActiveSearch;
  RaverieShaderGenerator* mShaderGenerator;

  typedef Array<ShaderLanguageEntry> LanguageArray;
  typedef Array<ShaderTranslationEntry> TranslationEntryArray;
  // The possible languages to translate to
  LanguageArray mLanguages;
  // The results from translation
  TranslationEntryArray mTranslationEntries;

  RaverieShaderIRProject mShaderProject;

  // Data sources needed for binding
  ContainerSource<LanguageArray> mLanguagesDataSource;
  ContainerSource<TranslationEntryArray> mTranslationEntriesDataSource;
};

} // namespace Raverie
