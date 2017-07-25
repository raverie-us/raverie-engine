///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorHotspots.cpp
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{


//http:\\ or https:\\ then any number of url elements till space
cstr HyperLinkRegex = "https?:\\/\\/[\\w\\d=/\\.\\-\\?\\%\\#]+";

HyperLinkHotspot::HyperLinkHotspot()
  :TextEditorHotspot(HyperLinkRegex)
{
}

void HyperLinkHotspot::OnClick(Matches& matches)
{
  String url = matches.Front();
  Os::SystemOpenNetworkFile(url.c_str());
}

// Command : any number of letters
cstr CommandRegex = "Command:(\\w+)";

CommandHotspot::CommandHotspot()
  :TextEditorHotspot(CommandRegex)
{
}

void CommandHotspot::OnClick(Matches& matches)
{
  String commandName = matches[1];
  Command* command = CommandManager::GetInstance()->GetCommand(commandName);
  if(command)
  {
    command->Execute();
  }
  else
  {
    String errorMessage = String::Format("Command %s was not found", commandName.c_str());
    DoNotifyError("Command not found", errorMessage);
  }
}


// Matches 16 hex digits and then and letter number or dot
// Example 5423f6b5995af33f:SomeName
cstr ResourceRegex = "(\\b[0-9a-fA-F]{16}\\b):?[\\w\\d\\.]*";

ResourceHotspot::ResourceHotspot()
  :TextEditorHotspot(ResourceRegex)
{

}

void ResourceHotspot::OnClick(Matches& matches)
{
  // If we received two different matches
  // (the whole string itself, then the one sub-group)...
  if(matches.Size() == 2)
  {
    ResourceId resourceId = 0;
    ToValue(matches[1], resourceId);
    Resource* resource = Z::gResources->GetResource(resourceId);

    // Edit all resources except for levels (clicking on level
    // in the console should not change levels)
    if(resource && ZilchVirtualTypeId(resource) != ZilchTypeId(Level))
    {
      // Edit the resource
      Z::gEditor->EditResource(resource);

      // Always show the properties window afterwards
      // (to make sure the user sees that the resource window that was opened)
      Z::gEditor->ShowWindow("Properties");
    }
    else if (ZilchVirtualTypeId(resource) == ZilchTypeId(Level))
    {
      return;
    }
    else
    {
      String resource = matches[0];
      String errorMessage = String::Format("Resource %s was not found", resource.c_str());
      DoNotifyWarning("Resource not found", errorMessage);
    }
  }
}


//Matches <Cog 'Cube' (WW) [142]>
cstr ObjectRegex = "<[\\w]+( '[\\w]+')?( \\([\\w]+\\))? \\[([0-9]+)\\]>";

ObjectHotspot::ObjectHotspot()
  :TextEditorHotspot(ObjectRegex)
{

}

void ObjectHotspot::OnClick(Matches& matches)
{
  if(matches.Size() > 0)
  {
    // Read the cog-id value
    uint cogIdValue;
    ToValue(matches[3], cogIdValue);

    Cog* cog = Z::gTracker->RawFind(cogIdValue);
    if(cog)
    {
      // Focus on the object
      Z::gEditor->SelectOnly(cog);
      Space* space = cog->GetSpace();
      //the space could be null if we were selecting the game or something not in a space
      if(space != NULL)
      {
        Cog* editorCamera = space->FindObjectByName(SpecialCogNames::EditorCamera);
        CameraFocusSpace(space, editorCamera, EditFocusMode::AutoTime);
      }
    }
  }
}



// File form of File "C:\File.z", line 33, message
cstr FileRegex = "File \"(.*)\", line ([0-9]+)";

FileHotspot::FileHotspot()
  :TextEditorHotspot(FileRegex)
{

}

void FileHotspot::OnClick(Matches& matches)
{
  // If we received three different matches
  // (the whole string itself, then the two sub-groups)...
  if(matches.Size() == 3)
  {
    // Read the file
    StringRange file = matches[1];

    // Read the line number
    int line;
    ToValue(matches[2], line);

    // Subtract 1 since line numbers are actually zero based, and the printed form is 1 based
    --line;

    // Navigate to the script/line
    DocumentEditor* editor = Z::gEditor->OpenTextFileAuto(file);

    if(editor)
    {
      editor->FocusWindow();
      editor->GoToLine(line);
    }
  }
}

}
