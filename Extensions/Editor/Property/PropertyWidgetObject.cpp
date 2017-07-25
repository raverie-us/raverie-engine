///////////////////////////////////////////////////////////////////////////////
///
/// \file PropertyWidgetObject.hpp
/// Declaration of PropertyEditorObject.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const String cPropArrowRight = "PropArrowRight";
const String cPropArrowDown = "PropArrowDown";

namespace ComponentUi
{
const cstr cLocation = "EditorUi/PropertyView/Component";
Tweakable(Vec4,  TitleColor,       Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  TitleHighlight,   Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  TitleRemove,      Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  BackgroundColor,  Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  BackgroundRemove, Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  ErrorPopUpColor,  Vec4(1,1,1,1), cLocation);
Tweakable(float, OpenTime,         0.2f,          cLocation);
Tweakable(float, MaxToolTipWidth,  Pixels(200),   cLocation);
Tweakable(Vec4,  LocallyAdded,            Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  LocallyAddedHighlight,   Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  LocallyRemoved,          Vec4(1,1,1,1), cLocation);
}

HashSet<String> PropertyWidgetObject::mExpandedTypes;

//------------------------------------------------------- Property Editor Object
ZilchDefineType(PropertyWidgetObject, builder, type)
{

}

//******************************************************************************
PropertyWidgetObject::PropertyWidgetObject(PropertyWidgetInitializer& initializer, 
                                           PropertyWidgetObject* parentWidgetObject,
                                           StringParam removedTypeName)
  : PropertyWidget(initializer, (initializer.ObjectNode && initializer.ObjectNode->mProperty) ? StyleMode::Regular : StyleMode::Node)
{
  mMouseOverTitle = false;
  mDragging = false;
  mNode = initializer.ObjectNode;
  ConnectThisTo(MetaDatabase::GetInstance(), Events::MetaModified, OnMetaModified);

  mLocalModificationIcon = nullptr;

  mLocallyRemoved = !removedTypeName.Empty();
  mLocallyRemovedTypeName = removedTypeName;

  mBackground = CreateAttached<Element>(cWhiteSquare);
  mTitleBackground = CreateAttached<Element>(cWhiteSquare);
  mTitleBackground->MoveToBack();
  mBackground->MoveToBack();
  ConnectThisTo(mTitleBackground, Events::MouseEnter, OnMouseEnterTitle);
  ConnectThisTo(mTitleBackground, Events::MouseExit, OnMouseExitTitle);
  mBackground->SetInteractive(false);

  mGrid = initializer.Grid;
  mRemoveIcon = nullptr;
  mParentWidgetObject = parentWidgetObject;
  SetClipping(true);
  mCustomUi = nullptr;

  if(mNode)
  {
    mComposition = mNode->mComposition;
    mMetaArray = mNode->mMetaArray;
  }
  if(mParentWidgetObject && mParentWidgetObject->mParent)
  {
    mParentComposition = mParentWidgetObject->mNode->mComposition;
    mParentMetaArray = mParentWidgetObject->mNode->mMetaArray;
  }

  mLocallyAdded = false;

  Handle object;
  BoundType* objectType = nullptr;

  if(mNode)
  {
    object = mNode->mObject;
    objectType = object.StoredType;

    if(initializer.ObjectNode)
    {
      if(MetaOwner* metaOwner = objectType->HasInherited<MetaOwner>())
        mLocallyAdded = LocalModifications::GetInstance()->IsObjectLocallyAdded(object, false);
    }
  }

  if(mLocallyRemoved)
  {
    mLocalModificationIcon = CreateAttached<Element>("ObjectLocallyRemoved");
    mLocalModificationIcon->SizeToContents();
  }
  else if(mLocallyAdded && mParentWidgetObject)
  {
    mLocalModificationIcon = CreateAttached<Element>("ObjectLocallyAdded");
    mLocalModificationIcon->SizeToContents();
  }

  // If we're a removable object, add an 'X' button to remove us
  if(!mLocallyRemoved)
  {
    bool canBeRemoved = false;
    if(mParentComposition)
      canBeRemoved = mParentComposition->mSupportsComponentRemoval;
    else if(mParentMetaArray)
      canBeRemoved = true;

    if(canBeRemoved)
    {
      mRemoveIcon = CreateAttached<Element>("RemoveX");
      ConnectThisTo(mRemoveIcon, Events::MouseEnter, OnMouseEnterX);
      ConnectThisTo(mRemoveIcon, Events::LeftClick, OnLeftClickRemove);
      ConnectThisTo(mRemoveIcon, Events::MouseExit, OnMouseExitX);
    }
  }

  // Add an expand icon
  mExpandNode = CreateAttached<Element>(cPropArrowRight);
  mExpandNode->SetInteractive(false);
  mExpandNode->SetActive(!mLocallyRemoved);
  mLabel->SetInteractive(false);

  mEditScriptButton = nullptr;
  mProxyIcon = nullptr;
  if(objectType)
  {
    bool isProxy = objectType->HasAttribute(ObjectAttributes::cProxy);

    // Update the resource location
    if (isProxy)
      EngineLibraryExtensions::FindProxiedTypeOrigin(objectType);

    if(MetaResource* metaResource = objectType->HasInherited<MetaResource>())
    {
      if(mNode->mProperty == nullptr)
      {
        mEditScriptButton = new IconButton(this);
        mEditScriptButton->SetIcon("EditScript");
        mEditScriptButton->mIconColor = ToByteColor(Vec4(1, 1, 1, 0.75f));
        mEditScriptButton->mIconHoverColor = ToByteColor(Vec4(1, 1, 1, 0.95f));
        mEditScriptButton->mIconClickedColor = ToByteColor(Vec4(1, 1, 1, 0.8f));

        String message = "Edit Script Source";
        Resource* resource = Z::gResources->GetResource(metaResource->mResourceId);
        ErrorIf(resource == nullptr, "Could not find resource to edit");
        if(resource)
          message = String::Format("Edit '%s' Script", resource->Name.c_str());

        mEditScriptButton->SetToolTip(message);
        ConnectThisTo(mEditScriptButton, Events::ButtonPressed, OnEditScriptPressed);
        ConnectThisTo(mEditScriptButton, Events::MouseEnter, OnMouseEnterTitle);
      }
    }

    if (isProxy)
    {
      mProxyIcon = CreateAttached<Element>("WarningSmall");
      mProxyIcon->SetInteractive(false);
    }
  }

  mNodeState = NodeState::Closed;

  RefreshLabel();

  //Expanded
  if(objectType)
  {
    if(mExpandedTypes.Contains(objectType->Name) || objectType->HasAttribute(ObjectAttributes::cExpanded))
      OpenNode(false);
  }

  ConnectThisTo(mTitleBackground, Events::LeftClick, OnLeftClickTitle);
  ConnectThisTo(mTitleBackground, Events::RightClick, OnRightClick);
  ConnectThisTo(mTitleBackground, Events::LeftMouseDrag, OnMouseDragTitle);
}

