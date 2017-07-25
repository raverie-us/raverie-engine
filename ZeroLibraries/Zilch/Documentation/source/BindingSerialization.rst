Common Serialization Patterns
=============================

When binding Zilch it is common to use reflection to serialize members or properties of a class written in Zilch. This guide shows common approaches that we use to serialize data types. Because C++ classes can be bound using Zilch's binding library, you can also technically use this to serialize C++ classes (though most game engines provide this functionality themselves).

At the end of the day, serialization always comes down to writing out primitives such as integers, floats, strings, and characters, as well as representing larger data structures such as arrays, maps and objects.

Every Zilch class or struct creates a ``BoundType`` which defines all the members (fields, properties, and methods) on that type (binding a C++ type also creates this). A *Property* is a member that has a *get* or *set* function, and a *Field* is a typical member variable. In Zilch, a *Field* is implicitly a *Property* and automatically implements a *get* and *set* function. This means we can always just generically use *Property* instead of having to distinguish from *Field* and *Property*, because *Field* **is-a** *Property*.

If your serialization format reads in names of members, then you can find a data-field or property via:

::
  
  void SerializeMember(const String& name, BoundType* type)
  {
    // This will find an instance member that we can call 'get' or 'set' on
    Property* member = boundType->FindPropertyOrField(name, FindMemberOptions::None);
    
    // If we didn't find the member, early out
    if (member == nullptr)
    {
      printf("Unable to find member of name %s on type %s", 
      return;
    }

    // Because the user can choose to define only 'get' or only 'set', then we must check for null
    if (member->Get == nullptr || member->Set == nullptr)
      return;
    
    // Check if we're deserializing / reading, or if we're writing (this is your variable)
    if (reading)
    {
      // Getters take no parameters, and return a value
      ExceptionReport report;
      Call call(member->Get, state);
      call.Invoke(report);
      
      if (report.HasThrownExceptions())
        return;
      
      
    }
  }
