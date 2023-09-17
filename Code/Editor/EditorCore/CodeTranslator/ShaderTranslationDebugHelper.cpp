// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

FragmentSearchProvider::FragmentSearchProvider(StringParam attribute) : SearchProvider("Fragment")
{
  mAttribute = attribute;
}

void FragmentSearchProvider::Search(SearchData& search)
{
  ZilchShaderGenerator* generator = Z::gEngine->has(GraphicsEngine)->mShaderGenerator;
  ZilchShaderIRLibraryRef shaderLibrary = generator->GetCurrentInternalProjectLibrary();

  // Search this library and all dependencies
  HashSet<ZilchShaderIRLibrary*> visitedLibraries;
  Search(search, shaderLibrary, visitedLibraries);
}

void FragmentSearchProvider::Search(SearchData& search,
                                    ZilchShaderIRLibrary* shaderLibrary,
                                    HashSet<ZilchShaderIRLibrary*>& visitedLibraries)
{
  forRange (ZilchShaderIRType* shaderType, shaderLibrary->mTypes.Values())
  {
    ShaderIRTypeMeta* shaderTypeMeta = shaderType->mMeta;
    if (shaderTypeMeta == nullptr)
      continue;

    if (!shaderTypeMeta->ContainsAttribute(mAttribute))
      continue;

    int priority = PartialMatch(search.SearchString.All(), shaderTypeMeta->mZilchName.All(), CaseInsensitiveCompare);
    if (priority != cNoMatch)
    {
      // Add a match
      SearchViewResult& result = search.Results.PushBack();
      result.Data = shaderTypeMeta;
      result.Interface = this;
      result.Name = shaderTypeMeta->mZilchName;
      result.Priority = priority;
    }
  }

  // Handle having no dependencies
  ZilchShaderIRModule* dependencies = shaderLibrary->mDependencies;
  if (dependencies == nullptr)
    return;

  // Walk all dependencies (making sure to not walk any library twice)
  for (size_t i = 0; i < dependencies->Size(); ++i)
  {
    ZilchShaderIRLibrary* dependency = (*dependencies)[i];
    if (visitedLibraries.Contains(dependency))
      return;
    visitedLibraries.Insert(dependency);
    Search(search, dependency, visitedLibraries);
  }
}

ShaderLanguageEntry::ShaderLanguageEntry()
{
  mBackend = nullptr;
}

String ShaderLanguageEntry::ToString(bool shortFormat) const
{
  return mName;
}

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

ShaderTranslationDebugHelper::ShaderTranslationDebugHelper(Composite* parent) :
    Composite(parent),
    mShaderGenerator(nullptr),
    mShaderProject("ShaderProject"),
    mLanguagesDataSource(&mLanguages),
    mTranslationEntriesDataSource(&mTranslationEntries)
{
  SetName("Shader Translation Debug Helper");

  // Set our allowed translation languages
  CreateGlslShaderLanguageEntry(130, false);
  CreateGlslShaderLanguageEntry(150, false);
  CreateGlslShaderLanguageEntry(330, false);
  CreateGlslShaderLanguageEntry(400, false);
  CreateGlslShaderLanguageEntry(410, false);
  CreateGlslShaderLanguageEntry(420, false);
  CreateGlslShaderLanguageEntry(430, false);
  CreateGlslShaderLanguageEntry(440, false);
  CreateGlslShaderLanguageEntry(450, false);
  CreateGlslShaderLanguageEntry(460, false);
  CreateGlslShaderLanguageEntry(100, true);
  CreateGlslShaderLanguageEntry(300, true);

  // Create the root as a row layout
  SetLayout(CreateRowLayout());

  // Start with a panel to the left for all of the main options (core vertex,
  // material, etc...)
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

  mOptimizerCheckBox = new TextCheckBox(leftPanel);
  mOptimizerCheckBox->SetText("Optimize");
  mOptimizerCheckBox->SetChecked(false);

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
  // So instead, to allow the user to switch between all of the results we
  // populate a list box on the right side.
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

  // GraphicsEngine* graphicsEngine = Z::gEngine->has(GraphicsEngine);
  // graphicsEngine->mShaderGenerator->mCoreVertexFragments;

  // Filter based upon the provided search provider
  SearchView* searchView = viewPopUp->mView;
  searchView->mSearch->SearchProviders.PushBack(searchProvider);
  for (size_t i = 0; i < hiddenTags.Size(); ++i)
    searchView->AddHiddenTag(hiddenTags[i]);
  searchView->Search(String());
  searchView->TakeFocus();

  return searchView;
}

