// MIT Licensed (see LICENSE.md).
#pragma once
namespace Zero
{

class CommandManager;

void SetupGraphCommands(Cog* configCog, CommandManager* commands);

void BindAppCommands(Cog* config, CommandManager* commands);
void BindArchiveCommands(Cog* config, CommandManager* commands);
void BindGraphicsCommands(Cog* config, CommandManager* commands);
void BindCreationCommands(Cog* configCog, CommandManager* commands);
void BindPhysicsTestCommands(Cog* configCog, CommandManager* commands);
void BindDocumentationCommands(Cog* config, CommandManager* commands);
void BindContentCommands(Cog* configCog, CommandManager* commands);

} // namespace Zero
