///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Andrew Colean
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace NetPropertyUi
{
  const cstr cLocation = "EditorUi/Net";
  Tweakable(Vec4, DisabledNetPropertyIcon, Vec4(1, 1, 1, 1), cLocation);
  Tweakable(Vec4, EnabledNetPropertyIcon,  Vec4(1, 1, 1, 1), cLocation);
}

//---------------------------------------------------------------------------------//
//                               NetPropertyIcon                                   //
//---------------------------------------------------------------------------------//

//******************************************************************************
NetPropertyIcon::NetPropertyIcon(Composite* parent, HandleParam object,
                                 Property* metaProperty)
  : Composite(parent),
    mMouseOver(false),
    mIcon(nullptr),
    mComponentHandle(object),
    mProperty(metaProperty),
    mActiveSearch(),
    mTooltip()
{
  // Create net property icon element
  mIcon = this->CreateAttached<Element>("NetworkIcon");
  Assert(mIcon);

  // Unsupported net property?
  if(!IsValidNetProperty(metaProperty))
    mIcon->SetActive(false); // Hide net property icon

  // Size our widget to the icon's size
  SetSize(mIcon->GetSize());

  // Connect event handlers
  ConnectThisTo(mIcon, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(mIcon, Events::MouseExit, OnMouseExit);
  ConnectThisTo(mIcon, Events::LeftClick, OnLeftClick);
  ConnectThisTo(mIcon, Events::RightClick, OnRightClick);
}

//
// Composite Interface
//

//******************************************************************************
void NetPropertyIcon::UpdateTransform()
{
  // Get component
  Component* component = mComponentHandle.Get<Component*>();
  if(component == nullptr)
  {
    // Update icon transform
    Composite::UpdateTransform();
    return;
  }

  // Set highlight color during mouse over
  float highlight = mMouseOver ? 1.2f : 0.9f;
  Vec4 color;


  // Get net object
  NetObject* netObject = component->GetOwner()->has(NetObject);
  if(netObject == nullptr) // Unable?
  {
    // Hide net property icon
    mIcon->SetActive(false);
    // Update icon transform
    Composite::UpdateTransform();
    return;
  }

  // Get component meta type
  BoundType* componentType = ZilchVirtualTypeId(component);

  // Is the net object component?
  if(componentType->IsA(ZilchTypeId(NetObject)))
  {
    // Hide net property icon
    mIcon->SetActive(false);
    // Update icon transform
    Composite::UpdateTransform();
    return;
  }

  // Get property name
  String propertyName = mProperty->Name;

  //    Has net property attribute?
  // OR Net object has this net property info added? (Property is enabled for network replication?)
  if(mProperty->HasAttribute(cNetProperty)
  || netObject->HasNetPropertyInfo(componentType, propertyName))
    color = NetPropertyUi::EnabledNetPropertyIcon; // Use enabled color
  else
    color = NetPropertyUi::DisabledNetPropertyIcon; // Use disabled color

  // Update icon color
  color *= highlight;
  color = Math::Clamp(color, Vec4::cZero, Vec4(1, 1, 1, 1));
  color.w = 1.0f;
  mIcon->SetColor(color);
  // Update icon transform
  Composite::UpdateTransform();
}

//
// Event Handlers
//

//******************************************************************************
void NetPropertyIcon::OnMouseEnter(Event* event)
{
  // Create description tooltip
  ToolTip* toolTip = new ToolTip(this);
  toolTip->SetDestroyOnMouseExit(false);
  toolTip->mContentPadding = Thickness(2, 2, 2, 2);

  // DebugPrint("NetPropertyIcon: [%f, %f]\n", mTranslation.x, mTranslation.y);

  // Has net property attribute?
  if(mProperty->HasAttribute(cNetProperty))
    toolTip->SetText("This property will always be replicated over the network. Specified in script via the [NetProperty] attribute.");
  else
  {
    // Property is enabled for network replication?
    if(IsEnabled())
      toolTip->SetText("This property will be replicated over the network. Right click to choose a channel.");
    // Property is not enabled for network replication?
    else
      toolTip->SetText("Left click to replicate this property over the network.");

    // Is mousing over
    mMouseOver = true;
    UpdateTransform(); // (Update icon color)
  }

  // Place tooltip
  ToolTipPlacement placement;
  placement.SetScreenRect(GetParent()->GetParent()->GetScreenRect());
  placement.mHotSpot = mIcon->GetScreenRect().Center();
  placement.SetPriority(IndicatorSide::Left, IndicatorSide::Right,
                        IndicatorSide::Bottom, IndicatorSide::Top);
  toolTip->SetArrowTipTranslation(placement);
  mTooltip = toolTip;

  // Mark as needs an update
  MarkAsNeedsUpdate();
}

//****************************************************************************
void NetPropertyIcon::OnMouseExit(Event* event)
{
  // Destroy tooltip
  mTooltip.SafeDestroy();

  // Is not mousing over
  mMouseOver = false;
  UpdateTransform(); // (Update icon color)

  // Mark as needs an update
  MarkAsNeedsUpdate();
}

//******************************************************************************
void NetPropertyIcon::OnLeftClick(Event* event)
{
  // Get component
  Component* component = mComponentHandle.Get<Component*>();

  if(component == nullptr)
    return;

  // Get net object
  NetObject* netObject = component->GetOwner()->has(NetObject);
  if(netObject == nullptr) // Unable?
  {
    Error("NetPropertyIcon was visible on a Cog without a NetObject component");

    // Hide net property icon
    mIcon->SetActive(false);
    return;
  }

  // Get component meta type
  BoundType* componentType = ZilchVirtualTypeId(component);

  // Is the net object component?
  if(componentType->IsA(ZilchTypeId(NetObject)))
  {
    // Hide net property icon
    mIcon->SetActive(false);
    return;
  }

  // Get property name
  String propertyName = mProperty->Name;

  // Get operation queue
  OperationQueue* opQueue = Z::gEditor->GetOperationQueue();

  // Does not have net property attribute?
  if(!mProperty->HasAttribute(cNetProperty))
  {
    // Net object has this net property info added? (Property is enabled for network replication?)
    if(netObject->HasNetPropertyInfo(componentType, propertyName))
    {
      // (Operation) Remove net property info
      RemoveNetPropertyInfo(opQueue, netObject, componentType, propertyName);
    }
    else
    {
      // (Operation) Add net property info
      AddNetPropertyInfo(opQueue, netObject, componentType, propertyName);
    }
  }

  // Destroy tooltip
  mTooltip.SafeDestroy();

  // Update tooltip using mouse enter again
  OnMouseEnter(event);

  // Mark as needs an update
  MarkAsNeedsUpdate();
}

//******************************************************************************
void NetPropertyIcon::OnRightClick(MouseEvent* event)
{
  // Property is not enabled for network replication?
  if(!IsEnabled())
    return;

  // No active search view?
  FloatingSearchView* searchView = mActiveSearch;
  if(searchView == NULL)
  {
    // Display NetChannelConfig resource list
    FloatingSearchView* viewPopUp = new FloatingSearchView(this);
    Vec3 mousePos = ToVector3(event->GetMouse()->GetClientPosition());
    SearchView* searchView = viewPopUp->mView;
    viewPopUp->SetSize(Pixels(300, 400));
    viewPopUp->ShiftOntoScreen(mousePos);
    viewPopUp->UpdateTransformExternal();

    searchView->AddHiddenTag("Resources");
    searchView->AddHiddenTag("NetChannelConfig");
    searchView->mSearch->SearchProviders.PushBack(GetResourceSearchProvider());

    searchView->TakeFocus();
    viewPopUp->UpdateTransformExternal();
    searchView->Search(String());

    // Connect to search result event
    ConnectThisTo(searchView, Events::SearchCompleted, OnSearchCompleted);

    mActiveSearch = viewPopUp;

    // We don't want the Text object under this to grab focus from us
    event->Handled = true;
  }
}

//****************************************************************************
void NetPropertyIcon::OnSearchCompleted(SearchViewEvent* event)
{
  // Get net channel configuration resource from search result
  NetChannelConfig* netChannelConfig = (NetChannelConfig*)event->Element->Data;

  // Destroy active search
  mActiveSearch.SafeDestroy();

  // Get component
  Component* component = mComponentHandle.Get<Component*>();
  if(component == nullptr)
    return;

  // Get net object
  NetObject* netObject = component->GetOwner()->has(NetObject);
  if(netObject == nullptr) // Unable?
    return;

  // Get component meta type
  BoundType* componentType = ZilchVirtualTypeId(component);

  // Get property name
  String propertyName = mProperty->Name;

  // Get operation queue
  OperationQueue* opQueue = Z::gEditor->GetOperationQueue();

  // (Operation) Remove net property info
  SetNetPropertyInfoChannel(opQueue, netObject, componentType, propertyName, netChannelConfig);
}

//
// Icon Interface
//

//****************************************************************************
bool NetPropertyIcon::IsEnabled()
{
  return GetNetPropertyInfo() != nullptr;
}

//****************************************************************************
NetPropertyInfo* NetPropertyIcon::GetNetPropertyInfo()
{
  // Get component
  Component* component = mComponentHandle.Get<Component*>();
  if(component == nullptr)
    return nullptr;

  // Get net object
  NetObject* netObject = component->GetOwner()->has(NetObject);
  if(netObject == nullptr) // Unable?
    return nullptr;

  // Get component meta type
  BoundType* componentType = ZilchVirtualTypeId(component);

  // Get property name
  String propertyName = mProperty->Name;

  // Return net property info added on net object (if it exists)
  return netObject->GetNetPropertyInfo(componentType, propertyName);
}

//******************************************************************************
bool ShouldDisplayNetPropertyIcon(HandleParam selectedObject)
{
  // Is a single Cog selection?
  if(Cog* cog = selectedObject.Get<Cog*>())
  {
    // Has NetObject component?
    if(cog->has(NetObject))
    {
      // Should display net property icons
      return true;
    }
  }

  // Disabled until we have the new property interface changes

  //else if(MetaSelection* selection = selectedObject.As<MetaSelection>(nullptr))
  //{
  //  // To display the icon for multi-select, all objects in the selection
  //  // must be Cogs with the NetObject Component
  //  forRange(MetaObjectInstance object, selection->All())
  //  {
  //    Cog* cog = object.As<Cog>(nullptr);
  //    if(cog == nullptr || !cog->has(NetObject))
  //      return false;
  //  }
  //
  //  return true;
  //}

  // Otherwise, should not display net property icons
  return false;
}

//******************************************************************************
Widget* CreateNetPropertyIcon(Composite* parent, HandleParam object,
                              Property* metaProperty, void* clientData)
{
  // Is a component property?
  if(metaProperty && object.Get<Component*>())
  {
    // To Do:
    // We need to determine if we're representing a single object or multiple.
    // We can do this by searching up the object tree that was used to create the 
    // property grid.

    // Create net property icon
    return new NetPropertyIcon(parent, object, metaProperty);
  }

  // Do not create net property icon
  return nullptr;
}

}//namespace Zero
