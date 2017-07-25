.. _LibraryBuilder:

LibraryBuilder
==============

.. note::

  All code written here assumes ``using namespace Zilch``

The *LibraryBuilder* allows us to create Zilch types from C++ and add native functions, properties, or fields to those types.

Creating A Value Type (Struct)
------------------------------

Say we want to create a Ray type from C++ that takes a position and normalized direction in 3D. Our type will contain a Real3 for translation and a Real3 for direction. We'll want this type to be considered a struct type (ValueType) in Zilch which means that it gets fully copied on every assignment. The first thing we're going to want to do is define the offset of the members.

.. code-block:: cpp

  const size_t PositionOffset = 0;
  const size_t DirectionOffset = PositionOffset + sizeof(Real3);
  
It would obviously be more beneficial to actually create a Ray struct in C++ and use the automatically generated member offsets (via the standard offsetof macro). This would be actually safer too due to alignment issues on some platforms.

.. code-block:: cpp

  // Use whatever name you would like for your library
  LibraryBuilder builder("Wallaby");
  
  // This doesn't take into account possible alignment requirements of data types, but we'll keep it simple
  size_t raySize = sizeof(Real3) + sizeof(Real3);
  
  // Create the Ray struct type (ValueType means struct)
  BoundType* rayType = builder.AddBoundType("Ray", TypeCopyMode::ValueType, raySize);
  
  // We can bind the members either as Fields or Properties
  // Fields are faster to access because there is no call overhead for the get/set
  // Sometimes you might want to bind a Property if its a primitive type Zilch doesn't support,
  // such as a 'short' and the property can automatically convert it into an Integer for Zilch
  builder.AddBoundField(rayType, "Position", ZilchTypeId(Real3), PositionOffset, MemberOptions::None);
  builder.AddBoundField(rayType, "Direction", ZilchTypeId(Real3), DirectionOffset, MemberOptions::None);

This type is now usable in Zilch. We can access the Position and Direction and copy it around, but it has no other functionality.

.. warning::

  A struct or ValueType is considered memory copyable, and it is always valid to zero out its memory. Do NOT add a field on a Zilch struct that requires proper construction, assignment or destruction.


Creating A Function
-------------------

When writing C++ code that can be called from Zilch there is only one type of signature that Zilch understands:

.. code-block:: cpp

  void Function(Call& call, ExceptionReport& report)

Even though Zilch provides a higher level API that allows it to call many C++ functions of different signatures, they all boil down to this. Zilch's binding API uses templates to generate a function of this signature.

In the early days of Zilch, when a user function was called they were basically given an *unsigned char\**  that pointed to the current Zilch stack frame. Parameters on the stack were sequentially ordered (with padding) and the users had to manually offset pointers to pull out the values. The *Call* makes this much easier by providing functions to automatically pull values off the stack and perform conversions if needed (e.g. our String to std::string). Even with the Call helper, it is still important to understand exactly what is on the Zilch stack. Remember that Call is also what we use when we're making a call to a Zilch function (this is just the other side of it when we're receiving a call).

Lets add a function to our Ray called GetPointOnRay which takes in a Real 'distance' value and outputs a position along the ray (starting from Position going along the Direction). First we start by writing a function in C++ with the above signature:

.. code-block:: cpp

  void GetPointOnRay(Call& call, ExceptionReport& report)

When we bind this function to Zilch via the LibraryBuilder, this function will look like:

.. code-block:: as

  function GetPointOnRay(distance : Real)

Since this function is going to be an *instance* function, then we know that 'this' is implicitly passed in. *Call* allows us to easily grab 'this' as a Handle. Zilch automatically protects against calling a member function on a null object, so we can assume our this will always be non-null here (other handles such as parameters may need to be checked!). Be sure to store handles by reference because copying them incurs a reference count cost. Once we have the Handle we can call *Dereference* to get a direct pointer to the object.

.. code-block:: cpp

  void GetPointOnRay(Call& call, ExceptionReport& report)
  {
    // Note: Call's Get and Set take parameter indices, however there are two special indices
    // Call::This and Call::Return which specify the return location and this handle location on the stack
    Handle& rayHandle = call.Get<Handle>(Call::This);
    byte* rayData = rayHandle.Dereference();
    
    // Get the distance from the parameter list using the Call object (distance is parameter 0)
    Real distance = call.Get<Real>(0);
    
    // Zilch's calling convention is to pass parameters first and then the implicit 'this' handle after
    // We could have used the old method to grab the stack from the call and then we know
    // 'distance' would be at position 0 and any other parameters we could pass would be after the distance sizeof(Real)
    
    // We know the byte offsets here, though as mentioned above you may
    // want to create a struct for Ray to make this much easier (or store these offsets as constants)
    Real3& position = *(Real3*)(rayData + PositionOffset);
    Real3& direction = *(Real3*)(rayData + DirectionOffset);
    
    // Compute the position along the ray given the distance and return the result via the Call
    Real3 result = position + direction * distance;
    call.Set<Real3>(Call::Return, result);
  }

We can use Zilch's exception handling to guard against bad parameter values, such as a negative distance:

