///////////////////////////////////////////////////////////////////////////////
///
/// \file PropertyView.cpp
/// Implementation of PropertyView and supporting classes.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace PropertyViewUi
{
const cstr cLocation = "EditorUi/PropertyView";
Tweakable(float, ObjectSize,      Pixels(24), cLocation); // Size of objects
Tweakable(float, PropertySize,    Pixels(20), cLocation); // Size of each property widget
Tweakable(float, PropertySpacing, Pixels(2),  cLocation); // Pixels in between each property
Tweakable(float, IndentSize,      Pixels(10), cLocation); // Indent per level
}

namespace Events
{
  DefineEvent(NameActivated);
  DefineEvent(OpenAdd);
  DefineEvent(PropertyContextMenu);
}//namespace Events

ZilchDefineType(ContextMenuEvent, builder, type)
{
}

//---------------------------------------------------------------- Property Grid
ZilchDefineType(PropertyView, builder, type)
{
  ZilchBindOverloadedMethod(SetObject, ZilchInstanceOverload(void, Object*));
  ZilchBindMethod(Refresh);
  ZilchBindMethod(Invalidate);
  ZilchBindMethod(ActivateAutoUpdate);
}

//******************************************************************************
PropertyView::PropertyView(Composite* parent)
  : Composite(parent)
  , mFixedHeight(false)
{
  mScrollArea = new ScrollArea(this);
  mScrollArea->SetClientSize(Pixels(10, 10));
  mScrollArea->DisableScrollBar(0);
  mMinSize = Vec2(100, 100);

  mNamePercent = 0.42f;

  mDefSet = parent->GetDefinitionSet()->GetDefinitionSet("PropertyGrid");
  mRoot = nullptr;
  mPropertyInterface = nullptr;

  SetPropertyInterface(&mDefaultPropertyInterface);
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
}

//******************************************************************************
PropertyView::~PropertyView()
{

}

//******************************************************************************
Handle PropertyView::GetObject()
{
  return mSelectedObject;
}

//******************************************************************************
void PropertyView::Invalidate()
{
  this->MarkAsNeedsUpdate();

  // Destroy the tree
  SafeDestroy(mRoot);

  // Clear the additional widgets
  forRange(Widget* widget, mAddtionalWidgets.All())
    widget->Destroy();
  mAddtionalWidgets.Clear();
}

//******************************************************************************
void PropertyView::Rebuild()
{
  Handle instance = GetObject();
  if(instance.IsNotNull())
  {
    //SafeDelete(mRootObjectNode);
    auto rootObjectNode = mPropertyInterface->BuildObjectTree(nullptr, instance);
    if(rootObjectNode == nullptr)
      return;

    PropertyWidgetInitializer initializer;
    initializer.Instance = instance;
    initializer.Grid = this;
    initializer.Parent = mScrollArea->GetClientWidget();
    initializer.Property = nullptr;
    initializer.CurrentInterface = mPropertyInterface;
    initializer.ObjectNode = rootObjectNode;

    // Create and open the root node
    PropertyWidgetObject* node = new PropertyWidgetObject(initializer, nullptr);
    mRoot = node;
    mRoot->OpenNode(false);
    mRoot->UpdateTransformExternal();
  }

  Refresh();
}

//******************************************************************************
void PropertyView::SetObject(HandleParam newObject, 
                             PropertyInterface* newInterface)
{
  // Disconnect from all old objects
  forRange(Handle oldObject, mSelectedObjects.All())
  {
    // Disconnect if the handle is a valid Object
    if(Object* object = oldObject.Get<Object*>())
      object->GetDispatcherObject()->Disconnect(this);
  }

  // We no longer care about the old objects
  mSelectedObjects.Clear();

  // If it's not a valid object, just rebuild the tree with nothing in it
  if(newObject.IsNull())
  {
    //not a valid object clear the grid.
    mSelectedObject = Handle();
    Invalidate();
    return;
  }

  //Store the handle
  mSelectedObject = newObject;

  // Set the property interface without rebuilding the tree (we're going
  // to rebuild it again in a second)
  SetPropertyInterface(newInterface);

  // We need to know when a component has changed on one of the objects
  // we have selected in order to properly rebuild the tree
  mPropertyInterface->GetObjects(newObject, mSelectedObjects);
  forRange(Handle object, mSelectedObjects.All())
  {
    // Connect if handle is a valid Object
    if(Object* objectPointer = object.Get<Object*>())
    {
      ConnectThisTo(objectPointer, Events::ComponentsModified, OnInvalidate);
      ConnectThisTo(objectPointer, Events::ObjectStructureModified, OnInvalidate);
    }
  }

  //Refresh and rebuild.
  Invalidate();
}

