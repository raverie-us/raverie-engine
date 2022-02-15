// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

PropertyEditAction::PropertyEditAction(PropertyWidgetInitializer& init, Function* method, HandleParam instance) :
    PropertyWidget(init)
{
  mLabel->SetActive(false);
  mMethod = method;
  mInstance = instance;
  mAction = CreateAttached<Element>("PropInvoke");
  mButton = new TextButton(this);
  mButton->SetText(method->Name);
  ConnectThisTo(mButton, Events::ButtonPressed, OnButtonPress);

  mButton->SetSizing(SizePolicy::Flex, 1.0f);
  Thickness thickness(20, 0, 0, 0);
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, thickness));
}

void PropertyEditAction::OnButtonPress(Event* event)
{
  mProp->InvokeFunction(mInstance, mMethod);

  if (mMethod->HasAttribute(FunctionAttributes::cInvalidatesObject))
    mProp->mPropertyGrid->Invalidate();
}

ZilchDefineType(AddObjectWidget, builder, type)
{
}

AddObjectWidget::AddObjectWidget(PropertyWidgetInitializer& init,
                                 PropertyWidgetObject* parentNode,
                                 PropertyView* grid,
                                 HandleParam instance) :
    PropertyWidget(init)
{
  mMouseOver = false;
  mParentWidgetObject = parentNode;

  mLabel->SetText("Add");
  mObject = instance;
  mComposition = parentNode->mNode->mComposition;
  mMetaArray = parentNode->mNode->mMetaArray;

  String addName;
  if (mComposition)
    addName = mComposition->GetAddName();
  else if (mMetaArray)
    addName = mMetaArray->mContainedType->Name;
  String text = BuildString("Add ", addName, "...");
  mLabel->SetText(text);
  mLabel->SizeToContents();

  mAddIcon = CreateAttached<Element>("Plus");
  mAddIcon->SetSize(Pixels(16, 16));
  mAddIcon->SetInteractive(false);
  mAddIcon->SetVisible(false);

  if (mComposition)
    ConnectThisTo(MetaDatabase::GetInstance(), Events::MetaModified, OnMetaModified);

  mBackground = CreateAttached<Element>(cWhiteSquare);
  ConnectThisTo(this, Events::MouseEnterHierarchy, OnMouseEnter);
  ConnectThisTo(this, Events::MouseExitHierarchy, OnMouseExit);
  ConnectThisTo(this, Events::LeftClick, OnLeftClick);

  ConnectThisTo(mGrid, Events::OpenAdd, OnOpenAdd);

  mBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mBorder->SetColor(Vec4(0.867f, 0.46f, 0.2f, 1.0f));
}

FloatingSearchView* AddObjectWidget::OpenSearch(Vec3 position)
{
  FloatingSearchView* viewPopUp = new FloatingSearchView(this);
  viewPopUp->SetSize(Pixels(mSize.x, 400));
  viewPopUp->UpdateTransformExternal();
  viewPopUp->ShiftOntoScreen(GetScreenPosition());
  viewPopUp->UpdateTransformExternal();

  SearchView* searchView = viewPopUp->mView;
  SearchProvider* provider = GetFactoryProvider(mObject, mComposition);
  searchView->mSearch->SearchProviders.PushBack(provider);
  searchView->Search(String());
  searchView->TakeFocus();

  mActiveSearch = viewPopUp;

  ConnectThisTo(provider, Events::AlternateSearchCompleted, OnAlternateSearchCompleted);
  ConnectThisTo(searchView, Events::SearchCompleted, OnSearchCompleted);

  return viewPopUp;
}

