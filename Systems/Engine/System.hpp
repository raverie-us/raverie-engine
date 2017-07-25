///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Trevor Sundberg
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Engine;

struct SystemInitializer
{
  Engine* mEngine;
  Cog* Config;
};

//-------------------------------------------------------------------System
/// System is a pure virtual base class that is the base class for all systems used by the game. 
class System : public EngineObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// All systems need a virtual destructor to have their destructor called.
  virtual ~System() {}

  /// The name of this system. Mostly for debugging.
  virtual cstr GetName() = 0;

  /// Initialize the system.
  virtual void Initialize(SystemInitializer& initializer) {}

  /// All systems are updated every game frame.
  virtual void Update() {}
};

}//namespace Zero
