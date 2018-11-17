///////////////////////////////////////////////////////////////////////////////
///
/// \file MatrixStorage.hpp
/// Declaration of the switch for how matrices are stored.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

///Used to set the way matrices are laid out. If 0, then the columns of an 
///orthogonal basis matrix represent the xyz-axes of the rotation. If 1, then 
///the rows of an orthogonal basis matrix represent the xyz-axes of the 
///rotation. This only changes how the memory is laid out for matrices, the 
///matrix interfaces do not change (and neither does the code that uses them).
///The reason why this should be changed is for the memory layout with respect 
///to SIMD operations. Early versions of SSE (2 and 3) are more efficient with
///column operations (so basis vectors in the rows, I know it's backwards but go 
///with it), and xmvector and SSE 4 are more efficient with row operations (so 
///basis vectors in the columns).
#ifndef ColumnBasis
#define ColumnBasis 1
#endif
