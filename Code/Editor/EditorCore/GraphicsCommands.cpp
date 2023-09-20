// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

void ForceCompileAllShaders(Editor* editor)
{
  Z::gEngine->has(GraphicsEngine)->ForceCompileAllShaders();
}

void BindGraphicsCommands(Cog* config, CommandManager* commands)
{
  commands->AddCommand("ForceCompileAllShaders", BindCommandFunction(ForceCompileAllShaders));
}

} // namespace Raverie
