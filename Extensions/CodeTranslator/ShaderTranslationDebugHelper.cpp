///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------FragmentSearchProvider
FragmentSearchProvider::FragmentSearchProvider(StringParam attribute)
{
  mAttribute = attribute;
}

void FragmentSearchProvider::Search(SearchData& search)
{
  ZilchShaderGenerator* generator = Z::gEngine->has(GraphicsEngine)->mShaderGenerator;
  ZilchShaderLibraryRef shaderLibrary = generator->GetCurrentInternalProjectLibrary();

  // Search this library and all dependencies
  HashSet<ZilchShaderLibrary*> visitedLibraries;
  Search(search, shaderLibrary, visitedLibraries);
}

void FragmentSearchProvider::Search(SearchData& search, ZilchShaderLibrary* shaderLibrary, HashSet<ZilchShaderLibrary*>& visitedLibraries)
{
  forRange(ShaderType* shaderType, shaderLibrary->mTypes.Values())
  {
    if(!shaderType->ContainsAttribute(mAttribute))
      continue;

    int priority = PartialMatch(search.SearchString.All(), shaderType->mZilchName.All(), CaseInsensitiveCompare);
    if(priority != cNoMatch)
    {
      //Add a match
      SearchViewResult& result = search.Results.PushBack();
      result.Data = shaderType;
      result.Interface = this;
      result.Name = shaderType->mZilchName;
      result.Priority = priority;
    }
  }

  // Handle having no dependencies
  ZilchShaderModule* dependencies = shaderLibrary->mDependencies;
  if(dependencies == nullptr)
    return;

  // Walk all dependencies (making sure to not walk any library twice)
  for(size_t i = 0; i < dependencies->Size(); ++i)
  {
    ZilchShaderLibrary* dependency = (*dependencies)[i];
    if(visitedLibraries.Contains(dependency))
      return;
    visitedLibraries.Insert(dependency);
    Search(search, dependency, visitedLibraries);
  }
}

//-------------------------------------------------------------------ShaderLanguageEntry
ShaderLanguageEntry::ShaderLanguageEntry(BaseShaderTranslator* translator)
{
  mTranslator = translator;
}
String ShaderLanguageEntry::ToString(bool shortFormat) const
{
  return mTranslator->GetFullLanguageString();
}

//-------------------------------------------------------------------ShaderTranslationEntry
ShaderTranslationEntry::ShaderTranslationEntry(Lexer::Enum lexerType, StringParam name, StringParam value)
{
  mLexerType = lexerType;
  mName = name;
  mValue = value;
}

String ShaderTranslationEntry::ToString(bool shortFormat) const
{
  return mName;
}