//******************************************************************************
PropertyWidgetObject::~PropertyWidgetObject()
{
  if(mParentWidgetObject == nullptr)
    SafeDelete(mNode);

  forRange(PropertyWidget& child, ChildWidgets.All())
  {
    child.Parent = nullptr;
    child.Destroy();
  }
  ChildWidgets.Clear();
  mToolTip.SafeDestroy();
}

//******************************************************************************
void PropertyWidgetObject::OnMouseEnterTitle(MouseEvent* event)
{
  mMouseOverTitle = true;

  if(mProxyIcon)
    CreateTooltip("This Component type does not exist. Either the type was removed or scripts aren't compiling. This is referred to as being proxied.", ToolTipColor::Yellow);
  if(mLocallyRemoved)
    CreateTooltip("This Component has been locally removed from the Archetype", ToolTipColor::Red);
  if(mLocallyAdded)
    CreateTooltip("This Component has been locally added to the Archetype", ToolTipColor::Green);
  MarkAsNeedsUpdate();
}

//******************************************************************************
void PropertyWidgetObject::OnMouseExitTitle(MouseEvent* event)
{
  mMouseOverTitle = false;
  mToolTip.SafeDestroy();
  MarkAsNeedsUpdate();
}

//******************************************************************************
void PropertyWidgetObject::OnMouseEnterX(MouseEvent* e)
{
  Handle parentInstance = mParentWidgetObject->mNode->mObject;
  Handle selfInstance = mNode->mObject;

  if(parentInstance.IsNull() || selfInstance.IsNull())
  {
    Error("Invalid object in property view");
    return;
  }

  // If it cannot be removed, notify the user
  String reason;
  if(mParentComposition)
    mParentComposition->CanRemoveComponent(parentInstance, selfInstance, reason);

  // Values in MetaArray can always be removed, so no need to check here

  HighlightRed(reason);
}

//******************************************************************************
void PropertyWidgetObject::OnMouseExitX(MouseEvent* e)
{
  RemoveRedHighlight();
}

//******************************************************************************
void BuildPath(ObjectPropertyNode* node, Handle& rootInstance, PropertyPath& path)
{
  if(node->mParent)
  {
    BuildPath(node->mParent, rootInstance, path);

    if(node->mProperty)
      path.AddPropertyToPath(node->mProperty);
    else
      path.AddComponentToPath(node->mObject.StoredType->Name);
  }
  else
  {
    rootInstance = node->mObject;
  }
}

//******************************************************************************
bool PropertyWidgetObject::ArePropertiesModified()
{
  LocalModifications* modifications = LocalModifications::GetInstance();
  return modifications->IsModified(mNode->mObject, true, false);
}

//******************************************************************************
void PropertyWidgetObject::RefreshLabel()
{
  if(mLocallyRemoved)
  {
    mLabel->SetText(mLocallyRemovedTypeName);
    mLabel->SizeToContents();
    mLabel->SetColor(Vec4(1, 1, 1, 0.25f));
    return;
  }

  mLabel->SetColor(Vec4(1, 1, 1, 1));

  Handle instance = mNode->mObject;
  String text;

  if(MetaDisplay* display = instance.StoredType->HasInherited<MetaDisplay>())
    text = display->GetName(instance);
  else if(mNode && mNode->mProperty)
    text = mNode->mProperty->Name;
  else
    text = instance.StoredType->Name;

  if (ArePropertiesModified())
    mLabel->SetColor(PropertyViewUi::ModifiedTextColor);

  mLabel->SetText(text);
  mLabel->SizeToContents();
}

//******************************************************************************
void PropertyWidgetObject::Refresh()
{
  RefreshLabel();

  InList<PropertyWidget>::range r = ChildWidgets.All();
  while(!r.Empty())
  {
    r.Front().Refresh();
    r.PopFront();
  }
}

//******************************************************************************
void PropertyWidgetObject::AddSubProperty(PropertyWidget* newChild)
{
  ChildWidgets.PushBack(newChild);
  newChild->Parent = this;
}

