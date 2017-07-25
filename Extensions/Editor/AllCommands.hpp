///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
namespace Zero
{

class CommandManager;

void SetupGraphCommands(Cog* configCog, CommandManager* commands);

void BindAppCommands(Cog* config, CommandManager* commands);
void BindArchiveCommands(Cog* config, CommandManager* commands);
void BindGraphicsCommands(Cog* config, CommandManager* commands);
void BindGeometryCommands(Cog* config, CommandManager* commands);
void BindCreationCommands(Cog* configCog, CommandManager* commands);
void BindPhysicsTestCommands(Cog* configCog, CommandManager* commands);
void BindDocumentationCommands(Cog* config, CommandManager* commands);
void BindContentCommands(Cog* configCog, CommandManager* commands);

}
