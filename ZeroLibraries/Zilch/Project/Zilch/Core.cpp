/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  ZilchDefineType(Enum, builder, type)
  {
    ErrorIf(sizeof(Enum) != sizeof(Integer),
      "The base Enum must be the same size as Integer");
    type->SpecialType = SpecialType::Enumeration;
    type->ToStringFunction = &ZZ::VirtualMachine::UnknownEnumerationToString;
  }

  //***************************************************************************
  ZilchDeclareExternalType(Members::Enum);
  ZilchDeclareExternalType(FileMode::Enum);
  ZilchDeclareExternalType(StreamCapabilities::Enum);
  ZilchDeclareExternalType(StreamOrigin::Enum);
  ZilchDeclareExternalType(Zero::ProcessStartInfo);

  //***************************************************************************
  ZilchDefineRange(ParameterArray::range);
  ZilchDefineRange(MemberRange<Member>);
  ZilchDefineRange(MemberRange<Property>);
  ZilchDefineRange(MemberRange<GetterSetter>);
  ZilchDefineRange(MemberRange<Field>);
  ZilchDefineRange(MemberRange<Function>);

  //***************************************************************************
  Core* Core::Instance = nullptr;

  //***************************************************************************
  void Core::InitializeInstance()
  {
    ReturnIf(Instance != nullptr,,
      "Can't initialize a static library more than once");
    Instance = new Core();
  }

  //***************************************************************************
  void Core::Destroy()
  {
    delete Instance;
    Instance = nullptr;
  }

  //***************************************************************************
  Core& Core::GetInstance()
  {
    ErrorIf(Instance == nullptr,
      "Attempted to get an uninitialized singleton static library");
    return *Instance;
  }

  //***************************************************************************
  Core::Core() :
    StaticLibrary("Core"),
    IntegerType(nullptr),
    DoubleIntegerType(nullptr),
    RealType(nullptr),
    DoubleRealType(nullptr),
    BooleanType(nullptr),
    Real2Type(nullptr),
    Real3Type(nullptr),
    Real4Type(nullptr),
    Integer2Type(nullptr),
    Integer3Type(nullptr),
    Integer4Type(nullptr),
    Boolean2Type(nullptr),
    Boolean3Type(nullptr),
    Boolean4Type(nullptr),
    QuaternionType(nullptr),
    Real2x2Type(nullptr),
    Real3x3Type(nullptr),
    Real4x4Type(nullptr),
    StringType(nullptr),
    ExceptionType(nullptr),
    MathType(nullptr),
    VoidType(nullptr),
    NullType(nullptr),
    ErrorType(nullptr),
    AnyDelegateType(nullptr),
    AnyHandleType(nullptr),
    AnythingType(nullptr)
  {
    // Clear out all the vector types
    memset(this->RealTypes,     0, sizeof(this->RealTypes));
    memset(this->IntegerTypes,  0, sizeof(this->IntegerTypes));
    memset(this->BooleanTypes,  0, sizeof(this->BooleanTypes));
  }
  
  //***************************************************************************
  Core::~Core()
  {
  }

  //***************************************************************************
  void IntegerAbs(Call& call, ExceptionReport& report)
  {
    // Get the input value
    Integer input = call.Get<Integer>(0);

    // Apply the absolute value function
    if (input < 0)
    {
      input = -input;
    }

    call.Set(Call::Return, input);
  }

  //***************************************************************************
  template <typename T>
  T Lerp(T start, T end, Real t)
  {
     return (Real(1.0) - t) * start + t * end;
  }

  //***************************************************************************
  Integer ReinterpretRealToInteger(Real value)
  {
    return *reinterpret_cast<Integer*>(&value);
  }

  //***************************************************************************
  Real ReinterpretIntegerToReal(Integer value)
  {
    return *reinterpret_cast<Real*>(&value);
  }

  //***************************************************************************
  void Pi(Call& call, ExceptionReport& report)
  {
    call.Set(Call::Return, 3.1415926535897932384626433832795f);
  }
  
  //***************************************************************************
  void E(Call& call, ExceptionReport& report)
  {
    call.Set(Call::Return, 2.7182818284590452353602874713526f);
  }

  //***************************************************************************
  template <size_t Components, typename ComponentType>
  void VectorDefaultConstructor(Call& call, ExceptionReport& report)
  {
    // Get the handle to ourselves
    Handle& selfHandle = call.GetHandle(Call::This);
    byte* self = selfHandle.Dereference();

    // Clear out the object's data (for all vector types)
    memset(self, 0, Components * sizeof(ComponentType));
  }

  //***************************************************************************
  template <size_t Components, typename ComponentType>
  void VectorScalarConstructor(Call& call, ExceptionReport& report)
  {
    // Get the handle to ourselves
    Handle& selfHandle = call.GetHandle(Call::This);
    ComponentType* self = (ComponentType*)selfHandle.Dereference();

    // The first thing on the stack will be the single value (scalar)
    ComponentType scalar = call.Get<ComponentType>(0);

    // Loop through all the components
    for (size_t i = 0; i < Components; ++i)
    {
      self[i] = scalar;
    }
  }

  //***************************************************************************
  template <size_t Components, typename ComponentType>
  void GenerateVectorScalarConstructor(LibraryBuilder& builder, BoundType* type, BoundType* scalarType)
  {
    // We only have one parameter
    ParameterArray parameters;
    DelegateParameter& scalarParam = parameters.PushBack();
    scalarParam.ParameterType = scalarType;
    scalarParam.Name = "scalar";

    // Bind constructors to the vector types
    builder.AddBoundConstructor(type,  VectorScalarConstructor<Components, ComponentType>, parameters);
  }

  //***************************************************************************
  template <size_t Components, typename ComponentType>
  void VectorComponentConstructor(Call& call, ExceptionReport& report)
  {
    // Get the handle to ourselves
    Handle& selfHandle = call.GetHandle(Call::This);
    ComponentType* self = (ComponentType*)selfHandle.Dereference();

    // Get the stack memory where all the values live
    byte* stackValues = call.GetParametersUnchecked();

    // We want to get the alignment of the stack values
    size_t alignedComponentSize = AlignToBusWidth(sizeof(ComponentType));

    // This is very subtle but because of padding we can't assume the values on the stack
    // directly align with the values inside our structure. For example, sizeof(bool) is typically 1,
    // however on the stack each bool is aligned by 4 bytes so therefore 3 bools in the stack is 3 * 4 = 12,
    // rather than a Boolean3 which is actually sizeof(bool) * 3 = 3 + padding = 4
    for (size_t i = 0; i < Components; ++i)
    {
      // Grab the current value of the stack
      ComponentType value = *(ComponentType*)(stackValues + i * alignedComponentSize);

      // Treat ourself like a contiguous array and directly set our members
      self[i] = value;
    }
  }

  //***************************************************************************
  void ScalarTypeRealOne(byte* outData)
  {
    *(Real*)outData = 1.0f;
  }

  //***************************************************************************
  void ScalarTypeIntegerOne(byte* outData)
  {
    *(Integer*)outData = 1;
  }

  //***************************************************************************
  void ScalarTypeBooleanOne(byte* outData)
  {
    *(Boolean*)outData = true;
  }

  //***************************************************************************
  template <size_t Components>
  void VectorCount(Call& call, ExceptionReport& report)
  {
    // Return the number of components in the vector
    call.Set(Call::Return, (Integer)Components);
  }

  //***************************************************************************
  template <size_t Components, typename ComponentType>
  void VectorGet(Call& call, ExceptionReport& report)
  {
    // Get the handle to ourselves
    Handle& selfHandle = call.GetHandle(Call::This);
    ComponentType* self = (ComponentType*)selfHandle.Dereference();

    // Read the index off the stack
    Integer index = call.Get<Integer>(0);
    
    // Make sure the index is within range
    if (index < 0 || index >= Components)
    {
      call.GetState()->ThrowException(report, String::Format("The index used to access a component of a vector was out of range [0-%d]", Components - 1));
      return;
    }

    // Return the vector's value at that index
    ComponentType result = self[index];
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  template <size_t Components, typename ComponentType>
  void VectorSet(Call& call, ExceptionReport& report)
  {
    // Get the handle to ourselves
    Handle& selfHandle = call.GetHandle(Call::This);
    ComponentType* self = (ComponentType*)selfHandle.Dereference();

    // Read the index off the stack
    Integer index = call.Get<Integer>(0);

    // Read the value we want to set
    ComponentType value = call.Get<ComponentType>(1);
    
    // Make sure the index is within range
    if (index < 0 || index >= Components)
    {
      call.GetState()->ThrowException(report, String::Format("The index used to access a component of a vector was out of range [0-%d]", Components - 1));
      return;
    }

    // Set the value on the vector
    self[index] = value;
  }

  //***************************************************************************
  void Vector3CrossProduct(Call& call, ExceptionReport& report)
  {
    // Read in the two vectors
    Real3& vector0 = call.Get<Real3&>(0);
    Real3& vector1 = call.Get<Real3&>(1);

    // Perform the cross product
    Real3 result = Math::Cross(vector0, vector1);

    // Output the result
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  template <size_t Components>
  Real VectorDotProduct(Real* vector0, Real* vector1)
  {
    // Initialize the result to zero
    Real returnValue = 0.0f;

    // Loop through all the components to apply the dot product
    for (size_t i = 0; i < Components; ++i)
    {
      // Perform the dot product operation
      returnValue += vector0[i] * vector1[i];
    }

    // Return the computed dot product
    return returnValue;
  }

  //***************************************************************************
  template <size_t Components>
  void VectorDotProduct(Call& call, ExceptionReport& report)
  {
    // Get pointers to the two vectors
    Real* vector0 = (Real*)call.GetParameterUnchecked(0);
    Real* vector1 = (Real*)call.GetParameterUnchecked(1);

    // Compute the dot product
    Real result = VectorDotProduct<Components>(vector0, vector1);
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  template <size_t Components>
  void VectorLengthSq(Call& call, ExceptionReport& report)
  {
    // Get a pointer to the vector
    Real* vector = (Real*)call.GetParameterUnchecked(0);

    // Dot the vector with itself (it's the equivalent of length squared)
    Real lengthSq = VectorDotProduct<Components>(vector, vector);
    call.Set(Call::Return, lengthSq);
  }

  //***************************************************************************
  template <size_t Components>
  void VectorLength(Call& call, ExceptionReport& report)
  {
    // Get a pointer to the vector
    Real* vector = (Real*)call.GetParameterUnchecked(0);

    // Dot the vector with itself (it's the equivalent of length squared)
    Real lengthSq = VectorDotProduct<Components>(vector, vector);
    Real length = std::sqrt(lengthSq);
    call.Set(Call::Return, length);
  }

  //***************************************************************************
  template <size_t Components>
  void VectorDistance(Call& call, ExceptionReport& report)
  {
    // Get a pointer to the vectors
    Real* vector1 = (Real*)call.GetParameterUnchecked(0);
    Real* vector2 = (Real*)call.GetParameterUnchecked(1);

    Real diff[Components];
    for (size_t i = 0; i < Components; ++i)
    {
      diff[i] = vector1[i] - vector2[i];
    }

    // Dot the vector with itself (it's the equivalent of length squared)
    Real distanceSq = VectorDotProduct<Components>(diff, diff);
    Real distance = std::sqrt(distanceSq);
    call.Set(Call::Return, distance);
  }

  //***************************************************************************
  template <size_t Components>
  void VectorDistanceSq(Call& call, ExceptionReport& report)
  {
    // Get a pointer to the vectors
    Real* vector1 = (Real*)call.GetParameterUnchecked(0);
    Real* vector2 = (Real*)call.GetParameterUnchecked(1);

    Real diff[Components];
    for(size_t i = 0; i < Components; ++i)
    {
      diff[i] = vector1[i] - vector2[i];
    }

    // Dot the vector with itself (it's the equivalent of length squared)
    Real distanceSq = VectorDotProduct<Components>(diff, diff);
    call.Set(Call::Return, distanceSq);
  }

  //***************************************************************************
  template <size_t Components>
  void VectorNormalize(Call& call, ExceptionReport& report)
  {
    // Get a pointer to the vector
    Real* vector = (Real*)call.GetParameterUnchecked(0);

    // Dot the vector with itself (it's the equivalent of length squared)
    Real lengthSq = VectorDotProduct<Components>(vector, vector);
    Real length = std::sqrt(lengthSq);

    Real* result = (Real*)call.GetReturnUnchecked();
    call.MarkReturnAsSet();

    // If the length is non zero (don't want undefined divisions)
    if (length > 1e-20f)
    {
      // Loop through all the components and divide out the length
      for (size_t i = 0; i < Components; ++i)
      {
        result[i] = vector[i]  / length;
      }
    }
    else
    {
      // Zero out all elements of the returned vector
      for (size_t i = 0; i < Components; ++i)
      {
        result[i] = 0.0f;
      }
    }
  }
  
  //***************************************************************************
  void QuaternionDefaultConstructor(Call& call, ExceptionReport& report)
  {
    // Get our own object
    Handle& selfHandle = call.GetHandle(Call::This);
    byte* self = selfHandle.Dereference();

    // Set the quaternion to the identity quaternion (0, 0, 0, 1)
    *(Quaternion*)self = Quaternion::cIdentity;
  }

  //***************************************************************************
  void QuaternionIdentity(Call& call, ExceptionReport& report)
  {
    call.Set(Call::Return, Quaternion::cIdentity);
  }

  //***************************************************************************
  void QuaternionAdd(Call& call, ExceptionReport& report)
  {
    Quaternion& input0 = call.Get<Quaternion&>(0);
    Quaternion& input1 = call.Get<Quaternion&>(1);

    Quaternion result = input0 + input1;
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  void QuaternionSubtract(Call& call, ExceptionReport& report)
  {
    Quaternion& input0 = call.Get<Quaternion&>(0);
    Quaternion& input1 = call.Get<Quaternion&>(1);

    Quaternion result = input0 - input1;
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  void QuaternionInvert(Call& call, ExceptionReport& report)
  {
    Quaternion& input = call.Get<Quaternion&>(0);
    Real lengthSq = input.LengthSq();

    // We can't invert a zero length quaternion
    if (lengthSq == 0.0f)
    {
      call.GetState()->ThrowException(report, "Attempting to invert a zero length quaternion which would result in a 0 division");
      return;
    }
    Real inverseLengthSq = 1.0f / lengthSq;

    Quaternion result;
    result.x = -input.x * inverseLengthSq;
    result.y = -input.y * inverseLengthSq;
    result.z = -input.z * inverseLengthSq;
    result.w = +input.w * inverseLengthSq;

    call.Set(Call::Return, result);
  }

  //***************************************************************************
  void QuaternionSlerp(Call& call, ExceptionReport& report)
  {
    // Get pointers to the two vectors
    Quaternion& start = call.Get<Quaternion&>(0);
    Quaternion& end = call.Get<Quaternion&>(1);

    // Get the interpolant value
    Real interpolant = call.Get<Real>(2);

    Quaternion result = Math::Slerp(start, end, interpolant);
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  void QuaternionAxisAngleConstructor(Call& call, ExceptionReport& report)
  {
    Real3& axis = call.Get<Real3&>(0);
    Real radian = call.Get<Real>(1);

    Quaternion result = ToQuaternion(axis, radian);
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  void QuaternionTransformQuaternion(Call& call, ExceptionReport& report)
  {
    Quaternion& value = call.Get<Quaternion&>(0);
    Quaternion& by = call.Get<Quaternion&>(1);

    Quaternion result = by * value;
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  void QuaternionTransformVector3(Call& call, ExceptionReport& report)
  {
    Real3& value = call.Get<Real3&>(0);
    Quaternion& by = call.Get<Quaternion&>(1);

    Real3 result = Math::Multiply(by, value);
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  void QuaternionMultiplyQuaternion(Call& call, ExceptionReport& report)
  {
    Quaternion& by = call.Get<Quaternion&>(0);
    Quaternion& the = call.Get<Quaternion&>(1);

    Quaternion result = by * the;
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  void QuaternionMultiplyVector3(Call& call, ExceptionReport& report)
  {
    Quaternion& by = call.Get<Quaternion&>(0);
    Real3& the = call.Get<Real3&>(1);

    Real3 result = Math::Multiply(by, the);
    call.Set(Call::Return, result);
  }
  
  //***************************************************************************
  bool EvolvePermutation(size_t indices[], size_t count, size_t upperBound)
  {
    // Increment the first index
    ++indices[0];

    // Now we need to check if we need to overflow into the next slot
    size_t innerIndex = 0;

    // Loop while we're overflowing and our index hasn't gone outside the 
    while (indices[innerIndex] == upperBound)
    {
      // If we hit the last index, and it's at the upper bound (confirmed above)
      if (innerIndex == (count - 1))
        return false;

      // Reset the current counter
      indices[innerIndex] = 0;

      // Increment the index so we can increment the next value (overflow)
      ++innerIndex;
      ++indices[innerIndex];
    }
    
    // We need to continue the permutation!
    return true;
  }
  
  //***************************************************************************
  template <typename ComponentType>
  void VectorSwizzleSet(Call& call, ExceptionReport& report)
  {
    // Get the function that's executing us
    Function* function = call.GetFunction();

    // Get the order of the swizzle from the user data (directly!)
    unsigned order = *(unsigned*)&function->UserData;

    // Get how many components the swizzle uses
    unsigned swizzleCount = (order & 7);  // 00 00 00 00 111

    // Read the userdata to figure out which components are being swizzled
    const size_t MaxComponents = 4;
    size_t componentIndex[MaxComponents] =
    {
      (order >> 3) & 3, // 00 00 00 11 000
      (order >> 5) & 3, // 00 00 11 00 000
      (order >> 7) & 3, // 00 11 00 00 000
      (order >> 9) & 3, // 11 00 00 00 000
    };

    // Get a pointer to the vector that we're being set to
    ComponentType* vector = (ComponentType*)call.GetParameterUnchecked(0);

    // Get a pointer to our handle
    Handle& selfHandle = call.GetHandle(Call::This);
    ComponentType* self = (ComponentType*)selfHandle.Dereference();

    // Loop through all the components and read the index that we're supposed to swizzle
    for (size_t i = 0; i < swizzleCount; ++i)
    {
      // Get the index for the current component swizzle
      size_t index = (size_t)componentIndex[i];
      
      // Perform the component wise set
      self[index] = vector[i];
    }
  }
  
  //***************************************************************************
  template <typename ComponentType>
  void VectorSwizzleGet(Call& call, ExceptionReport& report)
  {
    // Get the function that's executing us
    Function* function = call.GetFunction();

    // Get the order of the swizzle from the user data (directly!)
    unsigned order = *(unsigned*)&function->UserData;

    // Get how many components the swizzle uses
    unsigned swizzleCount = (order & 7);  // 00 00 00 00 111

    // Read the userdata to figure out which components are being swizzled
    const size_t MaxComponents = 4;
    size_t componentIndex[MaxComponents] =
    {
      (order >> 3) & 3, // 00 00 00 11 000
      (order >> 5) & 3, // 00 00 11 00 000
      (order >> 7) & 3, // 00 11 00 00 000
      (order >> 9) & 3, // 11 00 00 00 000
    };

    // Get a pointer to the the resulting vector
    ComponentType* result = (ComponentType*)call.GetReturnUnchecked();
    call.MarkReturnAsSet();

    // Get a pointer to our handle
    Handle& selfHandle = call.GetHandle(Call::This);
    ComponentType* self = (ComponentType*)selfHandle.Dereference();

    // Loop through all the components and read the index that we're supposed to swizzle
    for (size_t i = 0; i < swizzleCount; ++i)
    {
      // Get the index for the current component swizzle
      size_t index = (size_t)componentIndex[i];
      
      // Perform the component wise get
      result[i] = self[index];
    }
  }

  //***************************************************************************
  template <size_t Components, typename ComponentType>
  void GenerateVectorSwizzles(LibraryBuilder& builder, BoundType* type, BoundType* componentTypes[Core::MaxComponents])
  {
    // Get a reference to the core so we can access all the vector/real types
    Core& core = Core::GetInstance();

    char componentNamesSet[2][4] = {{'X', 'Y', 'Z', 'W'}, {'R', 'G', 'B', 'A'}};

    // For now only bind the XYZW components (until code completion is better)
    for (size_t namesIndex = 0; namesIndex < 1; ++namesIndex)
    {
      // Loop through the number of components, down to one
      for (size_t count = Core::MaxComponents; count > 0; --count)
      {
        // Indices into a specific component (x, y, z, w)
        char *componentNames = componentNamesSet[namesIndex];
        size_t componentIndex[Core::MaxComponents] = {0};

        Type* sizeType = componentTypes[count - 1];

        // Loop through all the permutations
        do
        {
          StringBuilder nameBuilder;

          bool inOrder = true;
          bool noDuplicateComponents = true;
          bool usedComponent[Core::MaxComponents] = {0};

          for (size_t i = 0; i < count; ++i)
          {
            size_t index = componentIndex[i];

            // Check if the components are in order
            if (i > 0 && componentIndex[i - 1] != index - 1)
            {
              inOrder = false;
            }

            bool& used = usedComponent[index];

            if (used)
            {
              noDuplicateComponents = false;
            }

            used = true;

            nameBuilder.Append(componentNames[index]);
          }

          String name = nameBuilder.ToString();

          // Store whatever property we make in here so we can set whether it's hidden afterwards
          Property* property = nullptr;

          // If the swizzle is a completely in order swizzle
          // That means that we can directly access the memory of the structure (no properties needed)
          if (inOrder)
          {
            // Compute the offset into the vector
            // X is at the front, and therefore is 0, y should be 1 real in, etc
            size_t offset = componentIndex[0] * sizeof(ComponentType);

            // Add the swizzle field
            property = builder.AddBoundField(type, name, sizeType, offset, MemberOptions::None);
          }
          // It must be a property then
          else
          {
            // Create a single integer to be used as userdata to describe the order of the swizzle
            void* orderUserData = nullptr;

            // Put the order into a series of bytes that we store directly as the user data pointer
            size_t& order = *(size_t*)&orderUserData;

            order = count |
              (componentIndex[0] << 3)  |
              (componentIndex[1] << 5)  |
              (componentIndex[2] << 7)  |
              (componentIndex[3] << 9);

            // As long as we have no duplicate components, we
            // can at least make it a read/write property (with a get/set)
            if (noDuplicateComponents)
            {
              // Add the get/set property
              property = builder.AddBoundGetterSetter
                (
                type,
                name,
                sizeType,
                VectorSwizzleSet<ComponentType>,
                VectorSwizzleGet<ComponentType>,
                MemberOptions::None
                );

              // Store the userdata on the property get/set functions
              property->Get->UserData = orderUserData;
              property->Set->UserData = orderUserData;
            }
            else
            {
              // Add the get (read only) property
              property = builder.AddBoundGetterSetter
                (
                type,
                name,
                sizeType,
                nullptr,
                VectorSwizzleGet<ComponentType>,
                MemberOptions::None
                );

              // Store the userdata on the property get function
              property->Get->UserData = orderUserData;
            }
          }

          // Hide all properties that aren't just .X, .Y, .Z, .W
          // Also hide .X on single values
          if (count != 1 || type == componentTypes[0])
            property->IsHidden = true;
        }
        while (EvolvePermutation(componentIndex, count, Components));
      }
    }
  }

  //***************************************************************************
  template <size_t Components, typename ComponentType>
  void GenerateVectorComponentConstructors(LibraryBuilder& builder, BoundType* type, BoundType* componentTypes[Core::MaxComponents])
  {
    // Get a reference to the core so we can access all the vector/real types
    Core& core = Core::GetInstance();

    // We're generating constructors for a fixed number of components (a specific vector type)
    for (size_t count = Components; count > 0; --count)
    {
      // Only ever index up to 'count' in size
      // This array is an array of indices into the component types above
      // This array will be incremented much like an N counter
      size_t typeIndex[Core::MaxComponents] = {0};

      // Loop through all the 
      do
      {
        // Create an array to hold all the parameters
        ParameterArray parameters;
        
        // With this permutation of constructor, what does the size end up being?
        size_t size = 0;

        // Walk through all the types, make sure the sum of the sizes of types we could take matches our size
        for (size_t i = 0; i < count; ++i)
        {
          // The type index corresponds to its size (component type) + 1
          size += (typeIndex[i] + 1);
        }

        // If the size of all the arguments matched our vector size...
        if (size == Components)
        {
          // Then walk through the arguments and generate a constructor signature for it
          for (size_t i = 0; i < count; ++i)
          {
            // Each parameter contributes to the signature (set it's type to the permuted vector types)
            DelegateParameter& param = parameters.PushBack();
            param.ParameterType = componentTypes[typeIndex[i]];
          }

          // Finally, add the constructor to both our type and the library itself
          builder.AddBoundConstructor(type, VectorComponentConstructor<Components, ComponentType>, parameters);
        }
      }
      // Walk through all possible constructor permutations
      while (EvolvePermutation(typeIndex, count, count + 1));
    }
  }
  
  //***************************************************************************
  BoundType* Core::InstantiatePropertyDelegate
  (
    LibraryBuilder& builder,
    StringParam baseName,
    StringParam fullyQualifiedName,
    const Array<Constant>& templateTypes,
    const void* userData
  )
  {
    // Get the type that we're getting / setting
    Type* innerType = templateTypes.Front().TypeValue;

    // We could have put core in our userdata, but no real need
    Core& core = Core::GetInstance();

    ParameterArray setParameters;
    DelegateParameter& setParameter = setParameters.PushBack();
    setParameter.Name = "value";
    setParameter.ParameterType = innerType;

    DelegateType* setDelegateType = builder.GetDelegateType(setParameters, core.VoidType);
    DelegateType* getDelegateType = builder.GetDelegateType(ParameterArray(), innerType);

    BoundType* type = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ReferenceType, sizeof(PropertyDelegateTemplate));
    builder.AddBoundField(type, "Set", setDelegateType, offsetof(PropertyDelegateTemplate, Set), MemberOptions::None);
    builder.AddBoundField(type, "Get", getDelegateType, offsetof(PropertyDelegateTemplate, Get), MemberOptions::None);
    builder.AddBoundField(type, "Property", ZilchTypeId(Property), offsetof(PropertyDelegateTemplate, ReferencedProperty), MemberOptions::None);
    builder.AddBoundField(type, "Instance", ZilchTypeId(Any), offsetof(PropertyDelegateTemplate, ReferencedProperty), MemberOptions::None);
    
    // Bind the constructor
    ZilchFullBindDestructor(builder, type, PropertyDelegateTemplate);
    ZilchFullBindConstructor(builder, type, PropertyDelegateTemplate, nullptr);

    return type;
  }

  void SubString(Call& call, ExceptionReport& report)
  {
    // Get this string object
    String& self = call.Get<String&>(Call::This);

    RuneIterator start = call.Get<RuneIterator>(0);
    RuneIterator end = call.Get<RuneIterator>(1);

    // Validate range contains the zilch throw exception so just return
    if(StringRangeExtended::ValidateRange(self, start.mRange) == false)
      return;
    if(StringRangeExtended::ValidateRange(self, end.mRange) == false)
      return;

    String result(start.mRange.Begin(), end.mRange.Begin());
    
    if(result.Empty() < 0)
    {
      call.GetState()->ThrowException(report, "A negative substring length is not supported");
      return;
    }
    
    call.Set(Call::Return, &result);
  }

  void SubStringFromRuneIndices(Call& call, ExceptionReport& report)
  {
    // Get this string object
    String& self = call.Get<String&>(Call::This);

    Integer startIndice = call.Get<Integer>(0);
    Integer endIndice = call.Get<Integer>(1);
    Integer stringCount = (Integer)self.ComputeRuneCount();

    if (startIndice < 0 || endIndice > stringCount)
    {
      call.GetState()->ThrowException(report, "String index was out of bounds");
      return;
    }
    if ((endIndice - startIndice) < 0)
    {
      call.GetState()->ThrowException(report, "A negative substring length is not supported");
      return;
    }
    StringIterator stringStart = self.Begin() + startIndice;
    StringIterator stringEnd = self.Begin() + endIndice;
    String result(stringStart, stringEnd);
    call.Set(Call::Return, &result);
  }

  //***************************************************************************
  void SubStringBytes(Call& call, ExceptionReport& report)
  {
    // Get this string object
    String& self = call.Get<String&>(Call::This);

    Integer start = call.Get<Integer>(0);
    Integer length = call.Get<Integer>(1);
    Integer stringCount = (Integer)self.SizeInBytes();
    Integer end = start + length;

    // Check the character index (note that 'end' being equal to the string count is valid)
    if (start < 0 || end > stringCount)
    {
      call.GetState()->ThrowException(report, "String index was out of bounds");
      return;
    }
    if (length < 0)
    {
      call.GetState()->ThrowException(report, "A negative substring length is not supported");
      return;
    }
    cstr stringStart = self.c_str() + start;
    cstr stringEnd = self.c_str() + end;
    StringRange result(self, stringStart, stringEnd);
    StringRangeExtended::SetResultStringRange(call, report, self, result);
  }

  //***************************************************************************
  void StringGetRune(Call& call, ExceptionReport& report)
  {
    // Get this string object
    String& self = call.Get<String&>(Call::This);
    Integer index = call.Get<Integer>(0);
    Integer stringCount = (Integer)self.ComputeRuneCount();
  
    // Check the character index (here, index CANNOT be the string count)
    if (index < 0 || index >= stringCount)
    {
      call.GetState()->ThrowException(report, "String index was out of bounds");
      return;
    }
  
    Rune result = *(self.Begin() + index);
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  void StringRuneIteratorFromByteIndex(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);
    int byteIndex = call.Get<int>(0);

    StringRangeExtended::RuneIteratorFromByteIndexInternal(call, report, self, self.All(), byteIndex);
  }

  //***************************************************************************
  void StringRuneIteratorFromRuneIndex(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);
    int runeIndex = call.Get<int>(0);

    StringRangeExtended::RuneIteratorFromRuneIndexInternal(call, report, self, self.All(), runeIndex);
  }

  //***************************************************************************
  void StringEmpty(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);

    call.Set(Call::Return, self.Empty());
  }

  //***************************************************************************
  void StringIsNotEmpty(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);

    call.Set(Call::Return, !self.Empty());
  }

  //***************************************************************************
  void StringBegin(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);

    StringRange result = StringRange(self.Begin(), self.End());
    StringRangeExtended::SetResultIterator(call, report, result);
  }

  //***************************************************************************
  void StringEnd(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);

    StringRange result = StringRange(self.End(), self.End());
    StringRangeExtended::SetResultIterator(call, report, result);
  }

  //***************************************************************************
  void StringAll(Call& call, ExceptionReport& report)
  {
    // Get this string object
    String& self = call.Get<String&>(Call::This);

    StringRangeExtended::SetResultStringRange(call, report, self, self.All());
  }

  //***************************************************************************
  void StringByteCount(Call& call, ExceptionReport& report)
  {
    // Get this string object
    String& self = call.Get<String&>(Call::This);

    Integer count = (Integer)self.SizeInBytes();
    call.Set(Call::Return, count);
  }

  //***************************************************************************
  void StringComputeRuneCount(Call& call, ExceptionReport& report)
  {
    // Get this string object
    String& self = call.Get<String&>(Call::This);

    Integer count = (Integer)self.ComputeRuneCount();
    call.Set(Call::Return, count);
  }

  //***************************************************************************
  void StringConcatenate(Call& call, ExceptionReport& report)
  {
    String* first = call.Get<String*>(0);
    String* second = call.Get<String*>(1);

    if (first == nullptr || second == nullptr)
    {
      call.GetState()->ThrowNullReferenceException(report);
      return;
    }

    String result = BuildString(*first, *second);
    call.Set(Call::Return, &result);
  }

  //***************************************************************************
  void StringRangeConcatenate(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* first = call.Get<StringRangeExtended*>(0);
    StringRangeExtended* second = call.Get<StringRangeExtended*>(1);

    if (first == nullptr || second == nullptr)
    {
      call.GetState()->ThrowNullReferenceException(report);
      return;
    }

    String result = BuildString(first->mRange, second->mRange);
    call.Set(Call::Return, &result);
  }

  //***************************************************************************
  void StringFromChar(Call& call, ExceptionReport& report)
  {
    char character = (char)call.Get<Integer>(0);
    String result(&character, 1);
    call.Set(Call::Return, &result);
  }

  //***************************************************************************
  void StringContains(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);
    StringRangeExtended* first = call.Get<StringRangeExtended*>(0);

    bool result = self.Contains(first->mRange);
    call.Set(Call::Return, result);
  }
  
  //***************************************************************************
  void StringCompare(Call& call, ExceptionReport& report)
  {
    String* first = call.Get<String*>(0);
    String* second = call.Get<String*>(1);

    int result = first->CompareTo(*second);
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  void StringRangeCompare(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* first = call.Get<StringRangeExtended*>(0);
    StringRangeExtended* second = call.Get<StringRangeExtended*>(1);

    int result = first->mRange.CompareTo(second->mRange);
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  void StringCompareTo(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);
    StringRangeExtended* first = call.Get<StringRangeExtended*>(0);

    int result = self.CompareTo(first->mRange);
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  void StringStartsWith(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);
    StringRangeExtended* first = call.Get<StringRangeExtended*>(0);

    bool result = self.StartsWith(first->mRange);
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  void StringEndsWith(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);
    StringRangeExtended* first = call.Get<StringRangeExtended*>(0);

    bool result = self.EndsWith(first->mRange);
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  void StringTrimStart(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);

    StringRangeExtended::SetResultStringRange(call, report, self, self.TrimStart());
  }

  //***************************************************************************
  void StringTrimEnd(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);

    StringRangeExtended::SetResultStringRange(call, report, self, self.TrimEnd());
  }

  //***************************************************************************
  void StringTrim(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);

    StringRangeExtended::SetResultStringRange(call, report, self, self.Trim());
  }

  //***************************************************************************
  void StringToLower(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);

    String result = self.ToLower();
    call.Set(Call::Return, &result);
  }

  //***************************************************************************
  void StringToUpper(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);

    String result = self.ToUpper();
    call.Set(Call::Return, &result);
  }

  //***************************************************************************
  void StringReplace(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);
    StringRangeExtended* first = call.Get<StringRangeExtended*>(0);
    StringRangeExtended* second = call.Get<StringRangeExtended*>(1);

    String result = self.Replace(first->mRange, second->mRange);
    call.Set(Call::Return, &result);
  }

  //***************************************************************************
  void StringFindRangeInclusive(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);
    StringRangeExtended* first = call.Get<StringRangeExtended*>(0);
    StringRangeExtended* second = call.Get<StringRangeExtended*>(1);

    StringRange result = self.FindRangeInclusive(first->mRange, second->mRange);
    StringRangeExtended::SetResultStringRange(call, report, self, result);
  }

  //***************************************************************************
  void StringFindRangeExclusive(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);
    StringRangeExtended* first = call.Get<StringRangeExtended*>(0);
    StringRangeExtended* second = call.Get<StringRangeExtended*>(1);

    StringRange result = self.FindRangeExclusive(first->mRange, second->mRange);
    StringRangeExtended::SetResultStringRange(call, report, self, result);
  }

  //***************************************************************************
  void StringFindFirstOf(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);
    StringRangeExtended* first = call.Get<StringRangeExtended*>(0);

    StringRange result = self.FindFirstOf(first->mRange);
    StringRangeExtended::SetResultStringRange(call, report, self, result);
  }

  //***************************************************************************
  void StringFindLastOf(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);
    StringRangeExtended* first = call.Get<StringRangeExtended*>(0);

    StringRange result = self.FindLastOf(first->mRange);
    StringRangeExtended::SetResultStringRange(call, report, self, result);
  }

  //***************************************************************************
  void JoinTwoStrings(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* separator = call.Get<StringRangeExtended*>(0);
    StringRangeExtended* first = call.Get<StringRangeExtended*>(1);
    StringRangeExtended* second = call.Get<StringRangeExtended*>(2);
    if(separator == NULL || first == NULL || second == NULL)
    {
      call.GetState()->ThrowNullReferenceException(report);
      return;
    }

    String result = Zero::String::Join(separator->mRange, first->mRange, second->mRange);
    call.Set(Call::Return, &result);
  }

  //***************************************************************************
  void JoinThreeStrings(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* separator = call.Get<StringRangeExtended*>(0);
    StringRangeExtended* first = call.Get<StringRangeExtended*>(1);
    StringRangeExtended* second = call.Get<StringRangeExtended*>(2);
    StringRangeExtended* third = call.Get<StringRangeExtended*>(3);
    if(separator == nullptr || first == nullptr || second == nullptr || third == nullptr)
    {
      call.GetState()->ThrowNullReferenceException(report);
      return;
    }

    String result = Zero::String::Join(separator->mRange, first->mRange, second->mRange, third->mRange);
    call.Set(Call::Return, &result);
  }

  //***************************************************************************
  void JoinFourStrings(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* separator = call.Get<StringRangeExtended*>(0);
    StringRangeExtended* first = call.Get<StringRangeExtended*>(1);
    StringRangeExtended* second = call.Get<StringRangeExtended*>(2);
    StringRangeExtended* third = call.Get<StringRangeExtended*>(3);
    StringRangeExtended* fourth = call.Get<StringRangeExtended*>(4);
    if(separator == nullptr || first == nullptr || second == nullptr || third == nullptr || fourth == nullptr)
    {
      call.GetState()->ThrowNullReferenceException(report);
      return;
    }

    String result = Zero::String::Join(separator->mRange, first->mRange, second->mRange, third->mRange, fourth->mRange);
    call.Set(Call::Return, &result);
  }

  //***************************************************************************
  void StringSplit(Call& call, ExceptionReport& report)
  {
    String& self = call.Get<String&>(Call::This);
    StringRangeExtended* separatorStr = call.Get<StringRangeExtended*>(0);

    //if(ValidateRange(self->mOriginalStringReference, self->mRange) == false)
    //  return;

    StringRangeExtended::SetResultStringSplitRange(call, report, self, self.Split(separatorStr->mRange));
  }

  //***************************************************************************
  void StringRangeIsNullOrEmpty(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* separator = call.Get<StringRangeExtended*>(0);
    bool result = true;
    if(separator != nullptr)
      result = separator->mRange.Empty();
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  void StringRangeIsNullOrWhitespace(Call& call, ExceptionReport& report)
  {
    StringRangeExtended* separator = call.Get<StringRangeExtended*>(0);
    bool result = true;
    if(separator != nullptr)
      result = separator->mRange.IsAllWhitespace();
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  class FormatCState;
  class FormatCEdge
  {
  public:

    // The characters we test against
    HashSet<int> Characters;

    // The name of this state, just for debugging
    cstr Name;

    // The state we want to transition to
    FormatCState* TransitionTo;

    // Create an edge that looks for any of the given characters
    FormatCEdge(cstr name, size_t charcterCount, ...) :
      Name(name),
      TransitionTo(nullptr)
    {
      va_list args;
      va_start(args, charcterCount);

      for (size_t i = 0; i < charcterCount; ++i)
      {
        int character = va_arg(args, int);
        this->Characters.InsertOrError(character, "Two characters mapped to the same edge");
      }

      va_end(args);
    }
      
    // Check an edge for a transition on a character
    bool CheckEdge(Rune rune)
    {
      return this->Characters.Contains(rune.mValue.value);
    }
  };

  //***************************************************************************
  class FormatCState
  {
  public:

    // All the edges to be evaluated, in order
    Array<FormatCEdge*> Edges;

    // The name of this state, just for debugging
    cstr Name;

    // Create a state with a given number of edges
    FormatCState(cstr name) :
      Name(name)
    {
    }
  };

  //***************************************************************************
  static FormatCState FormatCStateRoot                      ("Root");
  //---------------------------------------------------------------------------
  static FormatCState FormatCStateParsedFlags               ("ParsedFlags");
  //---------------------------------------------------------------------------
  static FormatCState FormatCStateParsedWidthStar           ("ParsedWidthStar");
  static FormatCState FormatCStateParsedWidthNumber         ("ParsedWidthNumber");
  //---------------------------------------------------------------------------
  static FormatCState FormatCStateParsedPrecisionDot        ("ParsedPrecisionDot");
  static FormatCState FormatCStateParsedPrecisionStar       ("ParsedPrecisionStar");
  static FormatCState FormatCStateParsedPrecisionNumber     ("ParsedPrecisionNumber");
  //---------------------------------------------------------------------------
  static FormatCState FormatCStateParsedSpecifier           ("ParsedSpecifier");
  
  //***************************************************************************
  static FormatCEdge  FormatCEdgeFlags                      ("Flags",               5, '-', '+', ' ', '#', '0');
  static FormatCEdge  FormatCEdgeFlagsLoop                  ("FlagsLoop",           5, '-', '+', ' ', '#', '0');
  //---------------------------------------------------------------------------
  static FormatCEdge  FormatCEdgeWidthStar                  ("WidthStar",           1, '*');
  static FormatCEdge  FormatCEdgeWidthNumber                ("WidthNumber",         10, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9');
  static FormatCEdge  FormatCEdgeWidthNumberLoop            ("WidthNumberLoop",     10, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9');
  //---------------------------------------------------------------------------
  static FormatCEdge  FormatCEdgePrecisionDot               ("PrecisionDot",        1, '.');
  static FormatCEdge  FormatCEdgePrecisionStar              ("PrecisionStar",       1, '*');
  static FormatCEdge  FormatCEdgePrecisionNumber            ("PrecisionNumber",     10, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9');
  static FormatCEdge  FormatCEdgePrecisionNumberLoop        ("PrecisionNumberLoop", 10, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9');
  //---------------------------------------------------------------------------
  static FormatCEdge  FormatCEdgeSpecifiers                 ("Specifiers",          18, 'd', 'i', 'u', 'o', 'x', 'X', 'f', 'F', 'e', 'E', 'g', 'G', 'a', 'A', 'c', 's', 'p', '%');

  //***************************************************************************
  static void InitializeFormatCStates()
  {
    // If we never initialized the state machine...
    static bool initialized = false;
    if (initialized == false)
    {
      // One time initialization
      initialized = true;

      /**** States ****/
      // From the root, we can basically go down any of the edges (except the loops and precision star/number, because that requires a .)
      FormatCStateRoot.Edges.PushBack(&FormatCEdgeFlags);
      FormatCStateRoot.Edges.PushBack(&FormatCEdgeWidthStar);
      FormatCStateRoot.Edges.PushBack(&FormatCEdgeWidthNumber);
      FormatCStateRoot.Edges.PushBack(&FormatCEdgePrecisionDot);
      FormatCStateRoot.Edges.PushBack(&FormatCEdgeSpecifiers);

      // Once we've parsed flags, we can parse width, precision, or a specifier
      FormatCStateParsedFlags.Edges.PushBack(&FormatCEdgeWidthStar);
      FormatCStateParsedFlags.Edges.PushBack(&FormatCEdgeWidthNumber);
      FormatCStateParsedFlags.Edges.PushBack(&FormatCEdgePrecisionDot);
      FormatCStateParsedFlags.Edges.PushBack(&FormatCEdgeSpecifiers);

      // The user can technically specify as many flags as they want
      FormatCStateParsedFlags.Edges.PushBack(&FormatCEdgeFlagsLoop);

      // Once we've parsed width, we can parse precision or a specifier (note we have to handle both width states here)
      FormatCStateParsedWidthStar.Edges.PushBack(&FormatCEdgePrecisionDot);
      FormatCStateParsedWidthStar.Edges.PushBack(&FormatCEdgeSpecifiers);
      FormatCStateParsedWidthNumber.Edges.PushBack(&FormatCEdgePrecisionDot);
      FormatCStateParsedWidthNumber.Edges.PushBack(&FormatCEdgeSpecifiers);

      // Make sure the width number edge loops back to itself
      FormatCStateParsedWidthNumber.Edges.PushBack(&FormatCEdgeWidthNumberLoop);

      // The dot state we have to handle specially, because it's not legal to transition to anything except the precision star/number
      FormatCStateParsedPrecisionDot.Edges.PushBack(&FormatCEdgePrecisionStar);
      FormatCStateParsedPrecisionDot.Edges.PushBack(&FormatCEdgePrecisionNumber);

      // Once we've parsed precision, we can only parse a specifier (note we have to handle both precision states here, but not dot!)
      FormatCStateParsedPrecisionStar.Edges.PushBack(&FormatCEdgeSpecifiers);
      FormatCStateParsedPrecisionNumber.Edges.PushBack(&FormatCEdgeSpecifiers);

      // Make sure the precision number edge loops back to itself
      FormatCStateParsedPrecisionNumber.Edges.PushBack(&FormatCEdgePrecisionNumberLoop);

      /**** Edges ****/
      FormatCEdgeFlags.TransitionTo               = &FormatCStateParsedFlags;
      FormatCEdgeFlagsLoop.TransitionTo           = &FormatCStateParsedFlags;
      FormatCEdgeWidthStar.TransitionTo           = &FormatCStateParsedWidthStar;
      FormatCEdgeWidthNumber.TransitionTo         = &FormatCStateParsedWidthNumber;
      FormatCEdgeWidthNumberLoop.TransitionTo     = &FormatCStateParsedWidthNumber;
      FormatCEdgePrecisionDot.TransitionTo        = &FormatCStateParsedPrecisionDot;
      FormatCEdgePrecisionStar.TransitionTo       = &FormatCStateParsedPrecisionStar;
      FormatCEdgePrecisionNumber.TransitionTo     = &FormatCStateParsedPrecisionNumber;
      FormatCEdgePrecisionNumberLoop.TransitionTo = &FormatCStateParsedPrecisionNumber;
      FormatCEdgeSpecifiers.TransitionTo          = &FormatCStateParsedSpecifier;
    }
  }

  //***************************************************************************
  namespace FormatCInputType
  {
    enum Enum
    {
      DoubleInteger,
      DoubleReal,
      String,
      Pointer
    };
  }

  //***************************************************************************
  template <typename T>
  int FormatCSprintf(char* buffer, cstr format, int widthStar, int precisionStar, T value)
  {
    // If no buffer was provided, it means we're just counting the length
    if (buffer == nullptr)
    {
      // We have to determine the correct arguments to pass in based on whether the * was used for with or precision
      if (widthStar >= 0 && precisionStar >= 0)
        return ZeroSPrintfCount(format, widthStar, precisionStar, value);
      else if (widthStar >= 0)
        return ZeroSPrintfCount(format, widthStar, value);
      else if (precisionStar >= 0)
        return ZeroSPrintfCount(format, widthStar, value);
      else
        return ZeroSPrintfCount(format, value);
    }
    else
    {
      // We have to determine the correct arguments to pass in based on whether the * was used for with or precision
      if (widthStar >= 0 && precisionStar >= 0)
        return sprintf(buffer, format, widthStar, precisionStar, value);
      else if (widthStar >= 0)
        return sprintf(buffer, format, widthStar, value);
      else if (precisionStar >= 0)
        return sprintf(buffer, format, precisionStar, value);
      else
        return sprintf(buffer, format, value);
    }
  }
  
  //***************************************************************************
  template <typename T>
  void FormatCPrint(ExecutableState* state, ExceptionReport& report, StringBuilder& builder, StringParam format, Array<char>& temporaryBuffer, int widthStar, int precisionStar, const Any& argument, T value)
  {
    // Grab the stored type of the argument, for error printing
    Type* argumentType = argument.StoredType;

    // First, get the length that the printf would output
    int sprintfLength = FormatCSprintf(nullptr, format.c_str(), widthStar, precisionStar, value);

    // If the print failed, let the user know
    if (sprintfLength < 0)
    {
      state->ThrowException(report, String::Format("The format specifier '%s' was invalid. The argument was '%s' of type '%s'", format.c_str(), argument.ToString().c_str(), argumentType->ToString().c_str()));
      return;
    }

    // This must be less than or equal to, because we need extra space for the null
    if (temporaryBuffer.Size() <= (size_t)sprintfLength)
    {
      // Expand it just a bit to prevent a lot of re-allocating (and +1 just for the null)
      temporaryBuffer.Resize(sprintfLength * 2 + 1);

      // Clear the memory just in case for some reason sprintf fails
      memset(temporaryBuffer.Data(), 0, temporaryBuffer.Size());
    }

    // Now that we've sized our buffer to fit the text, actually print out to the buffer
    sprintfLength = FormatCSprintf(temporaryBuffer.Data(), format.c_str(), widthStar, precisionStar, value);

    // This should basically never happen... but just in case it does!
    if (sprintfLength < 0)
    {
      state->ThrowException(report, String::Format("The format specifier '%s' was invalid. The argument was '%s' of type '%s'", format.c_str(), argument.ToString().c_str(), argumentType->ToString().c_str()));
      return;
    }

    // Finally, Append the formatted text to the builder
    builder.Append(temporaryBuffer.Data());
  }

  //***************************************************************************
  void StringFormatC(Call& call, ExceptionReport& report)
  {
    // Make sure all the edges and states are setup for our DFA
    InitializeFormatCStates();

    // How many arguments this overload is going to read off the stack (from 1 to 10)
    size_t argumentCount = (size_t)call.GetFunction()->UserData;

    // The format string (as used with sprintf)
    StringParam format = call.Get<StringParam>(0);

    // Create a stack array to hold all the arguments we read off the Zilch stack
    // The 'Any' type can hold any value from Zilch, including primitives and classes
    const Any** arguments = (const Any**)alloca(sizeof(const Any*) * argumentCount);

    // Read each argument off the stack
    for (size_t i = 0; i < argumentCount; ++i)
    {
      // Put each argument into the static array we created
      // The +1 is for the format argument at the beginning
      const Any* argument = call.Get<const Any*>(i + 1);
      arguments[i] = argument;
    }

    // Get the current state (for throwing exceptions)
    ExecutableState* state = call.GetState();

    // What we'll use to concatenate all the strings together
    StringBuilder builder;

    // The index we use when reading in format specifiers
    size_t argumentIndex = 0;

    // A temporary buffer where we print out each argumnent / format specifier to
    Array<char> temporaryBuffer;

    // Loop through all characters in the format string
    StringIterator it = format.Begin();
    StringIterator end = format.End();
    for (; it < end; ++it)
    {
      // Grab the current character
      Rune r = *it;

      // If this is a format specifier...
      if (r.mValue == '%')
      {
        // Format specifier syntax: (we do not support length)
        // %[flags][width][.precision]specifier

        // We use a state machine / DFA to parse the format specifiers
        FormatCState* currentState = &FormatCStateRoot;

        // Save where this format specifier started, right at the % sign
        StringIterator formatSpecifierStart = it;

        // Because we already read the '%', we want to advance to the first character of the specifier
        ++it;

        // Loop until we read the rest of the specifier, or the end of the string (null terminates the DFA!)
        for (; it < end; ++it)
        {
          // Read the current character
          r = *it;

          // This will tell us if, after testing all the edges, we transitioned to a new state
          // Note: Not transitioning is considered an error
          bool transitioned = false;

          // Loop through all the current state's outgoing edges
          for (size_t e = 0; e < currentState->Edges.Size(); ++e)
          {
            // Grab the current edge
            FormatCEdge* edge = currentState->Edges[e];

            // Check to see if this edge would transition on the character
            if (edge->CheckEdge(r))
            {
              // We transitioned, move to the next state
              currentState = edge->TransitionTo;
              transitioned = true;
              break;
            }
          }

          // Create a range that represents the format specifier up to this point (the +1 is because i is actually at the current character)
          //  StringRange formatSpecifierRange(format.c_str() + formatSpecifierStart, format.c_str() + (i - formatSpecifierStart + 1));
          StringRange formatSpecifierRange(formatSpecifierStart, it + 1);
          // If we didn't transition, that means we found an invalid format specifier
          if (transitioned == false)
          {
            state->ThrowException(report, BuildString("Invalid or unsupported format specifier: '", formatSpecifierRange, "'"));
            return;
          }
          // If we reached the end (parsed the format specifier)
          else if (currentState == &FormatCStateParsedSpecifier)
          {
            // We finished parsing the format specifier, time to try it out!
//             String formatSpecifierString = formatSpecifierRange;
            
            // Get the format specifier character
            Rune formatSpecifier = formatSpecifierRange.Back();

            // Sting is the safest fallback, just in case something happens
            FormatCInputType::Enum inputType = FormatCInputType::String;

            // Check for the integral, float, and pointer types
            switch (formatSpecifier.mValue.value)
            {
              // Printf integer format specifiers
              case 'd':
              case 'i':
              case 'o':
              case 'u':
              case 'x':
              case 'X':
                inputType = FormatCInputType::DoubleInteger;
                break;
                
              // Printf float format specifiers
              case 'f':
              case 'F':
              case 'e':
              case 'E':
              case 'g':
              case 'G':
              case 'a':
              case 'A':
                inputType = FormatCInputType::DoubleReal;
                break;
                
              // Printf pointer format specifier
              case 'p':
                inputType = FormatCInputType::Pointer;
                break;
            }

            // Usign a * in printf can add extra integer arguments to the va-list
            Integer widthStar = -1;
            Integer precisionStar = -1;
            
            // Count the number of extra integral arguments we need
            StringIterator formatIt = formatSpecifierRange.Begin();
            StringIterator formatEnd = formatSpecifierRange.End();
            for (; formatIt < formatEnd; ++formatIt)
            {
              // If we hit the * operator...
              if (*formatIt == '*')
              {
                // So long as we haven't run out of arguments...
                if (argumentIndex >= argumentCount)
                {
                  state->ThrowException(report, "There are more '%' format specifiers or '*' operators then there are arguments");
                  return;
                }

                // This is always valid to check the previous character because we know format specifiers must start with %
                // If the previous character was a '.', then we know this is a 'precision' star
                bool isPrecision = (*(formatIt - 1) == '.');

                // Grab the current integer argument (for either width or precision)
                const Any& argument = *arguments[argumentIndex];
                Type* argumentType = argument.StoredType;
                const byte* argumentData = argument.GetData();
                ++argumentIndex;

                // Make sure the type was an integer
                if (Type::IsSame(argumentType, ZilchTypeId(Integer)) == false)
                {
                  // It wasn't an integer, inform the user of which * was wrong
                  cstr starType = "width";
                  if (isPrecision)
                    starType = "precision";
                  
                  // Let the user know about the * requiring an integer argument, including as much info as possible
                  state->ThrowException
                  (
                    report,
                    String::Format("When using * for %s an Integer must be provided before the argument, instead we found '%s' of type '%s' for the '%s' format specifier",
                    starType,
                    argument.ToString().c_str(),
                    argumentType->ToString().c_str(),
                    formatSpecifierRange.Data())
                  );
                  return;
                }

                // Finally, if this is a precision or width star get the actual integral value...
                if (isPrecision)
                  precisionStar = *(Integer*)argumentData;
                else
                  widthStar = *(Integer*)argumentData;
              }
            }
            
            // After we may have read any * arguments, we need to check if the current format specifier actually has a valid argument (too few?)
            if (argumentIndex >= argumentCount)
            {
              state->ThrowException(report, "There are more '%' format specifiers or '*' operators then there are arguments");
              return;
            }

            // Grab the current argument for the format specifier
            const Any& argument = *arguments[argumentIndex];
            Type* argumentType = argument.StoredType;
            const byte* argumentData = argument.GetData();

            // We're using a format specifier that accepts a DoubleInteger
            if (inputType == FormatCInputType::DoubleInteger)
            {
              // If it's an integer, we always just print out long longs
              // We basically add the 'll' format specifier before the last character, which is the format specifier that told us it was an integer
              StringIterator lastrune = formatSpecifierRange.End() - 1;
              String integralFormatSpecifierString = BuildString
              (
                StringRange(formatSpecifierRange.Begin(), lastrune),
                "ll",
                StringRange(lastrune, formatSpecifierRange.End())
              );

              // Convert as many types as we can to the DoubleInteger value
              DoubleInteger value = 0;

              // Look for Integer, DoubleInteger, Real, and DoubleReal
              if (Type::IsSame(argumentType, ZilchTypeId(Byte)))
                value = (DoubleInteger)*(Byte*)argumentData;
              else if (Type::IsSame(argumentType, ZilchTypeId(Integer)))
                value = (DoubleInteger)*(Integer*)argumentData;
              else if (Type::IsSame(argumentType, ZilchTypeId(DoubleInteger)))
                value = (DoubleInteger)*(DoubleInteger*)argumentData;
              else if (Type::IsSame(argumentType, ZilchTypeId(Real)))
                value = (DoubleInteger)*(Real*)argumentData;
              else if (Type::IsSame(argumentType, ZilchTypeId(DoubleReal)))
                value = (DoubleInteger)*(DoubleReal*)argumentData;
              else
                // Anything else must be an error since we don't know what it is
                return state->ThrowException(report, String::Format("The format specifier '%s' requires an integral or real type, instead we got '%s' of type '%s'", formatSpecifierRange.Data(), argument.ToString().c_str(), argumentType->ToString().c_str()));

              // Print the value out and store it in the builder (this can throw an exception)
              FormatCPrint(state, report, builder, integralFormatSpecifierString, temporaryBuffer, widthStar, precisionStar, argument, value);
              if (report.HasThrownExceptions())
                return;
            }
            // We're using a format specifier that accepts a DoubleReal
            else if (inputType == FormatCInputType::DoubleReal)
            {
              // Convert as many types as we can to the DoubleReal value
              DoubleReal value = 0.0;

              // Look for Integer, DoubleInteger, Real, and DoubleReal
              if (Type::IsSame(argumentType, ZilchTypeId(Byte)))
                value = (DoubleReal)*(Byte*)argumentData;
              else if (Type::IsSame(argumentType, ZilchTypeId(Integer)))
                value = (DoubleReal)*(Integer*)argumentData;
              else if (Type::IsSame(argumentType, ZilchTypeId(DoubleInteger)))
                value = (DoubleReal)*(DoubleInteger*)argumentData;
              else if (Type::IsSame(argumentType, ZilchTypeId(Real)))
                value = (DoubleReal)*(Real*)argumentData;
              else if (Type::IsSame(argumentType, ZilchTypeId(DoubleReal)))
                value = (DoubleReal)*(DoubleReal*)argumentData;
              else
                // Anything else must be an error since we don't know what it is
                return state->ThrowException(report, String::Format("The format specifier '%s' requires an integral or real type, instead we got '%s' of type '%s'", formatSpecifierRange.Data(), argument.ToString().c_str(), argumentType->ToString().c_str()));
              
              // Print the value out and store it in the builder (this can throw an exception)
              FormatCPrint(state, report, builder, formatSpecifierRange, temporaryBuffer, widthStar, precisionStar, argument, value);
              if (report.HasThrownExceptions())
                return;
            }
            // When expecting a string, we actually just allow the user to print anything (%s is our one size fits all)
            // String's 'ToString' just results in itself
            else if (inputType == FormatCInputType::String)
            {
              // Print the value out and store it in the builder (this can throw an exception)
              FormatCPrint(state, report, builder, formatSpecifierRange, temporaryBuffer, widthStar, precisionStar, argument, argument.ToString().c_str());
              if (report.HasThrownExceptions())
                return;
            }
            // The pointer format specifier just prints out special pointer values
            else if (inputType == FormatCInputType::Pointer)
            {
              // The meaning of the pointer is not entirely relevant, but can be used for debugging
              void* pointerValue = nullptr;
              bool isValidValue = false;

              // If the argument is a handle type (bound type as a class/reference type, or an indirect / ref struct)
              if (Type::IsHandleType(argumentType))
              {
                // Dereference the handle to get the actual value we point at
                pointerValue = ((Handle*)argumentData)->Dereference();
                isValidValue = true;
              }
              // If the argument is a delegate, we can at least give the 'Function' pointer
              else if (Type::IsDelegateType(argumentType))
              {
                // Just tell the user the location of the function
                pointerValue = ((Delegate*)argumentData)->BoundFunction;
                isValidValue = true;
              }

              // If we didn't detect the type of value, throw an exception
              if (isValidValue == false)
              {
                state->ThrowException(report, String::Format("The format specifier '%s' requires a class, ref struct, or delegate type. Instead we got '%s' of type '%s'", formatSpecifierRange.Data(), argument.ToString().c_str(), argumentType->ToString().c_str()));
                return;
              }
              
              // Print the value out and store it in the builder (this can throw an exception)
              FormatCPrint(state, report, builder, formatSpecifierRange, temporaryBuffer, widthStar, precisionStar, argument, pointerValue);
              if (report.HasThrownExceptions())
                return;
            }

            // We read and processed the format specifier, no need to continue
            // This will break to the outer loop where we keep reading the format string
            break;
          }
        }

        // We parsed a format specifier, so move to the next argument
        ++argumentIndex;
      }
      else
      {
        // Nothing special, just Append the character directly from the format
        builder.Append(r.mValue);
      }
    }

    // If we didn't consume all the arguments...
    if (argumentIndex != argumentCount)
    {
      state->ThrowException(report, "There are more arguments then there are '%' format specifiers or '*' operators");
      return;
    }

    // Return the string we built to the Zilch stack
    String result = builder.ToString();
    call.Set(Call::Return, result);
  }

  //***************************************************************************
  String BooleanToString(Boolean value)
  {
    // Basically because we only have two outcomes, just pre-create the strings
    static const String True("true");
    static const String False("false");

    if (value)
    {
      return True;
    }
    else
    {
      return False;
    }
  }

  //***************************************************************************
  cstr TrueFalseCstr(Boolean value)
  {
    return value ? "true" : "false";
  }

  //***************************************************************************
  String Boolean2ToString(Boolean2Param value)
  {
    return String::Format("(%s, %s)", TrueFalseCstr(value.x), TrueFalseCstr(value.y));
  }

  //***************************************************************************
  String Boolean3ToString(Boolean3Param value)
  {
    return String::Format("(%s, %s, %s)", TrueFalseCstr(value.x), TrueFalseCstr(value.y), TrueFalseCstr(value.z));
  }

  //***************************************************************************
  String Boolean4ToString(Boolean4Param value)
  {
    return String::Format("(%s, %s, %s, %s)", TrueFalseCstr(value.x), TrueFalseCstr(value.y), TrueFalseCstr(value.z), TrueFalseCstr(value.w));
  }

  //***************************************************************************
  String ByteToString(Byte value)
  {
    return String::Format("%d", value);
  }

  //***************************************************************************
  String IntegerToString(Integer value)
  {
    return String::Format("%d", value);
  }

  //***************************************************************************
  String Integer2ToString(Integer2Param value)
  {
    return String::Format("(%d, %d)", value.x, value.y);
  }

  //***************************************************************************
  String Integer3ToString(Integer3Param value)
  {
    return String::Format("(%d, %d, %d)", value.x, value.y, value.z);
  }

  //***************************************************************************
  String Integer4ToString(Integer4Param value)
  {
    return String::Format("(%d, %d, %d, %d)", value.x, value.y, value.z, value.w);
  }

  //***************************************************************************
  String RealToString(Real value)
  {
    return String::Format("%g", value);
  }

  //***************************************************************************
  String Real2ToString(Real2Param value)
  {
    return String::Format("(%g, %g)", value.x, value.y);
  }

  //***************************************************************************
  String Real3ToString(Real3Param value)
  {
    return String::Format("(%g, %g, %g)", value.x, value.y, value.z);
  }

  //***************************************************************************
  String Real4ToString(Real4Param value)
  {
    return String::Format("(%g, %g, %g, %g)", value.x, value.y, value.z, value.w);
  }

  //***************************************************************************
  String QuaternionToString(QuaternionParam value)
  {
    // First, convert the quaternion to Euler Angles (simple to read/understand)
    EulerAngles angles(Math::EulerOrders::XYZs);
    ToEulerAngles(value, &angles);

    // Also convert the quaternion to Axis/Angle form, as it may be more intuitive
    Real3 axis;
    Real angle;
    ToAxisAngle(value, &axis, &angle);

    // Print out the quaternion, then the Euler Angles form, then the Axis/Angle form
    return String::Format
    (
      "(%g, %g, %g, %g), Euler: (%g, %g, %g), Axis: (%g, %g, %g), Angle: %g",
      value.x, value.y, value.z, value.w,
      angles.Angles.x, angles.Angles.y, angles.Angles.z,
      axis.x, axis.y, axis.z,
      angle
    );
  }

  //***************************************************************************
  String DoubleIntegerToString(DoubleInteger value)
  {
    return String::Format("%lldd", value);
  }

  //***************************************************************************
  String DoubleRealToString(DoubleReal value)
  {
    return String::Format("%gd", value);
  }

  //***************************************************************************
  String StringToString(const BoundType* type, const byte* data)
  {
    return *(String*)data;
  }

  //***************************************************************************
  String ByteToString(const BoundType* type, const byte* data)
  {
    return ByteToString(*(Byte*)data);
  }

  //***************************************************************************
  String BooleanToString(const BoundType* type, const byte* data)
  {
    return BooleanToString(*(Boolean*)data);
  }

  //***************************************************************************
  String Boolean2ToString(const BoundType* type, const byte* data)
  {
    return Boolean2ToString(*(Boolean2*)data);
  }

  //***************************************************************************
  String Boolean3ToString(const BoundType* type, const byte* data)
  {
    return Boolean3ToString(*(Boolean3*)data);
  }

  //***************************************************************************
  String Boolean4ToString(const BoundType* type, const byte* data)
  {
    return Boolean4ToString(*(Boolean4*)data);
  }

  //***************************************************************************
  String IntegerToString(const BoundType* type, const byte* data)
  {
    return IntegerToString(*(Integer*)data);
  }

  //***************************************************************************
  String Integer2ToString(const BoundType* type, const byte* data)
  {
    return Integer2ToString(*(Integer2*)data);
  }

  //***************************************************************************
  String Integer3ToString(const BoundType* type, const byte* data)
  {
    return Integer3ToString(*(Integer3*)data);
  }

  //***************************************************************************
  String Integer4ToString(const BoundType* type, const byte* data)
  {
    return Integer4ToString(*(Integer4*)data);
  }

  //***************************************************************************
  String RealToString(const BoundType* type, const byte* data)
  {
    return RealToString(*(Real*)data);
  }

  //***************************************************************************
  String Real2ToString(const BoundType* type, const byte* data)
  {
    return Real2ToString(*(Real2*)data);
  }

  //***************************************************************************
  String Real3ToString(const BoundType* type, const byte* data)
  {
    return Real3ToString(*(Real3*)data);
  }

  //***************************************************************************
  String Real4ToString(const BoundType* type, const byte* data)
  {
    return Real4ToString(*(Real4*)data);
  }

  //***************************************************************************
  String QuaternionToString(const BoundType* type, const byte* data)
  {
    return QuaternionToString(*(Quaternion*)data);
  }

  //***************************************************************************
  String DoubleIntegerToString(const BoundType* type, const byte* data)
  {
    return DoubleIntegerToString(*(DoubleInteger*)data);
  }

  //***************************************************************************
  String DoubleRealToString(const BoundType* type, const byte* data)
  {
    return DoubleRealToString(*(DoubleReal*)data);
  }

  //***************************************************************************
  String VectorToString(Real* vector, size_t vectorSize)
  {
    if (vectorSize == 1)
      return RealToString(*(Real*)vector);
    else if (vectorSize == 2)
      return Real2ToString(*(Real2*)vector);
    else if (vectorSize == 3)
      return Real3ToString(*(Real3*)vector);
    else if (vectorSize == 4)
      return Real4ToString(*(Real4*)vector);

    Error("Somehow we tried to turn a vector into a string that has a size greater than 4");
    return String();
  }

  //***************************************************************************
  void RealApproximatelyEqual(Call& call, ExceptionReport& report)
  {
    call.DisableReturnChecks();
    size_t count = (size_t)call.GetFunction()->UserData;

    Real* lhs = (Real*)call.GetParameterUnchecked(0);
    Real* rhs = (Real*)call.GetParameterUnchecked(1);
    Real epsilon = call.Get<Real>(2);
    Boolean* result = (Boolean*)call.GetReturnUnchecked();

    // Check each element
    for(size_t i = 0; i < count; ++i)
      result[i] = Math::Abs(lhs[i] - rhs[i]) <= epsilon;
  }

  //***************************************************************************
  Integer ZilchParseInteger(StringRangeExtended range)
  {
    Integer value;
    Zero::ToValue(range.mRange, value);
    return value;
  }

  //***************************************************************************
  Real ZilchParseReal(StringRangeExtended range)
  {
    Real value;
    ToValue(range.mRange, value);
    return value;
  }

  //***************************************************************************
  Real ZilchRealPositiveMax()
  {
    return Math::PositiveMax();
  }

  //***************************************************************************
  Real ZilchRealPositiveValueClosestToZero()
  {
    return Math::PositiveMin();
  }

  //***************************************************************************
  Real ZilchRealNegativeValueClosestToZero()
  {
    return -Math::PositiveMin();
  }

  //***************************************************************************
  Real ZilchRealNegativeMin()
  {
    return -Math::PositiveMax();
  }

  //***************************************************************************
  Integer ZilchIntegerPositiveMax()
  {
    return Math::IntegerPositiveMax();
  }

  //***************************************************************************
  Integer ZilchIntegerPositiveValueClosestToZero()
  {
    return 1;
  }

  //***************************************************************************
  Integer ZilchIntegerNegativeValueClosestToZero()
  {
    return -1;
  }

  //***************************************************************************
  Integer ZilchIntegerNegativeMin()
  {
    return Math::IntegerNegativeMin();
  }

  //***************************************************************************
  // User data for vector (Real2, Integer2, etc...) types to implement generic functionality.
  class VectorUserData
  {
  public:
    VectorUserData(size_t count, size_t elementTypeIndex)
    {
      Count = count;
      ElementTypeIndex = elementTypeIndex;
    }

    size_t Count;
    size_t ElementTypeIndex;
  };

  //***************************************************************************
  void VectorCount(Call& call, ExceptionReport& report)
  {
    // The dimension of the vector was stored as the user data
    size_t dimension = *(size_t*)&(call.GetFunction()->UserData);
    call.Set(Call::Return, dimension);
  }

  //***************************************************************************
  void SetVectorAxis(byte* returnData, VectorUserData& userData, size_t axis, size_t typeIndex)
  {
    // Get the element type of the vector (Real, Integer, etc...)
    Core& core = Core::GetInstance();
    BoundType* elementType = core.VectorScalarBoundTypes[userData.ElementTypeIndex];

    size_t elementSize = elementType->Size;
    for(size_t i = 0; i < userData.Count; ++i)
    {
      byte* data = returnData + i * elementSize;

      // If this is the specified axis then call the appropriate function to set the value to
      // 1 for the current element type (1.0f for floats, 1 for ints, etc...)
      if(i == axis)
        core.ScalarTypeOneFunctions[userData.ElementTypeIndex](data);
      //otherwise just memset to zero
      else
        memset(data, 0, elementSize);
    }
  }

  //***************************************************************************
  void VectorGetAxis(Call& call, ExceptionReport& report)
  {
    call.DisableReturnChecks();

    // Get the user data for the vector and what axis to operate on
    VectorUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<VectorUserData>(0);
    Integer axis = call.Get<Integer>(0);

    // Set the return data to be a vector of all zero except for the specified axis
    byte* returnData = call.GetReturnUnchecked();
    SetVectorAxis(returnData, userData, axis, userData.ElementTypeIndex);
  }

  //***************************************************************************
  void VectorAxisFunction(Call& call, ExceptionReport& report)
  {
    call.DisableReturnChecks();

    // Get the user data for the vector and what axis to operate on (from the regular user data)
    VectorUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<VectorUserData>(0);
    size_t axis = (size_t)(call.GetFunction()->UserData);

    // Set the return data to be a vector of all zero except for the specified axis
    byte* returnData = call.GetReturnUnchecked();
    SetVectorAxis(returnData, userData, axis, userData.ElementTypeIndex);
  }

  //***************************************************************************
  void VectorZeroFunction(Call& call, ExceptionReport& report)
  {
    call.DisableReturnChecks();

    // Get the user data for the function
    VectorUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<VectorUserData>(0);

    // Get the element type of the vector (Real, Integer, etc...)
    Core& core = Core::GetInstance();
    BoundType* elementType = core.VectorScalarBoundTypes[userData.ElementTypeIndex];

    // Set the entire array to zero (based upon the size of the element type and the number of elements)
    byte* returnData = call.GetReturnUnchecked();
    memset(returnData, 0, elementType->Size * userData.Count);
  }

  //***************************************************************************
  // Splats a function of the type Scalar Fn(Scalar) to
  // Vec(n) Fn(Vec(n)) (vector could be of whatever scalar type is)
  template <size_t Components, typename ScalarType, ScalarType (*Function)(ScalarType)>
  void SplatVecToVec(Call& call, ExceptionReport& report)
  {
    // Get a pointer to our input vector
    ScalarType* input = (ScalarType*)call.GetParameterUnchecked(0);

    // Get a pointer to our return/output vector
    ScalarType* result = (ScalarType*)call.GetReturnUnchecked();
    call.MarkReturnAsSet();

    // Loop through all the components to apply the given function (splat)
    for (size_t i = 0; i < Components; ++i)
    {
      // Call the function on the single input (splat)
      result[i] = Function(input[i]);
    }
  }

  //***************************************************************************
  // Splats a function of the type Scalar Fn(Scalar) to
  // Vec(n) Fn(Vec(n), a) (vector could be of whatever scalar type is)
  template <size_t Components, typename ScalarType, typename ExtraA, ScalarType (*Function)(ScalarType, ExtraA)>
  void SplatVecToVecOneExtra(Call& call, ExceptionReport& report)
  {
    // Get a pointer to our input vector
    ScalarType* input = (ScalarType*)call.GetParameterUnchecked(0);

    // Grab the extra parameter
    ExtraA a = call.Get<ExtraA>(1);

    // Get a pointer to our return/output vector
    ScalarType* result = (ScalarType*)call.GetReturnUnchecked();
    call.MarkReturnAsSet();

    // Loop through all the components to apply the given function (splat)
    for (size_t i = 0; i < Components; ++i)
    {
      // Call the function on the single input (splat)
      result[i] = Function(input[i], a);
    }
  }

  //***************************************************************************
  // Splats a function of the type Scalar Fn(Scalar) to
  // Vec(n) Fn(Vec(n), a, b) (vector could be of whatever scalar type is)
  template <size_t Components, typename ScalarType, typename ExtraA, typename ExtraB, ScalarType (*Function)(ScalarType, ExtraA, ExtraB)>
  void SplatVecToVecTwoExtra(Call& call, ExceptionReport& report)
  {
    // Get a pointer to our input vector
    ScalarType* input = (ScalarType*)call.GetParameterUnchecked(0);

    // Grab the extra two parameters
    ExtraA a = call.Get<ExtraA>(1);
    ExtraB b = call.Get<ExtraB>(2);

    // Get a pointer to our return/output vector
    ScalarType* result = (ScalarType*)call.GetReturnUnchecked();
    call.MarkReturnAsSet();

    // Loop through all the components to apply the given function (splat)
    for (size_t i = 0; i < Components; ++i)
    {
      // Call the function on the single input (splat)
      result[i] = Function(input[i], a, b);
    }
  }

  //***************************************************************************
  // Splats a function of the type Boolean Fn(Scalar, Scalar&) to
  // Vec(n) Fn(Vec(n)). The function being splatted is a Scalar Fn(Scalar)
  // function that has been altered to return if it failed so an exception
  // can be thrown.
  template <size_t Components, typename ScalarType, bool (*Function)(ScalarType, ScalarType&)>
  void SplatVecToVecWithError(Call& call, ExceptionReport& report)
  {
    // Get a pointer to our input vector
    ScalarType* input = (ScalarType*)call.GetParameterUnchecked(0);

    // Get a pointer to our return/output vector
    ScalarType* result = (ScalarType*)call.GetReturnUnchecked();
    call.MarkReturnAsSet();

    bool success = true;
    // Loop through all the components to apply the given function (splat)
    for (size_t i = 0; i < Components; ++i)
    {
      // Call the function on the single input (splat)
      success = Function(input[i], result[i]);

      // If the function failed for whatever reason then throw an exception
      if (success == false)
      {
        // The user data of this function should contain a format string with
        // one %s in it to display the input value that failed.
        cstr errFormat = (cstr)call.GetFunction()->UserData;
        String vectorValue = VectorToString(input, Components);
        String msg = String::Format(errFormat, vectorValue.c_str());
        call.GetState()->ThrowException(report, msg);
        return;
      }
    }
  }

  //***************************************************************************
  // Splats a function of the type Scalar Fn(Scalar, Scalar) to
  // Vec(n) Fn(Vec(n), Vec(n)) (vector could be of whatever scalar type is)
  template <size_t Components, typename ScalarType, ScalarType (*Function)(ScalarType, ScalarType)>
  void SplatTwoVecToVec(Call& call, ExceptionReport& report)
  {
    // Get a pointer to our input vector
    ScalarType* input0 = (ScalarType*)call.GetParameterUnchecked(0);
    ScalarType* input1 = (ScalarType*)call.GetParameterUnchecked(1);

    // Get a pointer to our return/output vector
    ScalarType* result = (ScalarType*)call.GetReturnUnchecked();
    call.DisableReturnChecks();
    
    // Loop through all the components to apply the given function (splat)
    for (size_t i = 0; i < Components; ++i)
    {
      // Call the function on the single inputs (splat)
      result[i] = Function(input0[i], input1[i]);
    }
  }

  //***************************************************************************
  //Splats a function of the type Scalar Fn(Scalar, Scalar, Scalar) to
  //Vec(n) Fn(Vec(n), Vec(n), Vec(n)) (vector could be of whatever scalar type is)
  template <size_t Components, typename ScalarType, ScalarType (*Function)(ScalarType, ScalarType, ScalarType)>
  void SplatThreeVecToVec(Call& call, ExceptionReport& report)
  {
    // Get a pointer to our input vector
    ScalarType* input0 = (ScalarType*)call.GetParameterUnchecked(0);
    ScalarType* input1 = (ScalarType*)call.GetParameterUnchecked(1);
    ScalarType* input2 = (ScalarType*)call.GetParameterUnchecked(2);

    // Get a pointer to our return/output vector
    ScalarType* result = (ScalarType*)call.GetReturnUnchecked();
    call.DisableReturnChecks();
    
    // Loop through all the components to apply the given function (splat)
    for (size_t i = 0; i < Components; ++i)
    {
      // Call the function on the single inputs (splat)
      result[i] = Function(input0[i], input1[i], input2[i]);
    }
  }

  //***************************************************************************
  //Splats a function of the type Scalar Fn(Scalar, Scalar, Real) to
  //Vec(n) Fn(Vec(n), Vec(n), Real) (vector could be of whatever scalar type is)
  template <size_t Components, typename ScalarType, ScalarType (*Function)(ScalarType, ScalarType, Real)>
  void SplatTwoVecAndRealToVec(Call& call, ExceptionReport& report)
  {
    // Get pointers to the two vectors
    ScalarType* input0 = (ScalarType*)call.GetParameterUnchecked(0);
    ScalarType* input1 = (ScalarType*)call.GetParameterUnchecked(1);
    Real inputReal = call.Get<Real>(2);

    // Get a pointer to our return/output vector
    ScalarType* result = (ScalarType*)call.GetReturnUnchecked();
    call.MarkReturnAsSet();
    
    // Loop through all the components to apply the given function (splat)
    for (size_t i = 0; i < Components; ++i)
    {
      // Call the function on the single inputs (splat)
      result[i] = Function(input0[i], input1[i], inputReal);
    }
  }

#define ZilchSplatAllVectorOperationsAs(ZilchBuilder, ZilchType, NamespaceAndClass, type, Method, Name, UserDescription) \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatVecToVec<1, type, NamespaceAndClass::Method>, OneParameter(this->type##Type ), this->type##Type , FunctionOptions::Static)->Description = ZilchDocumentString(UserDescription); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatVecToVec<2, type, NamespaceAndClass::Method>, OneParameter(this->type##2Type), this->type##2Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatVecToVec<3, type, NamespaceAndClass::Method>, OneParameter(this->type##3Type), this->type##3Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatVecToVec<4, type, NamespaceAndClass::Method>, OneParameter(this->type##4Type), this->type##4Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); 

#define ZilchSplatNamedAllVectorOperationsTwoExtraAs(ZilchBuilder, ZilchType, NamespaceAndClass, type, typeA, typeB, Method, Name, p1, p2, p3, UserDescription) \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatVecToVecTwoExtra<1, type, typeA, typeB, NamespaceAndClass::Method>, ThreeParameters(this->type##Type , p1, ZilchTypeId(typeA), p2, ZilchTypeId(typeB), p3), this->type##Type , FunctionOptions::Static)->Description = ZilchDocumentString(UserDescription); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatVecToVecTwoExtra<2, type, typeA, typeB, NamespaceAndClass::Method>, ThreeParameters(this->type##2Type, p1, ZilchTypeId(typeA), p2, ZilchTypeId(typeB), p3), this->type##2Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatVecToVecTwoExtra<3, type, typeA, typeB, NamespaceAndClass::Method>, ThreeParameters(this->type##3Type, p1, ZilchTypeId(typeA), p2, ZilchTypeId(typeB), p3), this->type##3Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatVecToVecTwoExtra<4, type, typeA, typeB, NamespaceAndClass::Method>, ThreeParameters(this->type##4Type, p1, ZilchTypeId(typeA), p2, ZilchTypeId(typeB), p3), this->type##4Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); 
  
#define ZilchSplatNamedAllVectorOperationsOneExtraAs(ZilchBuilder, ZilchType, NamespaceAndClass, type, typeA, Method, Name, p1, p2, UserDescription) \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatVecToVecOneExtra<1, type, typeA, NamespaceAndClass::Method>, TwoParameters(this->type##Type , p1, ZilchTypeId(typeA), p2), this->type##Type , FunctionOptions::Static)->Description = ZilchDocumentString(UserDescription); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatVecToVecOneExtra<2, type, typeA, NamespaceAndClass::Method>, TwoParameters(this->type##2Type, p1, ZilchTypeId(typeA), p2), this->type##2Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatVecToVecOneExtra<3, type, typeA, NamespaceAndClass::Method>, TwoParameters(this->type##3Type, p1, ZilchTypeId(typeA), p2), this->type##3Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatVecToVecOneExtra<4, type, typeA, NamespaceAndClass::Method>, TwoParameters(this->type##4Type, p1, ZilchTypeId(typeA), p2), this->type##4Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); 

#define ZilchSplatNamedAllVectorOperationsAs(ZilchBuilder, ZilchType, NamespaceAndClass, type, Method, FunctionName, ParamName, UserDescription) \
  ZilchBuilder.AddBoundFunction(ZilchType, FunctionName, SplatVecToVec<1, type, NamespaceAndClass::Method>, OneParameter(this->type##Type , ParamName), this->type##Type,  FunctionOptions::Static)->Description = ZilchDocumentString(UserDescription); \
  ZilchBuilder.AddBoundFunction(ZilchType, FunctionName, SplatVecToVec<2, type, NamespaceAndClass::Method>, OneParameter(this->type##2Type, ParamName), this->type##2Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); \
  ZilchBuilder.AddBoundFunction(ZilchType, FunctionName, SplatVecToVec<3, type, NamespaceAndClass::Method>, OneParameter(this->type##3Type, ParamName), this->type##3Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); \
  ZilchBuilder.AddBoundFunction(ZilchType, FunctionName, SplatVecToVec<4, type, NamespaceAndClass::Method>, OneParameter(this->type##4Type, ParamName), this->type##4Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise."));

#define ZilchSplatAllVectorOperationsWithErrorAs(ZilchBuilder, ZilchType, NamespaceAndClass, type, Method, FunctionName, ErrorFormatString) \
  ZilchBuilder.AddBoundFunction(ZilchType, FunctionName, SplatVecToVecWithError<1, type, NamespaceAndClass::Method>, OneParameter(this->type##Type ),  this->type##Type, FunctionOptions::Static)->UserData = ErrorFormatString; \
  ZilchBuilder.AddBoundFunction(ZilchType, FunctionName, SplatVecToVecWithError<2, type, NamespaceAndClass::Method>, OneParameter(this->type##2Type), this->type##2Type, FunctionOptions::Static)->UserData = ErrorFormatString; \
  ZilchBuilder.AddBoundFunction(ZilchType, FunctionName, SplatVecToVecWithError<3, type, NamespaceAndClass::Method>, OneParameter(this->type##3Type), this->type##3Type, FunctionOptions::Static)->UserData = ErrorFormatString; \
  ZilchBuilder.AddBoundFunction(ZilchType, FunctionName, SplatVecToVecWithError<4, type, NamespaceAndClass::Method>, OneParameter(this->type##4Type), this->type##4Type, FunctionOptions::Static)->UserData = ErrorFormatString;

#define ZilchSplatNamedAllVectorOperationsWithErrorAs(ZilchBuilder, ZilchType, NamespaceAndClass, type, Method, FunctionName, ParamName, ErrorFormatString) \
  ZilchBuilder.AddBoundFunction(ZilchType, FunctionName, SplatVecToVecWithError<1, type, NamespaceAndClass::Method>, OneParameter(this->type##Type , ParamName),  this->type##Type, FunctionOptions::Static)->UserData = ErrorFormatString; \
  ZilchBuilder.AddBoundFunction(ZilchType, FunctionName, SplatVecToVecWithError<2, type, NamespaceAndClass::Method>, OneParameter(this->type##2Type, ParamName), this->type##2Type, FunctionOptions::Static)->UserData = ErrorFormatString; \
  ZilchBuilder.AddBoundFunction(ZilchType, FunctionName, SplatVecToVecWithError<3, type, NamespaceAndClass::Method>, OneParameter(this->type##3Type, ParamName), this->type##3Type, FunctionOptions::Static)->UserData = ErrorFormatString; \
  ZilchBuilder.AddBoundFunction(ZilchType, FunctionName, SplatVecToVecWithError<4, type, NamespaceAndClass::Method>, OneParameter(this->type##4Type, ParamName), this->type##4Type, FunctionOptions::Static)->UserData = ErrorFormatString;

#define ZilchSplatAllTwoVecToVecAs(ZilchBuilder, ZilchType, NamespaceAndClass, type, Method, Name, UserDescription) \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatTwoVecToVec<1, type, NamespaceAndClass::Method>, TwoParameters(this->type##Type),  this->type##Type,  FunctionOptions::Static)->Description = ZilchDocumentString(UserDescription); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatTwoVecToVec<2, type, NamespaceAndClass::Method>, TwoParameters(this->type##2Type), this->type##2Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatTwoVecToVec<3, type, NamespaceAndClass::Method>, TwoParameters(this->type##3Type), this->type##3Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatTwoVecToVec<4, type, NamespaceAndClass::Method>, TwoParameters(this->type##4Type), this->type##4Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise."));

#define ZilchSplatNamedAllTwoVecToVecAs(ZilchBuilder, ZilchType, NamespaceAndClass, type, Method, FunctionName, Param1Name, Param2Name, UserDescription) \
  ZilchBuilder.AddBoundFunction(ZilchType, FunctionName, SplatTwoVecToVec<1, type, NamespaceAndClass::Method>, TwoParameters( this->type##Type, Param1Name,  this->type##Type, Param2Name), this->type##Type,  FunctionOptions::Static)->Description = ZilchDocumentString(UserDescription); \
  ZilchBuilder.AddBoundFunction(ZilchType, FunctionName, SplatTwoVecToVec<2, type, NamespaceAndClass::Method>, TwoParameters(this->type##2Type, Param1Name, this->type##2Type, Param2Name), this->type##2Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); \
  ZilchBuilder.AddBoundFunction(ZilchType, FunctionName, SplatTwoVecToVec<3, type, NamespaceAndClass::Method>, TwoParameters(this->type##3Type, Param1Name, this->type##3Type, Param2Name), this->type##3Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); \
  ZilchBuilder.AddBoundFunction(ZilchType, FunctionName, SplatTwoVecToVec<4, type, NamespaceAndClass::Method>, TwoParameters(this->type##4Type, Param1Name, this->type##4Type, Param2Name), this->type##4Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise."));

#define ZilchSplatAllThreeVecToVecAs(ZilchBuilder, ZilchType, NamespaceAndClass, type, Method, Name, p1, p2, p3, UserDescription) \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatThreeVecToVec<1, type, NamespaceAndClass::Method>, ThreeParameters(this->type##Type,   p1, this->type##Type,   p2,  this->type##Type, p3),  this->type##Type, FunctionOptions::Static)->Description = ZilchDocumentString(UserDescription); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatThreeVecToVec<2, type, NamespaceAndClass::Method>, ThreeParameters(this->type##2Type,  p1, this->type##2Type,  p2, this->type##2Type, p3), this->type##2Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatThreeVecToVec<3, type, NamespaceAndClass::Method>, ThreeParameters(this->type##3Type,  p1, this->type##3Type,  p2, this->type##3Type, p3), this->type##3Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatThreeVecToVec<4, type, NamespaceAndClass::Method>, ThreeParameters(this->type##4Type,  p1, this->type##4Type,  p2, this->type##4Type, p3), this->type##4Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise."));

#define ZilchSplatAllTwoVecAndRealToVecAs(ZilchBuilder, ZilchType, NamespaceAndClass, type, Method, Name, p1, p2, p3, UserDescription) \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatTwoVecAndRealToVec<1, type, NamespaceAndClass::Method>, ThreeParameters(this->type##Type,  p1, this->type##Type,  p2, this->type##Type, p3),  this->type##Type, FunctionOptions::Static)->Description = ZilchDocumentString(UserDescription); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatTwoVecAndRealToVec<2, type, NamespaceAndClass::Method>, ThreeParameters(this->type##2Type, p1, this->type##2Type, p2, this->type##Type, p3), this->type##2Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatTwoVecAndRealToVec<3, type, NamespaceAndClass::Method>, ThreeParameters(this->type##3Type, p1, this->type##3Type, p2, this->type##Type, p3), this->type##3Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise.")); \
  ZilchBuilder.AddBoundFunction(ZilchType, Name, SplatTwoVecAndRealToVec<4, type, NamespaceAndClass::Method>, ThreeParameters(this->type##4Type, p1, this->type##4Type, p2, this->type##Type, p3), this->type##4Type, FunctionOptions::Static)->Description = ZilchDocumentString(BuildString(UserDescription, " Performed component-wise."));

  //***************************************************************************
  // Splat a zero parameter function across a vector/matrix.
  template <typename ResultType,
    typename FunctionPointer, FunctionPointer Fn>
    void FullNoParameterSplat(Call& call, ExceptionReport& report)
  {
    size_t size = (size_t)call.GetFunction()->UserData;

    // Get a pointer to our return/output vector
    ResultType* result = (ResultType*)call.GetReturnUnchecked();
    call.MarkReturnAsSet();

    // Loop through all the components to apply the given function (splat)
    for (size_t i = 0; i < size; ++i)
      result[i] = Fn();
  }

  //***************************************************************************
  // This version of no parameter splatting allows the user to specify an overloaded function (by providing the types manually)
  template <typename ResultType,
    ResultType (*Function)()>
    void FullNoParameterSplatAs(Call& call, ExceptionReport& report)
  {
    FullNoParameterSplat<ResultType,
      TypeOfFunctionPointer(Function), Function>(call, report);
  }

  //***************************************************************************
  // Splat a one parameter function across a vector/matrix.
  template <typename ScalarType0, typename ResultType,
    size_t ScalarType0Offset,
    typename FunctionPointer, FunctionPointer Fn>
    void FullOneParameterSplat(Call& call, ExceptionReport& report)
  {
    size_t size = (size_t)call.GetFunction()->UserData;

    // Get pointers to our input data
    ScalarType0* input0 = (ScalarType0*)call.GetParameterUnchecked(0);

    // Get a pointer to our return/output vector
    ResultType* result = (ResultType*)call.GetReturnUnchecked();
    call.MarkReturnAsSet();

    // Loop through all the components to apply the given function (splat)
    for (size_t i = 0; i < size; ++i)
    {
      // Call the function bound function
      // (the offset is used to generically control if an input should be only one scalar or another vector type)
      result[i] = Fn(input0[i * ScalarType0Offset]);
    }
  }

  //***************************************************************************
  // This version of one parameter splatting allows the user to specify an overloaded function (by providing the types manually)
  template <typename ScalarType0, typename ResultType,
    typename FnScalarType0,
    size_t ScalarType0Offset,
    ResultType (*Function)(FnScalarType0)>
    void FullOneParameterSplatAs(Call& call, ExceptionReport& report)
  {
    FullOneParameterSplat<ScalarType0, ResultType,
      ScalarType0Offset,
      TypeOfFunctionPointer(Function), Function>(call, report);
  }

  //***************************************************************************
  // Splats a function of the type Result Fn(Scalar0, Scalar1).
  // The offset numbers are used to generically control if an input is a scalar or a vector.
  // ie. scalarOffsets of <1, 1> are used to splat Real3 Max(Real3, Real3) while
  // scalarOffset of <1, 0> are used to splat Real3 Ceil(Real3, Real).

  template <typename ScalarType0, typename ScalarType1, typename ResultType,
    size_t ScalarType0Offset, size_t ScalarType1Offset,
    typename FunctionPointer, FunctionPointer Fn>
    void FullTwoParameterSplat(Call& call, ExceptionReport& report)
  {
    size_t size = (size_t)call.GetFunction()->UserData;

    // Get pointers to our input data
    ScalarType0* input0 = (ScalarType0*)call.GetParameterUnchecked(0);
    ScalarType1* input1 = (ScalarType1*)call.GetParameterUnchecked(1);

    // Get a pointer to our return/output vector
    ResultType* result = (ResultType*)call.GetReturnUnchecked();
    call.MarkReturnAsSet();

    // Loop through all the components to apply the given function (splat)
    for (size_t i = 0; i < size; ++i)
    {
      // Call the function bound function
      // (the offset is used to generically control if an input should be only one scalar or another vector type)
      result[i] = Fn(input0[i * ScalarType0Offset], input1[i * ScalarType1Offset]);
    }
  }

  //***************************************************************************
  // This version of two parameter splatting allows the user to specify an overloaded function (by providing the types manually)
  template <typename ScalarType0, typename ScalarType1, typename ResultType,
    typename FnScalarType0, typename FnScalarType1,
    size_t ScalarType0Offset, size_t ScalarType1Offset,
    ResultType (*Function)(ScalarType0, ScalarType1)>
    void FullTwoParameterSplatAs(Call& call, ExceptionReport& report)
  {
    FullTwoParameterSplat<ScalarType0, ScalarType1, ResultType,
      ScalarType0Offset, ScalarType1Offset,
      TypeOfFunctionPointer(Function), Function>(call, report);
  }

  //***************************************************************************
  // Splats a function of the type Result Fn(Scalar0, Scalar1, Scalar2).
  // The offset numbers are used to generically control if an input is a scalar or a vector.
  // ie. scalarOffsets of <1, 1, 1> are used to splat Real3 Clamp(Real3, Real3, Real3) while
  // scalarOffset of <1, 1, 0> are used to splat Real3 Lerp(Real3, Real3, Real).
  // This is the most general case splat so that the function parameters can be specified separately
  // (mainly so the the ScalarType can be Real while the function can take const Real&)
  template <typename ScalarType0, typename ScalarType1, typename ScalarType2, typename ResultType,
    size_t ScalarType0Offset, size_t ScalarType1Offset, size_t ScalarType2Offset,
    typename FunctionPointer, FunctionPointer Fn>
    void FullThreeParameterSplat(Call& call, ExceptionReport& report)
  {
    size_t size = (size_t)call.GetFunction()->UserData;

    // Get pointers to our input data
    ScalarType0* input0 = (ScalarType0*)call.GetParameterUnchecked(0);
    ScalarType1* input1 = (ScalarType1*)call.GetParameterUnchecked(1);
    ScalarType2* input2 = (ScalarType2*)call.GetParameterUnchecked(2);

    // Get a pointer to our return/output vector
    ResultType* result = (ResultType*)call.GetReturnUnchecked();
    call.MarkReturnAsSet();

    // Loop through all the components to apply the given function (splat)
    for (size_t i = 0; i < size; ++i)
    {
      // Call the function bound function
      // (the offset is used to generically control if an input should be only one scalar or another vector type)
      result[i] = Fn(input0[i * ScalarType0Offset], input1[i * ScalarType1Offset], input2[i * ScalarType2Offset]);
    }
  }

  //***************************************************************************
  // This version of two parameter splatting allows the user to specify an overloaded function (by providing the types manually)
  template <typename ScalarType0, typename ScalarType1, typename ScalarType2, typename ResultType,
    typename FnScalarType0, typename FnScalarType1, typename FnScalarType2,
    size_t ScalarType0Offset, size_t ScalarType1Offset, size_t ScalarType2Offset,
    ResultType (*Function)(FnScalarType0, FnScalarType1, FnScalarType2)>
    void FullThreeParameterSplatAs(Call& call, ExceptionReport& report)
  {
    FullThreeParameterSplat<ScalarType0, ScalarType1, ScalarType2, ResultType,
      ScalarType0Offset, ScalarType1Offset, ScalarType2Offset,
      TypeOfFunctionPointer(Function), Function>(call, report);

  }

  //***************************************************************************
  // User data for splatting a function with a custom error message. 
  // Used by Sqrt to deal with negative numbers and a few other functions.
  class SplatWithErrorUserData
  {
  public:

    SplatWithErrorUserData(size_t size, cstr errorFormat, BoundType* boundType) :
      Size(size),
      ErrorFormat(errorFormat),
      Type(boundType)
    {

    }

    size_t Size;
    cstr ErrorFormat;
    BoundType* Type;
  };

  //***************************************************************************
  void AllNonZero(Call& call, ExceptionReport& report)
  {
    SplatWithErrorUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<SplatWithErrorUserData>(0);

    size_t elementSize = userData.Type->Size / userData.Size;
    
    // This should always be a small allocation so just allocate it on the stack
    byte* data = (byte*)alloca(elementSize);
    memset(data, 0, elementSize);

    byte* input = call.GetParameterUnchecked(0);

    bool allTrue = true;
    for (size_t i = 0; i < userData.Size; ++i)
    {
      if (memcmp(input + i * elementSize, data, elementSize) == 0)
        allTrue = false;
    }

    call.Set(Call::Return, allTrue);
  }
  
  //***************************************************************************
  void AnyNonZero(Call& call, ExceptionReport& report)
  {
    SplatWithErrorUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<SplatWithErrorUserData>(0);

    size_t elementSize = userData.Type->Size / userData.Size;
    
    // This should always be a small allocation so just allocate it on the stack
    byte* data = (byte*)alloca(elementSize);
    memset(data, 0, elementSize);

    byte* input = call.GetParameterUnchecked(0);

    bool anyTrue = false;
    for (size_t i = 0; i < userData.Size; ++i)
    {
      if (memcmp(input + i * elementSize, data, elementSize) != 0)
      {
        anyTrue = true;
        break;
      }
    }

    call.Set(Call::Return, anyTrue);
  }

  //***************************************************************************
  // Splats a function of the type Boolean Fn(Scalar, Scalar&) to
  // Vec(n) Fn(Vec(n)). The function being splatted is a Scalar Fn(Scalar)
  // function that has been altered to return if it failed so an exception
  // can be thrown.
  template <typename ScalarType, bool (*Function)(ScalarType, ScalarType&)>
  void SimpleSplatWithError(Call& call, ExceptionReport& report)
  {
    SplatWithErrorUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<SplatWithErrorUserData>(0);
    
    // Get a pointer to our input vector
    ScalarType* input = (ScalarType*)call.GetParameterUnchecked(0);

    // Get a pointer to our return/output vector
    ScalarType* result = (ScalarType*)call.GetReturnUnchecked();
    call.MarkReturnAsSet();

    bool success = true;
    // Loop through all the components to apply the given function (splat)
    for (size_t i = 0; i < userData.Size; ++i)
    {
      // Call the function on the single input (splat)
      success = Function(input[i], result[i]);

      // If the function failed for whatever reason then throw an exception
      if (success == false)
      {
        // The user data of this function should contain a format string with
        // one %s in it to display the input value that failed.
        cstr errFormat = userData.ErrorFormat;
        String inputAsString = userData.Type->GenericToString((byte*)input);
        String msg = String::Format(errFormat, inputAsString.c_str());
        call.GetState()->ThrowException(report, msg);
        return;
      }
    }
  }

  //***************************************************************************
  // Splats a function of the type Boolean Fn(Scalar, Scalar&) to
  // Vec(n) Fn(Vec(n)). The function being splatted is a Scalar Fn(Scalar)
  // function that has been altered to return if it failed so an exception
  // can be thrown.
  template <typename ScalarType0, typename ScalarType1, typename ResultType,
    size_t ScalarType0Offset, size_t ScalarType1Offset,
    bool (*Function)(ScalarType0, ScalarType1, ResultType&)>
  void FullTwoParameterSplatWithError(Call& call, ExceptionReport& report)
  {
    SplatWithErrorUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<SplatWithErrorUserData>(0);
    
    // Get a pointer to our input vector
    ScalarType0* input0 = (ScalarType0*)call.GetParameterUnchecked(0);
    ScalarType1* input1 = (ScalarType1*)call.GetParameterUnchecked(1);

    // Get a pointer to our return/output vector
    ResultType* result = (ResultType*)call.GetReturnUnchecked();
    call.MarkReturnAsSet();

    bool success = true;
    // Loop through all the components to apply the given function (splat)
    for (size_t i = 0; i < userData.Size; ++i)
    {
      // Call the function on the single input (splat)
      success = Function(input0[i * ScalarType0Offset], input1[i * ScalarType0Offset], result[i]);

      // If the function failed for whatever reason then throw an exception
      if (success == false)
      {
        // The user data of this function should contain a format string with
        // two %s in it to display the input value that failed.
        cstr errFormat = userData.ErrorFormat;
        String input0AsString = userData.Type->GenericToString((byte*)input0);
        String input1AsString = userData.Type->GenericToString((byte*)input1);
        String msg = String::Format(errFormat, input0AsString.c_str(), input1AsString.c_str());
        call.GetState()->ThrowException(report, msg);
        return;
      }
    }
  }

#define ZilchSetUserDataAndDescription(function, boundType, scalarBoundType, docString)    \
  f->UserData = (void*)(boundType->Size / scalarBoundType->Size);                          \
  if (boundType == scalarBoundType)                                                        \
    f->Description = ZilchDocumentString(docString);                                            \
  else                                                                                     \
    f->Description = ZilchDocumentString(BuildString(docString, " Performed component-wise.")); 

#define ZilchComplexOneParameterSplatBinder(scalarType0, returnType, offset0, function) \
  FullOneParameterSplatAs<scalarType0, returnType, scalarType0, offset0, function>

#define ZilchBindBasicSplat(builder, owner, scalarType, scalarBoundType, functionName, function, boundType, parameters, docString) \
  {                                                                                                                                \
    Zilch::BoundFn boundFn = ZilchComplexOneParameterSplatBinder(scalarType, scalarType, 1, function);                             \
    Function* f = builder.AddBoundFunction(owner, functionName, boundFn, parameters, boundType, FunctionOptions::Static);          \
    ZilchSetUserDataAndDescription(f, boundType, scalarBoundType, docString);                                                      \
  }

#define ZilchBindBasicSplatWithError(builder, owner, scalarType, scalarBoundType, functionName, function, boundType, parameters, docString, errorFormat)     \
  {                                                                                                                                                          \
    Function* f = builder.AddBoundFunction(owner, functionName, SimpleSplatWithError<scalarType, function>, parameters, boundType, FunctionOptions::Static); \
    SplatWithErrorUserData userData(boundType->Size / scalarBoundType->Size, errorFormat, boundType);                                                        \
    f->ComplexUserData.WriteObject(userData);                                                                                                                \
    if (boundType == scalarBoundType)                                                                                                                        \
      f->Description = ZilchDocumentString(docString);                                                                                                            \
    else                                                                                                                                                     \
      f->Description = ZilchDocumentString(BuildString(docString, " Performed component-wise."));                                                                 \
  }

#define ZilchComplexTwoParameterSplatBinder(scalarType0, scalarType1, returnType, offset0, offset1, function) \
  FullTwoParameterSplatAs<scalarType0, scalarType1, returnType, scalarType0, scalarType1, offset0, offset1, function>

#define ZilchBindBasicTwoParamSplat(builder, owner, scalarType, scalarBoundType, functionName, function, boundType, parameters, docString) \
  {                                                                                                                                        \
    Zilch::BoundFn boundFn = ZilchComplexTwoParameterSplatBinder(scalarType, scalarType, scalarType, 1, 1, function);                      \
    Function* f = builder.AddBoundFunction(owner, functionName, boundFn, parameters, boundType, FunctionOptions::Static);                  \
    ZilchSetUserDataAndDescription(f, boundType, scalarBoundType, docString);                                                              \
  }

#define ZilchBindBasicTwoParamSplatWithError(builder, owner, scalarType, scalarBoundType, functionName, function, boundType, parameters, docString, errorFormat) \
  {                                                                                                                                                              \
    Zilch::BoundFn boundFn = FullTwoParameterSplatWithError<scalarType, scalarType, scalarType, 1, 1, function>;                                                 \
    Function* f = builder.AddBoundFunction(owner, functionName, boundFn, parameters, boundType, FunctionOptions::Static);                                        \
    SplatWithErrorUserData userData(boundType->Size / scalarBoundType->Size, errorFormat, boundType);                                                            \
    f->ComplexUserData.WriteObject(userData);                                                                                                                    \
    if (boundType == scalarBoundType)                                                                                                                            \
      f->Description = ZilchDocumentString(docString);                                                                                                                \
    else                                                                                                                                                         \
      f->Description = ZilchDocumentString(BuildString(docString, " Performed component-wise."));                                                                     \
  }

#define ZilchFullThreeParameterSplatBinder(Type0, Type1, Type2, ReturnType, Offset0, Offset1, Offset2, BoundFunction) \
  FullThreeParameterSplat<Type0, Type1, Type2, ReturnType, Offset0, Offset1, Offset2, ZilchTypeOfFunctionPointer(BoundFunction) , BoundFunction>

#define ZilchComplexThreeParameterSplatBinder(scalarType0, scalarType1, scalarType2, returnType, offset0, offset1, offset2, function) \
  FullThreeParameterSplatAs<scalarType0, scalarType1, scalarType2, returnType, scalarType0, scalarType1, scalarType2, offset0, offset1, offset2, function>

#define ZilchBindBasicThreeParamSplat(builder, owner, scalarType, scalarBoundType, functionName, function, boundType, parameters, docString) \
  {                                                                                                                                          \
    Zilch::BoundFn boundFn = ZilchComplexThreeParameterSplatBinder(scalarType, scalarType, scalarType, scalarType, 1, 1, 1, function);       \
    Function* f = builder.AddBoundFunction(owner, functionName, boundFn, parameters, boundType, FunctionOptions::Static);                    \
    ZilchSetUserDataAndDescription(f, boundType, scalarBoundType, docString);                                                                \
  }

  //***************************************************************************
  void Core::SetupBinding(LibraryBuilder& builder)
  {
    builder.BuiltLibrary->NamespaceForPlugins = "Zilch";

    ZilchInitializeExternalType(Boolean);
    ZilchInitializeExternalType(Boolean2);
    ZilchInitializeExternalType(Boolean3);
    ZilchInitializeExternalType(Boolean4);
    ZilchInitializeExternalType(Byte);
    ZilchInitializeExternalType(Integer);
    ZilchInitializeExternalType(Integer2);
    ZilchInitializeExternalType(Integer3);
    ZilchInitializeExternalType(Integer4);
    ZilchInitializeExternalType(Real);
    ZilchInitializeExternalType(Real2);
    ZilchInitializeExternalType(Real3);
    ZilchInitializeExternalType(Real4);
    ZilchInitializeExternalType(Quaternion);
    ZilchInitializeExternalType(Real2x2);
    ZilchInitializeExternalType(Real3x3);
    ZilchInitializeExternalType(Real4x4);
    ZilchInitializeExternalType(String);
    ZilchInitializeExternalType(DoubleReal);
    ZilchInitializeExternalType(DoubleInteger);
    ZilchInitializeExternalType(Zero::Rune);
    ZilchInitializeType(Enum);

    // Setup all the primitive types
    BoundType* doubleIntegerType      = ZilchTypeId(DoubleInteger);
    BoundType* doubleRealType         = ZilchTypeId(DoubleReal);
    BoundType* byteType               = ZilchTypeId(Byte);
    BoundType* booleanType            = ZilchTypeId(Boolean);
    BoundType* boolean2Type           = ZilchTypeId(Boolean2);
    BoundType* boolean3Type           = ZilchTypeId(Boolean3);
    BoundType* boolean4Type           = ZilchTypeId(Boolean4);
    BoundType* integerType            = ZilchTypeId(Integer);
    BoundType* integer2Type           = ZilchTypeId(Integer2);
    BoundType* integer3Type           = ZilchTypeId(Integer3);
    BoundType* integer4Type           = ZilchTypeId(Integer4);
    BoundType* realType               = ZilchTypeId(Real);
    BoundType* real2Type              = ZilchTypeId(Real2);
    BoundType* real3Type              = ZilchTypeId(Real3);
    BoundType* real4Type              = ZilchTypeId(Real4);
    BoundType* quaternionType         = ZilchTypeId(Quaternion);
    BoundType* real2x2Type            = ZilchTypeId(Real2x2);
    BoundType* real3x3Type            = ZilchTypeId(Real3x3);
    BoundType* real4x4Type            = ZilchTypeId(Real4x4);

    BoundType* anyHandleType          = builder.AddBoundType("AnyHandle", TypeCopyMode::ReferenceType, 0, 0);
    BoundType* nullType               = builder.AddBoundType("Null", TypeCopyMode::ReferenceType, 0, 0);
    BoundType* voidType               = builder.AddBoundType("Void", TypeCopyMode::ValueType, 0, 0);
    BoundType* errorType              = builder.AddBoundType("[ErrorType]", TypeCopyMode::ValueType, 0, 0);
    BoundType* overloadedMethodsType  = builder.AddBoundType("[MultipleMethodsOfTheSameName]", TypeCopyMode::ValueType, 0, 0);
    DelegateType* anyDelegateType     = builder.GetDelegateType(ParameterArray(), voidType);

    // Store the primitive types as global constants
    this->ByteType              = byteType;
    this->BooleanType           = booleanType;
    this->Boolean2Type          = boolean2Type;
    this->Boolean3Type          = boolean3Type;
    this->Boolean4Type          = boolean4Type;
    this->IntegerType           = integerType;
    this->Integer2Type          = integer2Type;
    this->Integer3Type          = integer3Type;
    this->Integer4Type          = integer4Type;
    this->RealType              = realType;
    this->Real2Type             = real2Type;
    this->Real3Type             = real3Type;
    this->Real4Type             = real4Type;
    this->QuaternionType        = quaternionType;
    this->Real2x2Type           = real2x2Type;
    this->Real3x3Type           = real3x3Type;
    this->Real4x4Type           = real4x4Type;
    this->DoubleIntegerType     = doubleIntegerType;
    this->DoubleRealType        = doubleRealType;
    this->VoidType              = voidType;
    this->NullType              = nullType;
    this->ErrorType             = errorType;
    this->OverloadedMethodsType = overloadedMethodsType;
    this->AnyDelegateType       = anyDelegateType;
    this->AnyHandleType         = anyHandleType;

    // Fill out the real array
    this->BooleanTypes[0] = booleanType;
    this->BooleanTypes[1] = boolean2Type;
    this->BooleanTypes[2] = boolean3Type;
    this->BooleanTypes[3] = boolean4Type;
    this->AllBooleanTypes.PushBack(booleanType);
    this->AllBooleanTypes.PushBack(boolean2Type);
    this->AllBooleanTypes.PushBack(boolean3Type);
    this->AllBooleanTypes.PushBack(boolean4Type);
    this->IntegerTypes[0] = integerType;
    this->IntegerTypes[1] = integer2Type;
    this->IntegerTypes[2] = integer3Type;
    this->IntegerTypes[3] = integer4Type;
    this->AllIntegerTypes.PushBack(integerType);
    this->AllIntegerTypes.PushBack(integer2Type);
    this->AllIntegerTypes.PushBack(integer3Type);
    this->AllIntegerTypes.PushBack(integer4Type);
    this->RealTypes[0] = realType;
    this->RealTypes[1] = real2Type;
    this->RealTypes[2] = real3Type;
    this->RealTypes[3] = real4Type;
    this->AllRealTypes.PushBack(realType);
    this->AllRealTypes.PushBack(real2Type);
    this->AllRealTypes.PushBack(real3Type);
    this->AllRealTypes.PushBack(real4Type);
    this->VectorScalarBoundTypes[VectorScalarTypes::Real] = realType;
    this->VectorScalarBoundTypes[VectorScalarTypes::Integer] = integerType;
    this->VectorScalarBoundTypes[VectorScalarTypes::Boolean] = booleanType;
    this->VectorTypes[VectorScalarTypes::Real] = this->RealTypes;
    this->VectorTypes[VectorScalarTypes::Integer] = this->IntegerTypes;
    this->VectorTypes[VectorScalarTypes::Boolean] = this->BooleanTypes;
    this->MatrixElementTypes[0] = realType;
    this->MatrixElementTypes[1] = integerType;
    this->MatrixElementTypes[2] = booleanType;

    // Setup the functions to set a value to 1 for the each primary scalar type
    ScalarTypeOneFunctions[VectorScalarTypes::Real] = ScalarTypeRealOne;
    ScalarTypeOneFunctions[VectorScalarTypes::Integer] = ScalarTypeIntegerOne;
    ScalarTypeOneFunctions[VectorScalarTypes::Boolean] = ScalarTypeBooleanOne;

    ZilchFullBindMethod(builder, this->IntegerType, &ReinterpretRealToInteger, ZilchNoOverload, "Reinterpret", ZilchNoNames);
    ZilchFullBindMethod(builder, this->RealType, &ReinterpretIntegerToReal, ZilchNoOverload, "Reinterpret", ZilchNoNames);

    // Create the one special any type (there should only ever be one instantiation of this!)
    this->AnythingType = new AnyType();
    this->AnythingType->SourceLibrary = builder.BuiltLibrary.GetObject();
    builder.BuiltLibrary->OwnedTypes.PushBack(this->AnythingType);

    // Add the array template instantiator
    StringArray arrayArguments;
    arrayArguments.PushBack("Type");
    builder.AddTemplateInstantiator("Property", InstantiatePropertyDelegate, arrayArguments, nullptr);
    builder.AddTemplateInstantiator("Array", InstantiateArray, arrayArguments, nullptr);

    StringArray keyValueArguments;
    keyValueArguments.PushBack("Key");
    keyValueArguments.PushBack("Value");

    // Add the hash-map template instantiator
    builder.AddTemplateInstantiator("HashMap", InstantiateHashMap, keyValueArguments, nullptr);
    builder.AddTemplateInstantiator("HashMapRange", InstantiateHashMapRange, keyValueArguments, (void*)(size_t)HashMapRangeMode::Pair);
    builder.AddTemplateInstantiator("HashMapKeyRange", InstantiateHashMapRange, StringArray(ZeroInit, "Key"), (void*)(size_t)HashMapRangeMode::Key);
    builder.AddTemplateInstantiator("HashMapValueRange", InstantiateHashMapRange, StringArray(ZeroInit, "Value"), (void*)(size_t)HashMapRangeMode::Value);
    builder.AddTemplateInstantiator("KeyValue", InstantiateKeyValue, keyValueArguments, nullptr);

    // Create the string type as a reference type
    BoundType* stringRangeType = ZilchTypeId(StringRangeExtended);
    BoundType* stringType = ZilchTypeId(String);
    BoundType* runeIteratorType = ZilchTypeId(RuneIterator);
    BoundType* splitRangeType = ZilchTypeId(StringSplitRangeExtended);

    stringType->CopyMode = TypeCopyMode::ReferenceType;
    stringType->HandleManager = ZilchManagerId(StringManager);
    // Old functions that should eventually be removed
    builder.AddBoundFunction(stringType, "SubString", SubString, TwoParameters(runeIteratorType, "start", "end"), stringType, FunctionOptions::None);
    builder.AddBoundFunction(stringType, "SubStringFromRuneIndices", SubStringFromRuneIndices, TwoParameters(integerType, "startIndex", "endIndex"), stringType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Creates a substring from start and end indices. WARNING: this may be slow as finding an index for a UTF8 string requires a linear search.");
    builder.AddBoundFunction(stringType, Zilch::OperatorGet, StringGetRune, OneParameter(integerType, "index"), ZilchTypeId(Rune), FunctionOptions::None);

    builder.AddBoundFunction(stringType, "SubStringBytes", SubStringBytes, TwoParameters(integerType, "startByteIndex", "lengthInBytes"), stringRangeType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Constructs a substring based upon a number of bytes. WARNING: strings are UTF8 so indexing by bytes could produce unexpected results on non-ascii strings.");
    builder.AddBoundFunction(stringType, "RuneIteratorFromByteIndex", StringRuneIteratorFromByteIndex, OneParameter(integerType, "byteIndex"), runeIteratorType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Finds the iterator from a byte index. WARNING: Strings are UTF8 and constructing an iterator from bytes indices can make an iterator in the middle of a rune.");
    builder.AddBoundFunction(stringType, "RuneIteratorFromRuneIndex", StringRuneIteratorFromRuneIndex, OneParameter(integerType, "runeIndex"), runeIteratorType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Finds the iterator from a rune index. WARNING: this may be slow as finding an iterator from rune index requires a linear search.");
    builder.AddBoundGetterSetter(stringType, "Empty", booleanType, nullptr, StringEmpty, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns true if the string is emtpy.");
    builder.AddBoundGetterSetter(stringType, "IsNotEmpty", booleanType, nullptr, StringIsNotEmpty, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns true if the string is not empty.");
    builder.AddBoundGetterSetter(stringType, "Begin", runeIteratorType, nullptr, StringBegin, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns the RuneIterator at the start of this string.");
    builder.AddBoundGetterSetter(stringType, "End", runeIteratorType, nullptr, StringEnd, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns the RuneIterator at the end (one past the last Rune) of this string.");
    builder.AddBoundGetterSetter(stringType, "All", stringRangeType, nullptr, StringAll, FunctionOptions::None)
      ->Description = ZilchDocumentString("Converts the string into a string range.");
    builder.AddBoundGetterSetter(stringType, "ByteCount", integerType, nullptr, StringByteCount, MemberOptions::None)
      ->Description = ZilchDocumentString("Returns the number of bytes in the string.");
    builder.AddBoundGetterSetter(stringType, "Count", integerType, nullptr, StringByteCount,  MemberOptions::None)
      ->Description = ZilchDocumentString("Returns the number of bytes in the string.");
    builder.AddBoundFunction(stringType, "ComputeRuneCount", StringComputeRuneCount, ParameterArray(), integerType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Compute the number of runes in the string.");
    builder.AddBoundFunction(stringType, "Concatenate", StringConcatenate, TwoParameters(stringType), stringType, FunctionOptions::Static)
      ->Description = ZilchDocumentString("Combines the two strings into a new string.");
    builder.AddBoundFunction(stringType, "Concatenate", StringRangeConcatenate, TwoParameters(stringRangeType), stringType, FunctionOptions::Static)
      ->Description = ZilchDocumentString("Combines the two string ranges into a new string.");
    builder.AddBoundFunction(stringType, "FromChar", StringFromChar, OneParameter(integerType), stringType, FunctionOptions::Static)
      ->Description = ZilchDocumentString("Constructs a string from the ascii index of a character.");
    builder.AddBoundFunction(stringType, "Contains", StringContains, OneParameter(stringRangeType), booleanType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns if the string Contains the specified substring.");
    builder.AddBoundFunction(stringType, "Compare", StringCompare, TwoParameters(stringType, "left", "right"), integerType, FunctionOptions::Static)
      ->Description = ZilchDocumentString("Compares the two strings and returns an integer to denote their relative sort order.");
    builder.AddBoundFunction(stringRangeType, "Compare", StringRangeCompare, TwoParameters(stringRangeType, "left", "right"), integerType, FunctionOptions::Static)
      ->Description = ZilchDocumentString("Compares the two string ranges and returns an integer to denote their relative sort order.");
    builder.AddBoundFunction(stringType, "CompareTo", StringCompareTo, OneParameter(stringRangeType), integerType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Compares this string to the given string and returns an integer to denote their relative sort order.");
    builder.AddBoundFunction(stringType, "StartsWith", StringStartsWith, OneParameter(stringRangeType), booleanType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns if the string starts with the specified substring.");
    builder.AddBoundFunction(stringType, "EndsWith", StringEndsWith, OneParameter(stringRangeType), booleanType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns if the string ends with the specified substring.");
    builder.AddBoundFunction(stringType, "TrimStart", StringTrimStart, ParameterArray(), stringRangeType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Trims all leading whitespace.");
    builder.AddBoundFunction(stringType, "TrimEnd", StringTrimEnd, ParameterArray(), stringRangeType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Trims all trailing whitespace.");
    builder.AddBoundFunction(stringType, "Trim", StringTrim, ParameterArray(), stringRangeType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Trims all leading and trailing whitespace.");
    builder.AddBoundFunction(stringType, "ToLower", StringToLower, ParameterArray(), stringType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns a copy of the string that has been converted to lowercase.");
    builder.AddBoundFunction(stringType, "ToUpper", StringToUpper, ParameterArray(), stringType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns a copy of the string that has been converted to uppercase.");
    builder.AddBoundFunction(stringType, "Replace", StringReplace, TwoParameters(stringRangeType, "oldValue", "newValue"), stringType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns a new string with all occurances of a substrings replaced with another substring.");
    builder.AddBoundFunction(stringType, "FindRangeInclusive", StringFindRangeInclusive, TwoParameters(stringRangeType, "startRange", "endRange"), stringRangeType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Finds the first StringRange that starts with 'startRange' and ends with 'endRange'. This substring includes 'startRange' and 'endRange'.");
    builder.AddBoundFunction(stringType, "FindRangeExclusive", StringFindRangeExclusive, TwoParameters(stringRangeType, "startRange", "endRange"), stringRangeType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Finds the first StringRange that starts with 'startRange' and ends with 'endRange'. This substring excludes 'startRange' and 'endRange'.");
    builder.AddBoundFunction(stringType, "FindFirstOf", StringFindFirstOf, OneParameter(stringRangeType), stringRangeType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns a StringRange that Contains the first occurrence of given StringRange.");
    builder.AddBoundFunction(stringType, "FindLastOf", StringFindLastOf, OneParameter(stringRangeType), stringRangeType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Returns a StringRange that Contains the last occurrence of given StringRange.");
    builder.AddBoundFunction(stringType, "Join", JoinTwoStrings, ThreeParameters(stringRangeType, "separator", "value0", "value1"), stringType, FunctionOptions::Static)
      ->Description = ZilchDocumentString("Concatenates the given strings with the given separator string.");
    builder.AddBoundFunction(stringType, "Join", JoinThreeStrings, FourParameters(stringRangeType, "separator", "value0", "value1", "value2"), stringType, FunctionOptions::Static)
      ->Description = ZilchDocumentString("Concatenates the given strings with the given separator string.");
    builder.AddBoundFunction(stringType, "Join", JoinFourStrings, FiveParameters(stringRangeType, "separator", "value0", "value1", "value2", "value3"), stringType, FunctionOptions::Static)
      ->Description = ZilchDocumentString("Concatenates the given strings with the given separator string.");
    builder.AddBoundFunction(stringType, "Split", StringSplit, OneParameter(stringRangeType, "separator"), splitRangeType, FunctionOptions::None)
      ->Description = ZilchDocumentString("Splits the string, according to the separator string, into a range of substrings.");
    builder.AddBoundFunction(stringType, "IsNullOrEmpty", StringRangeIsNullOrEmpty, OneParameter(stringRangeType), booleanType, FunctionOptions::Static)
      ->Description = ZilchDocumentString("Returns if the given string is null or empty.");
    builder.AddBoundFunction(stringType, "IsNullOrWhitespace", StringRangeIsNullOrWhitespace, OneParameter(stringRangeType), booleanType, FunctionOptions::Static)
      ->Description = ZilchDocumentString("Returns if the given string is null, empty, or all whitespace.");

    // Bind the FormatC function which has "variadic arguments"
    ParameterArray parameters;
    DelegateParameter& formatParameter = parameters.PushBack();
    formatParameter.Name = "format";
    formatParameter.ParameterType = stringType;
    for (size_t i = 0; i < 10; ++i)
    {
      DelegateParameter& parameter = parameters.PushBack();
      parameter.ParameterType = ZilchTypeId(Any);
      Function* formatFunction = builder.AddBoundFunction(stringType, "FormatC", StringFormatC, parameters, stringType, FunctionOptions::Static);
      formatFunction->UserData = (void*)(i + 1);
    }
    
    this->StringType = stringType;
    this->StringRangeType = stringRangeType;
    ZilchFullBindMethod(builder, integerType, &ZilchParseInteger, ZilchNoOverload, "Parse", ZilchNoNames)->Description = ZilchDocumentString("Attempt to convert the given StringRange to an Integer. If parsing fails 0 is returned.");
    ZilchFullBindMethod(builder, realType, &ZilchParseReal, ZilchNoOverload, "Parse", ZilchNoNames)->Description = ZilchDocumentString("Attempt to convert the given StringRange to a Real. If parsing fails 0 is returned.");

    // Bind any stringify functions
    byteType          ->ToStringFunction = ByteToString;
    booleanType       ->ToStringFunction = BooleanToString;
    boolean2Type      ->ToStringFunction = Boolean2ToString;
    boolean3Type      ->ToStringFunction = Boolean3ToString;
    boolean4Type      ->ToStringFunction = Boolean4ToString;
    integerType       ->ToStringFunction = IntegerToString;
    integer2Type      ->ToStringFunction = Integer2ToString;
    integer3Type      ->ToStringFunction = Integer3ToString;
    integer4Type      ->ToStringFunction = Integer4ToString;
    realType          ->ToStringFunction = RealToString;
    real2Type         ->ToStringFunction = Real2ToString;
    real3Type         ->ToStringFunction = Real3ToString;
    real4Type         ->ToStringFunction = Real4ToString;
    quaternionType    ->ToStringFunction = QuaternionToString;
    doubleIntegerType ->ToStringFunction = DoubleIntegerToString;
    doubleRealType    ->ToStringFunction = DoubleRealToString;
    stringType        ->ToStringFunction = StringToString;
    
    // Add ourselves to the library
    BoundType* math = builder.AddBoundType("Math", TypeCopyMode::ReferenceType, 0);
    MathType = math;

    // Bind default constructors to the vector types
    builder.AddBoundDefaultConstructor(booleanType,     VectorDefaultConstructor<1, Boolean>);
    builder.AddBoundDefaultConstructor(boolean2Type,    VectorDefaultConstructor<2, Boolean>);
    builder.AddBoundDefaultConstructor(boolean3Type,    VectorDefaultConstructor<3, Boolean>);
    builder.AddBoundDefaultConstructor(boolean4Type,    VectorDefaultConstructor<4, Boolean>);
    builder.AddBoundDefaultConstructor(integerType,     VectorDefaultConstructor<1, Integer>);
    builder.AddBoundDefaultConstructor(integer2Type,    VectorDefaultConstructor<2, Integer>);
    builder.AddBoundDefaultConstructor(integer3Type,    VectorDefaultConstructor<3, Integer>);
    builder.AddBoundDefaultConstructor(integer4Type,    VectorDefaultConstructor<4, Integer>);
    builder.AddBoundDefaultConstructor(realType,        VectorDefaultConstructor<1, Real>);
    builder.AddBoundDefaultConstructor(real2Type,       VectorDefaultConstructor<2, Real>);
    builder.AddBoundDefaultConstructor(real3Type,       VectorDefaultConstructor<3, Real>);
    builder.AddBoundDefaultConstructor(real4Type,       VectorDefaultConstructor<4, Real>);
    builder.AddBoundDefaultConstructor(quaternionType,  QuaternionDefaultConstructor);

    // The scalar constructors
    {
      ParameterArray parameters;
      DelegateParameter& scalarParam = parameters.PushBack();
      scalarParam.ParameterType = this->RealType;
      scalarParam.Name = "scalar";

      // Bind constructors to the vector types
      GenerateVectorScalarConstructor<1, Boolean>(builder, booleanType,   booleanType);
      GenerateVectorScalarConstructor<2, Boolean>(builder, boolean2Type,  booleanType);
      GenerateVectorScalarConstructor<3, Boolean>(builder, boolean3Type,  booleanType);
      GenerateVectorScalarConstructor<4, Boolean>(builder, boolean4Type,  booleanType);
      GenerateVectorScalarConstructor<1, Integer>(builder, integerType,   integerType);
      GenerateVectorScalarConstructor<2, Integer>(builder, integer2Type,  integerType);
      GenerateVectorScalarConstructor<3, Integer>(builder, integer3Type,  integerType);
      GenerateVectorScalarConstructor<4, Integer>(builder, integer4Type,  integerType);
      GenerateVectorScalarConstructor<1, Real   >(builder, realType,      realType);
      GenerateVectorScalarConstructor<2, Real   >(builder, real2Type,     realType);
      GenerateVectorScalarConstructor<3, Real   >(builder, real3Type,     realType);
      GenerateVectorScalarConstructor<4, Real   >(builder, real4Type,     realType);
      GenerateVectorScalarConstructor<4, Real   >(builder, quaternionType, realType);
    }

    // Generate the different permutations of constructors (no need to do the singles, they only have the scalar/default constructors)
    GenerateVectorComponentConstructors<2, Boolean>(builder, boolean2Type,    this->BooleanTypes);
    GenerateVectorComponentConstructors<3, Boolean>(builder, boolean3Type,    this->BooleanTypes);
    GenerateVectorComponentConstructors<4, Boolean>(builder, boolean4Type,    this->BooleanTypes);
    GenerateVectorComponentConstructors<2, Integer>(builder, integer2Type,    this->IntegerTypes);
    GenerateVectorComponentConstructors<3, Integer>(builder, integer3Type,    this->IntegerTypes);
    GenerateVectorComponentConstructors<4, Integer>(builder, integer4Type,    this->IntegerTypes);
    GenerateVectorComponentConstructors<2, Real   >(builder, real2Type,       this->RealTypes);
    GenerateVectorComponentConstructors<3, Real   >(builder, real3Type,       this->RealTypes);
    GenerateVectorComponentConstructors<4, Real   >(builder, real4Type,       this->RealTypes);
    GenerateVectorComponentConstructors<4, Real   >(builder, quaternionType,  this->RealTypes);
    
    // Generate swizzles for all of our vector types
    GenerateVectorSwizzles<1, Boolean >(builder, booleanType,     this->BooleanTypes);
    GenerateVectorSwizzles<2, Boolean >(builder, boolean2Type,    this->BooleanTypes);
    GenerateVectorSwizzles<3, Boolean >(builder, boolean3Type,    this->BooleanTypes);
    GenerateVectorSwizzles<4, Boolean >(builder, boolean4Type,    this->BooleanTypes);
    GenerateVectorSwizzles<1, Integer >(builder, integerType,     this->IntegerTypes);
    GenerateVectorSwizzles<2, Integer >(builder, integer2Type,    this->IntegerTypes);
    GenerateVectorSwizzles<3, Integer >(builder, integer3Type,    this->IntegerTypes);
    GenerateVectorSwizzles<4, Integer >(builder, integer4Type,    this->IntegerTypes);
    GenerateVectorSwizzles<1, Real    >(builder, realType,        this->RealTypes);
    GenerateVectorSwizzles<2, Real    >(builder, real2Type,       this->RealTypes);
    GenerateVectorSwizzles<3, Real    >(builder, real3Type,       this->RealTypes);
    GenerateVectorSwizzles<4, Real    >(builder, real4Type,       this->RealTypes);
    GenerateVectorSwizzles<4, Real    >(builder, quaternionType,  this->RealTypes);

    // Every vector gets a count which tells you how many elements there are, for generic programming
    builder.AddBoundGetterSetter(booleanType,    "Count", this->IntegerType, nullptr, VectorCount<1>, FunctionOptions::None)->IsHidden = true;
    builder.AddBoundGetterSetter(boolean2Type,   "Count", this->IntegerType, nullptr, VectorCount<2>, FunctionOptions::None)->IsHidden = true;
    builder.AddBoundGetterSetter(boolean3Type,   "Count", this->IntegerType, nullptr, VectorCount<3>, FunctionOptions::None)->IsHidden = true;
    builder.AddBoundGetterSetter(boolean4Type,   "Count", this->IntegerType, nullptr, VectorCount<4>, FunctionOptions::None)->IsHidden = true;
    builder.AddBoundGetterSetter(integerType,    "Count", this->IntegerType, nullptr, VectorCount<1>, FunctionOptions::None)->IsHidden = true;
    builder.AddBoundGetterSetter(integer2Type,   "Count", this->IntegerType, nullptr, VectorCount<2>, FunctionOptions::None)->IsHidden = true;
    builder.AddBoundGetterSetter(integer3Type,   "Count", this->IntegerType, nullptr, VectorCount<3>, FunctionOptions::None)->IsHidden = true;
    builder.AddBoundGetterSetter(integer4Type,   "Count", this->IntegerType, nullptr, VectorCount<4>, FunctionOptions::None)->IsHidden = true;
    builder.AddBoundGetterSetter(realType,       "Count", this->IntegerType, nullptr, VectorCount<1>, FunctionOptions::None)->IsHidden = true;
    builder.AddBoundGetterSetter(real2Type,      "Count", this->IntegerType, nullptr, VectorCount<2>, FunctionOptions::None)->IsHidden = true;
    builder.AddBoundGetterSetter(real3Type,      "Count", this->IntegerType, nullptr, VectorCount<3>, FunctionOptions::None)->IsHidden = true;
    builder.AddBoundGetterSetter(real4Type,      "Count", this->IntegerType, nullptr, VectorCount<4>, FunctionOptions::None)->IsHidden = true;
    builder.AddBoundGetterSetter(quaternionType, "Count", this->IntegerType, nullptr, VectorCount<4>, FunctionOptions::None)->IsHidden = true;
    
    // Bind the get functions for vectors (indexing)
    builder.AddBoundFunction(booleanType,     OperatorGet, VectorGet<1, Boolean>, OneParameter(this->IntegerType), this->BooleanType, FunctionOptions::None);
    builder.AddBoundFunction(boolean2Type,    OperatorGet, VectorGet<2, Boolean>, OneParameter(this->IntegerType), this->BooleanType, FunctionOptions::None);
    builder.AddBoundFunction(boolean3Type,    OperatorGet, VectorGet<3, Boolean>, OneParameter(this->IntegerType), this->BooleanType, FunctionOptions::None);
    builder.AddBoundFunction(boolean4Type,    OperatorGet, VectorGet<4, Boolean>, OneParameter(this->IntegerType), this->BooleanType, FunctionOptions::None);
    builder.AddBoundFunction(integerType,     OperatorGet, VectorGet<1, Integer>, OneParameter(this->IntegerType), this->IntegerType, FunctionOptions::None);
    builder.AddBoundFunction(integer2Type,    OperatorGet, VectorGet<2, Integer>, OneParameter(this->IntegerType), this->IntegerType, FunctionOptions::None);
    builder.AddBoundFunction(integer3Type,    OperatorGet, VectorGet<3, Integer>, OneParameter(this->IntegerType), this->IntegerType, FunctionOptions::None);
    builder.AddBoundFunction(integer4Type,    OperatorGet, VectorGet<4, Integer>, OneParameter(this->IntegerType), this->IntegerType, FunctionOptions::None);
    builder.AddBoundFunction(realType,        OperatorGet, VectorGet<1, Real>,    OneParameter(this->IntegerType), this->RealType,    FunctionOptions::None);
    builder.AddBoundFunction(real2Type,       OperatorGet, VectorGet<2, Real>,    OneParameter(this->IntegerType), this->RealType,    FunctionOptions::None);
    builder.AddBoundFunction(real3Type,       OperatorGet, VectorGet<3, Real>,    OneParameter(this->IntegerType), this->RealType,    FunctionOptions::None);
    builder.AddBoundFunction(real4Type,       OperatorGet, VectorGet<4, Real>,    OneParameter(this->IntegerType), this->RealType,    FunctionOptions::None);
    builder.AddBoundFunction(quaternionType,  OperatorGet, VectorGet<4, Real>,    OneParameter(this->IntegerType), this->RealType,    FunctionOptions::None);
    
    // Bind the set functions for vectors (indexing)
    builder.AddBoundFunction(booleanType,     OperatorSet, VectorSet<1, Boolean>, TwoParameters(this->IntegerType, this->BooleanType),  this->VoidType, FunctionOptions::None);
    builder.AddBoundFunction(boolean2Type,    OperatorSet, VectorSet<2, Boolean>, TwoParameters(this->IntegerType, this->BooleanType),  this->VoidType, FunctionOptions::None);
    builder.AddBoundFunction(boolean3Type,    OperatorSet, VectorSet<3, Boolean>, TwoParameters(this->IntegerType, this->BooleanType),  this->VoidType, FunctionOptions::None);
    builder.AddBoundFunction(boolean4Type,    OperatorSet, VectorSet<4, Boolean>, TwoParameters(this->IntegerType, this->BooleanType),  this->VoidType, FunctionOptions::None);
    builder.AddBoundFunction(integerType,     OperatorSet, VectorSet<1, Integer>, TwoParameters(this->IntegerType, this->IntegerType),  this->VoidType, FunctionOptions::None);
    builder.AddBoundFunction(integer2Type,    OperatorSet, VectorSet<2, Integer>, TwoParameters(this->IntegerType, this->IntegerType),  this->VoidType, FunctionOptions::None);
    builder.AddBoundFunction(integer3Type,    OperatorSet, VectorSet<3, Integer>, TwoParameters(this->IntegerType, this->IntegerType),  this->VoidType, FunctionOptions::None);
    builder.AddBoundFunction(integer4Type,    OperatorSet, VectorSet<4, Integer>, TwoParameters(this->IntegerType, this->IntegerType),  this->VoidType, FunctionOptions::None);
    builder.AddBoundFunction(realType,        OperatorSet, VectorSet<1, Real>,    TwoParameters(this->IntegerType, this->RealType),     this->VoidType, FunctionOptions::None);
    builder.AddBoundFunction(real2Type,       OperatorSet, VectorSet<2, Real>,    TwoParameters(this->IntegerType, this->RealType),     this->VoidType, FunctionOptions::None);
    builder.AddBoundFunction(real3Type,       OperatorSet, VectorSet<3, Real>,    TwoParameters(this->IntegerType, this->RealType),     this->VoidType, FunctionOptions::None);
    builder.AddBoundFunction(real4Type,       OperatorSet, VectorSet<4, Real>,    TwoParameters(this->IntegerType, this->RealType),     this->VoidType, FunctionOptions::None);
    builder.AddBoundFunction(quaternionType,  OperatorSet, VectorSet<4, Real>,    TwoParameters(this->IntegerType, this->RealType),     this->VoidType, FunctionOptions::None);

    // The names of the axes for each index
    const char* axes[4] = {"XAxis", "YAxis", "ZAxis", "WAxis"};
    // Setup some generic functions for all vector types
    for(size_t typeIndex = 0; typeIndex < VectorScalarTypes::Size; ++typeIndex)
    {
      // The Vector1/2/3/4 types for the current scalar type
      BoundType** vectorTypes = this->VectorTypes[typeIndex];

      for(size_t dimension = 0; dimension < MaxComponents; ++dimension)
      {
        // Don't forget to +1 the dimension
        VectorUserData userData(dimension + 1, typeIndex);

        BoundType* vectorType = vectorTypes[dimension];
        Property* prop = nullptr;
        Function* fn = nullptr;

        // Add a get property for the number of elements in the vector
        prop = builder.AddBoundGetterSetter(vectorType, "Count", integerType, nullptr, VectorCount, FunctionOptions::Static);
        *(size_t*)(&prop->Get->UserData) = userData.Count;
        prop->Description = ZilchDocumentString("The number of elements in the vector.");

        // Add a method to get an axis by index
        fn = builder.AddBoundFunction(vectorType, "GetAxis", VectorGetAxis, OneParameter(integerType), vectorType, FunctionOptions::Static);
        fn->ComplexUserData.WriteObject(userData);
        fn->Description = ZilchDocumentString("Returns an axis vector from the given index (ie. 0 is XAxis, 1 is YAxis, etc...");
       
        // Add a property for the zero vector
        prop = builder.AddBoundGetterSetter(vectorType, "Zero", vectorType, nullptr, VectorZeroFunction, FunctionOptions::Static);
        prop->Get->ComplexUserData.WriteObject(userData);
        prop->Description = ZilchDocumentString("The zero vector (a vector containing all zeroes).");

        // Add a property for each axis (e.g. Real3.XAxis, Real3.YAxis, etc...)
        for(size_t axis = 0; axis <= dimension; ++axis)
        {
          prop = builder.AddBoundGetterSetter(vectorType, axes[axis], vectorType, nullptr, VectorAxisFunction, FunctionOptions::Static);
          prop->Get->UserData = (void*)axis;
          prop->Get->ComplexUserData.WriteObject(userData); 
        }

        // Simple helper macro to make binding the below splats easier
      #define ZilchNoParameterSplat(builder, type, name, scalarType, fn, count, description)            \
        {                                                                                               \
          BoundFn boundFn = FullNoParameterSplatAs<scalarType, fn>;                                     \
          prop = builder.AddBoundGetterSetter(type, name, type, nullptr, boundFn, FunctionOptions::Static); \
          prop->Get->UserData = (void*)count;                                                           \
          prop->Description = ZilchDocumentString(description);                                         \
        }

        // Add splats for the extremal values for types that matter (Real and Integer)
        if(typeIndex == VectorScalarTypes::Real)
        {
          ZilchNoParameterSplat(builder, vectorType, "PositiveMax", Real, ZilchRealPositiveMax, userData.Count, "The largest (most positive) value that can be represented by a Real.");
          ZilchNoParameterSplat(builder, vectorType, "PositiveValueClosestToZero", Real, ZilchRealPositiveValueClosestToZero, userData.Count, "The positive value closest to zero that can be represented by a Real.");
          ZilchNoParameterSplat(builder, vectorType, "NegativeValueClosestToZero", Real, ZilchRealNegativeValueClosestToZero, userData.Count, "The negative value closest to zero that can be represented by a Real.");
          ZilchNoParameterSplat(builder, vectorType, "NegativeMin", Real, ZilchRealNegativeMin, userData.Count, "The smallest (most negative) value that can be represented by a Real.");
        }
        else if(typeIndex == VectorScalarTypes::Integer)
        {
          ZilchNoParameterSplat(builder, vectorType, "PositiveMax", Integer, ZilchIntegerPositiveMax, userData.Count, "The largest (most positive) value that can be represented by an Integer.");
          ZilchNoParameterSplat(builder, vectorType, "PositiveValueClosestToZero", Integer, ZilchIntegerPositiveValueClosestToZero, userData.Count, "The positive value closest to zero that can be represented by an Integer.");
          ZilchNoParameterSplat(builder, vectorType, "NegativeValueClosestToZero", Integer, ZilchIntegerNegativeValueClosestToZero, userData.Count, "The negative value closest to zero that can be represented by an Integer.");
          ZilchNoParameterSplat(builder, vectorType, "NegativeMin", Integer, ZilchIntegerNegativeMin, userData.Count, "The smallest (most negative) value that can be represented by an Integer.");
        }
      #undef ZilchNoParameterSplat

      }
    }

    // Quaternion static bindings
    {
      Property* prop = nullptr;
      Function* fn = nullptr;

      // Add a get property for the number of elements in the vector
      prop = builder.AddBoundGetterSetter(quaternionType, "Count", integerType, nullptr, VectorCount, FunctionOptions::Static);
      *(size_t*)(&prop->Get->UserData) = 4;
      prop->Description = ZilchDocumentString("The number of elements in the quaternion.");
    }

    FunctionOptions::Enum options = FunctionOptions::Static;

    //ZilchFullBindMethod(builder, math, (Real3 (*)(Real3Param, Real3Param, Real)) &Math::RotateVector
    ZilchFullBindMethod(builder, math, &Math::RotateVector, ZilchNoOverload, "RotateVector", "vector, axis, radians")
      ->Description = ZilchDocumentString("Rotate a vector about an axis by the given radians.");

    ZilchFullBindMethod(builder, math, &Math::AngleBetween, (Real (*)(Real2Param, Real2Param)), "AngleBetween", ZilchNoNames)
      ->Description = ZilchDocumentString("Returns the angle between two Real2s in radians.");
    ZilchFullBindMethod(builder, math, &Math::AngleBetween, (Real (*)(Real3Param, Real3Param)), "AngleBetween", ZilchNoNames)
      ->Description = ZilchDocumentString("Returns the angle between two Real3s in radians.");
    ZilchFullBindMethod(builder, math, &Math::AngleBetween, (Real(*)(Real4Param, Real4Param)), "AngleBetween", ZilchNoNames)
      ->Description = ZilchDocumentString("Returns the angle between two Real4s in radians.");

    ZilchFullBindMethod(builder, math, &Math::AngleBetween, (Real (*)(QuaternionParam, QuaternionParam)), "AngleBetween", ZilchNoNames)
      ->Description = ZilchDocumentString("Returns the angle between two Quaternions in radians.");

    ZilchFullBindMethod(builder, math, &Math::SafeSlerp, (Real2       (*)(Real2Param,       Real2Param,       Real)), "Slerp", "start, end, t")
      ->Description = ZilchDocumentString("Spherical linear interpolation. Used to interpolate between two vectors by the parameter t.");
    ZilchFullBindMethod(builder, math, &Math::SafeSlerp, (Real3       (*)(Real3Param,       Real3Param,       Real)), "Slerp", "start, end, t")
      ->Description = ZilchDocumentString("Spherical linear interpolation. Used to interpolate between two vectors by the parameter t.");
    ZilchFullBindMethod(builder, math, &Math::Slerp,     (Quaternion  (*)(QuaternionParam,  QuaternionParam,  Real)), "Slerp", "start, end, t")
      ->Description = ZilchDocumentString("Spherical linear interpolation. Used to interpolate between two rotations by the parameter t.");

    ZilchFullBindMethod(builder, math, &Math::SafeRotateTowards, (Real2 (*)(Real2Param, Real2Param, Real)), "RotateTowards", "p0, p1, maxRadians")
      ->Description = ZilchDocumentString("Rotate a vector towards another vector changing at most maxRadians.");

    ZilchFullBindMethod(builder, math, &Math::SafeRotateTowards, (Real3 (*)(Real3Param, Real3Param, Real)), "RotateTowards", "p0, p1, maxRadians")
      ->Description = ZilchDocumentString("Rotate a vector towards another vector changing at most maxRadians.");

    ZilchFullBindMethod(builder, math, &Math::RotateTowards, (Quaternion (*)(QuaternionParam, QuaternionParam, Real)), "RotateTowards", "p0, p1, maxRadians")
      ->Description = ZilchDocumentString("Rotate a quaternion towards another quaternion changing at most maxRadians.");

    ZilchFullBindMethod(builder, math, &Math::SignedAngle, ZilchNoOverload, "SignedAngle", "p0, p1, up")
      ->Description = ZilchDocumentString("Get the rotation angle between two vectors in radians.");

    ZilchFullBindMethod(builder, math, &Math::Angle2D, ZilchNoOverload, "Angle2D", ZilchNoNames)
      ->Description = ZilchDocumentString("Computes the angle (in radians) about the z-axis between the vector and the x-axis.");

    ZilchFullBindMethod(builder, math, &Math::ProjectOnVector, (Real2(*)(Real2Param, Real2Param)), "ProjectOnVector", "toBeProjected, normalizedVector")
      ->Description = ZilchDocumentString("Projects the input vector onto the given normalized vector.");
    ZilchFullBindMethod(builder, math, &Math::ProjectOnVector, (Real3(*)(Real3Param, Real3Param)), "ProjectOnVector", "toBeProjected, normalizedVector")
      ->Description = ZilchDocumentString("Projects the input vector onto the given normalized vector.");
    ZilchFullBindMethod(builder, math, &Math::ProjectOnVector, (Real4(*)(Real4Param, Real4Param)), "ProjectOnVector", "toBeProjected, normalizedVector")
      ->Description = ZilchDocumentString("Projects the input vector onto the given normalized vector.");
    // Legacy project function (mostly to not break things like the swept controller)
    ZilchFullBindMethod(builder, math, &Math::ProjectOnVector, (Real3(*)(Real3Param, Real3Param)), "Project", "toBeProjected, normalizedVector")
      ->Description = ZilchDocumentString("Projects the input vector onto the given normalized vector. Note: This function is legacy. Instead call ProjectOnVector.");

    ZilchFullBindMethod(builder, math, &Math::ProjectOnPlane, (Real2(*)(Real2Param, Real2Param)), "ProjectOnPlane", "toBeProjected, planeNormal")
      ->Description = ZilchDocumentString("Projects the input vector onto plane defined by the given normal.");
    ZilchFullBindMethod(builder, math, &Math::ProjectOnPlane, (Real3(*)(Real3Param, Real3Param)), "ProjectOnPlane", "toBeProjected, planeNormal")
      ->Description = ZilchDocumentString("Projects the input vector onto plane defined by the given normal.");
    ZilchFullBindMethod(builder, math, &Math::ProjectOnPlane, (Real4(*)(Real4Param, Real4Param)), "ProjectOnPlane", "toBeProjected, planeNormal")
      ->Description = ZilchDocumentString("Projects the input vector onto plane defined by the given normal.");

    ZilchFullBindMethod(builder, math, &Math::ReflectAcrossPlane, (Real2(*)(Real2Param, Real2Param)), "ReflectAcrossPlane", "toBeReflected, planeNormal")
      ->Description = ZilchDocumentString("Reflects the input vector across the plane defined by the given normal.");
    ZilchFullBindMethod(builder, math, &Math::ReflectAcrossPlane, (Real3(*)(Real3Param, Real3Param)), "ReflectAcrossPlane", "toBeReflected, planeNormal")
      ->Description = ZilchDocumentString("Reflects the input vector across the plane defined by the given normal.");
    ZilchFullBindMethod(builder, math, &Math::ReflectAcrossPlane, (Real4(*)(Real4Param, Real4Param)), "ReflectAcrossPlane", "toBeReflected, planeNormal")
      ->Description = ZilchDocumentString("Reflects the input vector across the plane defined by the given normal.");

    ZilchFullBindMethod(builder, math, &Math::ReflectAcrossVector, (Real2(*)(Real2Param, Real2Param)), "ReflectAcrossVector", "toBeReflected, vector")
      ->Description = ZilchDocumentString("Reflects the input vector across the given vector.");
    ZilchFullBindMethod(builder, math, &Math::ReflectAcrossVector, (Real3(*)(Real3Param, Real3Param)), "ReflectAcrossVector", "toBeReflected, vector")
      ->Description = ZilchDocumentString("Reflects the input vector across the given vector.");
    ZilchFullBindMethod(builder, math, &Math::ReflectAcrossVector, (Real4(*)(Real4Param, Real4Param)), "ReflectAcrossVector", "toBeReflected, vector")
      ->Description = ZilchDocumentString("Reflects the input vector across the given vector.");

    ZilchFullBindMethod(builder, math, &Math::Refract, (Real2(*)(Real2Param, Real2Param, Real)), "Refract", "toBeRefracted, planeNormal, refractionIndex")
      ->Description = ZilchDocumentString("Calculates the refraction vector through a plane given a certain index of refraction.");
    ZilchFullBindMethod(builder, math, &Math::Refract, (Real3(*)(Real3Param, Real3Param, Real)), "Refract", "toBeRefracted, planeNormal, refractionIndex")
      ->Description = ZilchDocumentString("Calculates the refraction vector through a plane given a certain index of refraction.");
    ZilchFullBindMethod(builder, math, &Math::Refract, (Real4(*)(Real4Param, Real4Param, Real)), "Refract", "toBeRefracted, planeNormal, refractionIndex")
      ->Description = ZilchDocumentString("Calculates the refraction vector through a plane given a certain index of refraction.");

    // Lots of quaternion construction functions
    ZilchFullBindMethod(builder, math, &Math::ToQuaternion, (Quaternion (*)(Real3Param, Real)), "ToQuaternion", "axis, radians")
      ->Description = ZilchDocumentString("Generates the quaternion that rotates about the axis vector by the given radians.");
    ZilchFullBindMethod(builder, math, &Math::ToQuaternion, (Quaternion (*)(Real3Param, Real)), "AxisAngle",    "axis, radians")
      ->Description = ZilchDocumentString("Generates the quaternion that rotates about the axis vector by the given radians.");
    ZilchFullBindMethod(builder, math, &Math::ToQuaternion, (Quaternion (*)(Real3Param, Real3Param)), "ToQuaternion", "facing, up")
      ->Description = ZilchDocumentString("Generates the orientation represented by the given facing and up vectors.");
    ZilchFullBindMethod(builder, math, &Math::ToQuaternion, (Quaternion (*)(Real3Param, Real3Param, Real3Param)), "ToQuaternion", "facing, up, right")
      ->Description = ZilchDocumentString("Generates the orientation represented by the given facing, up, and right vectors.");
    ZilchFullBindMethod(builder, math, &Math::ToQuaternion, (Quaternion (*)(Real, Real, Real)), "ToQuaternion", "xRadians, yRadians, zRadians")
      ->Description = ZilchDocumentString("Generates the orientation from the given Euler angles.");
    ZilchFullBindMethod(builder, math, &Math::ToQuaternion, (Quaternion (*)(Real3Param)), "ToQuaternion", "eulerRadians")
      ->Description = ZilchDocumentString("Generates the orientation from the given Euler angle vector");
    ZilchFullBindMethod(builder, math, &Math::ToQuaternion, (Quaternion (*)(Real3Param)), "Euler",         "eulerRadians")
      ->Description = ZilchDocumentString("Generates the orientation from the given Euler angle vector");
    ZilchFullBindMethod(builder, math, &Math::ToQuaternion, (Quaternion(*)(Real3x3Param)), "ToQuaternion", "rotationMatrix")
      ->Description = ZilchDocumentString("Converts a rotation matrix into a quaternion.");
    ZilchFullBindMethod(builder, math, &Math::RotationQuaternionBetween, ZilchNoOverload, "RotationQuaternionBetween", "start, end")
      ->Description = ZilchDocumentString("Generates the quaternion that rotates from parameter 1 to parameter 2.");

    // Conversion to Real3x3 from various rotation formats
    ZilchFullBindMethod(builder, math, &Math::ToMatrix3, (Real3x3 (*)(Real3Param, Real)), "ToReal3x3", "axis, radians")
      ->Description = ZilchDocumentString("Generates the three dimensional rotation matrix that rotates about 'axis' by 'radians'.");
    ZilchFullBindMethod(builder, math, &Math::ToMatrix3, (Real3x3(*)(Real3Param, Real3Param)), "ToReal3x3", "facing, up")
      ->Description = ZilchDocumentString("Generates the orientation represented by the given facing and up vectors.");
    ZilchFullBindMethod(builder, math, &Math::ToMatrix3, (Real3x3(*)(Real3Param, Real3Param, Real3Param)), "ToReal3x3", "facing, up, right")
      ->Description = ZilchDocumentString("Generates the orientation represented by the given facing, up, and right vectors.");
    ZilchFullBindMethod(builder, math, &Math::ToMatrix3, (Real3x3(*)(Real, Real, Real)), "ToReal3x3", "xRadians, yRadians, zRadians")
      ->Description = ZilchDocumentString("Generates the orientation from the given Euler angles.");
    ZilchFullBindMethod(builder, math, &Math::ToMatrix3, (Real3x3(*)(QuaternionParam)), "ToReal3x3", "rotation")
      ->Description = ZilchDocumentString("Converts a quaternion into a rotation matrix.");

    builder.AddBoundFunction(math, "Dot", VectorDotProduct<2>, TwoParameters(this->Real2Type), this->RealType, options)->Description = ZilchDocumentString("The vector dot product");
    builder.AddBoundFunction(math, "Dot", VectorDotProduct<3>, TwoParameters(this->Real3Type), this->RealType, options)->Description = ZilchDocumentString("The vector dot product");
    builder.AddBoundFunction(math, "Dot", VectorDotProduct<4>, TwoParameters(this->Real4Type), this->RealType, options)->Description = ZilchDocumentString("The vector dot product");
    builder.AddBoundFunction(math, "Dot", VectorDotProduct<4>, TwoParameters(this->QuaternionType), this->RealType, options)->Description = ZilchDocumentString("The vector dot product");

    builder.AddBoundFunction(math, "Cross",   Vector3CrossProduct, TwoParameters(this->Real3Type), this->Real3Type, options)->Description = ZilchDocumentString("The vector cross product. Creates a new vector perpendicular to p0 and p1 using the right hand rule.");
    ZilchFullBindMethod(builder, math, &Math::Cross, (float (*)(Real2Param, Real2Param)), "Cross", ZilchNoNames)
      ->Description = ZilchDocumentString("2D cross product. Equivalent to Cross(Real3(p0.x, p0.y, 0), Real3(p1.x, p1.y, 0)).");
    ZilchFullBindMethod(builder, math, &Math::Cross, (Real2(*)(float, Real2Param)), "Cross", ZilchNoNames)
      ->Description = ZilchDocumentString("2D cross product. Equivalent to Cross(Real3(0, 0, p0), Real3(p1.x, p1.y, 0)).");
    ZilchFullBindMethod(builder, math, &Math::Cross, (Real2(*)(Real2Param, float)), "Cross", ZilchNoNames)
      ->Description = ZilchDocumentString("2D cross product. Equivalent to Cross(Real3(p0.x, p0.y, 0), Real3(0, 0, p1)).");

    builder.AddBoundFunction(math, "LengthSq", VectorLengthSq<2>, OneParameter(this->Real2Type), this->RealType, options)->Description = ZilchDocumentString("The squared length of the vector. Used to avoid a square root when possible.");
    builder.AddBoundFunction(math, "LengthSq", VectorLengthSq<3>, OneParameter(this->Real3Type), this->RealType, options)->Description = ZilchDocumentString("The squared length of the vector. Used to avoid a square root when possible.");
    builder.AddBoundFunction(math, "LengthSq", VectorLengthSq<4>, OneParameter(this->Real4Type), this->RealType, options)->Description = ZilchDocumentString("The squared length of the vector. Used to avoid a square root when possible.");
    builder.AddBoundFunction(math, "LengthSq", VectorLengthSq<4>, OneParameter(this->QuaternionType), this->RealType, options)->Description = ZilchDocumentString("The squared length of the vector. Used to avoid a square root when possible.");

    builder.AddBoundFunction(math, "Length", VectorLength<2>, OneParameter(this->Real2Type), this->RealType, options)->Description = ZilchDocumentString("The length of the vector.");
    builder.AddBoundFunction(math, "Length", VectorLength<3>, OneParameter(this->Real3Type), this->RealType, options)->Description = ZilchDocumentString("The length of the vector.");
    builder.AddBoundFunction(math, "Length", VectorLength<4>, OneParameter(this->Real4Type), this->RealType, options)->Description = ZilchDocumentString("The length of the vector.");
    builder.AddBoundFunction(math, "Length", VectorLength<4>, OneParameter(this->QuaternionType), this->RealType, options)->Description = ZilchDocumentString("The length of the vector.");

    builder.AddBoundFunction(math, "Distance", VectorDistance<2>, TwoParameters(this->Real2Type), this->RealType, options)->Description = ZilchDocumentString("Returns the distance between two points.");
    builder.AddBoundFunction(math, "Distance", VectorDistance<3>, TwoParameters(this->Real3Type), this->RealType, options)->Description = ZilchDocumentString("Returns the distance between two points.");
    builder.AddBoundFunction(math, "Distance", VectorDistance<4>, TwoParameters(this->Real4Type), this->RealType, options)->Description = ZilchDocumentString("Returns the distance between two points.");
    builder.AddBoundFunction(math, "DistanceSq", VectorDistanceSq<2>, TwoParameters(this->Real2Type), this->RealType, options)->Description = ZilchDocumentString("Returns the squared distance between two points.");
    builder.AddBoundFunction(math, "DistanceSq", VectorDistanceSq<3>, TwoParameters(this->Real3Type), this->RealType, options)->Description = ZilchDocumentString("Returns the squared distance between two points.");
    builder.AddBoundFunction(math, "DistanceSq", VectorDistanceSq<4>, TwoParameters(this->Real4Type), this->RealType, options)->Description = ZilchDocumentString("Returns the squared distance between two points.");

    builder.AddBoundFunction(math, "Normalize", VectorNormalize<2>, OneParameter(this->Real2Type), this->Real2Type, options)->Description = ZilchDocumentString("Returns a vector that points in the same direction but has a length of 1.");
    builder.AddBoundFunction(math, "Normalize", VectorNormalize<3>, OneParameter(this->Real3Type), this->Real3Type, options)->Description = ZilchDocumentString("Returns a vector that points in the same direction but has a length of 1.");
    builder.AddBoundFunction(math, "Normalize", VectorNormalize<4>, OneParameter(this->Real4Type), this->Real4Type, options)->Description = ZilchDocumentString("Returns a vector that points in the same direction but has a length of 1.");
    builder.AddBoundFunction(math, "Normalize", VectorNormalize<4>, OneParameter(this->QuaternionType), this->QuaternionType, options)->Description = ZilchDocumentString("Returns a unit quaternion that represents a pure rotation.");
    
    builder.AddBoundGetterSetter(math, "Pi", this->RealType, nullptr, Pi, MemberOptions::Static);
    builder.AddBoundGetterSetter(math, "E", this->RealType, nullptr, E, MemberOptions::Static)->Description = ZilchDocumentString("Euler's number.");

    builder.AddBoundFunction(math, "Multiply", QuaternionMultiplyQuaternion, TwoParameters(this->QuaternionType, "by", this->QuaternionType, "the"), this->QuaternionType, FunctionOptions::Static)->Description = ZilchDocumentString("Creates a new rotation that represents rotating by parameter 2 and then parameter 1.");
    builder.AddBoundFunction(math, "Multiply", QuaternionMultiplyVector3, TwoParameters(this->QuaternionType, "by", this->Real3Type, "the"), this->Real3Type, FunctionOptions::Static)->Description = ZilchDocumentString("Creates a new vector that represents parameter 2 being rotated by parameter 1.");
    builder.AddBoundFunction(math, "Invert", QuaternionInvert, OneParameter(this->QuaternionType), this->QuaternionType, options)->Description = ZilchDocumentString("Returns the inverse rotation.");

    builder.AddBoundGetterSetter(quaternionType, "Identity", this->QuaternionType, nullptr, QuaternionIdentity, MemberOptions::Static);

    CreateMatrixTypes(builder);

    for (size_t i = 0; i < AllRealTypes.Size(); ++i)
    {
      BoundType* boundType = AllRealTypes[i];
      BoundType* boundIntegerType = AllIntegerTypes[i];
      Function* f = nullptr;

      ZilchBindBasicSplat(builder, math, Real, realType, "Abs", Math::Abs, boundType, OneParameter(boundType), "Returns the absolute value of value.");
      ZilchBindBasicSplatWithError(builder, math, Real, realType, "ACos", Math::SafeArcCos, boundType, OneParameter(boundType, "units"),"The transcendental function arc-cosine", "ACos of '%s' is invalid. Values must be in the range [-1, 1].");
      f = builder.AddBoundFunction(math, "AllNonZero", AllNonZero, OneParameter(boundType), booleanType, FunctionOptions::Static);
      f->Description = ZilchDocumentString("Returns true if all values are not zero.");
      f->ComplexUserData.WriteObject(SplatWithErrorUserData(boundType->Size / realType->Size, nullptr, boundType));
      f = builder.AddBoundFunction(math, "AnyNonZero", AnyNonZero, OneParameter(boundType), booleanType, FunctionOptions::Static);
      f->Description = ZilchDocumentString("Returns true if any value is not zero.");
      f->ComplexUserData.WriteObject(SplatWithErrorUserData(boundType->Size / realType->Size, nullptr, boundType));
      ZilchBindBasicSplatWithError(builder, math, Real, realType, "ASin", Math::SafeArcSin, boundType, OneParameter(boundType, "units"), "The transcendental function arc-sine", "ASin of '%s' is invalid. Values must be in the range [-1, 1].");    
      ZilchBindBasicSplat(builder, math, Real, realType, "ATan", Math::ArcTan, boundType, OneParameter(boundType, "units"), "The transcendental function arc-tangent. The return type is in radians.");
      ZilchBindBasicTwoParamSplat(builder, math, Real, realType, "ATan2", Math::ArcTan2, boundType, TwoParameters(boundType, "y", "x"), "Performs the arc-tangent using the signs of x and y to determine what quadrant the angle lies in. Returns a value in the range of [-pi, pi]. The return type is in radians.");
      ZilchBindBasicSplat(builder, math, Real, realType, "Ceil", Math::Ceil, boundType, OneParameter(boundType), "Rounds value upward.");
      f = builder.AddBoundFunction(math, "Ceil", ZilchComplexTwoParameterSplatBinder(Real, Integer, Real, 1, 0, Math::Ceil),
        TwoParameters(boundType, "value", integerType, "places"), boundType, FunctionOptions::Static);
      ZilchSetUserDataAndDescription(f, boundType, realType, "Rounds value upward. The place represents where in the number we want to perform rounding (0 is the 1s place, 1 is the 10s place, -1 is the tenths place, etc).");
      f = builder.AddBoundFunction(math, "Ceil", ZilchComplexThreeParameterSplatBinder(Real, Integer, Integer, Real, 1, 0, 0, Math::Ceil),
        ThreeParameters(boundType, "value", integerType, "places", integerType, "numericalBase"), boundType, FunctionOptions::Static);
      ZilchSetUserDataAndDescription(f, boundType, realType, "Rounds value upward. The place represents where in the number we want to perform rounding (0 is the 1s place, 1 is the 10s place, -1 is the tenths place, etc).");
      
      ZilchBindBasicThreeParamSplat(builder, math, Real, realType, "Clamp", Math::Clamp<Real>, boundType, ThreeParameters(boundType, "value", "min", "max"), "Limits the value between the provided min and max.");
      ZilchBindBasicSplat(builder, math, Real, realType, "Cos", Math::Cos, boundType, OneParameter(boundType, "radians"), "The transcendental function cosine.");
      ZilchBindBasicSplat(builder, math, Real, realType, "Cosh", Math::Cosh, boundType, OneParameter(boundType, "radians"), "The hyperbolic cosine function.");
      ZilchBindBasicSplat(builder, math, Real, realType, "Exp", Math::Exp, boundType, OneParameter(boundType), "Returns the base-e exponentiation of value, which is e^value.");
      ZilchBindBasicSplat(builder, math, Real, realType, "Exp2", Math::Exp2, boundType, OneParameter(boundType), "Returns the base-2 exponentiation of value, which is 2^value.");
      ZilchBindBasicSplat(builder, math, Real, realType, "Floor", Math::Floor, boundType, OneParameter(boundType), "Rounds value downward.");

      f = builder.AddBoundFunction(math, "Floor", ZilchComplexTwoParameterSplatBinder(Real, Integer, Real, 1, 0, Math::Floor),
        TwoParameters(boundType, "value", integerType, "places"), boundType, FunctionOptions::Static);
      ZilchSetUserDataAndDescription(f, boundType, realType, "Rounds value downward. The place represents where in the number we want to perform rounding (0 is the 1s place, 1 is the 10s place, -1 is the tenths place, etc).");
      f = builder.AddBoundFunction(math, "Floor", ZilchComplexThreeParameterSplatBinder(Real, Integer, Integer, Real, 1, 0, 0, Math::Floor),
        ThreeParameters(boundType, "value", integerType, "places", integerType, "numericalBase"), boundType, FunctionOptions::Static);
      ZilchSetUserDataAndDescription(f, boundType, realType, "Rounds value downward. The place represents where in the number we want to perform rounding (0 is the 1s place, 1 is the 10s place, -1 is the tenths place, etc).");

      ZilchBindBasicTwoParamSplatWithError(builder, math, Real, realType, "FMod", Math::SafeFMod, boundType, TwoParameters(boundType, "numerator", "denominator"), "Returns the floating-point remainder of numerator/denominator (rounded towards zero).", "Fmod(%s, %s) is invalid because the denominator would produce a zero division");
      ZilchBindBasicSplat(builder, math, Real, realType, "Frac", Math::Fractional, boundType, OneParameter(boundType), "Returns the fractional part of value, a value between 0 and 1.");

      f = builder.AddBoundFunction(math, "Lerp", ZilchComplexThreeParameterSplatBinder(Real, Real, Real, Real, 1, 1, 1, Zilch::Lerp<Real>),
        ThreeParameters(boundType, "start", boundType, "end", boundType, "t"), boundType, FunctionOptions::Static);
      ZilchSetUserDataAndDescription(f, boundType, realType, "Linearly interpolates from start to end by the fraction t. T of 0 is start and t of 1 is end.");
      // Add another version for lerp that is always of real type
      if (boundType != realType)
      {
        f = builder.AddBoundFunction(math, "Lerp", ZilchComplexThreeParameterSplatBinder(Real, Real, Real, Real, 1, 1, 0, Zilch::Lerp<Real>),
          ThreeParameters(boundType, "start", boundType, "end", realType, "t"), boundType, FunctionOptions::Static);
        ZilchSetUserDataAndDescription(f, boundType, realType, "Linearly interpolates from start to end by the fraction t. T of 0 is start and t of 1 is end.");
      }

      ZilchBindBasicSplat(builder, math, Real, realType, "Log", Math::Log, boundType, OneParameter(boundType), "Base e logarithm.");
      ZilchBindBasicSplat(builder, math, Real, realType, "Log10", Math::Log10, boundType, OneParameter(boundType), "Base 10 logarithm.");
      ZilchBindBasicSplat(builder, math, Real, realType, "Log2", Math::Log2, boundType, OneParameter(boundType), "Base 2 logarithm.");
      ZilchBindBasicTwoParamSplat(builder, math, Real, realType, "Max", Math::Max<Real>, boundType, TwoParameters(boundType), "Returns whichever value is larger.");
      ZilchBindBasicTwoParamSplat(builder, math, Real, realType, "Min", Math::Min<Real>, boundType, TwoParameters(boundType), "Returns whichever value is smaller.");
      ZilchBindBasicTwoParamSplat(builder, math, Real, realType, "Pow", Math::Pow, boundType, TwoParameters(boundType, "base", "exponent"), "Returns base raised to the power of the exponent.");

      ZilchBindBasicSplat(builder, math, Real, realType, "Round", Math::Round, boundType, OneParameter(boundType), "Returns the integer value closest to value.");
      f = builder.AddBoundFunction(math, "Round", ZilchComplexTwoParameterSplatBinder(Real, Integer, Real, 1, 0, Math::Round),
        TwoParameters(boundType, "value", integerType, "places"), boundType, FunctionOptions::Static);
      ZilchSetUserDataAndDescription(f, boundType, realType, "Returns the integer value closest to value. The place represents where in the number we want to perform rounding (0 is the 1s place, 1 is the 10s place, -1 is the tenths place, etc).");
      f = builder.AddBoundFunction(math, "Round", ZilchComplexThreeParameterSplatBinder(Real, Integer, Integer, Real, 1, 0, 0, Math::Round),
        ThreeParameters(boundType, "value", integerType, "places", integerType, "numericalBase"), boundType, FunctionOptions::Static);
      ZilchSetUserDataAndDescription(f, boundType, realType, "Returns the integer value closest to value. The place represents where in the number we want to perform rounding (0 is the 1s place, 1 is the 10s place, -1 is the tenths place, etc).");
      
      ZilchBindBasicSplat(builder, math, Real, realType, "RSqrt", Math::Rsqrt, boundType, OneParameter(boundType), "Reciprocal square root approximation. Used for efficiency when higher accuracy is not need.");
      ZilchBindBasicSplat(builder, math, Real, realType, "Saturate", Math::Clamp<Real>, boundType, OneParameter(boundType), "Limits the value between 0 and 1");
      
      f = builder.AddBoundFunction(math, "Sign", ZilchComplexOneParameterSplatBinder(Real, Integer, 1, Math::Sign),
        OneParameter(boundType), boundIntegerType, FunctionOptions::Static);
      ZilchSetUserDataAndDescription(f, boundType, realType, "Returns the sign of the value as either 1 or -1.");

      ZilchBindBasicSplat(builder, math, Real, realType, "Sin", Math::Sin, boundType, OneParameter(boundType, "radians"), "The transcendental function sine.");
      ZilchBindBasicSplat(builder, math, Real, realType, "Sinh", Math::Sinh, boundType, OneParameter(boundType, "radians"), "The hyperbolic sine function.");

      f = builder.AddBoundFunction(math, "SmoothStep", ZilchFullThreeParameterSplatBinder(Real, Real, Real, Real, 1, 1, 1, Math::SmoothStep<Real>),
        ThreeParameters(boundType, "min", boundType, "max", boundType, "x"), boundType, FunctionOptions::Static);
      ZilchSetUserDataAndDescription(f, boundType, realType, "Returns a smooth Hermite interpolation between 0 and 1 if x is in-between min and max.");
      // Add another version for smoothstep that is always of real type
      if (boundType != realType)
      {
        f = builder.AddBoundFunction(math, "SmoothStep", ZilchFullThreeParameterSplatBinder(Real, Real, Real, Real, 1, 1, 0, Math::SmoothStep<Real>),
          ThreeParameters(boundType, "min", boundType, "max", realType, "t"), boundType, FunctionOptions::Static);
        ZilchSetUserDataAndDescription(f, boundType, realType, "Returns a smooth Hermite interpolation between 0 and 1 if t is in-between min and max.");
      }

      ZilchBindBasicSplatWithError(builder, math, Real, realType, "Sqrt", Math::SafeSqrt, boundType, OneParameter(boundType), "Computes the square root", "Sqrt of the negative number '%s' is invalid.");
      ZilchBindBasicTwoParamSplat(builder, math, Real, realType, "Step", Math::Step, boundType, TwoParameters(boundType, "y", "x"), "If y < x then 0 is returned, otherwise 1 is returned.")
      ZilchBindBasicSplat(builder, math, Real, realType, "Tan", Math::Tan, boundType, OneParameter(boundType, "radians"), "The transcendental function tangent.");
      ZilchBindBasicSplat(builder, math, Real, realType, "Tanh", Math::Tanh, boundType, OneParameter(boundType,  "radians"), "The hyperbolic tangent function.");
      ZilchBindBasicSplat(builder, math, Real, realType, "ToRadians", Math::DegToRad, boundType, OneParameter(boundType, "degrees"), "Converts the given degrees to radians.");
      ZilchBindBasicSplat(builder, math, Real, realType, "ToDegrees", Math::RadToDeg, boundType, OneParameter(boundType, "radians"), "Converts the given radians to degrees.");

      ZilchBindBasicSplat(builder, math, Real, realType, "Truncate", Math::Truncate, boundType, OneParameter(boundType), "Rounds value towards zero.");
      f = builder.AddBoundFunction(math, "Truncate", ZilchComplexTwoParameterSplatBinder(Real, Integer, Real, 1, 0, Math::Truncate),
        TwoParameters(boundType, "value", integerType, "places"), boundType, FunctionOptions::Static);
      ZilchSetUserDataAndDescription(f, boundType, realType, "Rounds value towards zero. The place represents where in the number we want to perform rounding (0 is the 1s place, 1 is the 10s place, -1 is the tenths place, etc).");
      f = builder.AddBoundFunction(math, "Truncate", ZilchComplexThreeParameterSplatBinder(Real, Integer, Integer, Real, 1, 0, 0, Math::Truncate),
        ThreeParameters(boundType, "value", integerType, "places", integerType, "numericalBase"), boundType, FunctionOptions::Static);
      ZilchSetUserDataAndDescription(f, boundType, realType, "Rounds value towards zero. The place represents where in the number we want to perform rounding (0 is the 1s place, 1 is the 10s place, -1 is the tenths place, etc).");

      // Approximately Equal
      BoundType* resultMatrixType = AllBooleanTypes[i];
      f = builder.AddBoundFunction(math, "ApproximatelyEqual", RealApproximatelyEqual, ThreeParameters(boundType, "p0", boundType, "p1", realType, "epsilon"), resultMatrixType, FunctionOptions::Static);
      ZilchSetUserDataAndDescription(f, boundType, realType, "Checks if the two values are within epsilon distance from each other.");
    }

    // Bind the integer functions separately (not many functions make sense to be splatted on integers)
    for (size_t i = 0; i < AllIntegerTypes.Size(); ++i)
    {
      BoundType* boundType = AllIntegerTypes[i];
      Function* f = nullptr;
      
      ZilchBindBasicSplat(builder, math, Integer, integerType, "Abs", Math::Abs, boundType, OneParameter(boundType), "Returns the absolute value of value.");
      f = builder.AddBoundFunction(math, "AllNonZero", AllNonZero, OneParameter(boundType), booleanType, FunctionOptions::Static);
      f->Description = ZilchDocumentString("Returns true if all values are not zero.");
      f->ComplexUserData.WriteObject(SplatWithErrorUserData(boundType->Size / integerType->Size, nullptr, boundType));
      f = builder.AddBoundFunction(math, "AnyNonZero", AnyNonZero, OneParameter(boundType), booleanType, FunctionOptions::Static);
      f->Description = ZilchDocumentString("Returns true if any value is not zero.");
      f->ComplexUserData.WriteObject(SplatWithErrorUserData(boundType->Size / integerType->Size, nullptr, boundType));
      ZilchBindBasicThreeParamSplat(builder, math, Integer, integerType, "Clamp", Math::Clamp<Integer>, boundType, ThreeParameters(boundType, "value", "min", "max"), "Limits the value between the provided min and max.");
      ZilchBindBasicSplat(builder, math, Integer, integerType, "CountBits", Math::CountBits, boundType, OneParameter(boundType), "Counts the number of bits set on the input.");
      ZilchBindBasicTwoParamSplat(builder, math, Integer, integerType, "Max", Math::Max<Integer>, boundType, TwoParameters(boundType), "Returns whichever value is larger.");
      ZilchBindBasicTwoParamSplat(builder, math, Integer, integerType, "Min", Math::Min<Integer>, boundType, TwoParameters(boundType), "Returns whichever value is smaller.");
      ZilchBindBasicSplat(builder, math, Integer, integerType, "Sign", Math::Sign, boundType, OneParameter(boundType), "Returns the sign of the value as either 1 or -1.");
    }

    // Bind the boolean functions
    for (size_t i = 0; i < AllBooleanTypes.Size(); ++i)
    {
      BoundType* boundType = AllBooleanTypes[i];
      Function* f = nullptr;

      f = builder.AddBoundFunction(math, "AllNonZero", AllNonZero, OneParameter(boundType), booleanType, FunctionOptions::Static);
      f->Description = ZilchDocumentString("Returns true if all values are true.");
      f->ComplexUserData.WriteObject(SplatWithErrorUserData(boundType->Size / booleanType->Size, nullptr, boundType));
      f = builder.AddBoundFunction(math, "AnyNonZero", AnyNonZero, OneParameter(boundType), booleanType, FunctionOptions::Static);
      f->Description = ZilchDocumentString("Returns true if any value is true.");
      f->ComplexUserData.WriteObject(SplatWithErrorUserData(boundType->Size / booleanType->Size, nullptr, boundType));
    }
  
    ZilchInitializeExternalTypeAs(Members::Enum, "Members");
    ZilchInitializeExternalTypeAs(FileMode::Enum, "FileMode");
    ZilchInitializeExternalTypeAs(StreamCapabilities::Enum, "StreamCapabilities");
    ZilchInitializeExternalTypeAs(StreamOrigin::Enum, "StreamOrigin");
    ZilchInitializeExternalType(ProcessStartInfo);

    ZilchInitializeTypeAs(Zilch::Library, "ZilchLibrary");
    ZilchInitializeType(ReflectionObject);
    ZilchInitializeType(Type);
    ZilchInitializeType(AnyType);
    ZilchInitializeType(IndirectionType);
    ZilchInitializeType(DelegateType);
    ZilchInitializeType(BoundType);
    ZilchInitializeType(DelegateParameter);
    ZilchInitializeType(Console);
    ZilchInitializeType(Exception);
    ZilchInitializeType(EventHandler);
    ZilchInitializeTypeAs(EventsClass, "Events");
    ZilchInitializeType(EventData);
    ZilchInitializeType(ErrorEvent);
    ZilchInitializeType(ExceptionEvent);
    ZilchInitializeType(ConsoleEvent);
    ZilchInitializeType(DebuggerEvent);
    ZilchInitializeType(MemoryLeakEvent);
    ZilchInitializeType(OpcodeEvent);
    ZilchInitializeType(FatalErrorEvent);
    ZilchInitializeType(BuildEvent);
    ZilchInitializeType(PluginEvent);
    ZilchInitializeType(ParseEvent);
    ZilchInitializeType(ExecutableState);
    ZilchInitializeTypeAs(FilePathClass, "FilePath");
    ZilchInitializeTypeAs(IStreamClass, "IStream");
    ZilchInitializeType(FileStream);
    ZilchInitializeType(Member);
    ZilchInitializeType(Function);
    ZilchInitializeType(Property);
    ZilchInitializeType(GetterSetter);
    ZilchInitializeType(Field);
    ZilchInitializeType(Variable);
    ZilchInitializeType(Random);
    ZilchInitializeType(IEncoding);
    ZilchInitializeType(AsciiEncoding);
    ZilchInitializeType(Utf8Encoding);
    ZilchInitializeTypeAs(StringBuilderExtended, "StringBuilder");
    ZilchInitializeType(Rune);
    ZilchInitializeType(RuneIterator);
    ZilchInitializeTypeAs(StringRangeExtended, "StringRange");
    ZilchInitializeTypeAs(StringSplitRangeExtended, "StringSplitRange");
    ZilchInitializeType(MultiPrimitive);
    ZilchInitializeType(Wrapper);

    // Add multi primitive type components
    //doubleIntegerType->Add(new MultiPrimitive(doubleIntegerType,  1));
    //doubleRealType   ->Add(new MultiPrimitive(doubleRealType,     1));
    //byteType         ->Add(new MultiPrimitive(byteType,           1));
    //booleanType      ->Add(new MultiPrimitive(booleanType,        1));
    //boolean2Type     ->Add(new MultiPrimitive(booleanType,        2));
    //boolean3Type     ->Add(new MultiPrimitive(booleanType,        3));
    //boolean4Type     ->Add(new MultiPrimitive(booleanType,        4));
    //integerType      ->Add(new MultiPrimitive(integerType,        1));
    //integer2Type     ->Add(new MultiPrimitive(integerType,        2));
    //integer3Type     ->Add(new MultiPrimitive(integerType,        3));
    //integer4Type     ->Add(new MultiPrimitive(integerType,        4));
    //realType         ->Add(new MultiPrimitive(realType,           1));
    //real2Type        ->Add(new MultiPrimitive(realType,           2));
    //real3Type        ->Add(new MultiPrimitive(realType,           3));
    //real4Type        ->Add(new MultiPrimitive(realType,           4));
    //quaternionType   ->Add(new MultiPrimitive(realType,           4));
    //real3x3Type      ->Add(new MultiPrimitive(realType,           9));
    //real4x4Type      ->Add(new MultiPrimitive(realType,          16));

    ZilchInitializeRangeAs(ParameterArray::range, "DelegateParameterRange");
    ZilchInitializeRangeAs(MemberRange<Member>, "MemberRange");
    ZilchInitializeRangeAs(MemberRange<Property>, "PropertyRange");
    ZilchInitializeRangeAs(MemberRange<GetterSetter>, "GetterSetterRange");
    ZilchInitializeRangeAs(MemberRange<Field>, "FieldRange");
    ZilchInitializeRangeAs(MemberRange<Function>, "FunctionRange");

    ZilchInitializeType(Zero::ProcessStartInfo);
    ZilchInitializeType(ProcessClass);

    forRange(BoundType* boundType, builder.BoundTypes.Values())
      boundType->AddAttribute(ExportDocumentation);
  }

  //***************************************************************************
  StackEntry::StackEntry() :
    ExecutingFunction(nullptr)
  {
  }

  //***************************************************************************
  StackEntry* StackTrace::GetMostRecentNonNativeStackEntry()
  {
    // Walk in newest from oldest to oldest calls
    for (int i = (int)(this->Stack.Size() - 1); i >= 0; --i)
    {
      // Grab the current stack entry
      StackEntry& entry = this->Stack[i];

      // If we reached a non-native location, then return it
      if (entry.Location.IsNative == false)
      {
        // Return the most recent entry
        return &entry;
      }
    }

    // Either our stack trace was empty or we had an entirely native stack, return nothing
    return nullptr;
  }

  //***************************************************************************
  CodeLocation StackTrace::GetMostRecentNonNativeLocation()
  {
    // Get the most recent stack entry
    StackEntry* entry = this->GetMostRecentNonNativeStackEntry();

    // If the stack entry doesn't exist, return an empty location
    if (entry == nullptr)
      return CodeLocation();

    // Otherwise, return the location at the last entry
    return entry->Location;
  }

  //***************************************************************************
  String StackTrace::GetFormattedMessage(MessageFormat::Enum format)
  {
    // Create a string builder to output the stack trace
    StringBuilder builder;

    // Walk in order from oldest to newest calls
    for (size_t i = 0; i < this->Stack.Size(); ++i)
    {
      // Get the current stack 
      StackEntry& stack = this->Stack[i];

      // Get the location in a formatted string
      String locationText = stack.Location.GetFormattedString(format);

      // Append the location to the full exception printout
      builder.Append(locationText);
      builder.Append('\n');
    }
    
    // Now return the full trace message
    return builder.ToString();
  }

  //***************************************************************************
  ZilchDefineType(Exception, builder, type)
  {
    // Store this type on the core library
    Core& core = Core::GetInstance();
    core.ExceptionType = type;
    
    ZilchFullBindDestructor(builder, type, Exception);
    ZilchFullBindConstructor(builder, type, Exception, nullptr);
    ZilchFullBindConstructor(builder, type, Exception, nullptr, StringParam);

    ZilchFullBindField(builder, type, &Exception::Message, "Message", PropertyBinding::GetSet);
    //ZilchFullBindMethod(GetFormattedMessage);
  }

  //***************************************************************************
  Exception::Exception()
  {
  }

  //***************************************************************************
  Exception::Exception(StringParam message) :
    Message(message)
  {
  }

  //***************************************************************************
  String Exception::GetFormattedMessage(MessageFormat::Enum format)
  {
    // Create a string builder to output the exception message and stack trace
    StringBuilder builder;
    builder.Append("********************** Zilch Exception **********************\n");

    // Convert the stack trace into a string and Append it to our output
    String traceOutput = this->Trace.GetFormattedMessage(format);
    builder.Append(traceOutput);

    // If we have an error...
    if (this->Message.Empty() == false)
    {
      // Append the error and a newline
      builder.Append(this->Message);
      builder.Append("\n");
    }

    // Print the end of the exception
    builder.Append("*************************************************************\n");

    // Now return the full error message
    return builder.ToString();
  }
}