//******************************************************************************
void PropertyWidgetObject::UpdateTransform()
{
  // Enable clipping only while we're animating
  SetClipping(mAnimating);

  bool isRoot = (mParentWidgetObject == nullptr);

  // If this is the root object, we need to disable the title bar Ui 
  // and not indent the properties. This could be done better
  if(isRoot)
  {
    mBackground->SetVisible(false);
    mTitleBackground->SetVisible(false);
    mLabel->SetActive(false);
    mExpandNode->SetVisible(false);
    if(mEditScriptButton)
      mEditScriptButton->SetVisible(false);
  }
  else
  {
    // Fill the background
    mBackground->SetSize(mSize);

    // The title is at the very top
    mTitleBackground->SetSize(Vec2(mSize.x, PropertyViewUi::ObjectSize));

    // Let the animations modify the color of the title bar
    if(!mAnimating)
    {
      if(mMouseOverTitle)
        mTitleBackground->SetColor(ComponentUi::TitleHighlight);
      else
        mTitleBackground->SetColor(ComponentUi::TitleColor);

      mBackground->SetColor(ComponentUi::BackgroundColor);
    }

    // Start with the expand icon on the left
    mExpandNode->SetTranslation(Pixels(0,2,0));
    if(mEditScriptButton)
      mEditScriptButton->SetTranslation(Pixels(20, 4, 0));
    if(mProxyIcon)
    {
      if (mEditScriptButton)
        mProxyIcon->SetTranslation(Pixels(36, 3, 0));
      else
        mProxyIcon->SetTranslation(Pixels(20, 3, 0));
    }

    // Center the name 
    Vec2 namePos = SnapToPixels((mTitleBackground->GetSize() * 0.5f) - (mLabel->GetSize() * 0.5f));
    namePos.y += Pixels(1);
    if(mNode && mNode->mProperty)
      namePos.x = Pixels(14);
    mLabel->SetTranslation(ToVector3(namePos));

    // Move the locally modified icon to the left of the name
    if(mLocalModificationIcon)
    {
      Vec2 iconPos = namePos;
      iconPos.x -= Pixels(5) + mLocalModificationIcon->GetSize().x;
      iconPos.y += Pixels(5);
      mLocalModificationIcon->SetTranslation(ToVector3(iconPos));
    }

    // We've pushed over the translation of the name, so we need to
    // fit the size of it correctly
    //nameLayout.Size.x = mSize.x - nameLayout.Translation.x;
    //nameLayout.Size.y = PropertyViewUi::PropertySize;
    // Shift up to center it better
    //nameLayout.Translation.y -= Pixels(2);
    //PlaceWithLayout(nameLayout, mLabel);

    // Layout the remove icon if it exists
    if(mRemoveIcon)
    {
      Vec3 rightSide = Vec3(mSize.x, 0, 0);
      rightSide.x -= mRemoveIcon->GetSize().x;
      mRemoveIcon->SetTranslation(rightSide + Pixels(0, 2, 0));
    }
  }

  // If we're dragging, don't bother laying us out
  if(!mDragging)
    LayoutChildren();

  PropertyWidget::UpdateTransform();
}

//******************************************************************************
void PropertyWidgetObject::LayoutChildren(bool animate)
{
  bool isRoot = (mParentWidgetObject == nullptr);

  // Start our child widgets below the title bar
  float currY = PropertyViewUi::ObjectSize + Pixels(2);

  // If we're the root, there's no title bar, so start at the top
  if(isRoot)
    currY = 0.0f;

  BoundType* widgetObjectType = ZilchTypeId(PropertyWidgetObject);

  Vec2 parentSize = GetParent()->GetSize();

  float childWidth = mSize.x - PropertyViewUi::IndentSize * 2.0f;

  // Layout each child widget
  forRange(PropertyWidget& child, ChildWidgets.All())
  {
    bool childIsObject = (ZilchVirtualTypeId(&child) == widgetObjectType);

    child.mSize.x = childWidth;

    // Force an update transform on the child because we need
    // its size to be correct for us to lay it out
    child.UpdateTransformExternal();

    // Add extra spacing before the AddComponent button
    if (ZilchVirtualTypeId(&child) == ZilchTypeId(AddObjectWidget))
      currY += PropertyViewUi::PropertySpacing;

    // The height of the current object
    float childHeight = child.mSize.y;

    // If it's the root, don't indent it in
    if(isRoot)
    {
      if(animate)
      {
        ActionSequence* sequence = new ActionSequence(&child);
        child.mDestination = Vec3(PropertyViewUi::IndentSize, currY, 0);
        sequence->Add(MoveWidgetAction(&child, child.mDestination, ComponentUi::OpenTime));
      }
      else
      {
        child.mDestination = Vec3(PropertyViewUi::IndentSize, currY, 0);
        child.SetSize(Vec2(childWidth, childHeight));
        child.SetTranslation(child.mDestination);
      }
    }
    else
    {
      child.mDestination = Vec3(PropertyViewUi::IndentSize, currY, 0);
      child.SetTranslation(child.mDestination);
      // Remove an extra two pixels from the right side for aesthetics
      if (childIsObject)
        child.SetSize(Vec2(childWidth + PropertyViewUi::IndentSize, childHeight));
      else
        child.SetSize(Vec2(childWidth, childHeight));
    }

    // Move to the next child
    currY += (childHeight + PropertyViewUi::PropertySpacing);
  }

  // Layout custom ui
  if(mCustomUi)
  {
    mCustomUi->SetTranslation(Vec3(PropertyViewUi::IndentSize, currY, 0));
    float height = mCustomUi->mSize.y;
    mCustomUi->SetSize(Vec2(childWidth, height));
    currY += (height + PropertyViewUi::PropertySpacing);
  }

  // Only update our size if we're not animating
  if(!mAnimating)
  {
    // If we don't have any children, just set our size to the title bar size
    if(ChildWidgets.Empty())
      SetSize(Vec2(parentSize.x - PropertyViewUi::IndentSize * 2.0f,  PropertyViewUi::ObjectSize));
    // Otherwise, set our size to the total size of the children
    else
      SetSize(Vec2(parentSize.x - PropertyViewUi::IndentSize * 2.0f, currY + PropertyViewUi::PropertySpacing));
  }
}