void AddObjectWidget::OnLeftClick(MouseEvent* event)
{
  if (Z::gEngine->IsReadOnly())
  {
    DoNotifyWarning("Property View", "Cannot add components while in read-only mode");
    return;
  }

  Handle parentObject = mParentWidgetObject->mNode->mObject;
  if (MetaArray* metaArray = mMetaArray)
  {
    Any value;
    value.DefaultConstruct(metaArray->mContainedType);
    metaArray->Add(parentObject, value);
    // mProp->AddComponent(parentObject, String());
    mParentWidgetObject->mGrid->Invalidate();
  }
  else
  {
    Vec3 mousePos = ToVector3(event->GetMouse()->GetClientPosition());
    OpenSearch(mousePos);
  }
}

void AddObjectWidget::OnPostResourceAdded(PostAddResourceEvent* event)
{
  Resource* resource = event->mResourceAdd->SourceResource;
  BoundType* resourceType = ZilchVirtualTypeId(resource);

  // If a cog, or selection of cogs, invoked the add resource dialog to create
  // a ZilchComponent - then add that new component to those compositions.
  if (resourceType == ZilchTypeId(ZilchScript))
  {
    String componentName = event->mResourceAdd->Name;

    // This should never return a valid type. When scripts are added, we do not
    // compile until the next update; however to future proof it incase we
    // ever change it - first search for the type, then create a proxy
    // if it doesn't exist.
    BoundType* componentType = MetaDatabase::FindType(componentName);

    if (componentType == nullptr)
    {
      // Create a Proxy to be used for this component
      componentType = ProxyObject<Component>::CreateProxyType(componentName, ProxyReason::TypeDidntExist);
      if (componentType == nullptr)
      {
        Error("Could not create proxy type");
        return;
      }

      MetaResource* metaResource = componentType->HasOrAdd<MetaResource>();
      metaResource->SetResource(resource);
    }

    mComposition->AddComponent(mObject, componentType);
  }
}

void AddObjectWidget::OnAlternateSearchCompleted(AlternateSearchCompletedEvent* event)
{
  AddResourceWindow* addDialog = OpenAddWindow(ZilchTypeId(ZilchScript), nullptr, event->mSearchText);
  ConnectThisTo(addDialog, Events::PostAddResource, OnPostResourceAdded);
}

void AddObjectWidget::OnSearchCompleted(SearchViewEvent* event)
{
  if (Z::gEngine->IsReadOnly())
  {
    DoNotifyWarning("Property View", "Cannot add components while in read-only mode");
    return;
  }

  BoundType* boundType = (BoundType*)event->Element->Data;
  Handle parentObject = mParentWidgetObject->mNode->mObject;

  mActiveSearch.SafeDestroy();

  if (parentObject.IsNull())
    return;

  mComposition->AddComponent(parentObject, boundType);
}

void AddObjectWidget::OnMetaModified(Event* event)
{
  mActiveSearch.SafeDestroy();
}

void AddObjectWidget::UpdateTransform()
{
  mBackground->SetSize(mSize);
  mBorder->SetSize(mSize);
  mBorder->SetColor(Vec4(0.867f, 0.46f, 0.2f, 0.75f));
  mBackground->MoveToBack();
  if (mMouseOver)
    mBackground->SetColor(ComponentUi::TitleHighlight);
  else
    mBackground->SetColor(ComponentUi::TitleColor);

  float center = mSize.x * 0.5f;
  float totalWidth = mAddIcon->mSize.x + Pixels(4) + mLabel->mSize.x;

  float iconPos = center - totalWidth * 0.5f;
  float labelPos = iconPos + mAddIcon->mSize.x + Pixels(4);

  mAddIcon->SetTranslation(Vec3(iconPos, 1, 0));
  mLabel->SetTranslation(Vec3(labelPos, 0, 0));

  PropertyWidget::UpdateTransform();
}

void AddObjectWidget::OnMouseEnter(MouseEvent* event)
{
  mMouseOver = true;
  MarkAsNeedsUpdate();
}

void AddObjectWidget::OnMouseExit(MouseEvent* event)
{
  mMouseOver = false;
  MarkAsNeedsUpdate();
}

void AddObjectWidget::OnOpenAdd(Event* e)
{
  OpenSearch(this->GetScreenPosition());
}

} // namespace Zero