SearchView* ShaderTranslationDebugHelper::CreateFragmentSearchView(StringParam attribute)
{
  Array<String> hiddenTags = Array<String>(ZeroInit, "Resources", "ZilchFragment");
  // Create a search view to filter zilch fragments that have the provided
  // attribute
  SearchView* searchView = CreateSearchView(new FragmentSearchProvider(attribute), hiddenTags);
  searchView->Search(String());
  return searchView;
}

void ShaderTranslationDebugHelper::CreateGlslShaderLanguageEntry(int version, bool es)
{
  ZeroZilchShaderGlslBackend* glslBackend = new ZeroZilchShaderGlslBackend();
  glslBackend->mTargetVersion = version;
  glslBackend->mTargetGlslEs = es;

  ShaderLanguageEntry& entry = mLanguages.PushBack();
  entry.mName = String::Format("Glsl%d%s", version, es ? "Es" : "");
  entry.mBackend = glslBackend;
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
  if (mShaderGenerator == nullptr)
    return;

  mShaderGenerator->BuildFragmentsLibrary(event->mDependencies, event->mFragments);

  // This basically transfers all pending libraries into current libraries.
  // This is basically the logic of commit but we have to do this here. This
  // seems to be an issue when we haven't been listening for events since the
  // startup so we're missing all of the initial libraries in the map. I believe
  // on load it calls PrePatch after each library instead of at the end of all
  // of them.
  AutoDeclare(range, mShaderGenerator->mPendingToPendingInternal.All());
  for (; !range.Empty(); range.PopFront())
  {
    // Use the returned library here instead of the one in the map. The one in
    // the map is garbage.
    Library* externalLibrary = event->mReturnedLibrary;
    ZilchShaderIRLibraryRef shaderLibrary = range.Front().second;

    mShaderGenerator->mCurrentToInternal.Insert(externalLibrary, shaderLibrary);
  }
  mShaderGenerator->mPendingToPendingInternal.Clear();
  mShaderGenerator->MapFragmentTypes();
}