//******************************************************************************
void PropertyWidgetObject::CloseNode()
{
  if(mNodeState == NodeState::Closed)
    return;

  mNodeState = NodeState::Closed;
  mExpandedTypes.Erase(mNode->mObject.StoredType->Name);

  //Change the icon
  mExpandNode->ChangeDefinition(mDefSet->GetDefinition(cPropArrowRight));

  //Destroy all children
  InList<PropertyWidget>::range r = ChildWidgets.All();
  while(!r.Empty())
  {
    r.Front().Destroy();
    r.PopFront();
  }

  // Destroy the custom ui
  SafeDestroy(mCustomUi);

  this->MarkAsNeedsUpdate();
  //Signal re-layout
  mGrid->MarkAsNeedsUpdate(true);
}

//******************************************************************************
void PropertyWidgetObject::AnimateCloseNode()
{
  mAnimating = true;
  GetActions()->Cancel();
  ActionSequence* sequence = new ActionSequence(this);
  Vec2 destinationSize = Vec2(mSize.x, PropertyViewUi::ObjectSize);
  sequence->Add(SizeWidgetAction(this, destinationSize, ComponentUi::OpenTime));
  sequence->Add(new CallAction<ZilchSelf, &ZilchSelf::CloseNode>(this));
  sequence->Add(new ActionDelayOnce());
  sequence->Add(new CallAction<ZilchSelf, &ZilchSelf::AnimationFinished>(this));
}

//******************************************************************************
MetaPropertyEditor* GetPropertyEditor(Property* property)
{
  // First check the property for an editor extension
  if(EditorPropertyExtension* extension = property->HasInherited<EditorPropertyExtension>())
  {
    BoundType* extensionType = ZilchVirtualTypeId(extension);
    MetaPropertyEditor* editor = extensionType->HasInherited<MetaPropertyEditor>();
    ErrorIf(editor == nullptr, "Property extension didn't have a property Editor.");
    return editor;
  }

  // Check the property type
  return property->PropertyType->HasInherited<MetaPropertyEditor>();
}

//******************************************************************************
void PropertyWidgetObject::OpenNode(bool animate)
{
  mAnimating = false;

  if(mNodeState == NodeState::Open)
    return;

  mNodeState = NodeState::Open;
  mExpandedTypes.Insert(mNode->mObject.StoredType->Name);

  //Change the icon
  mExpandNode->ChangeDefinition(mDefSet->GetDefinition(cPropArrowDown));

  Handle instance = mNode->mObject;
  BoundType* boundType = instance.StoredType;

  // Create an initializer for all sub objects that we create
  PropertyWidgetInitializer initializer;
  initializer.Instance = instance;
  initializer.Grid = mGrid;
  initializer.Parent = this;
  initializer.CurrentInterface = mProp;
  initializer.ObjectNode = nullptr;

  // Expand properties
  forRange(ObjectPropertyNode* propertyNode, mNode->mProperties.All())
  {
    Property* property = propertyNode->mProperty;

    // Get the type of the property
    BoundType* propertyType = Type::GetBoundType(property->PropertyType);

    // Check to see if there's a custom filter hiding this property
    if(MetaPropertyFilter* filter = property->HasInherited<MetaPropertyFilter>())
    {
      if(MetaSelection* selection = instance.Get<MetaSelection*>())
      {
        bool shouldShow = false;

        forRange(Handle currInstance, selection->All())
        {
          shouldShow |= filter->Filter(property, currInstance);
          if (shouldShow)
            break;
        }

        if (shouldShow == false)
          continue;
      }
      else
      {
        bool visible = filter->Filter(property, instance);
        if (!visible)
          continue;
      }
    }

    // Create the property editor
    if(MetaPropertyEditor* editor = GetPropertyEditor(property))
    {
      initializer.Property = property;
      initializer.ObjectNode = propertyNode;
      PropertyWidget* propWidget = editor->CreateWidget(initializer);
      AddSubProperty(propWidget);
    }
    else
    {
      // Set the sub node
      initializer.Property = property;
      initializer.ObjectNode = propertyNode;

      // Create and add the editor
      PropertyWidgetObject* nodeEdit = new PropertyWidgetObject(initializer, this);
      AddSubProperty(nodeEdit);
    }
  }

  initializer.Property = nullptr;

  // Expand Methods with no parameters
  forRange(Function* function, mNode->mFunctions.All())
  {
    PropertyEditAction* actionEdit = new PropertyEditAction(initializer, function, instance);
    AddSubProperty(actionEdit);
  }

  // Add Dynamically contained objects
  forRange(ObjectPropertyNode* subNode, mNode->mContainedObjects.All())
  {
    // Set the sub node
    initializer.ObjectNode = subNode;

    // Create and add the editor
    PropertyWidgetObject* nodeEdit = new PropertyWidgetObject(initializer, 
                                                              this);
    AddSubProperty(nodeEdit);
    mComponents.PushBack(nodeEdit);
  }

  // Add locally removed objects
  LocalModifications* modifications = LocalModifications::GetInstance();
  if(ObjectState* state = modifications->GetObjectState(instance))
  {
    forRange(ObjectState::ChildId removedChild, state->GetRemovedChildren())
    {
      initializer.ObjectNode = nullptr;

      // Create and add the editor
      PropertyWidgetObject* nodeEdit = new PropertyWidgetObject(initializer, this, removedChild.mTypeName);
      AddSubProperty(nodeEdit);
    }
  }

  // Add the 'Add Object' widget if we're of a composite type, and we have addable types
  bool canAdd = false;

  if(MetaComposition* composition = mComposition)
  {
    Array<BoundType*> types;
    composition->Enumerate(types, EnumerateAction::All, instance);
    canAdd = (types.Empty() == false);
  }
  else if(MetaArray* metaArray = mMetaArray)
  {
    canAdd = true;
  }

  if(canAdd)
  {
    initializer.ObjectNode = nullptr;
    AddObjectWidget* nodeEdit = new AddObjectWidget(initializer, this, mGrid, instance);
    AddSubProperty(nodeEdit);
  }

  // Add custom ui
  if(MetaCustomUi* customUi = boundType->HasInherited<MetaCustomUi>())
  {
    mCustomUi = new Composite(this);
    customUi->CreateUi(mCustomUi, instance);
    if(mCustomUi->GetChildren().Empty())
      SafeDestroy(mCustomUi);
  }

  // Signal re layout
  mGrid->MarkAsNeedsUpdate();

  if(animate)
  {
    float currY = PropertyViewUi::ObjectSize + PropertyViewUi::PropertySpacing;
    forRange(PropertyWidget& child, ChildWidgets.All())
    {
      child.UpdateTransformExternal();
      float childHeight = child.mSize.y;
      currY += childHeight + PropertyViewUi::PropertySpacing;
    }

    // Add the size of the custom ui if it exists
    if(mCustomUi)
      currY += mCustomUi->mSize.y + PropertyViewUi::PropertySpacing;

    // Extra spacing at the bottom to look nice (separates object widgets)
    currY += PropertyViewUi::PropertySpacing;

    mAnimating = true;
    GetActions()->Cancel();
    ActionSequence* sequence = new ActionSequence(this);
    Vec2 destinationSize = Vec2(mSize.x, currY);
    sequence->Add(SizeWidgetAction(this, destinationSize, ComponentUi::OpenTime));
    sequence->Add(new CallAction<ZilchSelf, &ZilchSelf::AnimationFinished>(this));
  }
}

