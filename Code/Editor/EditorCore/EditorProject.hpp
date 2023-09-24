// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class Cog;
class CommandManager;
class Editor;

void BindProjectCommands(Cog* config, CommandManager* commands);
/// Returns false when the project file couldn't be opened.
bool OpenProjectFile(StringParam filename);

void NewProject();
void OpenProject();

void UnloadProject(Editor* editor, Cog* projectCog);
void LoadProject(Editor* editor, Cog* projectCog, StringParam path, StringParam projectFile);

} // namespace Raverie
