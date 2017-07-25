///////////////////////////////////////////////////////////////////////////////
///
/// \file FileElements.hpp
/// Declaration of the File element in the mesh file format.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

typedef unsigned int u32;

const u32 FileMark = 'dpm3';
const u32 MarkAnimation = 'anim';
const u32 MarkMesh = 'mesh';
const u32 MarkSkel = 'skel';
const u32 MarkSkelRef = 'sref';

//enum VertexType
//{
//  VertexTypeBasic,
//  VertexTypeSkin,
//  VertexTypeTan,
//  VertexTypeTanSkin
//};