//******************************************************************************
void PropertyWidgetObject::RemoveSelf()
{
  SetActive(false);
  Handle parentInstance = mParentWidgetObject->mNode->mObject;
  Handle selfInstance = mNode->mObject;

  if(MetaComposition* composition = mParentComposition)
  {
    composition->RemoveComponent(parentInstance, selfInstance);
  }
  else
  {
    uint index = GetComponentIndex();
    mParentMetaArray->EraseIndex(parentInstance, index);
  }


  //Since tree has changed it needs to be rebuilt
  mGrid->Invalidate();
}

//******************************************************************************
void PropertyWidgetObject::AnimateRemoveSelf()
{
  Handle parentInstance = mParentWidgetObject->mNode->mObject;
  Handle selfInstance = mNode->mObject;

  // If it cannot be removed, notify the user
  String reason;
  if(mParentComposition->CanRemoveComponent(parentInstance, selfInstance, reason) == false)
  {
    DoNotifyWarning("Can't remove component", reason);
    return;
  }

  // We want to let the animation layout ourself
  mAnimating = true;

  // Clear any active animations
  GetActions()->Cancel();

  // Animate ourself closed
  ActionSequence* sequence = new ActionSequence(this);
  Vec2 destinationSize = Vec2(mSize.x, 0);
  sequence->Add(SizeWidgetAction(this, destinationSize, ComponentUi::OpenTime * 3.0f));
  sequence->Add(new CallAction<ZilchSelf, &ZilchSelf::RemoveSelf>(this));

  // Animate the title bar to red
  ActionSequence* colorSequence = new ActionSequence(this);
  Vec4 color = ToFloatColor(ByteColorRGBA(111, 47, 47, 255));
  colorSequence->Add(AnimatePropertyGetSet(Element, Color, Ease::Quad::InOut,
                        mTitleBackground, ComponentUi::OpenTime, color));
}

//******************************************************************************
void PropertyWidgetObject::OnViewDoc(ObjectEvent* event)
{
  // View Doc
  TypeEvent e(mNode->mObject.StoredType);
  mGrid->DispatchEvent(Events::NameActivated, &e);
}

//******************************************************************************
void PropertyWidgetObject::OnViewOnlineDocs(ObjectEvent* event)
{
  // View Website with Search pre-filled
  // Set up Text
  static const String prefix = "https://docsapi.zeroengine.io/?";
  String searchTerm = mNode->mObject.StoredType->Name;
  
  // Assemble URL
  StringBuilder builder;
  builder.Append(prefix);
  builder.Append("BuildId=");
  builder.Append(GetBuildIdString());
  builder.Append("&ChangeSetDate=");
  builder.Append(GetChangeSetDateString());
  builder.Append("&ViewOnlineDocs=");
  builder.Append(mNode->mObject.StoredType->Name);
  String url = builder.ToString();

  // Open browser with URL
  Os::SystemOpenNetworkFile(url.c_str());
}

//******************************************************************************
void PropertyWidgetObject::OnRemove(ObjectEvent* event)
{
  AnimateRemoveSelf();
}

//******************************************************************************
void PropertyWidgetObject::OnRestore(ObjectEvent* e)
{
  LocalModifications* modifications = LocalModifications::GetInstance();

  OperationQueue* opQueue = Z::gEditor->GetOperationQueue();
  Handle parentObject = GetParentObject();
  ObjectState::ChildId childId(mLocallyRemovedTypeName);
  RestoreLocallyRemovedChild(opQueue, parentObject, childId);
}

//******************************************************************************
void PropertyWidgetObject::OnRightClick(MouseEvent* event)
{
  ContextMenu* menu = new ContextMenu(this);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0,0) );

  if(mLocallyRemoved)
  {
    ConnectMenu(menu, "Restore", OnRestore);
  }
  else
  {
    ConnectMenu(menu, "Remove", OnRemove);
    ConnectMenu(menu, "View Docs", OnViewDoc);
    ConnectMenu(menu, "View Online Docs", OnViewOnlineDocs);
  }
}

