///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2013-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
class Cog;
class CommandManager;
class Composite;
class Editor;
class ObjectEvent;
class Material;
class ShaderDefinition;
class ZilchShaderDefinition;
class SimpleZilchShaderGenerator;
class ZilchFragment;

//-------------------------------------------------------------------TranslatedShaderScriptEditor
// Base class for displaying translated scripts in the editor. Mostly manages a lot of
// helper functions on a ZilchCompositor and the actual script editor display.
class TranslatedShaderScriptEditor : public ScriptEditor
{
public:
  typedef TranslatedShaderScriptEditor ZilchSelf;
  TranslatedShaderScriptEditor(Composite* parent);
  ~TranslatedShaderScriptEditor();

  // Updates the script editor with the translated text from OnTranslate
  void Build();
  void OnBuild(Event* e);
  // Function meant to be implemented by derived classes that translate as needed
  virtual String OnTranslate();

  // Helper functions
  void CompileAndTranslateFragments(SimpleZilchShaderGenerator& shaderGenerator);
  void TranslateMaterial(SimpleZilchShaderGenerator& shaderGenerator, Material* material);
  
  // The Connect macro doesn't seem to work when trying to connect from a derived type on a base class function,
  // hence this function allows easy connection on a resource type being modified.
  template <typename ResourceType>
  void ListenForModified(ResourceType* resource)
  {
    ConnectThisTo(resource, Events::ResourceModified, OnBuild);
  }

  SimpleZilchShaderGenerator* mShaderGenerator;
};

//-------------------------------------------------------------------FragmentFileTranslatorScriptEditor
// Translates a given fragment. As long as this editor is open on save/modification
// of the fragment the script editor will be updated.
class FragmentFileTranslatorScriptEditor : public TranslatedShaderScriptEditor
{
public:
  typedef FragmentFileTranslatorScriptEditor ZilchSelf;
  FragmentFileTranslatorScriptEditor(Composite* parent);

  void SetResource(ZilchFragment* fragment);
  String OnTranslate() override;

  ZilchFragment* mFragment;
};

//-------------------------------------------------------------------ZilchCompositorScriptEditor
// Builds the zilch script representing a composite from a material. As long as this editor is open
// on save/modification of the fragment the script editor will be updated.
class ZilchCompositorScriptEditor : public TranslatedShaderScriptEditor
{
public:
  typedef ZilchCompositorScriptEditor ZilchSelf;
  ZilchCompositorScriptEditor(Composite* parent);

  void SetResource(Material* material);
  String OnTranslate() override;

  Material* mMaterial;
};

DeclareEnum4(TranslationDisplayMode, Pixel, Geometry, Vertex, Full);
//-------------------------------------------------------------------ZilchCompositorScriptEditor
// Translates the zilch script of a material. As long as this editor is open
// on save/modification of the fragment the script editor will be updated.
class TranslatedZilchCompositorScriptEditor : public TranslatedShaderScriptEditor
{
public:
  typedef TranslatedZilchCompositorScriptEditor self_type;
  TranslatedZilchCompositorScriptEditor(Composite* parent);

  void SetResource(Material* material);
  String OnTranslate() override;

  void SetDisplayMode(TranslationDisplayMode::Enum displayMode);

  Material* mMaterial;
  TranslationDisplayMode::Enum mDisplayMode;
};

//-------------------------------------------------------------------FragmentSplitScriptEditor
class BaseSplitScriptEditor : public Composite
{
public:
  typedef BaseSplitScriptEditor ZilchSelf;
  BaseSplitScriptEditor(Composite* parent);

  void Setup();
  virtual void SetTranslatedEditor();
  virtual void SaveCheck();
  virtual void Build();
  virtual void BuildFinalShader(ShaderTypeTranslation& shaderResult);

  void SetLexer(uint lexer);
  void OnSaveCheck(SavingEvent* e);
  void OnBuild(Event* e);
  void OnLeftMouseDown(MouseEvent* e);

  CodeRangeMapping* FindRange(int positionWithinParent, CodeRangeMapping* current);
  
  ScriptEditor* mSourceEditor;
  Splitter* mSplitter;
  TranslatedShaderScriptEditor* mTranslatedEditor;
};

//-------------------------------------------------------------------FragmentSplitScriptEditor
// A simple script editor to view a translated shader and a source fragment.
// Currently used to debug line number mappings (Cleanup later).
class FragmentSplitScriptEditor : public BaseSplitScriptEditor
{
public:
  typedef FragmentSplitScriptEditor ZilchSelf;
  FragmentSplitScriptEditor(Composite* parent);

  virtual void SetTranslatedEditor() override;
  virtual void SaveCheck() override;
  virtual void Build() override;
  virtual void BuildFinalShader(ShaderTypeTranslation& shaderResult) override;

  void SetResource(ZilchFragment* fragment);
  
  ZilchFragment* mFragment;
};

//-------------------------------------------------------------------MaterialSplitScriptEditor
// A simple script editor to view a translated shader and a source material.
// Currently used to debug line number mappings (Cleanup later).
class MaterialSplitScriptEditor : public BaseSplitScriptEditor
{
public:
  typedef MaterialSplitScriptEditor ZilchSelf;
  MaterialSplitScriptEditor(Composite* parent);

  void SetTranslatedEditor() override;
  void Build() override;
  void BuildFinalShader(ShaderTypeTranslation& shaderResult) override;

  void SetResource(Material* material);
  void SetDisplayMode(TranslationDisplayMode::Enum displayMode);
  
  TranslationDisplayMode::Enum mDisplayMode;
  Material* mMaterial;
};

//-------------------------------------------------------------------CodeTranslatorListener
// A simple class to listen for events. Ideally this should be a component.
class CodeTranslatorListener : public EventObject
{
public:
  typedef CodeTranslatorListener ZilchSelf;
  CodeTranslatorListener();

  void OnComposeZilchMaterial(ObjectEvent* e);
  void OnTranslateZilchFragment(ObjectEvent* e);
  void OnTranslateZilchPixelFragment(ObjectEvent* e);
  void OnTranslateZilchGeometryFragment(ObjectEvent* e);
  void OnTranslateZilchVertexFragment(ObjectEvent* e);

  void OnTranslateZilchFragmentWithLineNumbers(ObjectEvent* e);
  void OnTranslateZilchPixelFragmentWithLineNumbers(ObjectEvent* e);
  void OnTranslateZilchVertexFragmentWithLineNumbers(ObjectEvent* e);
};

void BindCodeTranslatorCommands(Cog* configCog, CommandManager* commands);

}//namespace Zero
