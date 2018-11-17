////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// \file ObjectRestoreState.hpp
///
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class Object;

//----------------------------------------------------------------------------- Object Restore State
/// In undo/redo, there are operations related to data inheritance that require re-creating the
/// object to be undone. This is an interface for the engines operation's to handle this generically.
///
/// An example when you revert the modified child order on a Cog, we need to store the Cog in its
/// current state, rebuild the Cog with the child order reverted. When we undo this, we need to
/// restore the old state of the object (with the modified child order).
///
/// This class should be responsible for storing and updating undo handles.
class ObjectRestoreState
{
public:
  /// Stores the object in its current state.
  virtual void StoreObjectState(Object* object) = 0;

  /// Restores and returns the object in its previous state.
  virtual Object* RestoreObject() = 0;
};

}//namespace Zero