//******************************************************************************
void PropertyWidgetObject::OnLeftClickRemove(MouseEvent* event)
{
  RemoveSelf();
}

//******************************************************************************
void PropertyWidgetObject::OnLeftClickTitle(MouseEvent* event)
{
  if(mLocallyRemoved)
    return;

  if(mNodeState == NodeState::Closed)
    OpenNode(true);
  else
    AnimateCloseNode();
}

//--------------------------------------------------------------- Component Drag
class ComponentDrag : public MouseManipulation
{
public:
  /// We shouldn't have to use handles here because nothing can be destroyed
  /// while the manipulation is active.
  PropertyWidgetObject* mParent;
  PropertyWidgetObject* mDragObject;
  PropertyWidgetObject* mBlocking;

  /// We don't want to change the actual component order on the object until we 
  /// let go of the mouse, so we shift around in this array to keep track
  /// of the current order.
  Array<PropertyWidgetObject*> mObjects;

  static const uint cInvalidIndex = uint(-1);

  //****************************************************************************
  ComponentDrag(Mouse* mouse, PropertyWidgetObject* objectNode)
    : MouseManipulation(mouse, objectNode->mParentWidgetObject)
  {
    mBlocking = nullptr;
    mDragObject = objectNode;
    mParent = mDragObject->mParentWidgetObject;
    mParent->mDragging = true;

    forRange(PropertyWidget& child, mParent->ChildWidgets.All())
    {
      // Ignore nodes that aren't object widgets
      if(child.IsObjectWidget())
        mObjects.PushBack((PropertyWidgetObject*)(&child));
    }
  }

  //****************************************************************************
  void OnMouseMove(MouseEvent* event) override
  {
    // The mouse position 
    Vec2 mousePos = this->ToLocal(event->Position);

    // The destination we should be moved to
    uint moveIndex;
    // The object we're testing against
    uint testIndex;
    GetDestinationIndex(mousePos.y, &moveIndex, &testIndex);

    uint blockingIndex = cInvalidIndex;

    // Our current index
    uint currentIndex = mObjects.FindIndex(mDragObject);

    // If there's a test index, we want to see if it's valid to move there,
    // and if not, highlight the object and snap to it
    if(testIndex != cInvalidIndex)
    {
      // Test against the test object
      String blockingMessage;
      blockingIndex = ValidateMove(testIndex, blockingMessage);

      // Highlight the object
      if(blockingIndex != cInvalidIndex)
        HighlightBlockingChild(blockingIndex, blockingMessage);
    }

    // Swap the locations if they're different
    if(moveIndex != cInvalidIndex && moveIndex != currentIndex)
    {
      // We need to validate the move in case of a blocking component
      String blockingMessage;
      blockingIndex = ValidateMove(moveIndex, blockingMessage);

      // If there was a blocking component, we need to highlight it in red
      // to signify we cannot move there, and set our destination
      // to one before the blocking
      if(blockingIndex != cInvalidIndex)
      {
        // Highlight the blocking child
        HighlightBlockingChild(blockingIndex, blockingMessage);

        // If we were trying to move the component up, we should be one
        // down from the blocking index
        if(currentIndex < moveIndex)
          moveIndex = blockingIndex;
        // Otherwise, we should be one up from the blocking index
        else
          moveIndex = blockingIndex + 1;
      }
      else
      {
        // If there was no blocking object, fade out any existing blocking
        FadeOutBlockingHighlight();
      }

      // Swap the objects
      if(moveIndex != currentIndex)
        MoveToBeforeObject(moveIndex);
    }
    else if(blockingIndex == cInvalidIndex)
    {
      FadeOutBlockingHighlight();
    }

    // Update the position of the current dragging object
    if(mBlocking)
    {
      // Snap to the blocking object
      float newTranslation = mBlocking->mDestination.y;

      // If we're below the component blocking us, snap to the bottom of it
      if(currentIndex > blockingIndex)
        newTranslation += mBlocking->GetSize().y;
      // Otherwise, snap to the top of it
      else
        newTranslation -= mDragObject->GetSize().y;
      mDragObject->SetTranslation(Vec3(PropertyViewUi::IndentSize, newTranslation, 0));
    }
    else
    {
      // Start with the mouse's position
      float yPos = mousePos.y - PropertyViewUi::ObjectSize * 0.5f;

      // Snap to the first and last component
      float min = mObjects.Front()->mDestination.y;
      float max = mObjects.Back()->mDestination.y + mObjects.Back()->GetSize().y;
      yPos = Math::Clamp(yPos, min - Pixels(2), max - mDragObject->GetSize().y + Pixels(2));

      // Set the object we're dragging to our position
      mDragObject->SetTranslation(Vec3(PropertyViewUi::IndentSize, yPos, 0));
    }
  }

  //****************************************************************************
  uint GetObjectRealIndex(uint localIndex)
  {
    if(localIndex == mObjects.Size())
      return localIndex;

    Handle parent = mParent->mNode->mObject;
    PropertyWidgetObject* object = mObjects[localIndex];
    Handle childInstance = object->mNode->mObject;
    return mParent->mComposition->GetComponentIndex(parent, childInstance);
  }

