/**************************************************************\
* Author: Joshua Davis
* Copyright 2014, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  // Helper functions to Index into a matrix while allowing them to be row or column basis
  byte* IndexIntoMatrix(byte* memory, size_t indexX, size_t indexY, size_t sizeX, size_t sizeY, size_t elementSize)
  {
#if ColumnBasis == 1
    return memory + (indexX + indexY * sizeX) * elementSize;
#else
    return memory + (indexY + indexX * sizeY) * elementSize;
#endif
  }
  
  const byte* IndexIntoMatrix(const byte* memory, size_t indexX, size_t indexY, size_t sizeX, size_t sizeY, size_t elementSize)
  {
#if ColumnBasis == 1
    return memory + (indexX + indexY * sizeX) * elementSize;
#else
    return memory + (indexY + indexX * sizeY) * elementSize;
#endif
  }
  
  //***************************************************************************
  bool ValidateMatrixIndices(size_t x, size_t y, size_t sizeX, size_t sizeY, Call& call, ExceptionReport& report)
  {
    // Make sure the indices are within range
    if (y >= sizeY)
    {
      call.GetState()->ThrowException(report, String::Format("The y index used to access a component of a matrix was out of range [0-%d]", sizeY - 1));
      return false;
    }
    if (x >= sizeX)
    {
      call.GetState()->ThrowException(report, String::Format("The x index used to access a component of a matrix was out of range [0-%d]", sizeX - 1));
      return false;
    }

    return true;
  }
  
  //***************************************************************************
  void MultiplyAddReal(byte* outData, byte* inputA, byte* inputB)
  {
    Real& out = *(Real*)outData;
    Real& a = *(Real*)inputA;
    Real& b = *(Real*)inputB;
    out += a * b;
  }
  
  //***************************************************************************
  void MultiplyAddInteger(byte* outData, byte* inputA, byte* inputB)
  {
    Integer& out = *(Integer*)outData;
    Integer& a = *(Integer*)inputA;
    Integer& b = *(Integer*)inputB;
    out += a * b;
  }
  
  //***************************************************************************
  void MultiplyAddBoolean(byte* outData, byte* inputA, byte* inputB)
  {
    Boolean& out = *(Boolean*)outData;
    Boolean& a = *(Boolean*)inputA;
    Boolean& b = *(Boolean*)inputB;
    out = out || (a && b);
  }
  
  //***************************************************************************
  String MatrixToString(const BoundType* type, const byte* data)
  {
    MatrixUserData& userData = type->ComplexUserData.ReadObject<MatrixUserData>(0);
    Core& core = Core::GetInstance();
    //get the type of the matrix (Real, etc...)
    BoundType* elementType = core.MatrixElementTypes[userData.ElementTypeIndex];

    StringBuilder builder;
    builder.Append("[");
    for (size_t y = 0; y < userData.SizeY; ++y)
    {
      builder.Append("(");
      for (size_t x = 0; x < userData.SizeX; ++x)
      {
        const byte* item = IndexIntoMatrix(data, x, y, userData.SizeX, userData.SizeY, elementType->Size);

        builder.Append(elementType->GenericToString(item));

        //don't add a comma after the last item
        if (x != userData.SizeX - 1)
          builder.Append(", ");
      }

      builder.Append(")");
      //don't add a comma after the last item
      if (y != userData.SizeY - 1)
        builder.Append(", ");
    }
    builder.Append("]");
    return builder.ToString();
  }
  
  //***************************************************************************
  void MatrixDefaultConstructor(Call& call, ExceptionReport& report)
  {
    MatrixUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixUserData>(0);
    Core& core = Core::GetInstance();
    BoundType* elementType = core.MatrixElementTypes[userData.ElementTypeIndex];

    // Get ourselves (the matrix)
    byte* memory = call.GetHandle(Call::This).Dereference();
    // Zero out the matrix memory
    memset(memory, 0, userData.SizeX * userData.SizeY * elementType->Size);
  }
  
  //***************************************************************************
  void MatrixConstructor(Call& call, ExceptionReport& report)
  {
    MatrixUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixUserData>(0);
    Core& core = Core::GetInstance();
    BoundType* elementType = core.MatrixElementTypes[userData.ElementTypeIndex];

    // Get ourselves (the matrix)
    byte* matrixData = call.GetHandle(Call::This).Dereference();
    byte* parameters = call.GetParametersUnchecked();
    for (size_t y = 0; y < userData.SizeY; ++y)
    {
      for (size_t x = 0; x < userData.SizeX; ++x)
      {
        byte* matrixItem = IndexIntoMatrix(matrixData, x, y, userData.SizeX, userData.SizeY, elementType->Size);
        byte* parameterItem = parameters + (x + y * userData.SizeX) * elementType->Size;
        
        memcpy(matrixItem, parameterItem, elementType->Size);
      }
    }
  }

  //***************************************************************************
  void MatrixSplatConstructor(Call& call, ExceptionReport& report)
  {
    MatrixUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixUserData>(0);
    Core& core = Core::GetInstance();
    BoundType* elementType = core.MatrixElementTypes[userData.ElementTypeIndex];

    // Get ourselves (the matrix)
    byte* matrixData = call.GetHandle(Call::This).Dereference();
    byte* parameter = call.GetParametersUnchecked();
    for(size_t y = 0; y < userData.SizeY; ++y)
    {
      for(size_t x = 0; x < userData.SizeX; ++x)
      {
        byte* matrixItem = IndexIntoMatrix(matrixData, x, y, userData.SizeX, userData.SizeY, elementType->Size);

        memcpy(matrixItem, parameter, elementType->Size);
      }
    }
  }
  
  //***************************************************************************
  void MatrixGet(Call& call, ExceptionReport& report)
  {
    call.DisableReturnChecks();
    MatrixUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixUserData>(0);

    // Read the index off the stack
    Integer indexY = call.Get<Integer>(0);
    Integer indexX = call.Get<Integer>(1);

    // Make sure the indices are correct
    if (ValidateMatrixIndices(indexX, indexY, userData.SizeX, userData.SizeY, call, report) == false)
      return;

    Core& core = Core::GetInstance();
    BoundType* elementType = core.MatrixElementTypes[userData.ElementTypeIndex];

    // Get ourselves (the matrix)
    byte* memory = call.GetHandle(Call::This).Dereference();

    // Index to the item we are getting and set the return to that
    byte* item = IndexIntoMatrix(memory, indexX, indexY, userData.SizeX, userData.SizeY, elementType->Size);
    byte* returnData = call.GetReturnUnchecked();
    elementType->GenericCopyConstruct(returnData, item);
  }
  
  //***************************************************************************
  void MatrixSet(Call& call, ExceptionReport& report)
  {
    MatrixUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixUserData>(0);

    // Read the index off the stack
    Integer indexY = call.Get<Integer>(0);
    Integer indexX = call.Get<Integer>(1);

    // Make sure the indices are correct
    if (ValidateMatrixIndices(indexX, indexY, userData.SizeX, userData.SizeY, call, report) == false)
      return;

    Core& core = Core::GetInstance();
    BoundType* elementType = core.MatrixElementTypes[userData.ElementTypeIndex];

    // Get ourselves (the matrix)
    byte* memory = call.GetHandle(Call::This).Dereference();

    // Index to the item in the matrix and set it to the passed in value
    byte* setData = call.GetParameterUnchecked(2);
    byte* item = IndexIntoMatrix(memory, indexX, indexY, userData.SizeX, userData.SizeY, elementType->Size);
    elementType->GenericCopyConstruct(item, setData);
  }

  //***************************************************************************
  void MatrixGetByIndex(Call& call, ExceptionReport& report)
  {
    call.DisableReturnChecks();
    MatrixUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixUserData>(0);

    // Read the index off the stack
    Integer index = call.Get<Integer>(0);

    size_t totalCount = userData.SizeX * userData.SizeY;
    if(index >= (Integer)totalCount)
    {
      call.GetState()->ThrowException(report, String::Format("The index used to access a component of a matrix was out of the range [0-%d]", totalCount - 1));
      return;
    }

    Core& core = Core::GetInstance();
    BoundType* elementType = core.MatrixElementTypes[userData.ElementTypeIndex];

    // Get ourselves (the matrix)
    byte* memory = call.GetHandle(Call::This).Dereference();

    // Index to the item we are getting and set the return to that
    byte* item = memory + index * elementType->Size;
    byte* returnData = call.GetReturnUnchecked();
    elementType->GenericCopyConstruct(returnData, item);
  }
  
  //***************************************************************************
  void MatrixSetByIndex(Call& call, ExceptionReport& report)
  {
    MatrixUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixUserData>(0);

    // Read the index off the stack
    Integer index = call.Get<Integer>(0);

    size_t totalCount = userData.SizeX * userData.SizeY;
    if(index >= (Integer)totalCount)
    {
      call.GetState()->ThrowException(report, String::Format("The index used to access a component of a matrix was out of the range [0-%d]", totalCount - 1));
      return;
    }

    Core& core = Core::GetInstance();
    BoundType* elementType = core.MatrixElementTypes[userData.ElementTypeIndex];

    // Get ourselves (the matrix)
    byte* memory = call.GetHandle(Call::This).Dereference();

    // Index to the item in the matrix and set it to the passed in value
    byte* setData = call.GetParameterUnchecked(1);
    byte* item = memory + index * elementType->Size;
    elementType->GenericCopyConstruct(item, setData);
  }
  
  //***************************************************************************
  void MatrixGetVector(Call& call, ExceptionReport& report)
  {
    call.DisableReturnChecks();
    MatrixUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixUserData>(0);

    // Read the index off the stack
    Integer indexY = call.Get<Integer>(0);

    if (indexY < 0 || indexY >= (Integer)userData.SizeY)
    {
      call.GetState()->ThrowException(report, String::Format("The y index used to access a component of a matrix was out of range [0-%d]", userData.SizeY - 1));
      return;
    }

    Core& core = Core::GetInstance();
    BoundType* elementType = core.MatrixElementTypes[userData.ElementTypeIndex];

    // Get ourselves (the matrix)
    byte* memory = call.GetHandle(Call::This).Dereference();
    byte* returnData = call.GetReturnUnchecked();

    for (size_t indexX = 0; indexX < userData.SizeX; ++indexX)
    {
      // Index to the item we are getting and set the return to that
      byte* matrixItem = IndexIntoMatrix(memory, indexX, indexY, userData.SizeX, userData.SizeY, elementType->Size);
      byte* returnItem = returnData + indexX * elementType->Size;

      memcpy(returnItem, matrixItem, elementType->Size);
    }
  }
  
  //***************************************************************************
  void MatrixSetVector(Call& call, ExceptionReport& report)
  {
    call.DisableReturnChecks();
    MatrixUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixUserData>(0);

    // Read the index off the stack
    Integer indexY = call.Get<Integer>(0);

    if (indexY < 0 || indexY >= (Integer)userData.SizeY)
    {
      call.GetState()->ThrowException(report, String::Format("The y index used to access a component of a matrix was out of range [0-%d]", userData.SizeY - 1));
      return;
    }

    Core& core = Core::GetInstance();
    BoundType* elementType = core.MatrixElementTypes[userData.ElementTypeIndex];

    // Get ourselves (the matrix)
    byte* memory = call.GetHandle(Call::This).Dereference();
    byte* vectorData = call.GetParameterUnchecked(1);

    for (size_t indexX = 0; indexX < userData.SizeX; ++indexX)
    {
      // Index to the item we are getting and set the return to that
      byte* vectorItem = vectorData + indexX * elementType->Size;
      byte* matrixItem = IndexIntoMatrix(memory, indexX, indexY, userData.SizeX, userData.SizeY, elementType->Size);
      
      memcpy(matrixItem, vectorItem, elementType->Size);
    }
  }
  
  //***************************************************************************
  void MatrixCount(Call& call, ExceptionReport& report)
  {
    MatrixUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixUserData>(0);
    call.Set(Call::Return, (Integer)(userData.SizeX * userData.SizeY));
  }

  //***************************************************************************
  void MatrixCountX(Call& call, ExceptionReport& report)
  {
    MatrixUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixUserData>(0);
    call.Set(Call::Return, (Integer)userData.SizeX);
  }
  
  //***************************************************************************
  void MatrixCountY(Call& call, ExceptionReport& report)
  {
    MatrixUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixUserData>(0);
    call.Set(Call::Return, (Integer)userData.SizeY);
  }
  
  //***************************************************************************
  void MatrixTranspose(Call& call, ExceptionReport& report)
  {
    call.DisableReturnChecks();
    MatrixUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixUserData>(0);
    Core& core = Core::GetInstance();
    BoundType* elementType = core.MatrixElementTypes[userData.ElementTypeIndex];
    
    byte* inMatrix = call.GetParameterUnchecked(0);
    byte* outMatrix = call.GetReturnUnchecked();

    // Swap the x and y elements
    for (size_t y = 0; y < userData.SizeY; ++y)
    {
      for (size_t x = 0; x < userData.SizeX; ++x)
      {
        byte* inputElement = IndexIntoMatrix(inMatrix, x, y, userData.SizeX, userData.SizeY, elementType->Size);
        byte* outputElement = IndexIntoMatrix(outMatrix, y, x, userData.SizeY, userData.SizeX, elementType->Size);

        elementType->GenericCopyConstruct(outputElement, inputElement);
      }
    }
  }
  
  //***************************************************************************
  // Only doing the determinant for RealNxN matrices
  // (by just calling the math library's matrix functions)
  template <typename MatrixType>
  void RealMatrixDeterminant(Call& call, ExceptionReport& report)
  {
    call.DisableReturnChecks();

    byte* inMatrix = call.GetParameterUnchecked(0);
    Real* output = (Real*)call.GetReturnUnchecked();

    MatrixType mat((Real*)inMatrix);
    *output = mat.Determinant();
  }

  //***************************************************************************
  // Only doing the determinant for RealNxN matrices
  // (by just calling the math library's matrix functions)
  template <typename MatrixType>
  void RealMatrixInverse(Call& call, ExceptionReport& report)
  {
    call.DisableReturnChecks();

    byte* inMatrix = call.GetParameterUnchecked(0);
    byte* output = call.GetReturnUnchecked();

    MatrixType mat((Real*)inMatrix);
    MatrixType result = mat.Inverted();
    memcpy(output, &result, sizeof(MatrixType));
  }
  
  //***************************************************************************
  void MatrixEqual(Call& call, ExceptionReport& report)
  {
    // This should eventually be the innards of operator== and return a bool matrix,
    // but for usage in unit tests this is implemented to verify results
    MatrixUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixUserData>(0);
    Core& core = Core::GetInstance();
    BoundType* elementType = core.MatrixElementTypes[userData.ElementTypeIndex];

    Handle& selfHandle = call.GetHandle(Call::This);
    byte* matrixA = (byte*)selfHandle.Dereference();
    byte* matrixB = call.GetParameterUnchecked(0);

    // See if all the elements in the matrices are equal
    bool IsEqual = true;
    for (size_t y = 0; y < userData.SizeY; ++y)
    {
      for (size_t x = 0; x < userData.SizeX; ++x)
      {
        byte* elementA = IndexIntoMatrix(matrixA, x, y, userData.SizeX, userData.SizeY, elementType->Size);
        byte* elementB = IndexIntoMatrix(matrixB, x, y, userData.SizeX, userData.SizeY, elementType->Size);

        bool result = memcmp(elementA, elementB, elementType->Size) == 0;
        IsEqual &= result;
      }
    }

    call.Set(Call::Return, IsEqual);
  }
  
  //***************************************************************************
  // Special user data for the transform function. This needs the
  // dimensionality of both matrices as well as the data type.
  class MatrixTransformUserData
  {
  public:
    MatrixTransformUserData(size_t matrix0SizeX, size_t matrix0SizeY,
                            size_t matrix1SizeX, size_t matrix1SizeY,
                            size_t elementTypeIndex) :
      Matrix0SizeX(matrix0SizeX),
      Matrix0SizeY(matrix0SizeY), 
      Matrix1SizeX(matrix1SizeX),
      Matrix1SizeY(matrix1SizeY), 
      ElementTypeIndex(elementTypeIndex)
    {
    }

    size_t Matrix0SizeX;
    size_t Matrix0SizeY;
    size_t Matrix1SizeX;
    size_t Matrix1SizeY;
    size_t ElementTypeIndex;
  };
  
  //***************************************************************************
  void MatrixMultiply(Call& call, ExceptionReport& report)
  {
    call.DisableReturnChecks();
    MatrixTransformUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixTransformUserData>(0);
    Core& core = Core::GetInstance();
    BoundType* elementType = core.MatrixElementTypes[userData.ElementTypeIndex];

    // We flip the matrix order in the function so it reads nicer (Transform(the, by))
    // but to do the math we need to flip them back to the right order
    byte* matrix0 = call.GetParameterUnchecked(0);
    byte* matrix1 = call.GetParameterUnchecked(1);
    byte* returnMatrix = call.GetReturnUnchecked();
    
    for (size_t matrix0Y = 0; matrix0Y < userData.Matrix0SizeY; ++matrix0Y)
    {
      for (size_t matrix1X = 0; matrix1X < userData.Matrix1SizeX; ++matrix1X)
      {
        byte* returnElement = IndexIntoMatrix(returnMatrix, matrix1X, matrix0Y, userData.Matrix1SizeX, userData.Matrix1SizeY, elementType->Size);
        // To properly accumulate the multiplications the initial value first needs to be zeroed out
        memset(returnElement, 0, elementType->Size);

        for (size_t matrix1Y = 0; matrix1Y < userData.Matrix1SizeY; ++matrix1Y)
        {
          // The x of matrix 0 and the y of matrix 1 are the
          // same (just make this variable for clarity)
          size_t matrix0X = matrix1Y;

          byte* matrix0Element = IndexIntoMatrix(matrix0, matrix0X, matrix0Y, userData.Matrix0SizeX, userData.Matrix0SizeY, elementType->Size);
          byte* matrix1Element = IndexIntoMatrix(matrix1, matrix1X, matrix1Y, userData.Matrix1SizeX, userData.Matrix1SizeY, elementType->Size);

          // We need to accumulate the multiplications of matrix0 and matrix1 but
          // we don't know what the inner type is or how to perform add or multiply,
          // so call a function that knows how to add our current type
          core.TypeMultiplyAddFunctions[userData.ElementTypeIndex](returnElement, matrix0Element, matrix1Element);
        }
      }
    }
  }
  
  //***************************************************************************
  // Hardcoded for reals (because I don't care to make it generic now...) (JoshD)
  void MatrixMultiplyPoint(Call& call, ExceptionReport& report)
  {
    call.DisableReturnChecks();
    MatrixTransformUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixTransformUserData>(0);
    Core& core = Core::GetInstance();
    BoundType* elementType = core.MatrixElementTypes[userData.ElementTypeIndex];

    // We flip the matrix order in the function so it reads nicer (Transform(the, by))
    // but to do the math we need to flip them back to the right order
    Real* matrix0 = (Real*)call.GetParameterUnchecked(0);
    Real* vector0 = (Real*)call.GetParameterUnchecked(1);
    Real* returnVector = (Real*)call.GetReturnUnchecked();

    size_t expandedVectorSize = userData.Matrix1SizeY + 1;
    Real* tempReturnVector = (Real*)alloca(elementType->Size * (userData.Matrix1SizeY + 1));

    for (size_t matrix0Y = 0; matrix0Y < userData.Matrix0SizeY; ++matrix0Y)
    {
      Real* returnElement = (Real*)IndexIntoMatrix((byte*)tempReturnVector, 0, matrix0Y, userData.Matrix1SizeX, userData.Matrix1SizeY, elementType->Size);
      // To properly accumulate the multiplications the initial value first needs to be zeroed out
      memset(returnElement, 0, elementType->Size);

      for (size_t vector0Y = 0; vector0Y < userData.Matrix1SizeY; ++vector0Y)
      {
        size_t matrix0X = vector0Y;

        Real* matrix0Element = (Real*)IndexIntoMatrix((byte*)matrix0, matrix0X, matrix0Y, userData.Matrix0SizeX, userData.Matrix0SizeY, elementType->Size);
        Real* vector0Element = (Real*)IndexIntoMatrix((byte*)vector0,        0, vector0Y, userData.Matrix1SizeX, userData.Matrix1SizeY, elementType->Size);
        
        *returnElement += (*matrix0Element) * (*vector0Element);
      }

      Real* matrix0Element = (Real*)IndexIntoMatrix((byte*)matrix0, userData.Matrix0SizeX - 1, matrix0Y, userData.Matrix0SizeX, userData.Matrix0SizeY, elementType->Size);
      *returnElement += *matrix0Element;
    }

    // Do the w division
    size_t lastElementIndex = expandedVectorSize - 1;
    for(size_t i = 0; i < expandedVectorSize - 1; ++i)
      returnVector[i] = tempReturnVector[i] / tempReturnVector[lastElementIndex];
  }

  //***************************************************************************
  // Hardcoded for reals (because I don't care to make it generic now...) (JoshD)
  void MatrixMultiplyPointNoDivide(Call& call, ExceptionReport& report)
  {
    call.DisableReturnChecks();
    MatrixTransformUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<MatrixTransformUserData>(0);
    Core& core = Core::GetInstance();
    BoundType* elementType = core.MatrixElementTypes[userData.ElementTypeIndex];

    // We flip the matrix order in the function so it reads nicer (Transform(the, by))
    // but to do the math we need to flip them back to the right order
    Real* matrix0 = (Real*)call.GetParameterUnchecked(0);
    Real* vector0 = (Real*)call.GetParameterUnchecked(1);
    Real* returnVector = (Real*)call.GetReturnUnchecked();

    size_t expandedVectorSize = userData.Matrix1SizeY + 1;
    Real* tempReturnVector = (Real*)alloca(elementType->Size * (userData.Matrix1SizeY + 1));

    for(size_t matrix0Y = 0; matrix0Y < userData.Matrix0SizeY; ++matrix0Y)
    {
      Real* returnElement = (Real*)IndexIntoMatrix((byte*)tempReturnVector, 0, matrix0Y, userData.Matrix1SizeX, userData.Matrix1SizeY, elementType->Size);
      // To properly accumulate the multiplications the initial value first needs to be zeroed out
      memset(returnElement, 0, elementType->Size);

      for(size_t vector0Y = 0; vector0Y < userData.Matrix1SizeY; ++vector0Y)
      {
        size_t matrix0X = vector0Y;

        Real* matrix0Element = (Real*)IndexIntoMatrix((byte*)matrix0, matrix0X, matrix0Y, userData.Matrix0SizeX, userData.Matrix0SizeY, elementType->Size);
        Real* vector0Element = (Real*)IndexIntoMatrix((byte*)vector0, 0, vector0Y, userData.Matrix1SizeX, userData.Matrix1SizeY, elementType->Size);

        *returnElement += (*matrix0Element) * (*vector0Element);
      }

      Real* matrix0Element = (Real*)IndexIntoMatrix((byte*)matrix0, userData.Matrix0SizeX - 1, matrix0Y, userData.Matrix0SizeX, userData.Matrix0SizeY, elementType->Size);
      *returnElement += *matrix0Element;
    }
  }
  
  //***************************************************************************
  void GenerateMatrixMembers(LibraryBuilder& builder, BoundType* type, MatrixUserData& matrixUserData)
  {
    Core& core = Core::GetInstance();
    BoundType* elementType = core.MatrixElementTypes[matrixUserData.ElementTypeIndex];

    char componentNames[] = {'0', '1', '2', '3'};

    for (size_t sizeY = 0; sizeY < matrixUserData.SizeY; ++sizeY)
    {
      for (size_t sizeX = 0; sizeX < matrixUserData.SizeX; ++sizeX)
      {
        StringBuilder nameBuilder;
        nameBuilder.Append("M");
        nameBuilder.Append(componentNames[sizeY]);
        nameBuilder.Append(componentNames[sizeX]);

        // Get the offset into the matrix structure for the current member (by offsetting from 0)
        size_t offset = (size_t)IndexIntoMatrix((byte*)nullptr, sizeX, sizeY, matrixUserData.SizeX, matrixUserData.SizeY, elementType->Size);
        builder.AddBoundField(type, nameBuilder.ToString(), elementType, offset, MemberOptions::None);
      }
    }
  }
  
  //***************************************************************************
  void CreateMatrixTypes(LibraryBuilder& builder)
  {
    Core& core = Core::GetInstance();
    // For now don't make 1xN or Nx1 matrices

    // Setup functions needed for Transform that'll tell us
    // how to perform the madd intrinsic on a matrix element
    core.TypeMultiplyAddFunctions[0] = MultiplyAddReal;
    core.TypeMultiplyAddFunctions[1] = MultiplyAddInteger;
    core.TypeMultiplyAddFunctions[2] = MultiplyAddBoolean;

    // Some later operations need to operate on different dimensions/typed
    // matrices so store them all locally here for a second pass
    BoundType* matrixTypes[Core::MaxMatrixElementTypes][Core::MaxMatrixComponents][Core::MaxMatrixComponents];

    for (size_t typeIndex = 0; typeIndex < Core::MaxMatrixElementTypes; ++typeIndex)
    {
      for (size_t sizeY = Core::MinMatrixComponents; sizeY <= Core::MaxMatrixComponents; ++sizeY)
      {
        for (size_t sizeX = Core::MinMatrixComponents; sizeX <= Core::MaxMatrixComponents; ++sizeX)
        {
          // The indices into the matrix types 3d array
          size_t indexX = sizeX - 1;
          size_t indexY = sizeY - 1;

          BoundType* elementType = core.MatrixElementTypes[typeIndex];

          // Build up the matrices's names (Real3x3, Integer3x3, etc...)
          StringBuilder typeNameBuilder;
          typeNameBuilder.Append(elementType->ToString());
          typeNameBuilder << sizeY;
          typeNameBuilder.Append('x');
          typeNameBuilder << sizeX;

          // Add the bound type for the matrix and store it for later use
          String fullyQualifiedName = typeNameBuilder.ToString();
          size_t matrixSize = elementType->Size * sizeX * sizeY;

          // BoundType created elsewhere for Real2x2/Real3x3/Real4x4 for ZeroZilch binding
          BoundType* matrixType = nullptr;
          if(sizeX == 2 && sizeY == 2 && elementType == ZilchTypeId(Real))
            matrixType = ZilchTypeId(Real2x2);
          else if (sizeX == 3 && sizeY == 3 && elementType == ZilchTypeId(Real))
            matrixType = ZilchTypeId(Real3x3);
          else if (sizeX == 4 && sizeY == 4 && elementType == ZilchTypeId(Real))
            matrixType = ZilchTypeId(Real4x4);
          else
            matrixType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, matrixSize);

          matrixTypes[typeIndex][indexY][indexX] = matrixType;

          // Create complex user data to store information needed to
          // generically handle matrix operations independently of size and type
          MatrixUserData matrixUserData(sizeX, sizeY, typeIndex);
          matrixType->ComplexUserData.WriteObject(matrixUserData);

          matrixType->ToStringFunction = MatrixToString;

          // Bind all of the functions that are only for the matrix with its own type
          Function* f = builder.AddBoundDefaultConstructor(matrixType, MatrixDefaultConstructor);
          f->ComplexUserData.WriteObject(matrixUserData);
          ParameterArray constructorParameters;
          for (size_t y = 0; y < sizeY; ++y)
          {
            for (size_t x = 0; x < sizeX; ++x)
            {
              DelegateParameter& param = constructorParameters.PushBack();
              param.Name = String::Format("m%d%d", y, x);
              param.ParameterType = elementType;
            }
          }
          f = builder.AddBoundConstructor(matrixType, MatrixConstructor, constructorParameters);
          f->ComplexUserData.WriteObject(matrixUserData);

          // Add a splat constructor (given 1 parameter set it across all elements)
          f = builder.AddBoundConstructor(matrixType, MatrixSplatConstructor, OneParameter(elementType));
          f->ComplexUserData.WriteObject(matrixUserData);

          f = builder.AddBoundFunction(matrixType, OperatorGet, MatrixGet, TwoParameters(core.IntegerType, "y", "x"), elementType, FunctionOptions::None);
          f->ComplexUserData.WriteObject(matrixUserData);
          f = builder.AddBoundFunction(matrixType, "GetByIndex", MatrixGetByIndex, OneParameter(core.IntegerType, "index"), elementType, FunctionOptions::None);
          f->ComplexUserData.WriteObject(matrixUserData);
          
          f = builder.AddBoundFunction(matrixType, OperatorSet, MatrixSet, ThreeParameters(core.IntegerType, "y", core.IntegerType, "x", elementType, "value"), core.VoidType, FunctionOptions::None);
          f->ComplexUserData.WriteObject(matrixUserData);
          f = builder.AddBoundFunction(matrixType, "SetByIndex", MatrixSetByIndex, TwoParameters(core.IntegerType, "index", elementType, "value"), core.VoidType, FunctionOptions::None);
          f->ComplexUserData.WriteObject(matrixUserData);
          f = builder.AddBoundFunction(matrixType, OperatorGet, MatrixGetVector, OneParameter(core.IntegerType, "y"), core.VectorTypes[typeIndex][indexX], FunctionOptions::None);
          f->ComplexUserData.WriteObject(matrixUserData);
          f = builder.AddBoundFunction(matrixType, OperatorSet, MatrixSetVector, TwoParameters(core.IntegerType, "y", core.VectorTypes[typeIndex][indexX], "value"), core.VoidType, FunctionOptions::None);
          f->ComplexUserData.WriteObject(matrixUserData);
          // Don't actually want to have this equal function bound, but for unit testing it can be nice
          //f = builder.AddBoundFunction(matrixType, "Equal", MatrixEqual, OneParameter(matrixType), core.BooleanType, FunctionOptions::None);
          //f->ComplexUserData.WriteObject(matrixUserData);

          // Bind properties
          Property* p = builder.AddBoundGetterSetter(matrixType, "Count", core.IntegerType, nullptr, MatrixCount, FunctionOptions::None);
          p->Get->ComplexUserData.WriteObject(matrixUserData);
          p = builder.AddBoundGetterSetter(matrixType, "Count", core.IntegerType, nullptr, MatrixCount, FunctionOptions::Static);
          p->Get->ComplexUserData.WriteObject(matrixUserData);
          p = builder.AddBoundGetterSetter(matrixType, "CountX", core.IntegerType, nullptr, MatrixCountX, FunctionOptions::None);
          p->Get->ComplexUserData.WriteObject(matrixUserData);
          p = builder.AddBoundGetterSetter(matrixType, "CountX", core.IntegerType, nullptr, MatrixCountX, FunctionOptions::Static);
          p->Get->ComplexUserData.WriteObject(matrixUserData);
          p = builder.AddBoundGetterSetter(matrixType, "CountY", core.IntegerType, nullptr, MatrixCountY, FunctionOptions::None);
          p->Get->ComplexUserData.WriteObject(matrixUserData);
          p = builder.AddBoundGetterSetter(matrixType, "CountY", core.IntegerType, nullptr, MatrixCountY, FunctionOptions::Static);
          p->Get->ComplexUserData.WriteObject(matrixUserData);

          // Generate all of the M00, M01, etc... members
          GenerateMatrixMembers(builder, matrixType, matrixUserData);
        }
      }
    }

    // Add the Real and Integer matrix types to an array of all of those types
    for (size_t sizeY = Core::MinMatrixComponents; sizeY <= Core::MaxMatrixComponents; ++sizeY)
    {
      for (size_t sizeX = Core::MinMatrixComponents; sizeX <= Core::MaxMatrixComponents; ++sizeX)
      {
        // Get the two matrix types (as a transpose can have different dimensions)
        BoundType* realMatrixType = matrixTypes[0][sizeY - 1][sizeX - 1];
        BoundType* integerMatrixType = matrixTypes[1][sizeY - 1][sizeX - 1];
        BoundType* booleanMatrixType = matrixTypes[2][sizeY - 1][sizeX - 1];

        core.AllRealTypes.PushBack(realMatrixType);
        core.AllIntegerTypes.PushBack(integerMatrixType);
        core.AllBooleanTypes.PushBack(booleanMatrixType);
      }
    }

    // Add the determinant functions (only for real because that's all hlsl has)
    String determinantDescription = ZilchDocumentString("Computes the determinant of the given matrix");
    Function* fn = builder.AddBoundFunction(core.MathType, "Determinant", RealMatrixDeterminant<Math::Matrix2>, OneParameter(matrixTypes[0][1][1]), core.RealType, FunctionOptions::Static);
    fn->Description = determinantDescription;
    fn = builder.AddBoundFunction(core.MathType, "Determinant", RealMatrixDeterminant<Math::Matrix3>, OneParameter(matrixTypes[0][2][2]), core.RealType, FunctionOptions::Static);
    fn->Description = determinantDescription;
    fn = builder.AddBoundFunction(core.MathType, "Determinant", RealMatrixDeterminant<Math::Matrix4>, OneParameter(matrixTypes[0][3][3]), core.RealType, FunctionOptions::Static);
    fn->Description = determinantDescription;

    // Add the inverse functions for only square real matrices
    String invertDescription = ZilchDocumentString("Computes the inverse of the given matrix if it exists. Undefined if the matrix is uninvertible");
    fn = builder.AddBoundFunction(core.MathType, "Invert", RealMatrixInverse<Math::Matrix2>, OneParameter(matrixTypes[0][1][1]), matrixTypes[0][1][1], FunctionOptions::Static);
    fn->Description = invertDescription;
    fn = builder.AddBoundFunction(core.MathType, "Invert", RealMatrixInverse<Math::Matrix3>, OneParameter(matrixTypes[0][2][2]), matrixTypes[0][2][2], FunctionOptions::Static);
    fn->Description = invertDescription;
    fn = builder.AddBoundFunction(core.MathType, "Invert", RealMatrixInverse<Math::Matrix4>, OneParameter(matrixTypes[0][3][3]), matrixTypes[0][3][3], FunctionOptions::Static);
    fn->Description = invertDescription;

    // Operations on one matrix that need to reference different matrix
    // types (only need a loop over x and y dimensions plus types)
    String transposeDescription = ZilchDocumentString("Returns the transposed matrix. A transposed matrix is one where all rows are turned into columns, i.e. A^T[j][i] = A[i][j]");
    for (size_t typeIndex = 0; typeIndex < Core::MaxMatrixElementTypes; ++typeIndex)
    {
      for (size_t sizeY = Core::MinMatrixComponents; sizeY <= Core::MaxMatrixComponents; ++sizeY)
      {
        for (size_t sizeX = Core::MinMatrixComponents; sizeX <= Core::MaxMatrixComponents; ++sizeX)
        {
          // Get the two matrix types (as a transpose can have different dimensions)
          BoundType* matrixType = matrixTypes[typeIndex][sizeY - 1][sizeX - 1];
          BoundType* resultType = matrixTypes[typeIndex][sizeX - 1][sizeY - 1];

          MatrixUserData matrixUserData(sizeX, sizeY, typeIndex);
          Function* f = builder.AddBoundFunction(core.MathType, "Transpose", MatrixTranspose, OneParameter(matrixType), resultType, FunctionOptions::Static);
          f->Description = transposeDescription;
          f->ComplexUserData.WriteObject(matrixUserData);
        }
      }
    }

    // Iterate over matrices that share one common dimension for multiplication
    // (but skip bools because boolean matrix multiplication is weird...)
    String matrixMultiplyDescription = ZilchDocumentString("Multiplies the two matrices together. Matrix multiplication is in right-to-left order, "
                                                           "that is if the matrices represent transformations, then 'the' is applied first followed by 'by'.");
    for (size_t typeIndex = 0; typeIndex < Core::MaxMatrixElementTypes - 1; ++typeIndex)
    {
      for (size_t matrixASizeY = Core::MinMatrixComponents; matrixASizeY <= Core::MaxMatrixComponents; ++matrixASizeY)
      {
        for (size_t matrixASizeX = Core::MinMatrixComponents; matrixASizeX <= Core::MaxMatrixComponents; ++matrixASizeX)
        {
          BoundType* matrixA = matrixTypes[typeIndex][matrixASizeY - 1][matrixASizeX - 1];

          for (size_t matrixBSizeX = Core::MinMatrixComponents; matrixBSizeX <= Core::MaxMatrixComponents; ++matrixBSizeX)
          {
            size_t matrixBSizeY = matrixASizeX;
            BoundType* matrixB = matrixTypes[typeIndex][matrixBSizeY - 1][matrixBSizeX - 1];
            BoundType* resultMatrix = matrixTypes[typeIndex][matrixASizeY - 1][matrixBSizeX - 1];

            MatrixTransformUserData transformUserData(matrixASizeX, matrixASizeY, matrixBSizeX, matrixBSizeY, typeIndex);
            Function* f = builder.AddBoundFunction(core.MathType, "Multiply", MatrixMultiply, TwoParameters(matrixA, "by", matrixB, "the"), resultMatrix, FunctionOptions::Static);
            f->Description = matrixMultiplyDescription;
            f->ComplexUserData.WriteObject(transformUserData);
          }

          // Also generate the matrix * vector versions
          MatrixTransformUserData transformUserData(matrixASizeX, matrixASizeY, 1, matrixASizeX, typeIndex);
          BoundType* inVectorType = core.VectorTypes[typeIndex][matrixASizeX - 1];
          BoundType* resultVectorType = core.VectorTypes[typeIndex][matrixASizeY - 1];
          Function* f = builder.AddBoundFunction(core.MathType, "Multiply", MatrixMultiply, TwoParameters(matrixA, "by", inVectorType, "the"), resultVectorType, FunctionOptions::Static);
          f->Description = matrixMultiplyDescription;
          f->ComplexUserData.WriteObject(transformUserData);
        }
      }
    }

    // Square matrix operations on Reals
    for (size_t matrixSize = Core::MinMatrixComponents; matrixSize <= Core::MaxMatrixComponents; ++matrixSize)
    {
      BoundType* matrixType = matrixTypes[VectorScalarTypes::Real][matrixSize - 1][matrixSize - 1];

      // Generate the MultiplyPoint (with the vector Real(N-1) version that assumes 1 as the last element)
      MatrixTransformUserData transformUserData = MatrixTransformUserData(matrixSize, matrixSize, 1, matrixSize - 1, VectorScalarTypes::Real);
      BoundType* inVectorType = core.RealTypes[matrixSize - 2];
      BoundType* resultVectorType = core.RealTypes[matrixSize - 2];
      Function* f = builder.AddBoundFunction(core.MathType, "MultiplyPoint", MatrixMultiplyPoint, TwoParameters(matrixType, "by", inVectorType, "the"), resultVectorType, FunctionOptions::Static);
      f->ComplexUserData.WriteObject(transformUserData);
      f->Description = ZilchDocumentString("Multiplies the given vector as a point while performing the homogeneous division");

      // Generate the MultiplyPointWithNoDivide (with the vector Real(N-1) version that assumes 1 as the last element)
      transformUserData = MatrixTransformUserData(matrixSize, matrixSize, 1, matrixSize - 1, VectorScalarTypes::Real);
      inVectorType = core.RealTypes[matrixSize - 2];
      resultVectorType = core.RealTypes[matrixSize - 2];
      f = builder.AddBoundFunction(core.MathType, "MultiplyPointNoDivide", MatrixMultiplyPointNoDivide, TwoParameters(matrixType, "by", inVectorType, "the"), resultVectorType, FunctionOptions::Static);
      f->ComplexUserData.WriteObject(transformUserData);
      f->Description = ZilchDocumentString("Multiplies the given vector as a point without performing the homogeneous division");

      // Also generate the MultiplyNormal (with the vector Real(N-1) version that assumes 0 as the last element)
      transformUserData = MatrixTransformUserData(matrixSize, matrixSize, 1, matrixSize - 1, VectorScalarTypes::Real);
      inVectorType = core.RealTypes[matrixSize - 2];
      resultVectorType = core.RealTypes[matrixSize - 2];
      f = builder.AddBoundFunction(core.MathType, "MultiplyNormal", MatrixMultiply, TwoParameters(matrixType, "by", inVectorType, "the"), resultVectorType, FunctionOptions::Static);
      f->Description = ZilchDocumentString("Multiplies the given vector as a vector (0 for the last component)");
      f->ComplexUserData.WriteObject(transformUserData);
    }

    // Bind matrix building functions
    {
      BoundType* real2x2Type = core.Real2x2Type;
      BoundType* real3x3Type = core.Real3x3Type;
      BoundType* real4x4Type = core.Real4x4Type;
      BoundType* real2Type = core.Real2Type;
      BoundType* real3Type = core.Real3Type;
      BoundType* real4Type = core.Real4Type;

      // Real2x2
      ZilchFullBindMethod(builder, core.MathType, Math::Matrix2::GenerateScale, ZilchNoOverload, "GenerateScaleMatrix2x2", "scale")
        ->Description = ZilchDocumentString("Generates a two-dimensional scale matrix.");
      ZilchFullBindMethod(builder, core.MathType, Math::Matrix2::GenerateRotation, ZilchNoOverload, "GenerateRotationMatrix2x2", "radians")
        ->Description = ZilchDocumentString("Generates a two-dimensional rotation matrix.");
      ZilchFullBindMethod(builder, core.MathType, Math::Matrix2::GenerateTransform, ZilchNoOverload, "GenerateTransformMatrix2x2", "radians, scale")
        ->Description = ZilchDocumentString("Generates a two-dimensional transform.");

      // Real3x3
      ZilchFullBindMethod(builder, core.MathType, Math::Matrix3::GenerateScale, (Real3x3 (*)(Real2Param)), "GenerateScaleMatrix3x3", "scale")
          ->Description = ZilchDocumentString("Generates a two-dimensional scale matrix.");
      ZilchFullBindMethod(builder, core.MathType, Math::Matrix3::GenerateScale, (Real3x3(*)(Real3Param)), "GenerateScaleMatrix3x3", "scale")
        ->Description = ZilchDocumentString("Generates a three-dimensional scale matrix.");
      ZilchFullBindMethod(builder, core.MathType, Math::Matrix3::GenerateRotation, (Real3x3(*)(Real)), "GenerateRotationMatrix3x3", "radians")
        ->Description = ZilchDocumentString("Generates a two-dimensional rotation matrix.");
      ZilchFullBindMethod(builder, core.MathType, Math::Matrix3::GenerateRotation, (Real3x3(*)(Real3Param, Real)), "GenerateRotationMatrix3x3", "axis, radians")
        ->Description = ZilchDocumentString("Generates a three-dimensional rotation matrix.");
      ZilchFullBindMethod(builder, core.MathType, Math::Matrix3::GenerateTranslation, (Real3x3(*)(Real2Param)), "GenerateTranslationMatrix3x3", "translation")
        ->Description = ZilchDocumentString("Generates a two-dimensional translation matrix.");
      ZilchFullBindMethod(builder, core.MathType, Math::Matrix3::GenerateTransform, (Real3x3(*)(Real2Param, Real, Real2Param)), "GenerateTransformMatrix3x3", "translation, radians, scale")
        ->Description = ZilchDocumentString("Generates a two-dimensions transform.");
      ZilchFullBindMethod(builder, core.MathType, Math::Matrix3::GenerateTransform, (Real3x3(*)(Real3x3Param, Real3Param)), "GenerateTransformMatrix3x3", "rotation, scale")
        ->Description = ZilchDocumentString("Generates a three-dimensions transform.");
      ZilchFullBindMethod(builder, core.MathType, Math::Matrix3::GenerateTransform, (Real3x3(*)(QuaternionParam, Real3Param)), "GenerateTransformMatrix3x3", "rotation, scale")
        ->Description = ZilchDocumentString("Generates a three-dimensions transform.");

      // Real3x3
      ZilchFullBindMethod(builder, core.MathType, Math::Matrix4::GenerateScale, (Real4x4(*)(Real3Param)), "GenerateScaleMatrix4x4", "scale")
        ->Description = ZilchDocumentString("Generates a three-dimensional scale matrix.");
      ZilchFullBindMethod(builder, core.MathType, Math::Matrix4::GenerateRotation, (Real4x4(*)(Real3Param, Real)), "GenerateRotationMatrix4x4", "axis, radians")
        ->Description = ZilchDocumentString("Generates a three-dimensional rotation matrix.");
      ZilchFullBindMethod(builder, core.MathType, Math::Matrix4::GenerateTranslation, (Real4x4(*)(Real3Param)), "GenerateTranslationMatrix4x4", "translation")
        ->Description = ZilchDocumentString("Generates a three-dimensional translation matrix.");
      ZilchFullBindMethod(builder, core.MathType, Math::Matrix4::GenerateTransform, (Real4x4(*)(Real3Param, QuaternionParam, Real3Param)), "GenerateTransformMatrix4x4", "translation, rotation, scale")
        ->Description = ZilchDocumentString("Generates a three-dimensional translation matrix.");
      ZilchFullBindMethod(builder, core.MathType, Math::Matrix4::GenerateTransform, (Real4x4(*)(Real3Param, Real3x3Param, Real3Param)), "GenerateTransformMatrix4x4", "translation, rotation, scale")
        ->Description = ZilchDocumentString("Generates a three-dimensional translation matrix.");
    }
  }

}//namespace Zilch
