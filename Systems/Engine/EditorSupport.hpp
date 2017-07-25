///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorSupport.hpp
/// Declaration of the Editor support classes EditorSpace and Selection.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class ContentLibrary;
class Resource;
class Level;

/// ResourceAdd is a collection of parameters used
/// to create a resource
struct ResourceAdd
{
  ResourceAdd()
  {
    Library = NULL;
    SourceResource = NULL;
    mSuccess = false;
    AddResourceId = 0;
    Template = nullptr;
  }

  /// Name for the resource.
  String Name;
  /// Filename for the resource.
  String FileName;
  /// Content library to add the generated content item.
  ContentLibrary* Library;
  /// File to base the resource on.
  String SourceFile;
  /// Active resource object to use
  Resource* SourceResource;
  /// The template to create from.  If empty, use default.
  Resource* Template;
  /// ResourceId to Use 0 for new id.
  ResourceId AddResourceId;
  /// If a resource is owned by a resource.
  String ResourceOwner;

  /// Was the operation successful
  bool WasSuccessful(){return mSuccess;}
  bool mSuccess;
};

// Interface to the editor for the engine
class RuntimeEditor
{
public:
  // Visual the component, usually by adding an icon
  virtual void Visualize(Component* component, StringParam icon) = 0;

  // Open an editor viewport for a space
  virtual void OpenEditorViewport(Space* space) = 0;

  // Set focus on the window with the current space open
  virtual void SetFocus(Space* space) = 0;

  // Get the level being edited
  virtual Level* GetEditingLevel() = 0;

  // Does this game session have focus in the editor?
  virtual bool HasFocus(GameSession* game) = 0;

  // Add a new resource
  virtual Resource* AddResource(ResourceManager* resourceManager, ResourceAdd& resourceAdd) = 0;

  // Determines if a new resource should be created, otherwise returns the original
  virtual Resource* NewResourceOnWrite(ResourceManager* resourceManager, BoundType* type, StringParam propertyName, Space* space,
                                       Resource* resource, Archetype* archetype, bool modified) = 0;

  // If there is a resource id conflict notify the editor
  virtual void OnResourceIdConflict(ResourceEntry& resourceEntry, Resource* previous) = 0;

  // Show an error in a text file or resource text file using the editor's text editor
  virtual void ShowTextError(StringParam file, int line, StringParam message) = 0;

  // Show a text block
  virtual void ShowTextBlock(StringParam name, StringRange text, int line, StringParam message) = 0;

  virtual MetaSelection* GetActiveSelection() = 0;

  virtual ~RuntimeEditor(){};
};

namespace Z
{
  // Enabled Debug Features used by the editor
  extern bool EditorDebugFeatures;
  // Enabled core developer features
  extern bool DeveloperMode;
  // Access the editor at runtime prevents including
  // editor in core engine
  // Runtime editor will be NULL outside the editor
  extern RuntimeEditor* gRuntimeEditor;
}//namespace Z

//Constants

// Special object names in a level
namespace SpecialCogNames
{
extern const String EditorCamera;
extern const String LevelSettings;
extern const String WorldAnchor;
extern const String LevelGeometry;
extern const String Main;
extern const String ViewCube;
}

// Special built in Archetypes
namespace CoreArchetypes
{
extern const String Game;
extern const String Space;
extern const String Transform;
extern const String Empty;
extern const String Sphere;
extern const String Cylinder;
extern const String LevelSettings;
extern const String WorldAnchor;
extern const String Camera;
extern const String EditorCamera;
extern const String PreviewCamera;
extern const String ObjectLink;
extern const String Default;
extern const String Sprite;
extern const String SpriteText;
extern const String SpriteParticles;
extern const String SplineParticleSystem;
extern const String DirectionalLight;
extern const String DirectionalLightShadows;
extern const String PointLight;
extern const String Cube;
extern const String Grid;
extern const String DefaultSpace;
extern const String EmptyTile;
extern const String ViewCube;
extern const String Wedge;
}


}//namespace Zero