void ShaderTranslationDebugHelper::OnScriptsCompiledPrePatch(ZilchCompileEvent* event)
{
  if (mShaderGenerator == nullptr)
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
//
// void ShaderTranslationDebugHelper::ValidateComposition(ZilchShaderGenerator&
// generator, ZilchFragmentInfo& info, FragmentType::Enum fragmentType)
//{
//  ShaderFragmentsInfoMap* fragmentMap = info.mFragmentMap;
//  if(fragmentMap == nullptr)
//    return;
//
//  // Keep track of errors if we found them (for formatting we group them
//  together) bool wasError = false; StringBuilder errorsBuilder;
//  errorsBuilder.AppendFormat("%s Shaders:\n",
//  FragmentType::Names[fragmentType]);
//
//  forRange(ShaderFragmentCompositeInfo* compositeInfo,
//  fragmentMap->mFragments.Values())
//  {
//    forRange(ShaderFieldCompositeInfo& fieldInfo,
//    compositeInfo->mFieldMap.Values())
//    {
//      if(fieldInfo.mInputError)
//      {
//        wasError = true;
//
//        // Get the shader field that had an error
//        String fragmentName = compositeInfo->mFragmentName;
//        String fieldName = fieldInfo.mZilchName;
//        ZilchShaderIRType* fragmentShaderType =
//        generator.GetCurrentInternalProjectLibrary()->FindType(fragmentName);
//        ShaderIRFieldMeta* shaderField =
//        fragmentShaderType->mMeta->FindField(fieldName);
//        // Build up a string of all of the attributes to make it easier for a
//        user to see what went wrong StringBuilder attributesBuilder;
//        forRange(ShaderIRAttribute& attribute, shaderField->mAttributes.All())
//        {
//          attributesBuilder.AppendFormat("[%s]",
//          attribute.mAttributeName.c_str());
//        }
//
//        // Print out fragment and field with the error.
//        String attributesList = attributesBuilder.ToString();
//        // @JoshD: Need to update
//        //String noFallbackName =
//        generator.mSettings->mSpirVNameSettings.mNoFallbackWarningAttributeName;
//        //errorsBuilder.AppendFormat("\tCouldn't resolve input attributes on
//        '%s.%s'. Provided attributes were '%s'. "
//        //  "Input will fallback to the default value. To suppress this
//        warning use the attribute '[%s]'.\n",
//        //  fragmentName.c_str(), fieldName.c_str(), attributesList.c_str(),
//        noFallbackName.c_str());
//      }
//    }
//  }
//
//  // If we had an error then print out the full list for this fragment type.
//  if(wasError)
//  {
//    String errorStr = errorsBuilder.ToString();
//    ZPrint(errorStr.c_str());
//  }
//}

void ShaderTranslationDebugHelper::OnRunTranslation(Event* e)
{
  GraphicsEngine* graphicsEngine = Z::gEngine->has(GraphicsEngine);
  mShaderGenerator = graphicsEngine->mShaderGenerator;
  ZilchShaderGenerator& generator = *mShaderGenerator;

  // Clear the old results
  mTranslationEntries.Clear();

  // Listen for all compilation events on the zilch manager
  ZilchManager* zilchManager = ZilchManager::GetInstance();
  ConnectThisTo(zilchManager, Events::CompileZilchFragments, OnCompileZilchFragments);
  ConnectThisTo(zilchManager, Events::ScriptsCompiledPrePatch, OnScriptsCompiledPrePatch);
  ConnectThisTo(zilchManager, Events::ScriptCompilationFailed, OnScriptCompilationFailed);

  // Ideally we'd force compile all fragments but this can crash right now.
  // Z::gEditor->SaveAll(false, false);

  // Disconnect all events for compilation
  EventDispatcher* dispatcher = zilchManager->GetDispatcher();
  dispatcher->DisconnectEvent(Events::CompileZilchFragments, this);
  dispatcher->DisconnectEvent(Events::ScriptsCompiledPrePatch, this);
  dispatcher->DisconnectEvent(Events::ScriptCompilationFailed, this);

  // Check if compilation failed
  if (mShaderGenerator == nullptr)
  {
    mScriptEditor->SetAllText("Compilation Failed");
    return;
  }
  // Clear the generator since it'll go out of scope after this
  mShaderGenerator = nullptr;

  // Build the shader library for this material
  ZilchShaderIRCompositor::ShaderDefinition shaderDef;
  ZilchShaderIRLibraryRef shaderLibrary = BuildShaderLibrary(generator, shaderDef);
  if (shaderLibrary == nullptr)
    return;

  // Construct the pipeline to run using the selected backend
  ShaderPipelineDescription pipelineDescription;
  if (mOptimizerCheckBox->GetChecked())
    pipelineDescription.mToolPasses.PushBack(new SpirVOptimizerPass());
  pipelineDescription.mDebugPasses.PushBack(new ZilchSpirVDisassemblerBackend());
  int index = mTranslationModeComboBox->GetSelectedItem();
  pipelineDescription.mBackend = mLanguagesDataSource[index].mBackend;

  // Grab the zilch, shader, and disassembler results (put them in separate
  // lists to control the order.
  Array<ShaderTranslationEntry> zilchResultEntries;
  Array<ShaderTranslationEntry> shaderResultEntries;
  Array<ShaderTranslationEntry> disassemblyResultEntries;
  for (size_t i = 0; i < FragmentType::Size; ++i)
  {
    ZilchShaderIRCompositor::ShaderStageDescription& stageDesc = shaderDef.mResults[i];
    if (stageDesc.mShaderCode.Empty())
      continue;

    // Find the generated type for this shader stage
    ZilchShaderIRType* shaderType = shaderLibrary->FindType(stageDesc.mClassName);
    // Run the pipeline
    Array<TranslationPassResultRef> pipelineResults, debugPipelineResults;
    CompilePipeline(shaderType, pipelineDescription, pipelineResults, debugPipelineResults);

    String stageName = FragmentType::Names[i];
    // Generate the zilch entry.
    zilchResultEntries.PushBack(
        ShaderTranslationEntry(Lexer::Zilch, BuildString("Zilch", stageName), stageDesc.mShaderCode));
    // Generate the translated shader entry.
    TranslationPassResultRef passResult = pipelineResults.Back();
    shaderResultEntries.PushBack(
        ShaderTranslationEntry(Lexer::Shader, BuildString("Shader", stageName), passResult->ToString()));
    // Generate the spir-v disassembler entry
    TranslationPassResultRef disassemblyPassResult = debugPipelineResults[0];
    disassemblyResultEntries.PushBack(
        ShaderTranslationEntry(Lexer::SpirV, BuildString("SpirV", stageName), disassemblyPassResult->ToString()));
  }

  // Re-order the entries so it's all zilch, then all shader, then all
  // disassembly
  mTranslationEntries.Insert(mTranslationEntries.End(), zilchResultEntries.All());
  mTranslationEntries.Insert(mTranslationEntries.End(), shaderResultEntries.All());
  mTranslationEntries.Insert(mTranslationEntries.End(), disassemblyResultEntries.All());

  OnScriptDisplayChanged(nullptr);
}

void ShaderTranslationDebugHelper::OnScriptDisplayChanged(Event* e)
{
  int index = mAvailableScriptsListBox->GetSelectedItem();
  if (index < 0 || index >= (int)mTranslationEntries.Size())
  {
    mScriptEditor->SetAllText(String());
    return;
  }

  ShaderTranslationEntry& entry = mTranslationEntries[index];
  Vec2 scrollPercentage = mScriptEditor->GetScrolledPercentage();
  mScriptEditor->SetAllText(entry.mValue);
  mScriptEditor->SetLexer(entry.mLexerType);
  mScriptEditor->SetScrolledPercentage(scrollPercentage);
}

ZilchShaderIRLibraryRef ShaderTranslationDebugHelper::BuildShaderLibrary(
    ZilchShaderGenerator& generator, ZilchShaderIRCompositor::ShaderDefinition& shaderDef)
{
  mShaderProject.Clear();

  ZilchShaderIRLibrary* fragmentLibrary = generator.GetCurrentInternalProjectLibrary();

  // Get the core vertex, material, and render pass values
  Material* material = MaterialManager::GetInstance()->Find(mMaterialTextBox->GetText());
  ErrorIf(material == nullptr, "Invalid material selected");
  String coreVertexName = mCoreVertexTextBox->GetText();
  String renderPassName = mRenderPassTextBox->GetText();
  String compositeName = material->mCompositeName;
  String shaderName = BuildString(coreVertexName, compositeName, renderPassName);
  shaderDef.mShaderName = shaderName;

  // Add all fragments for this material.
  // First is the core vertex.
  shaderDef.mFragments.PushBack(fragmentLibrary->FindType(coreVertexName));
  // Then add all fragments on the material.
  for (auto fragmentNames = material->mFragmentNames.All(); !fragmentNames.Empty(); fragmentNames.PopFront())
  {
    ZilchShaderIRType* fragmentType = fragmentLibrary->FindType(fragmentNames.Front());
    shaderDef.mFragments.PushBack(fragmentType);
  }
  // ApiPerspectiveOutput needs to be after vertex fragments (can be after pixel
  // too)
  shaderDef.mFragments.PushBack(fragmentLibrary->FindType("ApiPerspectiveOutput"));
  // Finally, run the render pass
  shaderDef.mFragments.PushBack(fragmentLibrary->FindType(renderPassName));

  // Composite the shader together
  ShaderCapabilities capabilities;
  ZilchShaderIRCompositor compositor;
  compositor.Composite(shaderDef, capabilities, generator.mSpirVSettings);

  // Add each non-empty shader stage to the shader project to be compiled
  for (size_t i = 0; i < FragmentType::Size; ++i)
  {
    ZilchShaderIRCompositor::ShaderStageDescription& stageDesc = shaderDef.mResults[i];
    if (!stageDesc.mShaderCode.Empty())
      mShaderProject.AddCodeFromString(stageDesc.mShaderCode, "");
  }

  // Compile the shader project to get a library
  ZilchShaderIRModuleRef shaderDependencies = new ZilchShaderIRModule();
  shaderDependencies->PushBack(fragmentLibrary);
  ZilchShaderIRLibraryRef shaderLibrary =
      mShaderProject.CompileAndTranslate(shaderDependencies, generator.mFrontEndTranslator);
  return shaderLibrary;
}

bool ShaderTranslationDebugHelper::CompilePipeline(ZilchShaderIRType* shaderType,
                                                   ShaderPipelineDescription& pipeline,
                                                   Array<TranslationPassResultRef>& pipelineResults,
                                                   Array<TranslationPassResultRef>& debugPipelineResults)
{
  if (shaderType == nullptr)
    return false;

  ShaderTranslationPassResult* binaryBackendData = new ShaderTranslationPassResult();
  pipelineResults.PushBack(binaryBackendData);

  // Convert from the in-memory format of spir-v to actual binary (array of
  // words)
  ShaderByteStreamWriter byteWriter(&binaryBackendData->mByteStream);
  ZilchShaderSpirVBinaryBackend binaryBackend;
  binaryBackend.TranslateType(shaderType, byteWriter, binaryBackendData->mReflectionData);

  // Run each tool in the pipeline
  for (size_t i = 0; i < pipeline.mToolPasses.Size(); ++i)
  {
    ZilchShaderIRTranslationPass* translationPass = pipeline.mToolPasses[i];

    ShaderTranslationPassResult* prevPassData = pipelineResults.Back();
    ShaderTranslationPassResult* toolData = new ShaderTranslationPassResult();
    pipelineResults.PushBack(toolData);

    translationPass->RunTranslationPass(*prevPassData, *toolData);
  }

  ShaderTranslationPassResult* lastPassData = pipelineResults.Back();

  for (size_t i = 0; i < pipeline.mDebugPasses.Size(); ++i)
  {
    ZilchShaderIRTranslationPass* debugBackend = pipeline.mDebugPasses[i];

    ShaderTranslationPassResult* prevPassData = pipelineResults.Back();
    ShaderTranslationPassResult* resultData = new ShaderTranslationPassResult();
    debugPipelineResults.PushBack(resultData);

    debugBackend->RunTranslationPass(*prevPassData, *resultData);
  }

  // Run the final backend
  ShaderTranslationPassResult* backendResult = new ShaderTranslationPassResult();
  pipelineResults.PushBack(backendResult);
  bool success = pipeline.mBackend->RunTranslationPass(*lastPassData, *backendResult);
  if (!success)
    backendResult->mByteStream.Load(pipeline.mBackend->GetErrorLog());

  return true;
}

} // namespace Zero