  //****************************************************************************
  /// The moveIndex is where to move the dragging object to. The test index
  /// is which object should be tested against (not yet ready to move).
  /// One operation could be to move the dragging object as well as testing
  /// against another object.
  /// The move index and testIndex will never be set to the same, unless
  /// both are invalid.
  void GetDestinationIndex(float localMousePos, uint* moveIndex, uint* testIndex)
  {
    // Start both off as invalid
    *moveIndex = cInvalidIndex;
    *testIndex = cInvalidIndex;

    // We will be testing to move when the center of the dragging
    // passes other objects
    float objectHalfHeight = mDragObject->GetSize().y * 0.5f;

    uint dragIndex = mObjects.FindIndex(mDragObject);

    // Check all objects above the dragging object from front to back
    for(uint i = 0; i < dragIndex; ++i)
    {
      PropertyWidgetObject* child = mObjects[i];

      // The location of the object being dragged that we're testing against
      float dragLocation = localMousePos - (PropertyViewUi::ObjectSize * 0.5f);

      // Check for testing
      float childMoveTestCheck = child->mDestination.y + Pixels(2);
      childMoveTestCheck += child->GetSize().y - Pixels(4);
      if(dragLocation < childMoveTestCheck)
        *testIndex = i;

      // Check for moving
      float childMoveCheck = child->mDestination.y + child->GetSize().y * 0.5f;
      if(dragLocation < childMoveCheck)
      {
        *moveIndex = i;

        // Move moved passed the point of just testing
        *testIndex = cInvalidIndex;
      }

      // If either are valid, we're done
      if(*testIndex != cInvalidIndex || *moveIndex != cInvalidIndex)
        return;
    }

    // Check all objects below the dragging object from back to front
    for(int i = (int)(mObjects.Size() - 1); i > (int)dragIndex; --i)
    {
      PropertyWidgetObject* child = mObjects[i];

      // We cannot go below locally removed components
      if (child->mLocallyRemoved)
        break;

      // The location of the object being dragged that we're testing against
      float dragLocation = localMousePos - (PropertyViewUi::ObjectSize * 0.5f);
      dragLocation += mDragObject->GetSize().y;

      // Check for testing
      float childMoveTestCheck = child->mDestination.y + Pixels(2);
      if(childMoveTestCheck < dragLocation)
      {
        *testIndex = i + 1;
      }

      // Check for moving
      float childMoveCheck = child->mDestination.y + child->GetSize().y * 0.5f;
      if(childMoveCheck < dragLocation)
      {
        *moveIndex = i + 1;

        // Move moved passed the point of just testing
        *testIndex = cInvalidIndex;
      }

      // If either are valid, we're done
      if(*testIndex != cInvalidIndex || *moveIndex != cInvalidIndex)
        return;
    }
  }

  //****************************************************************************
  uint ValidateMove(uint destinationIndex, String& message)
  {
    // The destination index given is the index into our temporary array of
    // objects, so we need to find its real index on the object
    uint swapIndex = GetObjectRealIndex(destinationIndex);

    // Check to see if we can move this component
    Handle parent = mParent->mNode->mObject;
    Handle blocking;
    bool canMove = mParent->mComposition->CanMoveComponent(parent, mDragObject->mNode->mObject,
                                                           swapIndex, blocking, message);

    // If the component can be moved, there's no blocking index
    if(canMove)
      return uint(-1);

    // Get the index of the object that's blocking us from moving
    PropertyWidgetObject* blockingObject = GetObjectNode(blocking);
    return mObjects.FindIndex(blockingObject);
  }

  //****************************************************************************
  void MoveToBeforeObject(uint moveIndex)
  {
    InList<PropertyWidget>::Unlink(mDragObject);

    // If we're moving it to the back
    if(moveIndex == mObjects.Size())
    {
      mParent->ChildWidgets.InsertAfter(mObjects.Back(), mDragObject);
      mObjects.EraseValueError(mDragObject);
      mObjects.PushBack(mDragObject);
    }
    else
    {
      PropertyWidgetObject* moveObject = mObjects[moveIndex];
      mParent->ChildWidgets.InsertBefore(moveObject, mDragObject);

      // Update the local array
      uint currentIndex = mObjects.FindIndex(mDragObject);
      mObjects.EraseAt(currentIndex);
      if(currentIndex < moveIndex)
        mObjects.InsertAt(moveIndex - 1, mDragObject);
      else
        mObjects.InsertAt(moveIndex, mDragObject);
    }
    
    // This will animate all objects to their desired positions
    mParent->LayoutChildren(true);

    // We need to cancel the animation on the object we're dragging so that
    // we aren't fighting the animation
    mDragObject->GetActions()->Cancel();
  }

  //****************************************************************************
  void HighlightBlockingChild(uint blockingIndex, StringParam message)
  {
    PropertyWidgetObject* blockingObject = mObjects[blockingIndex];

    if(mBlocking && (mBlocking != blockingObject))
      mBlocking->RemoveRedHighlight();
    blockingObject->HighlightRed(message);

    mBlocking = blockingObject;
  }

   //****************************************************************************
  PropertyWidgetObject* GetObjectNode(HandleParam instance)
  {
    forRange(PropertyWidgetObject* child, mObjects.All())
    {
      Handle currObject = child->mNode->mObject;

      if(currObject == instance)
        return child;
    }

    return nullptr;
  }

  //****************************************************************************
  void FadeOutBlockingHighlight()
  {
    if(mBlocking)
    {
      mBlocking->RemoveRedHighlight();
      mBlocking = nullptr;
    }
  }