//******************************************************************************
void PropertyView::SetObject(Object* object)
{
  PropertyView::SetObject(Handle(object));
}

//******************************************************************************
void PropertyView::UpdateTransform()
{
  mScrollArea->SetSize(mSize);

  if(mDestroyed)
    return;

  if(mRoot == nullptr)
     Rebuild();

  if(mRoot)
  {
    float rootHeight = mRoot->GetSize().y;

    // Subtract the width if the scroll bar is active
    float width = mSize.x;
    if(rootHeight > mSize.y)
      width -= mScrollArea->GetScrollBarSize();

    // If there is extra height use if for layouts
    float height = Math::Max(mSize.y, rootHeight);

    // Resize root widget with adjusted width
    mRoot->SetSize(Vec2(width, rootHeight));

    // Size client widget in case of layouts
    mScrollArea->GetClientWidget()->SetSize(Vec2(width, height));
    mScrollArea->SetClientSize(Vec2(width, height));
  }
  else
  {
    // Nothing selected reset the 
    mScrollArea->SetClientSize(Vec2(10, 10));
  }

  Composite::UpdateTransform();
}

void PropertyView::SizeToContents()
{
  UpdateTransform();
  ReturnIf(mRoot == nullptr,, "No valid on object selected on property grid. Size will be invalid");
  Vec2 sizeNeeded = mRoot->GetSize();
  this->SetSize(sizeNeeded);
}

Vec2 PropertyView::GetMinSize()
{
  if (mRoot && mFixedHeight)
    return Vec2(mMinSize.x, mRoot->GetSize().y);
  else
    return mMinSize;
}

//******************************************************************************
void PropertyView::ActivateAutoUpdate()
{
  ConnectThisTo(this->GetRootWidget(), Events::WidgetUpdate, OnWidgetUpdate);
}

//******************************************************************************
void PropertyView::Refresh()
{
  // Is the object still valid?
  Handle instance = GetObject();
  if(instance.IsNotNull())
  {
    if(mRoot == nullptr)
    {
      Rebuild();
    }
    else
    {
      mRoot->Refresh();
      mRoot->MarkAsNeedsUpdate();
    }
  }
  else
  {
    // Object is lost, clear the tree
    if(mRoot != nullptr)
      Invalidate();
  }
}

//******************************************************************************
void PropertyView::SetPropertyInterface(PropertyInterface* propInterface, 
                                        bool rebuild)
{
  mPropertyInterface = propInterface;

  // Use the default if none was specified
  if(mPropertyInterface == nullptr)
    mPropertyInterface = &mDefaultPropertyInterface;

  mPropertyInterface->mPropertyGrid = this;

  if(rebuild)
    Invalidate();
}

//******************************************************************************
void PropertyView::AddCustomPropertyIcon(CustomIconCreatorFunction callback,
                                         void* clientData)
{
  // If it's already in the array, no need to add it
  if(mCustomIconCallbacks.Contains(callback))
    return;

  // Insert the client data
  mCallbackClientData[(void*)callback] = clientData;
  mCustomIconCallbacks.PushBack(callback);

  Invalidate();
  Rebuild();
}

//******************************************************************************
void PropertyView::RemoveCustomPropertyIcon(CustomIconCreatorFunction callback)
{
  if(!mCustomIconCallbacks.Contains(callback))
    return;

  mCustomIconCallbacks.EraseValueError(callback);
  mCallbackClientData.Erase((void*)callback);

  Invalidate();
  Rebuild();
}

//******************************************************************************
void PropertyView::OnWidgetUpdate(UpdateEvent* update)
{
  // Only refresh if the property grid is active (could be hidden behind a tab)
  if(GetGlobalActive())
    Refresh();
}

//******************************************************************************
void PropertyView::OnInvalidate(Event* e)
{
  Invalidate();
}

//******************************************************************************
void PropertyView::OnKeyDown(KeyboardEvent* e)
{
  if(!e->CtrlPressed)
    return;

  if(e->Key == Keys::Z)
  {
    mPropertyInterface->Undo();
    e->Handled = true;
  }
  else if(e->Key == Keys::Y)
  {
    mPropertyInterface->Redo();
    e->Handled = true;
  }
}

}//namespace Zero