//-------------------------------------------------------------------ShaderTranslationDebugHelper
ShaderTranslationDebugHelper::ShaderTranslationDebugHelper(Composite* parent)
  : Composite(parent), mLanguagesDataSource(&mLanguages),
    mTranslationEntriesDataSource(&mTranslationEntries), mShaderGenerator(nullptr)
{
  SetName("Shader Translation Debug Helper");

  // Set our allowed translation languages
  mLanguages.PushBack(ShaderLanguageEntry(new Glsl130Translator()));
  mLanguages.PushBack(ShaderLanguageEntry(new Glsl150Translator()));

  // Create the root as a row layout
  SetLayout(CreateRowLayout());

  // Start with a panel to the left for all of the main options (core vertex, material, etc...)
  Composite* leftPanel = new Composite(this);
  leftPanel->SetLayout(CreateStackLayout());
  leftPanel->SetSizing(SizeAxis::X, SizePolicy::Fixed, 150);

  // Create a button + label for selecting the core vertex
  Label* coreVertexLabel = new Label(leftPanel);
  coreVertexLabel->SetText("CoreVertex");
  mCoreVertexTextBox = new TextBox(leftPanel);
  mCoreVertexTextBox->SetText("MeshVertex");
  ConnectThisTo(mCoreVertexTextBox, Events::MouseDown, OnCoreVertexClicked);

  // Create a button + label for selecting the material
  Label* materialLabel = new Label(leftPanel);
  materialLabel->SetText("Material");
  mMaterialTextBox = new TextBox(leftPanel);
  mMaterialTextBox->SetText(MaterialManager::GetInstance()->GetDefaultResource()->Name);
  ConnectThisTo(mMaterialTextBox, Events::MouseDown, OnMaterialClicked);

  // Create a button + label for selecting the render pass fragment
  Label* renderPassLabel = new Label(leftPanel);
  renderPassLabel->SetText("RenderPass");
  mRenderPassTextBox = new TextBox(leftPanel);
  mRenderPassTextBox->SetText("ForwardPass");
  ConnectThisTo(mRenderPassTextBox, Events::MouseDown, OnRenderPassClicked);
  
  // Create a drop-down combo-box to select the target translation language
  Label* translationModeLanguage = new Label(leftPanel);
  translationModeLanguage->SetText("Translation Language");
  mTranslationModeComboBox = new ComboBox(leftPanel);
  mTranslationModeComboBox->SetListSource(&mLanguagesDataSource);
  mTranslationModeComboBox->SetSelectedItem(1, false);

  // Finally, create a button to run the tests
  TextButton* runTranslationButton = new TextButton(leftPanel);
  runTranslationButton->SetText("RunTest");
  ConnectThisTo(runTranslationButton, Events::MouseDown, OnRunTranslation);
  
  // Create a splitter to divide the left and center panel
  Splitter* splitter1 = new Splitter(this);
  splitter1->SetSize(Pixels(10, 10));

  // Create a script editor in the center
  mScriptEditor = new ScriptEditor(this);
  mScriptEditor->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);

  // Create a splitter between the center and right panel
  Splitter* splitter2 = new Splitter(this);
  splitter2->SetSize(Pixels(10, 10));

  // Currently, there's no easy way to have a tabbed view as a child composite.
  // So instead, to allow the user to switch between all of the results we populate a list box on the right side.
  mAvailableScriptsListBox = new ListBox(this);
  mAvailableScriptsListBox->SetDataSource(&mTranslationEntriesDataSource);
  mAvailableScriptsListBox->SetSizing(SizeAxis::X, SizePolicy::Fixed, 150);
  mAvailableScriptsListBox->SetSelectedItem(0, false);
  ConnectThisTo(mAvailableScriptsListBox, Events::ItemSelected, OnScriptDisplayChanged);
}

SearchView* ShaderTranslationDebugHelper::CreateSearchView(SearchProvider* searchProvider, Array<String>& hiddenTags)
{
  Mouse* mouse = Z::gMouse;
  // Create search window at button
  FloatingSearchView* viewPopUp = new FloatingSearchView(this);
  viewPopUp->SetSize(Pixels(300, 400));
  viewPopUp->ShiftOntoScreen(ToVector3(mouse->GetClientPosition()));
  viewPopUp->UpdateTransformExternal();
  mActiveSearch = viewPopUp;

  GraphicsEngine* graphicsEngine = Z::gEngine->has(GraphicsEngine);
  graphicsEngine->mShaderGenerator->mCoreVertexFragments;

  // Filter based upon the provided search provider
  SearchView* searchView = viewPopUp->mView;
  searchView->mSearch->SearchProviders.PushBack(searchProvider);
  for(size_t i = 0; i < hiddenTags.Size(); ++i)
    searchView->AddHiddenTag(hiddenTags[i]);
  searchView->Search(String());
  searchView->TakeFocus();

  return searchView;
}

