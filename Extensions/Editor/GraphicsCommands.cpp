#include "Precompiled.hpp"

namespace Zero
{

void ForceCompileAllShaders(Editor* editor)
{
  Z::gEngine->has(GraphicsEngine)->ForceCompileAllShaders();
  DoNotify("Shaders Compiled", String(), "ZeroCommandDropdown");
}

void BindGraphicsCommands(Cog* config, CommandManager* commands)
{
  commands->AddCommand("ForceCompileAllShaders", BindCommandFunction(ForceCompileAllShaders));
}

} // namespace Zero
