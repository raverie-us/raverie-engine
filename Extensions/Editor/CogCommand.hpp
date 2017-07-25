///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class ResourceEvent;

//------------------------------------------------------------------ Cog Command
class CogCommand : public Command
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  CogCommand(Archetype* archetype);
  CogCommand(BoundType* componentType);

  /// Command Interface.
  void Execute() override;

  String GetName() { return Name; }

  /// The live command object.
  HandleOf<Cog> mCog;

  /// If the cog was created from an archetype.
  HandleOf<Archetype> mArchetype;

  /// If the cog was created from a single script component.
  BoundTypeHandle mScriptComponentType;
};

//---------------------------------------------------------- Cog Command Manager
/// This class will manage the creation and destruction of CogCommands.
/// There are two ways a CogCommand can be created:
/// 1. A Component with the [Command] attribute with the attribute parameter
///    'autoCommand' set to true.
/// 2. An Archetype with tag "Command".
class CogCommandManager : public EditorScriptObjects<CogCommand>
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  CogCommandManager();

  /// EditorScriptObject Interface.
  void AddObject(CogCommand* object) override;
  void RemoveObject(CogCommand* object) override;
  CogCommand* GetObject(StringParam objectName) override;
  uint GetObjectCount() override;
  CogCommand* GetObject(uint index) override;
  Space* GetSpace(CogCommand*) override;

  Space* GetSpace();

  /// All command objects will be created in this space.
  HandleOf<Space> mCommandSpace;

  CommandManager* mCommands;
};

}//namespace Zero
