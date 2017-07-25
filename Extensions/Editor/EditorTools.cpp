///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorTools.cpp
///
/// Authors: Chris Peters
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

class ToolSelectionCommand : public CommandExecuter
{
public:
  void Execute(Command* command, CommandManager* manager) override
  {
    ToolControl* tools = Z::gEditor->Tools;
    tools->SelectToolName(command->Name, ShowToolProperties::Show);
  }
};

void SetupTools(Editor* editor)
{
  ToolControl* Tools = new ToolControl(editor);
  editor->Tools = Tools;
  //editor->MetaGizmos = new ToolControl(editor);

  // Select Tool
  Cog* tool = Tools->AddOrUpdateTool(ArchetypeManager::FindOrNull("SelectTool"));
  Tools->mSelectTool = tool->has(SelectTool);

  Tools->AddOrUpdateTool(ArchetypeManager::FindOrNull("TranslateTool"));
  Tools->AddOrUpdateTool(ArchetypeManager::FindOrNull("RotateTool"));
  Tools->AddOrUpdateTool(ArchetypeManager::FindOrNull("ScaleTool"));
  Tools->AddOrUpdateTool(ArchetypeManager::FindOrNull("ManipulatorTool"));
         
  Tools->AddOrUpdateTool(ArchetypeManager::FindOrNull("TileEditor2D"));
  Tools->AddOrUpdateTool(ArchetypeManager::FindOrNull("HeightMapTool"));
  Tools->AddOrUpdateTool(ArchetypeManager::FindOrNull("JointTool"));

  // Creation Tool
  tool = Tools->AddOrUpdateTool(ArchetypeManager::FindOrNull("CreationTool"));
  Tools->mCreationTool = tool->has(CreationTool);

  Tools->AddOrUpdateTool(ArchetypeManager::FindOrNull("ParentingTool"));

  //Tools->AddOrUpdateTool(ArchetypeManager::FindOrNull("GeometryBuilderTool"));

  // Old tools
  if(Z::gEngine->GetConfigCog()->has(Zero::DeveloperConfig))
  {
    Tools->AddOrUpdateTool(ArchetypeManager::FindOrNull("SpringTools"));
  }

  CommandManager* commands = CommandManager::GetInstance();

  // Add commands for all tools
  forRange(ToolData* data, Tools->mTools.mToolArray.All())
  {
    commands->AddCommand(data->GetName(), new ToolSelectionCommand());
  }
}

}//namespace Zero