SearchView* ShaderTranslationDebugHelper::CreateFragmentSearchView(StringParam attribute)
{
  Array<String> hiddenTags = Array<String>(ZeroInit, "Resources", "ZilchFragment");
  // Create a search view to filter zilch fragments that have the provided attribute
  SearchView* searchView = CreateSearchView(new FragmentSearchProvider(attribute), hiddenTags);
  searchView->Search(String());
  return searchView;
}

void ShaderTranslationDebugHelper::OnCoreVertexClicked(Event* e)
{
  // Create a search view for core vertex fragments
  SearchView* searchView = CreateFragmentSearchView("CoreVertex");
  ConnectThisTo(searchView, Events::SearchCompleted, OnCoreVertexSelected);
}

void ShaderTranslationDebugHelper::OnCoreVertexSelected(SearchViewEvent* e)
{
  mCoreVertexTextBox->SetText(e->Element->Name);
  mActiveSearch->Destroy();
}

void ShaderTranslationDebugHelper::OnMaterialClicked(Event* e)
{
  // Create a search view for materials
  Array<String> hiddenTags = Array<String>(ZeroInit, "Resources", "Material");
  SearchView* searchView = CreateSearchView(GetResourceSearchProvider(), hiddenTags);
  searchView->Search(String());
  ConnectThisTo(searchView, Events::SearchCompleted, OnMaterialSelected);
}

void ShaderTranslationDebugHelper::OnMaterialSelected(SearchViewEvent* e)
{
  mMaterialTextBox->SetText(e->Element->Name);
  mActiveSearch->Destroy();
}

void ShaderTranslationDebugHelper::OnRenderPassClicked(Event* e)
{
  // Create a search view for render pass fragments
  SearchView* searchView = CreateFragmentSearchView("RenderPass");
  ConnectThisTo(searchView, Events::SearchCompleted, OnRenderPassSelected);
}

void ShaderTranslationDebugHelper::OnRenderPassSelected(SearchViewEvent* e)
{
  mRenderPassTextBox->SetText(e->Element->Name);
  mActiveSearch->Destroy();
}

void ShaderTranslationDebugHelper::OnCompileZilchFragments(ZilchCompileFragmentEvent* event)
{
  if(mShaderGenerator == nullptr)
    return;

  mShaderGenerator->BuildFragmentsLibrary(event->mDependencies, event->mFragments);

  // This basically transfers all pending libraries into current libraries.
  // This is basically the logic of commit but we have to do this here. This seems to be an issue when we haven't 
  // been listening for events since the startup so we're missing all of the initial libraries in the map. I believe
  // on load it calls PrePatch after each library instead of at the end of all of them. 
  AutoDeclare(range, mShaderGenerator->mPendingToPendingInternal.All());
  for(; !range.Empty(); range.PopFront())
  {
    // Use the returned library here instead of the one in the map. The one in the map is garbage.
    Library* externalLibrary = event->mReturnedLibrary;
    ZilchShaderLibraryRef shaderLibrary = range.Front().second;
    
    mShaderGenerator->mCurrentToInternal.Insert(externalLibrary, shaderLibrary);
  }
  mShaderGenerator->mPendingToPendingInternal.Clear();
  mShaderGenerator->MapFragmentTypes();
}

void ShaderTranslationDebugHelper::OnScriptsCompiledPrePatch(ZilchCompileEvent* event)
{
  if(mShaderGenerator == nullptr)
    return;

  // Probably not necessary here since OnCompileZilchFragments basically
  // takes care of everything. Leaving this in just in case.
  mShaderGenerator->Commit(event);
}

void ShaderTranslationDebugHelper::OnScriptCompilationFailed(Event* event)
{
  // We failed to compile so null out the shader generator for as our "flag"
  mShaderGenerator = nullptr;
}

