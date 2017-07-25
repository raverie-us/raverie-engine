////////////////////////////////////////////////////////////////////////////////
///
/// \file PropertyInterface.hpp
/// Declaration of the property interface.
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2013, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class PropertyView;
class Widget;

//--------------------------------------------------------- Object Property Node
/// Instead of building the property grid based on an object's MetaType, this
/// structure is used. It allows for customization of the property grid
/// for special cases (such as multi-select only displaying shared Components).
struct ObjectPropertyNode
{
  /// Constructors.
  ObjectPropertyNode(ObjectPropertyNode* parent, HandleParam object, Property* objectProperty = nullptr);
  ObjectPropertyNode(ObjectPropertyNode* parent, Property* property);

  /// Destructor.
  ~ObjectPropertyNode();

  ObjectPropertyNode* mParent;

  /// This object. This will be null for property nodes.
  Handle mObject;
  
  /// The meta composition of the current object. The property interface may override the
  /// composition of the object, so that's why we use this instead of querying the object
  /// itself for a meta composition.
  HandleOf<MetaComposition> mComposition;
  HandleOf<MetaArray> mMetaArray;
  
  // METAREFACTOR When we want to support Arrays of values (e.g. Array<float>), this needs to
  // be changed to a property path.
  Property* mProperty;

  /// This objects properties and methods.
  Array<ObjectPropertyNode*> mProperties;
  Array<Function*> mFunctions;

  /// Dynamically contained objects.
  Array<ObjectPropertyNode*> mContainedObjects;

  /// Custom widgets to add to the bottom of the property grid.
  Array<Widget*> mCustomWidgets;
};

//--------------------------------------------------------------- Property State
/// Property state is used to represent the value of a single property on
/// multiple objects. If the values on all the objects are the same,
/// it will be considered valid. If they're different (conflicted), it's
/// considered to be invalid. For Vector types, it can be considered
/// partially valid if only some of the elements in the vector are the same.
struct PropertyState
{
  /// Possible states.
  enum Enum{Valid, PartiallyValid, Invalid};

  /// Default constructor.
  PropertyState();
  PropertyState(AnyParam value, Enum state = PropertyState::Valid);

  /// Quick check to see if it's valid.
  bool IsValid();
  bool IsPartiallyValid();

  /// The state of the property.
  Enum State;

  /// The current value (if the property is valid).
  Any Value;

  /// The only way a value can be partially valid is if it's a vector type.
  uint PartialState[4];
};

// Collection of property values
// Used to capture property state for batch undo / redo
struct PropertyStateCapture
{
  struct CapturedProperty
  {
    Property* Property;
    Handle Object;
    Any Value;
  };

  // Are any values captured?
  bool HasCapture(){return !Properties.Empty();}
  // Clear all states
  void Clear(){Properties.Clear();}
  // Internal state
  Array<CapturedProperty> Properties;
};

//---------------------------------------------------------------------- Actions
DeclareEnum2(PropertyAction,
          Preview, // Executes the property change so that it shows up
                   // visually, but does not queue an operation.
          Commit); // Executes the property change and queue's an operation

//------------------------------------------------------------------------------- Property Interface
/// Used for undo / redo, networking, multiple object editing, etc...
class PropertyInterface
{
public:
  /// Constructor.
  PropertyInterface();
  virtual ~PropertyInterface(){};
   
  /// Changes the given property on the given object. This is considered the
  /// final commit of the property (not intermediate), and should
  /// be added to a queue for undo/redo if applicable.
  virtual void ChangeProperty(HandleParam object, PropertyPathParam property,
                              PropertyState& state, PropertyAction::Enum action);

  /// Returns whether or not the value is valid. For example, it could be
  /// invalid if this is a multi-selection and there is a conflict between
  /// the values on multiple objects.
  virtual PropertyState GetValue(HandleParam object, PropertyPathParam property);

  /// Invokes the given method on the given object(s).
  virtual void InvokeFunction(HandleParam object, Function* method);

  /// The reason we have a custom MetaComposition and MetaArray is because we want to add events for some modifications,
  /// undo/redo for some, and support for multi-select operations.
  virtual HandleOf<MetaComposition> GetMetaComposition(BoundType* objectType);
  virtual HandleOf<MetaArray> GetMetaArray(BoundType* objectType);

  /// Builds the tree of properties, methods, sub-objects, etc that
  /// are to be displayed in the property grid. Building the tree in the
  /// property interface allows custom behavior that the property view
  /// couldn't support by querying the objects meta directly.
  virtual ObjectPropertyNode* BuildObjectTree(ObjectPropertyNode* parent, HandleParam instance, Property* objectProperty = nullptr);

  /// When the property view is looking at an object or multiple objects,
  /// it needs to know when the components have changed on the object
  /// (possibly from undo/redo) so that it can refresh the grid.
  /// The objects filled out in the array should be a list of objects
  /// that the property view can connect to in order to get events.
  virtual void GetObjects(HandleParam instance, Array<Handle>& objects);

  /// Capture the state of a property for all objects
  virtual void CaptureState(PropertyStateCapture& capture, HandleParam object, Property* property);

  /// Restore the state of all properties on all objects
  virtual void RestoreState(PropertyStateCapture& capture);

  /// These should probably be removed...
  virtual void Undo() {}
  virtual void Redo() {}

  /// Events for notifying the property grid of changes.
  void SendPropertyModifiedOnGrid(HandleParam object, PropertyPathParam property,
                                  AnyParam oldValue, AnyParam newValue,
                                  PropertyAction::Enum action);
  void SendComponentsModifiedOnGrid(HandleParam object);

  PropertyView* mPropertyGrid;
};

//--------------------------------------------------------------------------- Event Meta Composition
class EventMetaComposition : public MetaCompositionWrapper
{
public:
  EventMetaComposition(PropertyInterface* propertyInterface, BoundType* typeToWrap);

  // MetaComposition Interface.
  void AddComponent(HandleParam owner, BoundType* typeToAdd, int index = -1,
                    bool ignoreDependencies = false) override;
  void RemoveComponent(HandleParam owner, HandleParam component,
                       bool ignoreDependencies = false) override;
  void MoveComponent(HandleParam owner, HandleParam component, uint destination) override;

  PropertyInterface* mPropertyInterface;
};

//--------------------------------------------------------------------------------- Event Meta Array
class EventMetaArray : public MetaArrayWrapper
{
public:
  EventMetaArray(BoundType* containedType, PropertyInterface* propertyInterface);

  void Add(HandleParam container, AnyParam value) override;
  void EraseIndex(HandleParam container, uint index) override;

  PropertyInterface* mPropertyInterface;
};

}//namespace Zero
