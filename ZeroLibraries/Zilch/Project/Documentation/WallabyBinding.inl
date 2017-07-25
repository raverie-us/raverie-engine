#include "Wallaby.hpp"

ZilchDefineStaticLibrary(Wallaby)
{
  // We have to initialize all types that we have bound to our library (automatically any type to Wallaby)
  // Ideally we could use pre-main or automatic registration, but there's a major issue where
  // compilers will automatically remove "unreferenced" classes, even if they are referenced
  // by globals/pre-main initializations. This method ensures that all classes will be properly bound
  ZilchInitializeType(LivingBeing);
  ZilchInitializeType(Player);
}

// This allows us to define all the members on LivingBeing
// The 'builder' and 'type' members are only there to let the user know they can do builder->... or type->...
// If you wanted to change the name that gets bound to Zilch, use ZilchFullDefineType
ZilchDefineType(LivingBeing, builder, type)
{
  // The 'ZilchNoNames' macro is simply a way of saying that there are no parameter names for the argument types
  // Zilch supports named parameter calling, so feel free to provide them

  // We should generally always bind a destructor and constructor,
  // especially if this is a dervied class and the base is constructable
  ZilchBindDestructor();
  ZilchBindConstructor();

  // This simplified binding macro allows us to quickly bind instance and static methods on our own class
  // This only works if there are no overloads of the method
  ZilchBindMethod(Speak);

  // The binding templates/macros can automatically determine if you're binding a static or instance member function
  // You can also use this to bind global functions to a class, even when they weren't originally defined within the class
  // The 'ZilchNoOverload' lets the binding know that there are no overloads of the same name
  // Otherwise, we'd have to pass in the type/signature of the member function in parentheses
  ZilchFullBindMethod(builder, type, &LivingBeing::ComputeLives, ZilchNoOverload, "ComputeLives", "mana, level");

  // Bind both overloads of Yell
  ZilchFullBindMethod(builder, type, &LivingBeing::Yell, (void (LivingBeing::*)(float) const), "Speak", "volume");
  ZilchFullBindMethod(builder, type, &LivingBeing::Yell, (void (LivingBeing::*)() const), "Speak", ZilchNoNames);

  // Simple macros for binding readable and writable data members (fields)
  ZilchBindField(Lives);

  // Zilch does not have the concept of 'const' (therefore we remove all consts from bound C++ members)
  // It is up to us to be very careful here and bind const members as 'Get' only
  ZilchFullBindField(builder, type, &LivingBeing::MaxLives, "MaxLives", PropertyBinding::Get);

  // We can specially bind getters and setters in C++ as a single property in Zilch
  ZilchBindGetterSetter(Health);

  // The above binding is the exact same as doing the following
  //ZilchFullBindGetterSetter(builder, type, &LivingBeing::GetHealth, ZilchNoOverload, &LivingBeing::SetHealth, ZilchNoOverload, "Health");
}

ZilchDefineType(Player, builder, type)
{
  // Be sure to always pass the correct types in to all the bindings
  // Do NOT pass LivingBeing, for example, and avoid copy pasting from other bindings!

  // Even though we only have a non-overloaded constructor, we unfortunately cannot
  // detect the argument types automatically for constructors due to a limitation in C++
  // The argument types must be explicitly passed in
  ZilchFullBindConstructor(builder, type, Player, "name, startingHealth", const String&, float);
  ZilchBindDestructor();

  ZilchBindField(Name);
  
  // Note that we don't bind Speak again because it gets inherited from the base
}