void ShaderTranslationDebugHelper::ValidateComposition(ZilchShaderGenerator& generator, ZilchFragmentInfo& info, FragmentType::Enum fragmentType)
{
  ShaderFragmentsInfoMap* fragmentMap = info.mFragmentMap;
  if(fragmentMap == nullptr)
    return;

  // Keep track of errors if we found them (for formatting we group them together)
  bool wasError = false;
  StringBuilder errorsBuilder;
  errorsBuilder.AppendFormat("%s Shaders:\n", FragmentType::Names[fragmentType]);

  forRange(ShaderFragmentCompositeInfo* compositeInfo, fragmentMap->mFragments.Values())
  {
    forRange(ShaderFieldCompositeInfo& fieldInfo, compositeInfo->mFieldMap.Values())
    {
      if(fieldInfo.mInputError)
      {
        wasError = true;

        // Get the shader field that had an error
        String fragmentName = compositeInfo->mFragmentName;
        String fieldName = fieldInfo.mZilchName;
        ShaderType* fragmentShaderType = generator.GetCurrentInternalProjectLibrary()->FindType(fragmentName);
        ShaderField* shaderField = fragmentShaderType->FindField(fieldName);
        // Build up a string of all of the attributes to make it easier for a user to see what went wrong
        StringBuilder attributesBuilder;
        forRange(ShaderAttribute& attribute, shaderField->mAttributes.All())
        {
          attributesBuilder.AppendFormat("[%s]", attribute.mAttributeName.c_str());
        }

        // Print out fragment and field with the error.
        String attributesList = attributesBuilder.ToString();
        errorsBuilder.AppendFormat("\tCouldn't resolve input attributes on '%s.%s'. Provided attributes were '%s'\n",
          fragmentName.c_str(), fieldName.c_str(), attributesList.c_str());
      }
    }
  }

  // If we had an error then print out the full list for this fragment type.
  if(wasError)
  {
    String errorStr = errorsBuilder.ToString();
    ZPrint(errorStr.c_str());
  }
}

