#include "Zilch.hpp"

using namespace Zilch;

int main()
{
  // Any one time startup and static initialization Zilch needs to do
  // This also registers a custom assertion handler (Zilch code has many user friendly asserts!)
  ZilchSetup setup(SetupFlags::None);

  // A project contains all of the code we combine together to make a single Zilch library
  // The project also sends events for all compilation errors that occur (which includes friendly messages and error codes)
  Project project;

  // Here, we can register our own callback for when compilation errors occur
  // The default callback prints the file, line/character number, and message to stderr
  EventConnect(&project, Events::CompilationError, DefaultErrorCallback);

  // Load the Zilch code for our Player class from disk (make sure the working directory is setup properly)
  project.AddCodeFromFile("Player.zilch");

  // Create a list of dependent libraries, in our case we're really not adding anything to this
  // A side note: the Core library in Zilch is always added as a dependency, because Core includes
  // things like Integer, Boolean, Real, the basic vector types, String, etc
  Module dependencies;

  // The 'Compile' function actually returns a Zilch Library
  // Be careful! If the code fails to compile, this function will return null
  LibraryRef library = project.Compile("Test", dependencies, EvaluationMode::Project);
  ErrorIf(library == nullptr, "Failed to compiler library");

  // We want to link together all the libraries that we depended upon, along with our own library
  dependencies.PushBack(library);

  // Link all the libraries together into one ExecutableState
  ExecutableState* state = dependencies.Link();
  ErrorIf(state == nullptr, "Failed to link libraries together");

  // Find the Player type we created in Zilch
  BoundType* playerType = library->BoundTypes.FindValue("Player", nullptr);
  ErrorIf(playerType == nullptr, "Failed to find a Zilch type named 'Player'");

  // Find the static SayHello function we defined on the Player type
  // We pass in an array of types to specify the arguments we want, in this case its an empty array
  // We also pass in the void type because we don't expect a return value
  Function* sayHelloFunction = playerType->FindFunction("SayHello", Array<Type*>(), ZilchTypeId(void), FindMemberOptions::Static);
  ErrorIf(sayHelloFunction == nullptr, "Failed to find function 'SayHello' on Zilch type 'Player'");

  // Find the instance 'TakeDamage' function on the Player which accepts a Real and returns nothing (Void)
  Function* takeDamageFunction = playerType->FindFunction("TakeDamage", Array<Type*>(ZeroInit, ZilchTypeId(Real)), ZilchTypeId(void), FindMemberOptions::None);
  ErrorIf(takeDamageFunction == nullptr, "Failed to find function 'TakeDamage' on Zilch type 'Player'");

  // Find the instance data member 'Health' on the Player (a data member is called a Field in Zilch)
  Field* healthField = playerType->InstanceFields.FindValue("Health", nullptr);
  ErrorIf(healthField == nullptr, "Failed to find field 'Health' on Zilch type 'Player'");

  // Setup the console so that when we call 'Console.WriteLine' it outputs to stdio
  EventConnect(&Console::Events, Events::ConsoleWrite, DefaultWriteText);

  // We can also setup the console so that any 'Read' functions will attempt to read from stdin
  EventConnect(&Console::Events, Events::ConsoleRead, DefaultReadText);

  // The call object uses its constructor to initialize a stack frame in
  // the ExecutableState, and removes that stack frame inside its destructor
  // It is important to wrap this in a scope to ensure destruction happens when we intend it to

  // Player.SayHello()
  {
    // The exception report stores any exceptions that may have occurred while executing code
    // Exceptions include accessing arrays out of bounds, dereferencing null, etc
    ExceptionReport report;

    // Invoke the SayHello function, which should print out to the console
    Call call(sayHelloFunction, state);
    call.Invoke(report);

    // You can check 'report.HasThrownExceptions()' to see if the script threw an exception
    // This is useful if you want to stop a particular script from running again, or abort the program
  }
    
  // var playerInstance = new Player();
  // playerInstance.Health = 200.0;
  // playerInstance.TakeDamage(10.0);
  {
    // Allocate the player and call a default constructor if one is provided
    // It is legal to allocate Zilch objects that have no defined default constructor
    // The object will first be entirely set to zero (all null and zero values)
    // Then the pre-constructor will run, which initializes any members in the class to the value after the '='
    ExceptionReport report;
    Handle playerInstance = state->AllocateDefaultConstructedHeapObject(playerType, report, HeapFlags::ReferenceCounted);

    // Set the health field by invoking its automatically generated setter function
    // Getters and setters always take the form of:
    //   Get() : PropertyType
    //   Set(value : PropertyType)
    // We're invoking the setter, so parameter 0 must be of type Real
    {
      Call call(healthField->Set, state);
      call.Set<Handle>(Call::This, playerInstance);
      call.Set<Real>(0, 200.0f);
      call.Invoke(report);
    }
      
    // Now invoke the member function 'TakeDamage'
    {
      Call call(takeDamageFunction, state);
      call.Set<Handle>(Call::This, playerInstance);
      call.Set<Real>(0, 10.0f);
      call.Invoke(report);
    }
  }

  // Finally, we're done with the executable state so get rid of it
  delete state;

  return 0;
}
