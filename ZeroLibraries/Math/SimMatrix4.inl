///////////////////////////////////////////////////////////////////////////////
///
/// \file SimMatrix4.cpp
/// Implementation of the SimMat4 functionality.
/// 
/// Authors: Joshua Davis
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////


//all matrices can be viewed in the form of
// [d c b a]
// [h g f e]
// [l k j i]
// [p o n m]
//where each [] is a basis vector of the matrix. As with vectors,
//the elements are inverted upon loading and storing.

namespace Math
{

namespace Simd
{

SimInline SimMat4 LoadMat4x4(const scalar vals[16])
{
  SimMat4 m;
  m.columns[0] = Load(vals + 0);
  m.columns[1] = Load(vals + 4);
  m.columns[2] = Load(vals + 8);
  m.columns[3] = Load(vals + 12);
  return m;
}

SimInline SimMat4 SetMat4x4(scalar m00, scalar m01, scalar m02, scalar m03, scalar m10, scalar m11, scalar m12, scalar m13,scalar m20, scalar m21, scalar m22, scalar m23,scalar m30, scalar m31, scalar m32, scalar m33)
{
  SimMat4 m;
  m.columns[0] = Set4(m00,m10,m20,m30);
  m.columns[1] = Set4(m01,m11,m21,m31);
  m.columns[2] = Set4(m02,m12,m22,m32);
  m.columns[3] = Set4(m03,m13,m23,m33);
  return m;
}

SimInline SimMat4 UnAlignedLoadMat4x4(const scalar vals[16])
{
  SimMat4 m;
  m.columns[0] = UnAlignedLoad(vals + 0);
  m.columns[1] = UnAlignedLoad(vals + 4);
  m.columns[2] = UnAlignedLoad(vals + 8);
  m.columns[3] = UnAlignedLoad(vals + 12);
  return m;
}

SimInline void StoreMat4x4(scalar vals[16], SimMat4Param mat)
{
  Store(mat.columns[0],vals + 0);
  Store(mat.columns[1],vals + 4);
  Store(mat.columns[2],vals + 8);
  Store(mat.columns[3],vals + 12);
}

SimInline void UnAlignedStoreMat4x4(scalar vals[16], SimMat4Param mat)
{
  UnAlignedStore(mat.columns[0],vals + 0);
  UnAlignedStore(mat.columns[1],vals + 4);
  UnAlignedStore(mat.columns[2],vals + 8);
  UnAlignedStore(mat.columns[3],vals + 12);
}

SimInline SimMat4 IdentityMat4x4()
{
  SimMat4 m;
  m.columns[0] = gSimBasisX;
  m.columns[1] = gSimBasisY;
  m.columns[2] = gSimBasisZ;
  m.columns[3] = gSimBasisW;
  return m;
}

SimInline SimMat4 ZeroOutMat4x4()
{
  SimMat4 m;
  m.columns[0] = gSimZero;
  m.columns[1] = gSimZero;
  m.columns[2] = gSimZero;
  m.columns[3] = gSimZero;
  return m;
}

SimInline SimVec BasisX(SimMat4Param mat)
{
  return mat.columns[0];
}

SimInline SimVec BasisY(SimMat4Param mat)
{
  return mat.columns[1];
}

SimInline SimVec BasisZ(SimMat4Param mat)
{
  return mat.columns[2];
}

SimInline SimVec BasisW(SimMat4Param mat)
{
  return mat.columns[3];
}

SimInline SimMat4 SetBasisX(SimMat4Param mat, SimVecParam value)
{
  SimMat4 m;
  m.columns[0] = value;
  m.columns[1] = mat.columns[1];
  m.columns[2] = mat.columns[2];
  m.columns[3] = mat.columns[3];
  return m;
}

SimInline SimMat4 SetBasisY(SimMat4Param mat, SimVecParam value)
{
  SimMat4 m;
  m.columns[0] = mat.columns[0];
  m.columns[1] = value;
  m.columns[2] = mat.columns[2];
  m.columns[3] = mat.columns[3];
  return m;
}

SimInline SimMat4 SetBasisZ(SimMat4Param mat, SimVecParam value)
{
  SimMat4 m;
  m.columns[0] = mat.columns[0];
  m.columns[1] = mat.columns[1];
  m.columns[2] = value;
  m.columns[3] = mat.columns[3];
  return m;
}

SimInline SimMat4 SetBasisW(SimMat4Param mat, SimVecParam value)
{
  SimMat4 m;
  m.columns[0] = mat.columns[0];
  m.columns[1] = mat.columns[1];
  m.columns[2] = mat.columns[2];
  m.columns[3] = value;
  return m;
}

SimInline SimMat4 Add(SimMat4Param lhs, SimMat4Param rhs)
{
  SimMat4 result;
  result.columns[0] = Add(lhs.columns[0],rhs.columns[0]);
  result.columns[1] = Add(lhs.columns[1],rhs.columns[1]);
  result.columns[2] = Add(lhs.columns[2],rhs.columns[2]);
  result.columns[3] = Add(lhs.columns[3],rhs.columns[3]);
  return result;
}

SimInline SimMat4 Subtract(SimMat4Param lhs, SimMat4Param rhs)
{
  SimMat4 result;
  result.columns[0] = Subtract(lhs.columns[0],rhs.columns[0]);
  result.columns[1] = Subtract(lhs.columns[1],rhs.columns[1]);
  result.columns[2] = Subtract(lhs.columns[2],rhs.columns[2]);
  result.columns[3] = Subtract(lhs.columns[3],rhs.columns[3]);
  return result;
}

SimInline SimMat4 Multiply(SimMat4Param lhs, SimMat4Param rhs)
{
  SimMat4 result;

  result.columns[0] = TransformPoint(lhs,rhs.columns[0]);
  result.columns[1] = TransformPoint(lhs,rhs.columns[1]);
  result.columns[2] = TransformPoint(lhs,rhs.columns[2]);
  result.columns[3] = TransformPoint(lhs,rhs.columns[3]);

  return result;
}

SimInline SimMat4 Scale(SimMat4Param mat, scalar scale)
{
  SimVec vec = Set(scale);
  SimMat4 result;
  result.columns[0] = Scale(mat.columns[0],scale);
  result.columns[1] = Scale(mat.columns[1],scale);
  result.columns[2] = Scale(mat.columns[2],scale);
  result.columns[3] = Scale(mat.columns[3],scale);
  return result;
}

SimInline SimMat4 ComponentScale(SimMat4Param lhs, SimMat4Param rhs)
{
  SimMat4 result;
  result.columns[0] = Multiply(lhs.columns[0],rhs.columns[0]);
  result.columns[1] = Multiply(lhs.columns[1],rhs.columns[1]);
  result.columns[2] = Multiply(lhs.columns[2],rhs.columns[2]);
  result.columns[3] = Multiply(lhs.columns[3],rhs.columns[3]);
  return result;
}

SimInline SimMat4 ComponentDivide(SimMat4Param lhs, SimMat4Param rhs)
{
  SimMat4 result;
  result.columns[0] = Divide(lhs.columns[0],rhs.columns[0]);
  result.columns[1] = Divide(lhs.columns[1],rhs.columns[1]);
  result.columns[2] = Divide(lhs.columns[2],rhs.columns[2]);
  result.columns[3] = Divide(lhs.columns[3],rhs.columns[3]);
  return result;
}

SimInline SimVec TransformPoint(SimMat4Param mat, SimVecParam vec)
{
  SimVec tempX = SplatX(vec);
  SimVec tempY = SplatY(vec);
  SimVec tempZ = SplatZ(vec);
  SimVec tempW = SplatW(vec);

  tempX = Multiply(tempX,mat.columns[0]);
  tempY = Multiply(tempY,mat.columns[1]);
  tempZ = Multiply(tempZ,mat.columns[2]);
  tempW = Multiply(tempW,mat.columns[3]);

  tempX = Add(tempX,tempY);
  tempX = Add(tempX,tempZ);
  return Add(tempX,tempW);
}

SimInline SimVec TransposeTransformPoint(SimMat4Param mat, SimVecParam vec)
{
  SimMat4Param transposed = Transpose4x4(mat);
  SimVec tempX = SplatX(vec);
  SimVec tempY = SplatY(vec);
  SimVec tempZ = SplatZ(vec);
  SimVec tempW = SplatW(vec);

  tempX = Multiply(tempX,transposed.columns[0]);
  tempY = Multiply(tempY,transposed.columns[1]);
  tempZ = Multiply(tempZ,transposed.columns[2]);
  tempW = Multiply(tempW,transposed.columns[3]);

  tempX = Add(tempX,tempY);
  tempX = Add(tempX,tempZ);
  return Add(tempX,tempW);
}

SimInline SimVec TransformNormal(SimMat4Param mat, SimVecParam vec)
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

SimInline SimVec TransposeTransformNormal(SimMat4Param mat, SimVecParam vec)
{
  SimMat4 transposed = TransposeUpper3x3(mat);
  SimVec tempX = SplatX(vec);
  SimVec tempY = SplatY(vec);
  SimVec tempZ = SplatZ(vec);

  tempX = Multiply(tempX,transposed.columns[0]);
  tempY = Multiply(tempY,transposed.columns[1]);
  tempZ = Multiply(tempZ,transposed.columns[2]);

  tempX = Add(tempX,tempY);
  return Add(tempX,tempZ);
}

SimInline SimVec TransformPointProjected(SimMat4Param mat, SimVecParam vec)
{
  SimVec tempX = SplatX(vec);
  SimVec tempY = SplatY(vec);
  SimVec tempZ = SplatZ(vec);

  tempX = Multiply(tempX,mat.columns[0]);
  tempY = Multiply(tempY,mat.columns[1]);
  tempZ = Multiply(tempZ,mat.columns[2]);

  tempX = Add(tempX,tempY);
  tempX = Add(tempX,tempZ);
  //we are assuming that vec.w = 1, so just add the last column in
  tempX = Add(tempX,mat.columns[3]);
  tempY = SplatW(tempX);
  return Divide(tempX,tempY);
}

SimInline SimMat4 BuildScale(SimVecParam scale)
{
  SimMat4 m;

  m.columns[0] = AndVec(scale,gSimMaskX);
  m.columns[1] = AndVec(scale,gSimMaskY);
  m.columns[2] = AndVec(scale,gSimMaskZ);
  m.columns[3] = gSimBasisW;

  return m;
}

SimInline SimMat4 BuildRotation(SimVecParam axis, scalar angle)
{
  //| x^2(1-c0)+c0  xy(1-c0)-zs0  xz(1-c0)+ys0 |
  //| xy(1-c0)+zs0  y^2(1-c0)+c0  yz(1-c0)-xs0 |
  //| xz(1-c0)-ys0  yz(1-c0)+xs0  z^2(1-c0)+c0 |

  //let c = cos(theta), s = sin(theta) and d = 1 - c
  //[0, xzd - ys, xyd + zs, dx^2 + c]
  //[0, yzd + xs, dy^2 + c, xyd - zs]
  //[0, dz^2 + c, yzd - xs, xzd + ys]
  //[1         0,        0,        0]
  SimMat4 m;
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
  //columns[3] = [1, 0, 0, 0]
  m.columns[3] = gSimBasisW;

  return m;
}

SimInline SimMat4 BuildRotation(SimVecParam quat)
{
  //let q = [w, z, y, x]
  //[0,       2xz - 2yw,       2xy + 2zw, 1 - 2y^2 - 2z^2]
  //[0,       2yz + 2xw, 1 - 2x^2 - 2z^2,       2xy - 2zw]
  //[0, 1 - 2x^2 - 2y^2,       2yz - 2xw,       2xz + 2yw]
  //[1                0,               0,               0]
  SimMat4 m;
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
  //columns[3] = [1, 0, 0, 0]
  m.columns[3] = gSimBasisW;
  return m;
}

SimInline SimMat4 BuildTranslation(SimVecParam translation)
{
  SimMat4 m;
  SimVec tempTrans = AndVec(translation,gSimVec3Mask);
  tempTrans = OrVec(tempTrans,gSimBasisW);

  m.columns[0] = gSimBasisX;
  m.columns[1] = gSimBasisY;
  m.columns[2] = gSimBasisZ;
  m.columns[3] = tempTrans;

  return m;
}

SimInline SimMat4 BuildTransform(SimVecParam translation, SimVecParam axis, scalar angle, SimVecParam scale)
{
  SimMat4 m = BuildRotation(axis,angle);
  m.columns[0] = Multiply(m.columns[0],SplatX(scale));
  m.columns[1] = Multiply(m.columns[1],SplatY(scale));
  m.columns[2] = Multiply(m.columns[2],SplatZ(scale));

  SimVec tempTrans = AndVec(translation,gSimVec3Mask);
  tempTrans = OrVec(tempTrans,gSimBasisW);
  m.columns[3] = tempTrans;
  return m;
}

SimInline SimMat4 BuildTransform(SimVecParam translation, SimVecParam quat, SimVecParam scale)
{
  SimMat4 m = BuildRotation(quat);
  m.columns[0] = Multiply(m.columns[0],SplatX(scale));
  m.columns[1] = Multiply(m.columns[1],SplatY(scale));
  m.columns[2] = Multiply(m.columns[2],SplatZ(scale));

  SimVec tempTrans = AndVec(translation,gSimVec3Mask);
  tempTrans = OrVec(tempTrans,gSimBasisW);
  m.columns[3] = tempTrans;
  return m;
}

SimInline SimMat4 BuildTransform(SimVecParam translation, SimMat4Param rot, SimVecParam scale)
{
  SimMat4 m;
  m.columns[0] = AndVec(Multiply(rot.columns[0],SplatX(scale)),gSimVec3Mask);
  m.columns[1] = AndVec(Multiply(rot.columns[1],SplatY(scale)),gSimVec3Mask);
  m.columns[2] = AndVec(Multiply(rot.columns[2],SplatZ(scale)),gSimVec3Mask);

  SimVec tempTrans = AndVec(translation,gSimVec3Mask);
  tempTrans = OrVec(tempTrans,gSimBasisW);
  m.columns[3] = tempTrans;

  return m;
}

SimInline SimMat4 TransposeUpper3x3(SimMat4Param mat)
{
  // [d c b a]           [d i e a]
  // [h g f e]    ==>    [h j f b]
  // [l k j i]    ==>    [l k g c]
  // [p o n m]           [p o n m]
  //temp1 = [e f a b]
  SimVec temp1 = VecShuffle(mat.columns[0],mat.columns[1],0,1,0,1);
  //temp2 = [l j h f]
  SimVec temp2 = VecShuffle(mat.columns[1],mat.columns[2],3,1,3,2);
  //temp3 = [i k d a]
  SimVec temp3 = VecShuffle(mat.columns[0],mat.columns[2],0,2,3,0);
  //temp4 = [h g d c]
  SimVec temp4 = VecShuffle(mat.columns[0],mat.columns[1],3,2,3,2);

  SimMat4 result;
  //column[0] = [d i e a]
  result.columns[0] = VecShuffle(temp1,temp3,1,3,3,1);
  //column[1] = [h j f b]
  result.columns[1] = VecShuffle(temp1,temp2,1,2,2,0);
  //column[2] = [l k g c]
  result.columns[2] = VecShuffle(temp4,mat.columns[2],3,2,2,0);
  //column[3] = [p o n m]
  result.columns[3] = mat.columns[3];
  return result;
}

SimInline SimMat4 Transpose4x4(SimMat4Param mat)
{
  // [d c b a]           [m i e a]
  // [h g f e]    ==>    [n j f b]
  // [l k j i]    ==>    [o k g c]
  // [p o n m]           [p l h d]
  //temp1 = [f e b a]
  SimVec temp1 = VecShuffle(mat.columns[0],mat.columns[1],1,0,1,0);
  //temp3 = [h g d c]
  SimVec temp3 = VecShuffle(mat.columns[0],mat.columns[1],3,2,3,2);
  //temp2 = [n m j i]
  SimVec temp2 = VecShuffle(mat.columns[2],mat.columns[3],1,0,1,0);
  //temp4 = [p o l k]
  SimVec temp4 = VecShuffle(mat.columns[2],mat.columns[3],3,2,3,2);

  SimMat4 result;
  //column[0] = [m i e a]
  result.columns[0] = VecShuffle(temp1,temp2,2,0,2,0);
  //column[1] = [n j f b]
  result.columns[1] = VecShuffle(temp1,temp2,3,1,3,1);
  //column[2] = [o k g c]
  result.columns[2] = VecShuffle(temp3,temp4,2,0,2,0);
  //column[3] = [p l h d]
  result.columns[3] = VecShuffle(temp3,temp4,3,1,3,1);
  return result;
}

SimInline SimMat4 AffineInverse4x4(SimMat4Param transform)
{
  /*A = [ M  b ]    ...     inv(A) = [ inv(M)   -inv(M) * b ]
        { 0  1 ]    ...              [   0            1     ]
  */

  //the rotation inverse is just the 3x3 transpose
  SimMat4 m = TransposeUpper3x3(transform);

  //now we have to invert the translation, but we need to translate
  //in the correct space so we have to multiply by the inverse 3x3
  SimVec invTranslation = Negate(transform.columns[3]);
  invTranslation = TransformNormal(m,invTranslation);
  //now fill in the translation and make sure the last element is 1
  SimVec temp1 = VecShuffle(invTranslation,gSimOne,0,0,2,2);
  m.columns[3] = VecShuffle(invTranslation,temp1,3,1,1,0);

  return m;
}

SimInline SimMat4 AffineInverseWithScale4x4(SimMat4Param transform)
{
  /*A = [ M  b ]    ...     inv(A) = [ inv(M)   -inv(M) * b ]
        { 0  1 ]    ...              [   0            1     ]
  */

  SimMat4 invScale;
  //invert the scale by dividing each column by its squared length (so the length is 1/length)
  invScale.columns[0] = Divide(transform.columns[0],LengthSq4(transform.columns[0]));
  invScale.columns[1] = Divide(transform.columns[1],LengthSq4(transform.columns[1]));
  invScale.columns[2] = Divide(transform.columns[2],LengthSq4(transform.columns[2]));

  //get the 3x3 inverse of the scale and the rotation
  SimMat4 m = TransposeUpper3x3(invScale);

  //now we have to invert the translation, but we need to translate
  //in the correct space so we have to multiply by the inverse 3x3
  SimVec invTranslation = Negate(transform.columns[3]);
  invTranslation = TransformNormal(m,invTranslation);
  //now fill in the translation and make sure the last element is 1
  SimVec temp1 = VecShuffle(invTranslation,gSimOne,0,0,2,2);
  m.columns[3] = VecShuffle(invTranslation,temp1,3,1,1,0);

  return m;
}

SimInline SimMat4 Equal(SimMat4Param mat1, SimMat4Param mat2)
{
  SimMat4 m;

  m.columns[0] = Equal(mat1.columns[0],mat1.columns[0]);
  m.columns[1] = Equal(mat1.columns[1],mat2.columns[1]);
  m.columns[2] = Equal(mat1.columns[2],mat2.columns[2]);
  m.columns[3] = Equal(mat1.columns[3],mat2.columns[3]);

  return m;
}

SimInline SimMat4 operator+(SimMat4Param lhs, SimMat4Param rhs)
{
  return Add(lhs,rhs);
}

SimInline SimMat4 operator-(SimMat4Param lhs, SimMat4Param rhs)
{
  return Subtract(lhs,rhs);
}

SimInline SimMat4 operator*(SimMat4Param lhs, SimMat4Param rhs)
{
  return Multiply(lhs,rhs);
}

SimInline SimMat4 operator*(SimMat4Param lhs, scalar rhs)
{
  return Scale(lhs,rhs);
}

SimInline SimMat4 operator/(SimMat4Param lhs, scalar rhs)
{
  return Scale(lhs, scalar(1) / rhs);
}

}//namespace Simd

}//namespace Math
      