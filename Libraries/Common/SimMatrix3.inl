///////////////////////////////////////////////////////////////////////////////
///
/// \file SimMat3.inl
/// Implementation of the SimMat3 functionality.
/// 
/// Authors: Joshua Davis
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

namespace Math
{

namespace Simd
{

SimInline SimMat3 LoadMat3(const scalar vals[12])
{
  SimMat3 m;
  m.columns[0] = Load(vals + 0);
  m.columns[1] = Load(vals + 4);
  m.columns[2] = Load(vals + 8);
  return m;
}

SimInline SimMat3 SetMat3(scalar m00, scalar m01, scalar m02, 
                          scalar m10, scalar m11, scalar m12,
                          scalar m20, scalar m21, scalar m22)
{
  SimMat3 m;
  m.columns[0] = Set3(m00,m10,m20);
  m.columns[1] = Set3(m01,m11,m21);
  m.columns[2] = Set3(m02,m12,m22);
  return m;
}

SimInline SimMat3 UnAlignedLoadMat3(const scalar vals[12])
{
  SimMat3 m;
  m.columns[0] = UnAlignedLoad(vals + 0);
  m.columns[1] = UnAlignedLoad(vals + 4);
  m.columns[2] = UnAlignedLoad(vals + 8);
  return m;
}

SimInline void StoreMat3(scalar vals[12], SimMat3Param mat)
{
  Store(mat.columns[0],vals + 0);
  Store(mat.columns[1],vals + 4);
  Store(mat.columns[2],vals + 8);
}

SimInline void UnAlignedStoreMat3(scalar vals[12], SimMat3Param mat)
{
  UnAlignedStore(mat.columns[0],vals + 0);
  UnAlignedStore(mat.columns[1],vals + 4);
  UnAlignedStore(mat.columns[2],vals + 8);
}

SimInline SimMat3 ZeroOutMat3()
{
  SimMat3 m;
  m.columns[0] = gSimZero;
  m.columns[1] = gSimZero;
  m.columns[2] = gSimZero;
  return m;
}

SimInline SimMat3 IdentityMat3()
{
  SimMat3 m;
  m.columns[0] = gSimBasisX;
  m.columns[1] = gSimBasisY;
  m.columns[2] = gSimBasisZ;
  return m;
}

SimInline SimVec BasisX(SimMat3Param mat)
{
  return mat.columns[0];
}

SimInline SimVec BasisY(SimMat3Param mat)
{
  return mat.columns[1];
}

SimInline SimVec BasisZ(SimMat3Param mat)
{
  return mat.columns[2];
}

SimInline SimMat3 SetBasisX(SimMat3Param mat, SimVecParam value)
{
  SimMat3 m;
  m.columns[0] = value;
  m.columns[1] = mat.columns[1];
  m.columns[2] = mat.columns[2];
  return m;
}

SimInline SimMat3 SetBasisY(SimMat3Param mat, SimVecParam value)
{
  SimMat3 m;
  m.columns[0] = mat.columns[0];
  m.columns[1] = value;
  m.columns[2] = mat.columns[2];
  return m;
}

SimInline SimMat3 SetBasisZ(SimMat3Param mat, SimVecParam value)
{
  SimMat3 m;
  m.columns[0] = mat.columns[0];
  m.columns[1] = mat.columns[1];
  m.columns[2] = value;
  return m;
}

SimInline SimMat3 Add(SimMat3Param lhs, SimMat3Param rhs)
{
  SimMat3 m;
  m.columns[0] = Add(lhs.columns[0],rhs.columns[0]);
  m.columns[1] = Add(lhs.columns[1],rhs.columns[1]);
  m.columns[2] = Add(lhs.columns[2],rhs.columns[2]);
  return m;
}

SimInline SimMat3 Subtract(SimMat3Param lhs, SimMat3Param rhs)
{
  SimMat3 m;
  m.columns[0] = Subtract(lhs.columns[0],rhs.columns[0]);
  m.columns[1] = Subtract(lhs.columns[1],rhs.columns[1]);
  m.columns[2] = Subtract(lhs.columns[2],rhs.columns[2]);
  return m;
}

SimInline SimMat3 Multiply(SimMat3Param lhs, SimMat3Param rhs)
{
  SimMat3 m;

  m.columns[0] = Transform(lhs,rhs.columns[0]);
  m.columns[1] = Transform(lhs,rhs.columns[1]);
  m.columns[2] = Transform(lhs,rhs.columns[2]);

  return m;
}

SimInline SimMat3 Scale(SimMat3Param mat, scalar scale)
{
  SimMat3 m;
  m.columns[0] = Scale(mat.columns[0],scale);
  m.columns[1] = Scale(mat.columns[1],scale);
  m.columns[2] = Scale(mat.columns[2],scale);
  return m;
}

SimInline SimMat3 ComponentScale(SimMat3Param lhs, SimMat3Param rhs)
{
  SimMat3 m;
  m.columns[0] = Multiply(lhs.columns[0],rhs.columns[0]);
  m.columns[1] = Multiply(lhs.columns[1],rhs.columns[1]);
  m.columns[2] = Multiply(lhs.columns[2],rhs.columns[2]);
  return m;
}

SimInline SimMat3 ComponentDivide(SimMat3Param lhs, SimMat3Param rhs)
{
  SimMat3 m;
  m.columns[0] = Divide(lhs.columns[0],rhs.columns[0]);
  m.columns[1] = Divide(lhs.columns[1],rhs.columns[1]);
  m.columns[2] = Divide(lhs.columns[2],rhs.columns[2]);
  return m;
}

SimInline SimVec Transform(SimMat3Param mat, SimVecParam vec)
{
  SimVec tempX = SplatX(vec);
  SimVec tempY = SplatY(vec);
  SimVec tempZ = SplatZ(vec);

  tempX = Multiply(tempX,mat.columns[0]);
  tempY = Multiply(tempY,mat.columns[1]);
  tempZ = Multiply(tempZ,mat.columns[2]);

  tempX = Add(tempX,tempY);
  return Add(tempX,tempZ);
}

SimInline SimVec TransposeTransform(SimMat3Param mat, SimVecParam vec)
{
  SimMat3 transposed = Transpose3(mat);
  SimVec tempX = SplatX(vec);
  SimVec tempY = SplatY(vec);
  SimVec tempZ = SplatZ(vec);

  tempX = Multiply(tempX,transposed.columns[0]);
  tempY = Multiply(tempY,transposed.columns[1]);
  tempZ = Multiply(tempZ,transposed.columns[2]);

  tempX = Add(tempX,tempY);
  return Add(tempX,tempZ);
}

SimInline SimMat3 BuildScale3(SimVecParam scale)
{
  SimMat3 m;

  m.columns[0] = AndVec(scale,gSimMaskX);
  m.columns[1] = AndVec(scale,gSimMaskY);
  m.columns[2] = AndVec(scale,gSimMaskZ);

  return m;
}

SimInline SimMat3 BuildRotation3(SimVecParam axis, scalar angle)
{
  //let c = cos(theta), s = sin(theta) and d = 1 - c
  //[0, xzd - ys, xyd + zs, dx^2 + c]
  //[0, yzd + xs, dy^2 + c, xyd - zs]
  //[0, dz^2 + c, yzd - xs, xzd + ys]
  SimMat3 m;
  scalar sinAngle = Math::Sin(angle);
  scalar cosAngle = Math::Cos(angle);

  SimVec n0, n1;
  SimVec v0, v1, v2;
  SimVec r0, r1, r2;
  //c2 = [d, d, d, d]
  SimVec c2 = Set(scalar(1.0) - cosAngle);
  //c1 = [c, c, c, c]
  SimVec c1 = Set(cosAngle);
  //c0 = [s, s, s, s]
  SimVec c0 = Set(sinAngle);

  //n0 = [w, x, z, y]
  n0 = VecShuffle(axis,axis,3,0,2,1);
  //n1 = [w, y, x, z]
  n1 = VecShuffle(axis,axis,3,1,0,2);

  //v0 = [dw, dx, dz, dy]
  v0 = Multiply(c2, n0);
  //v0 = [dw^2, dxy, dxz, dyz]
  v0 = Multiply(v0, n1);

  //r0 = [dw, dz, dy, dx]
  r0 = Multiply(c2, axis);
  //r0 = [dw^2, dz^2, dy^2, dx^2]
  r0 = Multiply(r0, axis);
  //r0 = [d2^2 + c, dz^2 + c, dy^2 + c, dx^2 + c]
  r0 = Add(r0, c1);

  //r1 = [sw, sz, sy, sx]
  r1 = Multiply(c0, axis);
  //r1 = [sw + dw^2, sz + dxy, sy + dxz, sx + dyz]
  r1 = Add(r1, v0);
  //r2 = [sw, sz, sy, sx]
  r2 = Multiply(c0, axis);
  //r2 = [dw^2 - sw, dxy - sz, dxz - sy, dyz - sx]
  r2 = Subtract(v0,r2);

  //v0 = [0, dz^2 + c, dy^2 + c, dx^2 + c]
  v0 = AndVec(r0,gSimVec3Mask);
  //v1 = [dxy - sz, dxz - sy, sz + dxy, sx + dyz]
  v1 = VecShuffle(r1,r2,2,1,2,0);
  //v1 = [sx + dyz, dxy - sz, dxz - sy, sz + dxy]
  v1 = VecShuffle(v1,v1,0,3,2,1);
  //v2 = [dyz - sx, dyz - sx, sy + dxz, sy + dxz]
  v2 = VecShuffle(r1,r2,0,0,1,1);
  //v2 = [dyz - sx, sy + dxz, dyz - sx, sy + dxz]
  v2 = VecShuffle(v2,v2,2,0,2,0);

  //r2 = [dxz - sy, sz + dxy, 0, dx^2 + c]
  r2 = VecShuffle(v0,v1,1,0,3,0);
  //r2 = [0, dxz - sy, sz + dxy, dx^2 + c]
  r2 = VecShuffle(r2,r2,1,3,2,0);
  //columns[0] = [0, dxz - sy, sz + dxy, dx^2 + c]
  m.columns[0] = r2;
  //r2 = [sx + dyz, dxy - sz, 0, dy^2 + c]
  r2 = VecShuffle(v0,v1,3,2,3,1);
  //r2 = [0, sx + dyz, dy^2 + c, dxy - sz]
  r2 = VecShuffle(r2,r2,1,3,0,2);
  //columns[1] = [0, sx + dyz, dy^2 + c, dxy - sz]
  m.columns[1] = r2;
  //v2 = [0, dz^2 + c, dyz - sx, sy + dxz]
  v2 = VecShuffle(v2,v0,3,2,1,0);
  //columns[2] = [0, dz^2 + c, dyz - sx, sy + dxz]
  m.columns[2] = v2;

  return m;
}

SimInline SimMat3 BuildRotation3(SimVecParam quat)
{
  //let q = [w, z, y, x]
  //[0,       2xz - 2yw,       2xy + 2zw, 1 - 2y^2 - 2z^2]
  //[0,       2yz + 2xw, 1 - 2x^2 - 2z^2,       2xy - 2zw]
  //[0, 1 - 2x^2 - 2y^2,       2yz - 2xw,       2xz + 2yw]
  SimMat3 m;
  SimVec q0, q1;
  SimVec v0, v1, v2;
  SimVec r0, r1, r2;

  //q0 = [2w, 2z, 2y, 2x]
  q0 = Add(quat,quat);
  //q1 = [2w^2, 2z^2, 2y^2, 2x^2]
  q1 = Multiply(quat,q0);

  //v0 = [2w^2, 2x^2, 2x^2, 2y^2]
  v0 = VecShuffle(q1,q1,3,0,0,1);
  //v0 = [0, 2x^2, 2x^2, 2y^2]
  v0 = AndVec(v0,gSimVec3Mask);
  //v1 = [2w^2, 2y^2, 2z^2, 2z^2]
  v1 = VecShuffle(q1,q1,3,1,2,2);
  //v1 = [0, 2y^2, 2z^2, 2z^2]
  v1 = AndVec(v1,gSimVec3Mask);
  //r0 = [0, 1 - 2x^2, 1 - 2x^2, 1 - 2y^2]
  r0 = Subtract(gSimOneVec3,v0);
  //r0 = [0, 1 - 2x^2 - 2y^2, 1 - 2x^2 - 2z^2, 1 - 2y^2 - 2z^2]
  r0 = Subtract(r0, v1);

  //v0 = [w, y, x, x]
  v0 = VecShuffle(quat,quat,3,1,0,0);
  //v1 = [2w, 2z, 2y, 2z]
  v1 = VecShuffle(q0,q0,3,2,1,2);
  //v0 = [2w^2, 2zy, 2yx, 2zx]
  v0 = Multiply(v0, v1);

  //v1 = [w, w, w, w]
  v1 = VecShuffle(quat,quat,3,3,3,3);
  //v2 = [2w, 2x, 2z, 2y]
  v2 = VecShuffle(q0,q0,3,0,2,1);
  //v1 = [2w^2, 2xw, 2zw, 2yw]
  v1 = Multiply(v1, v2);

  //v1 = [2w^2 + 2w^2, 2zy + 2xw, 2yx + 2zw, 2zx + 2yw]
  r1 = Add(v0, v1);
  //v2 = [0, 2zy - 2xw, 2yx - 2zw, 2zx - 2yw]
  r2 = Subtract(v0, v1);

  //v0 = [2yx - 2zw, 2zx - 2yw, 2zy + 2xw, 2yx + 2zw]
  v0 = VecShuffle(r1,r2,1,0,2,1);
  //v0 = [2zy + 2xw, 2yx - 2zw, 2zx - 2yw, 2yx + 2zw]
  v0 = VecShuffle(v0,v0,1,3,2,0);
  //v1 = [2zy - 2xw, 2zy - 2xw, 2zx + 2yw, 2zx + 2yw]
  v1 = VecShuffle(r1,r2,2,2,0,0);
  //v1 = [2zy - 2xw, 2zx + 2yw, 2zy - 2xw, 2zx + 2yw]
  v1 = VecShuffle(v1,v1,2,0,2,0);

  //q1 = [2zx - 2yw, 2yx + 2zw, 0, 1 - 2y^2 - 2z^2]
  q1 = VecShuffle(r0,v0,1,0,3,0);
  //q1 = [0, 2zx - 2yw, 2yx + 2zw, 1 - 2y^2 - 2z^2]
  q1 = VecShuffle(q1,q1,1,3,2,0);
  //columns[0] = [0, 2zx - 2yw, 2yx + 2zw, 1 - 2y^2 - 2z^2]
  m.columns[0] = q1;
  //q1 = [2zy + 2xw, 2yx - 2zw, 0, 1 - 2x^2 - 2z^2]
  q1 = VecShuffle(r0,v0,3,2,3,1);
  //q1 = [0, 2zy + 2xw, 1 - 2x^2 - 2z^2, 2yx - 2zw]
  q1 = VecShuffle(q1,q1,1,3,0,2);
  //columns[1] = [0, 2zy + 2xw, 1 - 2x^2 - 2z^2, 2yx - 2zw]
  m.columns[1] = q1;
  //q1 = [0, 1 - 2x^2 - 2y^2, 2zy - 2xw, 2zx + 2yw]
  q1 = VecShuffle(v1,r0,3,2,1,0);
  //columns[2] = [0, 1 - 2x^2 - 2y^2, 2zy - 2xw, 2zx + 2yw]
  m.columns[2] = q1;
  return m;
}

SimInline SimMat3 BuildTransform3(SimVecParam axis, scalar angle, SimVecParam scale)
{
  SimMat3 m = BuildRotation3(axis,angle);
  m.columns[0] = Multiply(m.columns[0],SplatX(scale));
  m.columns[1] = Multiply(m.columns[1],SplatY(scale));
  m.columns[2] = Multiply(m.columns[2],SplatZ(scale));
  return m;
}

SimInline SimMat3 BuildTransform3(SimVecParam quat, SimVecParam scale)
{
  SimMat3 m = BuildRotation3(quat);
  m.columns[0] = Multiply(m.columns[0],SplatX(scale));
  m.columns[1] = Multiply(m.columns[1],SplatY(scale));
  m.columns[2] = Multiply(m.columns[2],SplatZ(scale));
  return m;
}

SimInline SimMat3 BuildTransform3(SimMat3Param rot, SimVecParam scale)
{
  SimMat3 m;
  m.columns[0] = AndVec(Multiply(rot.columns[0],SplatX(scale)),gSimVec3Mask);
  m.columns[1] = AndVec(Multiply(rot.columns[1],SplatY(scale)),gSimVec3Mask);
  m.columns[2] = AndVec(Multiply(rot.columns[2],SplatZ(scale)),gSimVec3Mask);

  return m;
}

SimInline SimMat3 Transpose3(SimMat3Param mat)
{
  // [d c b a]           [d i e a]
  // [h g f e]    ==>    [h j f b]
  // [l k j i]    ==>    [l k g c]
  //temp1 = [e f a b]
  SimVec temp1 = VecShuffle(mat.columns[0],mat.columns[1],0,1,0,1);
  //temp2 = [l j h f]
  SimVec temp2 = VecShuffle(mat.columns[1],mat.columns[2],3,1,3,2);
  //temp3 = [i k d a]
  SimVec temp3 = VecShuffle(mat.columns[0],mat.columns[2],0,2,3,0);
  //temp4 = [h g d c]
  SimVec temp4 = VecShuffle(mat.columns[0],mat.columns[1],3,2,3,2);

  SimMat3 result;
  //column[0] = [d i e a]
  result.columns[0] = VecShuffle(temp1,temp3,1,3,3,1);
  //column[1] = [h j f b]
  result.columns[1] = VecShuffle(temp1,temp2,1,2,2,0);
  //column[2] = [l k g c]
  result.columns[2] = VecShuffle(temp4,mat.columns[2],3,2,2,0);
  return result;
}

SimInline SimMat3 AffineInverse3(SimMat3Param transform)
{
  //the rotation inverse is just the 3x3 transpose
  SimMat3 m = Transpose3(transform);

  return m;
}

SimInline SimMat3 AffineInverseWithScale3(SimMat3Param transform)
{
  SimMat3 invScale;
  //invert the scale by dividing each column by its squared length (so the length is 1/length)
  invScale.columns[0] = Divide(transform.columns[0],LengthSq3(transform.columns[0]));
  invScale.columns[1] = Divide(transform.columns[1],LengthSq3(transform.columns[1]));
  invScale.columns[2] = Divide(transform.columns[2],LengthSq3(transform.columns[2]));

  //get the 3x3 inverse of the scale and the rotation
  SimMat3 m = Transpose3(invScale);

  return m;
}

SimInline SimMat3 Equal(SimMat3Param mat1, SimMat3Param mat2)
{
  SimMat3 m;
  m.columns[0] = Equal(mat1.columns[0],mat2.columns[0]);
  m.columns[1] = Equal(mat1.columns[1],mat2.columns[1]);
  m.columns[2] = Equal(mat1.columns[2],mat2.columns[2]);
  return m;
}

}//namespace Simd

}//namespace Math
