Automatic Binding
=================
Zilch comes with a set of templates/macros that allow your C++ classes and functions to be called by Zilch code. We recommend that you read up on the :ref:`LibraryBuilder` documentation first. Internally, the macros simply create a LibraryBuilder and automatically add BoundTypes, Properties, and Functions to it.

Internal Binding
----------------
The first thing we need to do is create a class that derives from StaticLibrary. That class must be visible to all the places where we're going to be doing Zilch binding (generally in a header). We can use the ``ZilchDeclareStaticLibrary`` macro to declare the entire class and singleton implementation.

Below we have an example of a header file that contains the static library *Wallaby* and two example classes that might belong to that library. Remember, the name of the library can be anything you want.

.. literalinclude:: ../../Project/Documentation/Wallaby.hpp
  :language: cpp

With *Internal Binding* we must use the ``ZilchDeclareBaseType`` and ``ZilchDeclareDerivedType`` macros in a public section of the class (typically at the beginning, but make sure to use public!). The Character class also publicly inherits from ``IZilchObject``, which is not required but allows the ``character->ZilchGetDerivedType()`` function to be virtual, which will retrieve the BoundType for ``Player`` instead of ``Character``.

The following code should appear once within a translational unit (do not put this within a header):

.. literalinclude:: ../../Project/Documentation/WallabyBinding.inl
  :language: cpp

The macro ``ZilchDefineStaticLibrary`` is a place where we must initialize all types defined within our library. Because that list of types must exist somewhere within code, we often place that entire ``ZilchDefineStaticLibrary`` in its own translational unit (cpp). The macros ``ZilchDefineType`` can be placed all in a single file, or next to their respective class implementations (such as in a Character.cpp and Player.cpp). Be aware that placing them all in the same file encourages merge conflicts.

The parameters ``builder`` and ``type`` can be used to out

A call to ``GetLibrary`` will automatically build the library. If you place breakpoints or prints in the ``ZilchDefineStaticLibrary`` or ``ZilchDefineType`` macro-functions you will see the code running. GetLibrary will most likely get called when you populate dependencies:

.. code-block:: cpp

  dependencies.push_back(Wallaby::GetLibrary());

External Binding
----------------
If you don't want your classes to have knowledge about Zilch (for example you do not wish to include Zilch.hpp in your core class headers), you can use our *External Binding* macros. The external binding has one major limitation which is that you cannot call ``ZilchGetDerivedType`` on a C++ base class, because the function will not exist. It is possible to mix internal and external binding together, which is especially common when binding types that you cannot change the definition of.

External binding works the same as internal binding, except the following differences:

- ``ZilchDeclareBaseType`` and ``ZilchDeclareDerivedType`` become ``ZilchDeclareExternalBaseType`` and ``ZilchDeclareExternalDerivedType``
 
  - Must be used outside the class definition in a place where the rest of the binding can see. Typically you declare them in a header or at the top of a translational unit that you place all of the external binding.

- ``ZilchDefineType`` becomes ``ZilchDefineExternalType``

Binding Enumerations or Flags
-----------------------------
Since enumerations cannot be virtual or have methods declare inside of them, all enums must be bound using *External Binding*. At the moment, enumerations **MUST** be the same size as the int type, which you can generally force either be using the new C++ class enums or by making an enum value set to 0x7FFFFFFF. Inside the ``ZilchDefineExternalType`` macro at the beginning use the following lines:

.. code-block:: cpp

  // You can also pass in SpecialType::Flags if all the values of the enum are bit flags
  ZilchBindEnum(builder, type, SpecialType::Enumeration);
  ZilchBindEnumValue(builder, type, YourEnum::ValueName, "ValueName");

Redirecting To Zilch Types
--------------------------
Zilch also allows you to redirect your own types (such as your own ``Vector3`` or ``std::string``) to the Zilch equivalents. When using redirects, all functions, properties, and fields that you bind afterward will show up in Zilch as the Zilch type instead of your type. This is done via the ``ZilchDeclareRedirectType`` macro which must be placed in the header and **must** be visible to all *ZilchBind* macros. Similarly to other macros, you must use ``ZilchDefineRedirectType`` in one cpp file.

Place the following code within a header:

.. code-block:: cpp

  // Your own custom vector 3 type
  struct Vec3
  {
    float X;
    float Y;
    float Z;
  };
  
  // Must be seen by all places that do binding
  // Here we tell Zilch binding that everywhere it sees Vec3 it should think of it as Real3
  ZilchDeclareRedirectType(Vec3, Zilch::Real3);

Place the following code within a cpp file:

.. code-block:: cpp

  // These can appear in any single cpp
  Zilch::Real3 Vec3ToZilchReal3(const Vec3& vector)
  {
    return Zilch::Real3(vector.X, vector.Y, vector.Z);
  }
   
  Vec3 ZilchReal3ToVec3(const Zilch::Real3& vector)
  {
    Vec3 vec3 = { vector.x, vector.y, vector.z };
    return vec3;
  }

The following is a common example for being able to bind C++'s ``std::string`` (in the header):

.. code-block:: cpp

  // Must be seen by all places that do binding
  ZilchDeclareRedirectType(std::string, Zilch::String);
   

And in a cpp file:
   
.. code-block:: cpp

  // These can appear in any single cpp
  Zilch::String StdStringToZeroString(const std::string& string)
  {
    return Zilch::String(string.c_str(), string.size());
  }

  std::string ZeroStringToStdString(const Zilch::String& string)
  {
    return std::string(string.c_str(), string.size());
  }
  ZilchDefineRedirectType(std::string, StdStringToZeroString, ZeroStringToStdString);

  
Built-in Types Available To Binding
-----------------------------------
The following list is not exhaustive, but contains the most common types that we accept in binding automatically. Technically anything defined in Zilch.hpp that either internal or external binding and is bound to the Core library can be used. Note that most primitives (int, float, etc) are simply type-defined as Integer, Real, etc and can be used in binding.

- Boolean
- Boolean2
- Boolean3
- Boolean4
- Byte
- Integer
- Integer2
- Integer3
- Integer4
- Real
- Real2
- Real3
- Real4
- Quaternion
- String
- DoubleReal
- DoubleInteger
- Handle (binds to a special type that can accept any handle to any object type)
- Delegate (binds to a special type that can accept any delegate)
- Any (can accept any type in Zilch)
- StringBuilderExtended
- ArrayClass<Handle>
- ArrayClass<Delegate>
- ArrayClass<Boolean>
- ArrayClass<Boolean2>
- ArrayClass<Boolean3>
- ArrayClass<Boolean4>
- ArrayClass<Byte>
- ArrayClass<Integer>
- ArrayClass<Integer2>
- ArrayClass<Integer3>
- ArrayClass<Integer4>
- ArrayClass<Real>
- ArrayClass<Real2>
- ArrayClass<Real3>
- ArrayClass<Real4>
- ArrayClass<Quaternion>
- ArrayClass<DoubleInteger>
- ArrayClass<DoubleReal>
- ArrayClass<Any>

The following types are automatically redirected to the Zilch Integer type (except unsigned long long which redirects to DoubleInteger):

- char
- signed char
- signed short
- unsigned short
- unsigned int
- signed long
- unsigned long
- unsigned long long

Limitations
-----------
At the moment, there is no way in binding to accept templates (such as HashMapClass or ArrayClass) templated upon any other type other than specified above.