void ShaderTranslationDebugHelper::OnRunTranslation(Event* e)
{
  // Clear the old results
  mTranslationEntries.Clear();
  
  // Get the core vertex, material, and render pass values
  Material* material = MaterialManager::GetInstance()->Find(mMaterialTextBox->GetText());
  ErrorIf(material ==  nullptr, "Invalid material selected");
  String coreVertexName = mCoreVertexTextBox->GetText();
  String renderPassName = mRenderPassTextBox->GetText();
  String compositeName = material->mCompositeName;
  String shaderName = BuildString(coreVertexName, compositeName, renderPassName);
  int index = mTranslationModeComboBox->GetSelectedItem();
  BaseShaderTranslator* translator = mLanguagesDataSource[index].mTranslator;
  
  // Generate a unique composite to represent this CoreVertex + Material + RenderPass
  UniqueComposite composition;
  composition.mName = compositeName;
  composition.mFragmentNames = material->mFragmentNames;
  composition.mFragmentNameMap.Append(composition.mFragmentNames.All());
  HashMap<String, UniqueComposite> composites;
  composites.Insert(compositeName, composition);
  // Generate the shader
  Shader shader;
  shader.mComposite = compositeName;
  shader.mCoreVertex = coreVertexName;
  shader.mRenderPass = renderPassName;
  shader.mName = shaderName;
  ShaderSet shaders;
  shaders.Insert(&shader);
  
  // Create and initialize the shader generator
  ZilchShaderGenerator generator;
  generator.mTranslator = translator;
  generator.Initialize();
  // Store a pointer to it so we can access this in callbacks
  // (no need to allocate and a fresh generate each time is more "proper")
  mShaderGenerator = &generator;

  // Listen for all compilation events on the zilch manager
  ZilchManager* zilchManager = ZilchManager::GetInstance();
  ConnectThisTo(zilchManager, Events::CompileZilchFragments, OnCompileZilchFragments);
  ConnectThisTo(zilchManager, Events::ScriptsCompiledPrePatch, OnScriptsCompiledPrePatch);
  ConnectThisTo(zilchManager, Events::ScriptCompilationFailed, OnScriptCompilationFailed);

  // Force modify and recompile all zilch fragments
  forRange(ResourceLibrary* resourceLibrary, Z::gResources->LoadedResourceLibraries.Values())
    resourceLibrary->FragmentsModified();
  zilchManager->Compile();

  // Disconnect all events for compilation
  EventDispatcher* dispatcher = zilchManager->GetDispatcher();
  dispatcher->DisconnectEvent(Events::CompileZilchFragments, this);
  dispatcher->DisconnectEvent(Events::ScriptsCompiledPrePatch, this);
  dispatcher->DisconnectEvent(Events::ScriptCompilationFailed, this);

  // Check if compilation failed
  if(mShaderGenerator == nullptr)
  {
    mScriptEditor->SetAllText("Compilation Failed");
    return;
  }
  // Clear the generator since it'll go out of scope after this
  mShaderGenerator = nullptr;
  
  // Build the shader library for this composite
  Array<ShaderEntry> shaderEntries;
  Array<ZilchShaderDefinition> compositeDefinitions;
  generator.BuildShaders(shaders, composites, shaderEntries, &compositeDefinitions);
  
  // Add an entry for each shader stage, both for the zilch and shader composite results
  ZilchShaderDefinition& shaderDef = compositeDefinitions[0];
  ShaderEntry& entry = shaderEntries[0];
  ZilchFragmentInfo& vertexInfo = shaderDef.mShaderData[FragmentType::Vertex];
  ZilchFragmentInfo& geometryInfo = shaderDef.mShaderData[FragmentType::Geometry];
  ZilchFragmentInfo& pixelInfo = shaderDef.mShaderData[FragmentType::Pixel];
  // Validate that there weren't any incorrectly resolved compositions
  ValidateComposition(generator, vertexInfo, FragmentType::Vertex);
  ValidateComposition(generator, geometryInfo, FragmentType::Geometry);
  ValidateComposition(generator, pixelInfo, FragmentType::Pixel);
  
  mTranslationEntries.PushBack(ShaderTranslationEntry(Lexer::Zilch, "ZilchVertex", vertexInfo.mZilchCode));
  mTranslationEntries.PushBack(ShaderTranslationEntry(Lexer::Zilch, "ZilchGeometry", geometryInfo.mZilchCode));
  mTranslationEntries.PushBack(ShaderTranslationEntry(Lexer::Zilch, "ZilchPixel", pixelInfo.mZilchCode));
  mTranslationEntries.PushBack(ShaderTranslationEntry(Lexer::Shader, "ShaderVertex", entry.mVertexShader));
  mTranslationEntries.PushBack(ShaderTranslationEntry(Lexer::Shader, "ShaderGeometry", entry.mGeometryShader));
  mTranslationEntries.PushBack(ShaderTranslationEntry(Lexer::Shader, "ShaderPixel", entry.mPixelShader));
  
  for(size_t i = 0; i < shaderDef.mFragmentTypes.Size(); ++i)
  {
    ShaderType* fragment = shaderDef.mFragmentTypes[i];
    
    ShaderTypeTranslation result;
    translator->BuildFinalShader(fragment, result);
    mTranslationEntries.PushBack(ShaderTranslationEntry(Lexer::Shader, fragment->mZilchName, result.mTranslation));
  }
  
  // Reselect the current item with our new translations
  OnScriptDisplayChanged(nullptr);
}

void ShaderTranslationDebugHelper::OnScriptDisplayChanged(Event* e)
{
  int index = mAvailableScriptsListBox->GetSelectedItem();
  if(index < 0 || index >= (int)mTranslationEntries.Size())
  {
    mScriptEditor->SetAllText(String());
    return;
  }

  ShaderTranslationEntry& entry = mTranslationEntries[index];
  mScriptEditor->SetAllText(entry.mValue);
  mScriptEditor->SetLexer(entry.mLexerType);
}

}//namespace Zero
