// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

void ForceCompileAllShaders(Editor* editor)
{
  Z::gEngine->has(GraphicsEngine)->ForceCompileAllShaders();
}

void BindGraphicsCommands(Cog* config, CommandManager* commands)
{
  commands->AddCommand("ForceCompileAllShaders", BindCommandFunction(ForceCompileAllShaders));
}

} // namespace Zero
