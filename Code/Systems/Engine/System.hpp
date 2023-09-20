// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class Engine;

struct SystemInitializer
{
  Engine* mEngine;
  Cog* Config;
};

/// System is a pure virtual base class that is the base class for all systems
/// used by the game.
class System : public EngineObject
{
public:
  RaverieDeclareType(System, TypeCopyMode::ReferenceType);

  /// All systems need a virtual destructor to have their destructor called.
  virtual ~System()
  {
  }

  /// The name of this system. Mostly for debugging.
  virtual cstr GetName() = 0;

  /// Initialize the system.
  virtual void Initialize(SystemInitializer& initializer)
  {
  }

  /// All systems are updated every game frame.
  virtual void Update(bool debugger)
  {
  }
};

} // namespace Raverie
