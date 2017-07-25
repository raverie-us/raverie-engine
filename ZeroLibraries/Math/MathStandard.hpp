///////////////////////////////////////////////////////////////////////////////
///
/// \file MathStandard.hpp
/// The standard includes header file for the math project.
///
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once


#include "Common/CommonStandard.hpp"

namespace Math
{
#include "Utility/Typedefs.hpp"
}// namespace Math

#include "Reals.hpp"
#include "MatrixStorage.hpp"
#include "Vector2.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "Matrix2.hpp"
#include "Matrix3.hpp"
#include "Matrix4.hpp"
#include "IntVector2.hpp"
#include "IntVector3.hpp"
#include "IntVector4.hpp"
#include "BoolVector2.hpp"
#include "BoolVector3.hpp"
#include "BoolVector4.hpp"
#include "Quaternion.hpp"
#include "Math.hpp"
#include "SharedVectorFunctions.hpp"

#include "Random.hpp"
#include "ByteColor.hpp"
#include "Curve.hpp"
#include "DecomposedMatrix4.hpp"
#include "EulerOrder.hpp"
#include "EulerAngles.hpp"
#include "Numerical.hpp"

#include "VectorHashPolicy.hpp"
#include "WeightedProbabilityTable.hpp"

#include "GenericVector.hpp"
#include "ExtendableMath.hpp"
#include "ErrorCallbacks.hpp"
#include "BlockVector3.hpp"
#include "IndexPolicies.hpp"
#include "JacobiSolver.hpp"
#include "ConjugateGradient.hpp"
#include "SimpleCgPolicies.hpp"
#include "GaussSeidelSolver.hpp"

#include "ToString.hpp"

#include "SimMath.hpp"
#include "SimVectors.hpp"
#include "SimMatrix3.hpp"
#include "SimMatrix4.hpp"
#include "SimConversion.hpp"

namespace Zero
{
#include "BasicNativeTypesMath.inl"
}