  //****************************************************************************
  void OnMouseUp(MouseEvent* event) override
  {
    // Now we want to make the final move commit
    uint newIndex = mObjects.FindIndex(mDragObject);
    uint oldIndex = GetObjectRealIndex(newIndex);

    if(newIndex != oldIndex)
    {
      uint insertIndex = newIndex;
      if(oldIndex < newIndex)
        insertIndex += 1;

      Handle parent = mParent->mNode->mObject;
      mParent->mComposition->MoveComponent(parent, mDragObject->mNode->mObject, insertIndex);
    }

    // Animate back to the correct positions
    mParent->LayoutChildren(true);

    // Fade out any old blocking object
    FadeOutBlockingHighlight();

    // Let them animate then end the drag
    ActionSequence* sequence = new ActionSequence(mParent);
    sequence->Add(new ActionDelay(ComponentUi::OpenTime));
    sequence->Add(new CallAction<PropertyWidgetObject,
                                 &PropertyWidgetObject::EndDrag>(mParent));

    // Destroy the manipulation
    this->Destroy();
  }
};

//******************************************************************************
void PropertyWidgetObject::OnMouseDragTitle(MouseEvent* e)
{
  if (mLocallyRemoved)
    return;

  mParentWidgetObject->StartChildDrag(e->GetMouse(), this);
}

//******************************************************************************
void PropertyWidgetObject::HighlightRed(StringParam message)
{
  mAnimating = true;

  // Animate the title
  ActionSequence* seq = new ActionSequence(this);
  Vec4 color = ComponentUi::TitleRemove;
  seq->Add(AnimatePropertyGetSet(Element, Color, Ease::Quad::InOut,
           mTitleBackground, ComponentUi::OpenTime, color));

  // Animate the background
  if(mBackground->GetVisible())
  {
    seq = new ActionSequence(this);
    color = ComponentUi::BackgroundRemove;
    seq->Add(AnimatePropertyGetSet(Element, Color, Ease::Quad::InOut,
            mBackground, ComponentUi::OpenTime, color));
  }

  // Only create a tooltip if the message is valid and we don't
  // already have one made
  if(!message.Empty() && !mToolTip)
    CreateTooltip(message, ToolTipColor::Red);
}

//******************************************************************************
void PropertyWidgetObject::RemoveRedHighlight()
{
  // Animate the title
  ActionSequence* seq = new ActionSequence(this);
  Vec4 color = ComponentUi::TitleColor;
  if(mMouseOverTitle)
    color = ComponentUi::TitleHighlight;
  seq->Add(AnimatePropertyGetSet(Element, Color, Ease::Quad::InOut,
                   mTitleBackground, ComponentUi::OpenTime, color));
  seq->Add(new CallAction<ZilchSelf, &ZilchSelf::AnimationFinished>(this));

  // Animate the background
  seq = new ActionSequence(this);
  color = ComponentUi::BackgroundColor;
  seq->Add(AnimatePropertyGetSet(Element, Color, Ease::Quad::InOut,
           mBackground, ComponentUi::OpenTime, color));

  // Destroy the tool tip
  mToolTip.SafeDestroy();
}

//******************************************************************************
void PropertyWidgetObject::CreateTooltip(StringParam message, ToolTipColor::Enum color)
{
  // Destroy it if one already exists
  mToolTip.SafeDestroy();

  // Create an indicator
  ToolTip* toolTip = new ToolTip(this);
  toolTip->SetText(message);
  toolTip->SetColor(color);
  toolTip->SetDestroyOnMouseExit(false);

  ToolTipPlacement placement;
  placement.SetScreenRect(mTitleBackground->GetScreenRect());
  placement.mScreenRect.RemoveThickness(Thickness(2,2,2,2));
  // We want the hotspot to point at the remove icon
  placement.mHotSpot = mBackground->GetScreenRect().Center() - Pixels(0, 1);
  placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left, 
                        IndicatorSide::Bottom, IndicatorSide::Top);
  toolTip->SetArrowTipTranslation(placement);

  mToolTip = toolTip;
}

//******************************************************************************
Handle PropertyWidgetObject::GetParentObject()
{
  ObjectPropertyNode* parentNode = mParentWidgetObject->mNode;
  return parentNode->mObject;
}

//******************************************************************************
void PropertyWidgetObject::StartChildDrag(Mouse* mouse, PropertyWidgetObject* child)
{
  // Don't do anything if the components can't be reordered
  if(MetaComposition* composition = mComposition)
  {
    if (!composition->mSupportsComponentReorder)
      return;
  }

  // Dragging for meta arrays is disabled until fully supported
  if (mMetaArray.IsNotNull())
    return;

  // Make sure this component draws in front of the others
  child->MoveToFront();

  // Start the drag
  new ComponentDrag(mouse, child);
}

//******************************************************************************
void PropertyWidgetObject::EndDrag()
{
  mDragging = false;
}

//******************************************************************************
uint PropertyWidgetObject::GetComponentIndex()
{
  uint index = 0;
  forRange(PropertyWidgetObject& component, mParentWidgetObject->mComponents.All())
  {
    if(&component == this)
      return index;
    ++index;
  }

  return uint(-1);
}

//******************************************************************************
void PropertyWidgetObject::OnMetaModified(MetaTypeEvent* event)
{
  // Widget is destroy but may still get the event
  if(mDestroyed)
    return;

  // Is the type is node refers to been modified?
  // Rebuild the tree. This reloads properties that
  // may have been added or removed.
  Handle handle = mNode->mObject;
  BoundType* thisType = handle.StoredType;

  // METAREFACTOR - IsSameOrProxy - we're removing proxies, right?
  // This meta type or if this type is a proxy
  //if(thisType->IsSameOrProxy(event->Type))
  if(thisType->IsA(event->Type))
    mGrid->Invalidate();
}

//******************************************************************************
void PropertyWidgetObject::OnEditScriptPressed(Event* e)
{
  ResourceId resourceId = mNode->mObject.StoredType->HasInherited<MetaResource>()->mResourceId;
  Resource* resource = Z::gResources->GetResource(resourceId);
  ReturnIf(resource == nullptr, , "Could not find resource to edit");
  Z::gEditor->EditResource(resource);
}

}//namespace Zero