.. code-block:: cpp

  if (distance < 0)
  {
    // We should always make sure to return after throwing a Zilch exception
    // This does NOT invoke C++ exception handling and will not automatically
    // unwind the C++ stack (but it will unwind Zilch's stack up to the last Call unless caught)
    call.GetState()->ThrowException("The distance cannot be negative");
    return;
  }

In our experience (especially for game engines) it is best to keep the number of exceptions thrown low, and always have a way to prevent the exception via logic.. For example in this case the user could check for a negative distance themselves before calling to prevent the exception.

To actually bind this function to our Ray type:

.. code-block:: cpp

  builder.AddBoundFunction(rayType, "GetPointOnRay", GetPointOnRay, OneParameter(core.Real, "distance"), core.VoidType, FunctionOptions::None);

The 'OneParameter' function is a helper that creates an Array of DelegateParameters to describe all the parameter types and names to Zilch. You could otherwise just create the array yourself or use other helpers such as 'TwoParameters', etc. FunctionOptions allows us to specify whether this is a static function or not.

.. note::

  We encourage you to write your own macros and templates to wrap up function binding, or use our higher level binding API.

Creating A Property
-------------------

A Property in Zilch looks similar to a Field but when you attempt to read its value it will call a 'get' function, and when writing to its value it will call a 'set' function. The get takes no parameters and returns the value, and the set takes one parameter (the value to set) and returns nothing. Properties can also be made read only or write only just by passing in null for the set or the get (both cannot be null).

Lets make Direction a property that automatically normalizes itself upon being set. Start by making two Zilch style functions in C++.

.. code-block:: cpp

  void GetDirection(Call& call, ExceptionReport& report)
  {
    // Get takes no parameters (except an implicit 'this' if this is an instance property)
    Handle& rayHandle = call.Get<Handle>(Call::This);
    byte* rayData = rayHandle.Dereference();
    
    // Our getter does nothing special (just return the value directly)
    Real3& direction = *(Real3*)(rayData + DirectionOffset);
    call.Set<Real3>(Call::Return, direction);
  }
  
  void SetDirection(Call& call, ExceptionReport& report)
  {
    // Set takes one parameter as well as the an implicit 'this' if this is an instance property
    Handle& rayHandle = call.Get<Handle>(Call::This);
    byte* rayData = rayHandle.Dereference();
    
    // Get the value that the user wants to set
    Real3& newDirection = call.Get<Real3>(0);
    
    // Get a reference to the value on our type
    Real3& direction = *(Real3*)(rayData + DirectionOffset);
    
    // We're going to perform safe normalization on the direction (this will either normalize it or zero it out)
    direction = Math::AttemptNormalize(newDirection);
  }
  
The last thing we have to do is replace the call to *AddBoundField* for Direction with:

.. code-block:: cpp

  builder.AddBoundProperty(rayType, "Direction", ZilchTypeId(Real3), SetDirection, GetDirection, MemberOptions::None);

.. note::

  You can use AddExtensionProperty or AddExtensionFunction to add a pretend instance or static member to another type that is not your own. This works especially well for adding component properties to a composition (e.g. composition.RigidBody, composition.Model, etc). For extensions, the ``call.Get<Handle>(Call::This)`` will return the handle to the other type.

Creating A Reference Type (Class)
---------------------------------

The main difference between a class and a struct in Zilch is that classes are *always* allocated on the heap. Class types are zeroed out when they are allocated (all members become 0 or null, including composed structs on that class). This is guaranteed to be safe for all classes written entirely within Zilch and is the main reason why constructors are optional for Zilch types, however, for a type bound from C++ we often need to invoke constructors or destructors on members. Moreover, if the C++ class has a virtual table, we always have to be sure to invoke the constructor to initialize it. If we're allowing Zilch to allocate our C++ object (via the *HeapManager*) then we need to be sure to provide a constructor and destructor for it. Using placement new and explicitly invoking the C++ destructor is the best way to achieve proper behavior.

.. code-block:: cpp

  void ClassDefaultConstructor(Call& call, ExceptionReport& report)
  {
    // Get takes no parameters (except an implicit 'this' if this is an instance property)
    Handle& classHandle = call.Get<Handle>(Call::This);
    byte* classData = rayHandle.Dereference();
    
    // The class data at this point should be all zeroed out, we need to use *placement new* to construct our type
    new (classData) Class();
  }

  void ClassDestructor(Call& call, ExceptionReport& report)
  {
    // Get takes no parameters (except an implicit 'this' if this is an instance property)
    Handle& classHandle = call.Get<Handle>(Call::This);
    Class* classData = (Class*)rayHandle.Dereference();
    
    // Explicitly invoke the destructor on our type
    classData->~Class();
  }

And finally to binding the default constructor and destructor to our type using the LibraryBuilder:

.. code-block:: cpp

  builder.AddBoundDefaultConstructor(classType, ClassDefaultConstructor);
  builder.AddBoundDestructor(classType, ClassDestructor);

Take note that you can use other functions like ``AddBoundConstructor`` to bind constructors with parameters. It is often a good idea to have a default constructor (if it makes sense) because it simplifies inheritance and makes other features possibly like automatic serialization.
