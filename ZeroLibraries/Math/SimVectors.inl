///////////////////////////////////////////////////////////////////////////////
///
/// \file SimVectors.inl
/// Platform independent functionality for the SimVec3 and SimVec4.
/// 
/// Authors: Joshua Davis
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

//Vectors can be viewed in the form of [w, z, y, x].
//The reason that the vector is backwards is that the values
//are flipped when loaded into registers. They are also flipped
//when stored back out. Also, when indexing using the shuffle
//intrinsic, the values are access as [3, 2, 1, 0]. Therefore,
//x is still 0 and so on. The flipped order is preserved in comments
//so that debugging is more logical.
namespace Math
{

namespace Simd
{

SimInline SimVec GetXYZ(SimVecParam vec)
{
  return AndVec(vec,gSimVec3Mask);
}

SimInline SimVec SplatX(SimVecParam vec)
{
  return VecShuffle(vec,vec,0,0,0,0);
}

SimInline SimVec SplatY(SimVecParam vec)
{
  return VecShuffle(vec,vec,1,1,1,1);
}

SimInline SimVec SplatZ(SimVecParam vec)
{
  return VecShuffle(vec,vec,2,2,2,2);
}

SimInline SimVec SplatW(SimVecParam vec)
{
  return VecShuffle(vec,vec,3,3,3,3);
}

SimInline SimVec Scale(SimVecParam lhs, scalar rhs)
{
  SimVec scaleVec = Set(rhs);
  return Multiply(lhs,scaleVec);
}

SimInline SimVec Negate(SimVecParam lhs)
{
  return Subtract(gSimZero,lhs);
}

SimInline SimVec Reciprocal(SimVecParam vec)
{
  return Divide(gSimOne,vec);
}

SimInline SimVec ReciprocalSqrt(SimVecParam vec)
{
  SimVec sqrt = Sqrt(vec);
  return Reciprocal(sqrt);
}

SimInline SimVec Clamp(SimVecParam vec, SimVecParam minVec, SimVecParam maxVec)
{
  return Max(minVec, Min(vec, maxVec));
}

SimInline SimVec Abs(SimVecParam vec)
{
  SimVec negativeVec = Negate(vec);
  return Max(negativeVec,vec);
}

SimInline SimVec Lerp(SimVecParam start, SimVecParam end, SimVecParam t)
{
  //start + (end - start) * t
  SimVec dir = Subtract(end,start);
  return MultiplyAdd(dir,t,start);
}

SimInline SimVec Lerp(SimVecParam start, SimVecParam end, scalar t)
{
  return Lerp(start,end,Set4(t,t,t,t));
}

SimInline SimVec BaryCentric(SimVecParam p0, SimVecParam p1, SimVecParam p2, scalar u, scalar v)
{
  // Result = p0 + u * (p1 - p0) + v * (p2 - p0)
  SimVec result = p0;
  SimVec scaleU = Set(u);
  SimVec scaleV = Set(v);
  SimVec p10 = Subtract(p1,p0);
  SimVec p20 = Subtract(p2,p0);
  result = MultiplyAdd(scaleU,p10,result);
  result = MultiplyAdd(scaleV,p20,result);
  return result;
}

SimInline SimVec operator+(SimVecParam lhs, SimVecParam rhs)
{
  return Add(lhs,rhs);
}

SimInline SimVec operator-(SimVecParam lhs, SimVecParam rhs)
{
  return Subtract(lhs,rhs);
}

SimInline SimVec operator*(SimVecParam lhs, SimVecParam rhs)
{
  return Multiply(lhs,rhs);
}

SimInline SimVec operator/(SimVecParam lhs, SimVecParam rhs)
{
  return Divide(lhs,rhs);
}

SimInline void operator+=(SimVecRef lhs, SimVecParam rhs)
{
  lhs = Add(lhs,rhs);
}

SimInline void operator-=(SimVecRef lhs, SimVecParam rhs)
{
  lhs = Subtract(lhs,rhs);
}

SimInline void operator*=(SimVecRef lhs, SimVecParam rhs)
{
  lhs = Multiply(lhs,rhs);
}

SimInline void operator/=(SimVecRef lhs, SimVecParam rhs)
{
  lhs = Divide(lhs,rhs);
}

SimInline SimVec operator*(SimVecParam lhs, scalar rhs)
{
  return Scale(lhs,rhs);
}

SimInline SimVec operator/(SimVecParam lhs, scalar rhs)
{
  return Scale(lhs,scalar(1.0) / rhs);
}

SimInline void operator*=(SimVecRef lhs, scalar rhs)
{
  lhs = Scale(lhs,rhs);
}

SimInline void operator/=(SimVecRef lhs, scalar rhs)
{
  lhs = Scale(lhs,scalar(1.0) / rhs);
}

SimInline SimVec Set3(scalar x, scalar y, scalar z)
{
  return Set4(x,y,z,0);
}

SimInline SimVec InnerSum3(SimVec vec)
{
  //temp = [z,y,z,y]
  SimVec temp = VecShuffle(vec,vec,2,1,2,1);
  //temp = [z + w, y + z, z + y, z + x]
  temp = Add(temp,vec);
  //temp = [z + y, z + y, z + y, z + y]
  temp = VecShuffle(temp,temp,1,1,1,1);
  //temp = [z + w + w, z + y + z, z + y + y, z + y + x] 
  temp = Add(vec,temp);
  //return [z + y + x, z + y + x, z + y + x, z + y + x]
  return VecShuffle(temp,temp,0,0,0,0);
}

SimInline SimVec Dot3(SimVecParam lhs, SimVecParam rhs)
{
  SimVec scaleVec = Multiply(lhs,rhs);
  return InnerSum3(scaleVec);
}

SimInline SimVec Cross3(SimVecParam lhs, SimVecParam rhs)
{
  //tempA = [w1,x1,z1,y1]
  SimVec tempA = VecShuffle(lhs,lhs,3,0,2,1);
  //tempB = [w2,y2,x2,z2]
  SimVec tempB = VecShuffle(rhs,rhs,3,1,0,2);
  //result = [w1*w2, x1*y2, z1*x2, y1*z2]
  SimVec result = Multiply(tempA,tempB);
  //tempA = [w1,y1,x1,z1]
  tempA = VecShuffle(tempA,tempA,3,0,2,1);
  //tempB = [w2,x2,z2,x2]
  tempB = VecShuffle(tempB,tempB,3,1,0,2);
  //result = [w1*w2, x1*y2, z1*x2, y1*z2] - [w1*w1, y1*x2, x1*z2, z1*x2]
  result = MultiplySubtract(tempA,tempB,result);
  //return [0, x1*y2 - y1*x2, z1*x2 - x1*z2, y1*z2 - z1*x2]
  return AndVec(result,gSimVec3Mask);
}

SimInline SimVec LengthSq3(SimVecParam vec)
{
  return Dot3(vec,vec);
}

SimInline SimVec Length3(SimVecParam vec)
{
  SimVec lengthSq = LengthSq3(vec);
  return Sqrt(lengthSq);
}

SimInline SimVec Normalize3(SimVecParam vec)
{
  SimVec length = Length3(vec);
  SimVec invLength = Reciprocal(length);
  return Multiply(vec,invLength);
}

SimInline SimVec NormalizeEst3(SimVecParam vec)
{
  SimVec lengthSq = LengthSq3(vec);
  SimVec invLength = ReciprocalSqrtEst(lengthSq);
  return Multiply(vec,invLength);
}

SimInline SimVec AttemptNormalize3(SimVecParam lhs)
{
  SimVec length = Length3(lhs);
  SimVec invLength = Reciprocal(length);
  SimVec mask = NotEqual(length,gSimZero);
  SimVec vec = Multiply(lhs,invLength);
  return AndVec(vec,mask);
}

SimInline SimVec Reflect3(SimVecParam incident, SimVecParam normal)
{
  // Result = Incident - (2 * dot(Incident, Normal)) * Normal
  SimVec result = Dot3(incident,normal);
  result = Add(result,result);
  result = Multiply(result,normal);
  return MultiplySubtract(result,normal,incident);
}

SimInline SimVec Refract3(SimVecParam incident, SimVecParam normal, SimVecParam refractionIndex)
{
  //IDotN = dot(Incident,Normal)
  //R = 1.0f - RefractionIndex * RefractionIndex * (1.0f - IDotN * IDotN)
  //Result = RefractionIndex * Incident - Normal * (RefractionIndex * IDotN + sqrt(R))

  SimVec iDotN = Dot3(incident,normal);
  SimVec r = MultiplySubtract(iDotN,iDotN,gSimOne);
  r = Multiply(refractionIndex,r);
  r = MultiplySubtract(refractionIndex,r,gSimOne);

  //get a mask to kill any terms that are less than zero (the sqrt will fail)
  SimVec mask = Greater(r,gSimZero);
  SimVec sqrt = Sqrt(r);
  //result = (RefractionIndex * IDotN + sqrt(R))
  SimVec result = MultiplyAdd(refractionIndex,iDotN,sqrt);
  //result = RefractionIndex * Incident - Normal * result
  result = MultiplySubtract(normal,result,Multiply(refractionIndex,incident));
  //mask the terms that failed the sqrt
  return AndVec(result,mask);
}

SimInline SimVec InnerSum4(SimVec vec)
{
  SimVec yxwz = Add(VecShuffle(vec,vec,2,3,0,1),vec);
  return Add(VecShuffle(yxwz,yxwz,1,0,3,2),yxwz);
}

SimInline SimVec Dot4(SimVecParam lhs, SimVecParam rhs)
{
  SimVec scaleVec = Multiply(lhs,rhs);
  return InnerSum4(scaleVec);
}

SimInline SimVec LengthSq4(SimVecParam lhs)
{
  return Dot4(lhs,lhs);
}

SimInline SimVec Length4(SimVecParam lhs)
{
  SimVec lengthSq = LengthSq4(lhs);
  return Sqrt(lengthSq);
}

SimInline SimVec Normalize4(SimVecParam lhs)
{
  SimVec length = Length4(lhs);
  SimVec invLength = Reciprocal(length);
  return Multiply(lhs,invLength);
}

SimInline SimVec NormalizeEst4(SimVecParam lhs)
{
  SimVec lengthSq = LengthSq4(lhs);
  SimVec invLength = ReciprocalSqrtEst(lengthSq);
  return Multiply(lhs,invLength);
}

SimInline SimVec AttemptNormalize4(SimVecParam lhs)
{
  SimVec length = Length4(lhs);
  SimVec invLength = Reciprocal(length);
  SimVec mask = NotEqual(length,gSimZero);
  SimVec vec = Multiply(lhs,invLength);
  return AndVec(vec,mask);
}

SimInline SimVec Reflect4(SimVecParam incident, SimVecParam normal)
{
  // Result = Incident - (2 * dot(Incident, Normal)) * Normal
  SimVec result = Dot4(incident,normal);
  result = Add(result,result);
  result = Multiply(result,normal);
  return Subtract(incident,result);
}

SimInline SimVec Refract4(SimVecParam incident, SimVecParam normal, SimVecParam refractionIndex)
{
  //IDotN = dot(Incident,Normal)
  //R = 1.0f - RefractionIndex * RefractionIndex * (1.0f - IDotN * IDotN)
  //Result = RefractionIndex * Incident - Normal * (RefractionIndex * IDotN + sqrt(R))

  SimVec iDotN = Dot4(incident,normal);
  SimVec r = MultiplySubtract(iDotN,iDotN,gSimOne);
  r = Multiply(refractionIndex,r);
  r = MultiplySubtract(refractionIndex,r,gSimOne);

  //get a mask to kill any terms that are less than zero (the sqrt will fail)
  SimVec mask = Greater(r,gSimZero);
  SimVec sqrt = Sqrt(r);
  //result = (RefractionIndex * IDotN + sqrt(R))
  SimVec result = MultiplyAdd(refractionIndex,iDotN,sqrt);
  //result = RefractionIndex * Incident - Normal * result
  result = MultiplySubtract(normal,result,Multiply(refractionIndex,incident));
  //mask the terms that failed the sqrt
  return AndVec(result,mask);
}

SimInline SimVec DotXM(SimVecParam lhs, SimVecParam rhs)
{
  SimVec vTemp2 = rhs;
  SimVec vTemp = Multiply(lhs,vTemp2);
  vTemp2 = VecShuffle(vTemp2,vTemp,1,0,0,0); // Copy X to the Z position and Y to the W position
  vTemp2 = Add(vTemp2,vTemp);          // Add Z = X+Z; W = Y+W;
  vTemp = VecShuffle(vTemp,vTemp2,0,3,0,0);  // Copy W to the Z position
  vTemp = Add(vTemp,vTemp2);           // Add Z and W together
  vTemp = VecShuffle(vTemp,vTemp,2,2,2,2);    // Splat Z and return

  return vTemp;
}

}//namespace Simd

}//namespace Math
