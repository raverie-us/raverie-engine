///////////////////////////////////////////////////////////////////////////////
///
/// \file SimVectors.inl
/// Platform dependent functionality for the SimVec3 and SimVec4.
/// 
/// Authors: Joshua Davis
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
namespace Math
{

namespace Simd
{

SimInline SimVec Load(const scalar vals[4])
{
  return _mm_load_ps(vals);
}

SimInline SimVec UnAlignedLoad(const scalar vals[4])
{
  return _mm_loadu_ps(vals);
}

SimInline void Store(SimVecParam vec, scalar result[4])
{
  _mm_store_ps(result,vec);
}

SimInline void UnAlignedStore(SimVecParam vec, scalar result[4])
{
  _mm_storeu_ps(result,vec);
}

SimInline SimVec Set(scalar val)
{
  return _mm_set_ps1(val);
}

SimInline SimVec Set4(scalar x, scalar y, scalar z, scalar w)
{
  return _mm_set_ps(w,z,y,x);
}

SimInline SimVec ZeroOutVec()
{
  return _mm_setzero_ps();
}

SimInline SimVec Add(SimVecParam lhs, SimVecParam rhs)
{
  return _mm_add_ps(lhs,rhs);
}

SimInline SimVec Subtract(SimVecParam lhs, SimVecParam rhs)
{
  return _mm_sub_ps(lhs,rhs);
}

SimInline SimVec Multiply(SimVecParam lhs, SimVecParam rhs)
{
  return _mm_mul_ps(lhs,rhs);
}

SimInline SimVec Divide(SimVecParam lhs, SimVecParam rhs)
{
  return _mm_div_ps(lhs,rhs);
}

SimInline SimVec ReciprocalEst(SimVecParam vec)
{
  return _mm_rcp_ps(vec);
}

SimInline SimVec Sqrt(SimVecParam vec)
{
  return _mm_sqrt_ps(vec);
}

SimInline SimVec ReciprocalSqrtEst(SimVecParam vec)
{
  return _mm_rsqrt_ps(vec);
}

SimInline SimVec OrVec(SimVecParam vec, SimVecParam mask)
{
  return _mm_or_ps(vec,mask);
}

SimInline SimVec AndVec(SimVecParam vec, SimVecParam mask)
{
  return _mm_and_ps(vec,mask);
}

SimInline SimVec AndNotVec(SimVecParam vec, SimVecParam mask)
{
  return _mm_andnot_ps(vec,mask);
}

SimInline SimVec XorVec(SimVecParam vec, SimVecParam mask)
{
  return _mm_xor_ps(vec,mask);
}

SimInline SimVec Equal(SimVecParam lhs, SimVecParam rhs)
{
  return _mm_cmpeq_ps(lhs,rhs);
}

SimInline SimVec NotEqual(SimVecParam lhs, SimVecParam rhs)
{
  return _mm_cmpneq_ps(lhs,rhs);
}

SimInline SimVec Greater(SimVecParam lhs, SimVecParam rhs)
{
  return _mm_cmpgt_ps(lhs,rhs);
}

SimInline SimVec GreaterEqual(SimVecParam lhs, SimVecParam rhs)
{
  return _mm_cmpge_ps(lhs,rhs);
}

SimInline SimVec Less(SimVecParam lhs, SimVecParam rhs)
{
  return _mm_cmplt_ps(lhs,rhs);
}

SimInline SimVec LessEqual(SimVecParam lhs, SimVecParam rhs)
{
  return _mm_cmple_ps(lhs,rhs);
}

SimInline SimVec Min(SimVecParam vec, SimVecParam minVec)
{
  return _mm_min_ps(vec,minVec);
}

SimInline SimVec Max(SimVecParam vec, SimVecParam maxVec)
{
  return _mm_max_ps(vec,maxVec);
}

SimInline SimVec Select(SimVecParam v0, SimVecParam v1, SimVecParam select)
{
  return OrVec(AndNotVec(select,v0),AndVec(v1,select));
}

SimInline SimVec MultiplyAdd(SimVecParam v0, SimVecParam v1, SimVecParam v2)
{
  SimVec temp = Multiply(v0,v1);
  return Add(temp,v2);
}

SimInline SimVec MultiplySubtract(SimVecParam v0, SimVecParam v1, SimVecParam v2)
{
  SimVec temp = Multiply(v0,v1);
  return Subtract(v2,temp);
}

}//namespace Simd

}//namespace Math
