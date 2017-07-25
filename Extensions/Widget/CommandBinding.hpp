///////////////////////////////////////////////////////////////////////////////
///
/// Authors: 
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

inline void CommandFailed(Command* command, BoundType* contextType)
{
  if(false)
  {
    String message = String::Format("There is no '%s' available to run the command '%s'",
                                    contextType->Name.c_str(), command->DisplayName.c_str() );
    DoNotifyWarning("Command Not Run", message);
  }
}

template<typename pt0, typename pt1>
class Command2 : public CommandExecuter
{
public:
  typedef void (*Function)(pt0* param0, pt1* param1);

  Function mFunction;

  Command2(Function function)
    : mFunction(function)
  {}


  bool IsEnabled(Command* command, CommandManager* manager) override
  {
    return manager->GetContext<pt0>()!=NULL  &&  manager->GetContext<pt1>()!=NULL;
  }

  void Execute(Command* command, CommandManager* manager) override
  {
    pt0* param0 = manager->GetContext<pt0>();
    pt1* param1 = manager->GetContext<pt1>();

    if(param0 == NULL)
      return CommandFailed(command, ZilchTypeId(pt0));

    if(param1 == NULL)
      return CommandFailed(command, ZilchTypeId(pt1));

    mFunction(param0, param1);
  }
};

template<typename pt0, typename pt1>
inline CommandExecuter* BindCommandFunction(void (*function)(pt0*, pt1*) )
{
  return new Command2<pt0, pt1>(function);
}

template<typename pt0>
class Command1 : public CommandExecuter
{
public:
  typedef void (*Function)(pt0* param0);

  Function mFunction;

  Command1(Function function)
    : mFunction(function)
  {}

  bool IsEnabled(Command* command, CommandManager* manager) override
  {
    return manager->GetContext<pt0>() != NULL;
  }

  void Execute(Command* command, CommandManager* manager) override
  {
    pt0* param0 = manager->GetContext<pt0>();

    if(param0 == NULL)
      return CommandFailed(command, ZilchTypeId(pt0));

    mFunction(param0);
  }
};

template<typename pt0>
inline CommandExecuter* BindCommandFunction(void (*function)(pt0*) )
{
  return new Command1<pt0>(function);
}

class Command0 : public CommandExecuter
{
public:
  typedef void (*Function)();

  Function mFunction;

  Command0(Function function)
    : mFunction(function)
  {}

  void Execute(Command* command, CommandManager* manager) override
  {
    mFunction();
  }
};

inline CommandExecuter* BindCommandFunction(void (*function)() )
{
  return new Command0(function);
}

class MetaCommandExecuter : public CommandExecuter
{
public:
  Delegate mDelegate;

  void Execute(Command* command, CommandManager* manager) override
  {
    mDelegate.Invoke();
  }
};

}
