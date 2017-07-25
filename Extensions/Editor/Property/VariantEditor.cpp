///////////////////////////////////////////////////////////////////////////////
///
/// \file TreeView.cpp
/// Implementation of Tree View
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(TextUpdated);
} // namespace Events

//----------------------------------------------------------------------- Meta Initialization
ZilchDefineType(TextUpdatedEvent, builder, type)
{
  ZilchBindField(mChangeAccepted);
}

Zero::Object* TextUpdatedEvent::GetSource()
{
  return mSource;
}

ZilchDefineType(FormattedInPlaceText, builder, type)
{
  ZilchBindDefaultCopyDestructor();
}

//------------------------------------------------------------------ Text Editor
ZilchDefineType(InPlaceTextEditor, builder, type)
{

}

InPlaceTextEditor::InPlaceTextEditor(Composite* parent, u32 flags)
  :ValueEditor(parent)
{
  mText = new Label(this);
  mText->SetTextClipping(true);
  mEdit = nullptr;

  mAdditionalText = new Label(this);

  if(flags & InPlaceTextEditorFlags::EditOnDoubleClick)
    ConnectThisTo(this, Events::DoubleClick, OnDoubleClick);
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
}

void InPlaceTextEditor::SizeToContents()
{
  mText->SizeToContents();
  SetSize(mText->GetSize());
}

void InPlaceTextEditor::UpdateTransform()
{
  //if(mCustomIcon->GetActive())
  //{
  //  Vec3 pos(0, (mSize.y * 0.5) - (mCustomIcon->mSize.y * 0.5), 0);
  //  //pos.x += Pixels(5);
  //  mCustomIcon->SetTranslation(SnapToPixels(pos));
  //  //mCustomIcon->SetTranslation(Vec3::cZero);
  //}
  //float iconWidth = GetIconsWidth();

  // Place the text
  mText->SetTranslation(Pixels(0, 0, 0));
  mText->SizeToContents();

  // Place the hint text
  if(mAdditionalText->GetActive())
  {
    Vec2 textSize = mText->GetSize();
    mAdditionalText->SetTranslation(Pixels(textSize.x, 0, 0));
  }

  // Place icons in reverse order
  float currIconPos = mSize.x;
  for(uint i = mCustomIcons.Size() - 1; i < mCustomIcons.Size(); --i)
  {
    Element* icon = mCustomIcons[i];

    currIconPos -= icon->GetSize().x;
    
    // Padding
    currIconPos -= Pixels(2);

    icon->SetTranslation(Vec3(currIconPos, Pixels(5), 0));
  }
  SizeAllText();
  Composite::UpdateTransform();
}

void InPlaceTextEditor::Edit()
{
  if (mEdit)
    return;

  mText->SetVisible(false);

  TextBox* edit = new TextBox(this);
  edit->SetTranslation(Vec3(0, 0, 0) );
  edit->SetText( mText->GetText() );
  edit->SetSize( this->GetSize() - Vec2(GetIconsWidth(), 0) );
  edit->SetEditable(true);
  edit->TakeFocus();

  ConnectThisTo(edit, Events::TextBoxChanged, OnTextBoxChanged);
  ConnectThisTo(edit, Events::FocusLost, OnTextLostFocus);

  mEdit = edit;
  // Causes edit box resize when selected from right click menu
  Composite::UpdateTransform();
}

void InPlaceTextEditor::SetVariant(AnyParam variant)
{
  if(variant.StoredType == ZilchTypeId(FormattedInPlaceText))
  {
    FormattedInPlaceText formatting = variant.Get<FormattedInPlaceText>();
    mText->SetText(formatting.mText);
    mText->SetColor(formatting.mTextColor);

    // Update hint text
    mAdditionalText->SetText(formatting.mAdditionalText);
    mAdditionalText->SetColor(formatting.mAdditionalTextColor);
    mAdditionalText->SetActive(!formatting.mAdditionalText.Empty());

    // Update custom icons
    uint i = 0;
    for(; i < formatting.mCustomIcons.Size(); ++i)
    {
      String iconDefinition = formatting.mCustomIcons[i];

      Element* currIcon = nullptr;
      if(i >= mCustomIcons.Size())
      {
        currIcon = CreateAttached<Element>(iconDefinition);
        mCustomIcons.PushBack(currIcon);
      }
      else
      {
        currIcon = mCustomIcons[i];
        BaseDefinition* definition = mDefSet->GetDefinitionOrNull(iconDefinition);
        currIcon->ChangeDefinition(definition);
      }

      currIcon->SizeToContents();
    }

    // Delete any extra icons
    while(i < mCustomIcons.Size())
    {
      mCustomIcons.Back()->Destroy();
      mCustomIcons.PopBack();
    }
  }
  else
  {
    String text = variant.ToString();
    mText->SetText(text);
    mAdditionalText->SetActive(false);
  }
  // No matter which text is set we need to size the text itself to the text it contains
  mText->SizeToContents();
}

void InPlaceTextEditor::GetDragWidgets(Array<Widget*>& widgets)
{
  widgets.PushBack(mText);
  widgets.PushBack(mAdditionalText);
}

void InPlaceTextEditor::GetVariant(Any& variant)
{
  String value = mText->GetText();
  variant = value;
}

void InPlaceTextEditor::GetEditTextVariant(Any& variant)
{
  String value = mEdit->GetText();
  variant = value;
}

void InPlaceTextEditor::SizeAllText()
{
  // check if we have width left after just the text to display additional information
  float remainingWidth = mSize.x - mText->GetMinSize().x;
  if (remainingWidth > 0)
  {
    // size the text to itself
    mText->SizeToContents();
    // get the width remaining
    Vec2 size;
    size.x = remainingWidth;
    size.y = mSize.y;

    // And size the additional information text to what space remains
    Vec2 additionalMinSize = mAdditionalText->GetMinSize();
    mAdditionalText->SetSize(Math::Min(size, additionalMinSize));
  }
  else
    mText->SetSize(mSize);
}

void InPlaceTextEditor::OnTextBoxChanged(ObjectEvent* event)
{
  if(mEdit)
  {
    TextUpdatedEvent* textEvent = new TextUpdatedEvent(this);
    this->DispatchEvent(Events::TextUpdated, textEvent);
    //Store the new value if there was no resource name conflict
    if(textEvent->mChangeAccepted)
      mText->SetText( mEdit->GetText() );
    mText->SetVisible(true);
    mEdit->Destroy();
    delete textEvent;
    mEdit = nullptr;
  }
}

void InPlaceTextEditor::OnKeyDown(KeyboardEvent* event)
{
  if(Editable && event->Key == Keys::F2)
    Edit();
}

void InPlaceTextEditor::OnTextLostFocus(FocusEvent* event)
{
  if (mEdit)
  {
    mText->SetVisible(true);
    mEdit->Destroy();
    mEdit = nullptr;
  }
}

void InPlaceTextEditor::OnDoubleClick(Event* event)
{
  Edit();
}

float InPlaceTextEditor::GetIconsWidth()
{
  float width = 0.0f;
  for(uint i = 0; i < mCustomIcons.Size(); ++i)
  {
    width += mCustomIcons[i]->GetSize().x;

    // Padding
    width += Pixels(2.0f);
  }
  return width;
}

ValueEditor* CreateInPlaceTextEditor(Composite* composite, AnyParam data, u32 flags)
{
  return new InPlaceTextEditor(composite, flags);
}

//-------------------------------------------------------------- Resource Editor
class ResourceDisplay : public ValueEditor
{
public:
  typedef ResourceDisplay ZilchSelf;

  String mResourceIdName;
  String mResourceType;
  TextBox* mText;
  HandleOf<FloatingSearchView> mActiveSearch;

  ResourceDisplay(Composite* parent, AnyParam resourceType)
    : ValueEditor(parent)
  {
    mResourceType = resourceType.Get<String>();
    mText = new TextBox(this);
    mText->HideBackground(true);
    mText->SetEditable(false);
    mText->SetTextClipping(true);
    mText->SetTranslation(Pixels(0,-1,0));
    mText->SetText("TEST RESOURCE");

    ConnectThisTo(mText, Events::DoubleClick, OnDoubleClick);
  }

  void OnDoubleClick(MouseEvent* event)
  {
    if(!Editable)
      return;

    FloatingSearchView* searchView = mActiveSearch;
    if(searchView == nullptr)
    {
      FloatingSearchView* viewPopUp = new FloatingSearchView(this);
      Vec3 mousePos = ToVector3(event->GetMouse()->GetClientPosition());
      SearchView* searchView = viewPopUp->mView;
      viewPopUp->SetSize(Pixels(300,400));
      viewPopUp->ShiftOntoScreen(mousePos);
      viewPopUp->UpdateTransformExternal();

      searchView->AddHiddenTag("Resources");
      searchView->AddHiddenTag(mResourceType);
      searchView->mSearch->SearchProviders.PushBack(GetResourceSearchProvider()) ;

      searchView->TakeFocus();
      viewPopUp->UpdateTransformExternal();
      searchView->Search(String());
      ConnectThisTo(searchView, Events::SearchCompleted, OnSearchCompleted);

      mActiveSearch = viewPopUp;
    }
  }

  void OnSearchCompleted(SearchViewEvent* event)
  {
    Resource* resource = (Resource*)event->Element->Data;
    Any val = resource->ResourceIdName;
    SetVariant(val);
    ObjectEvent objectEvent(this);
    this->DispatchEvent(Events::ValueChanged, &objectEvent);
    mActiveSearch.SafeDestroy();
  }

  void Edit() override
  {
    
  }

  void SizeToContents() override
  {
    mText->SizeToContents();
    SetSize(mText->GetSize());
  }

  void UpdateTransform() override
  {
    mText->SetSize(this->GetSize());
    Composite::UpdateTransform();
  }

  void SetVariant(AnyParam variant) override
  {
    String resourceIdName = variant.ToString();
    mResourceIdName = resourceIdName;
    uint idLength = cHex64Size + 1;
    StringIterator startIt = mResourceIdName.Begin() + idLength;
    StringIterator endIt = resourceIdName.End();
    StringRange resourceName(startIt, endIt);
    mText->SetText(resourceName);
  }

  void GetVariant(Any& variant) override
  {
    variant = mResourceIdName;
  }
};

ValueEditor* CreateResourceDisplay(Composite* composite, AnyParam data, u32 flags)
{
  return new ResourceDisplay(composite, data);
}

//------------------------------------------------------------------ Icon Editor
const String cDefaultIcon = "ItemIcon";
class IconDisplay : public ValueEditor
{
public:
  typedef IconDisplay ZilchSelf;

  Element* mIcon;

  IconDisplay(Composite* parent)
    : ValueEditor(parent)
  {
    mIcon = CreateAttached<Element>(cDefaultIcon);
    SetLayout(CreateFillLayout());
    mIcon->SetTranslation(Pixels(0,6,0));
    mIcon->SetNotInLayout(false);
  }
  
  void SetVariant(AnyParam variant) override
  {
    ErrorIf(!variant.Is<String>(), "Variant for IconEditor must be a String.");
    String icon = variant.Get<String>();
    if(icon.Empty())
    {
      mIcon->SetActive(false);
      mIcon->SetVisible(false);
    }
    else
    {
      BaseDefinition* definition = mDefSet->GetDefinitionOrNull(icon);
      if(definition == nullptr)
        definition = mDefSet->GetDefinition(cDefaultIcon);
      mIcon->ChangeDefinition(definition);

      mIcon->SetVisible(true);
      mIcon->SetActive(true);
      mIcon->SetSizing(SizePolicy::Fixed, mIcon->GetMinSize());
      mIcon->mHorizontalAlignment = HorizontalAlignment::Center;
      mIcon->mVerticalAlignment = VerticalAlignment::Center;
    }
  }

  void GetVariant(Any& variant) override
  {
    variant = "";
  }
};

ValueEditor* CreateIconDisplay(Composite* composite, AnyParam data, u32 flags)
{
  return new IconDisplay(composite);
}

//------------------------------------------------------------------ Icon Editor
class BooleanEditor : public ValueEditor
{
public:
  typedef BooleanEditor ZilchSelf;

  CheckBox* mCheckBox;

  BooleanEditor(Composite* parent)
    : ValueEditor(parent)
  {
    mCheckBox = new CheckBox(this);
    ConnectThisTo(mCheckBox, Events::ValueChanged, OnValueChanged);
  }

  void UpdateTransform() override
  {
    Rect local = GetLocalRect();
    PlaceCenterToRect(local, mCheckBox);
    Composite::UpdateTransform();
  }

  void OnValueChanged(Event* e)
  {
    ObjectEvent objectEvent(this);
    this->DispatchEvent(Events::ValueChanged, &objectEvent);
  }

  void SetVariant(AnyParam variant) override
  {
    ErrorIf(!variant.Is<bool>(), "Variant for BooleanEditor must be a bool.");
    bool state = variant.Get<bool>();
    mCheckBox->SetChecked(state);
  }

  void GetVariant(Any& variant) override
  {
    variant = mCheckBox->GetChecked();
  }
};

ValueEditor* CreateBooleanEditor(Composite* composite, AnyParam data, u32 flags)
{
  return new BooleanEditor(composite);
}

//--------------------------------------------------------- Value Editor Factory
ZilchDefineType(ValueEditorFactory, builder, type)
{

}

ValueEditorFactory::ValueEditorFactory()
{
  RegisterEditor(cDefaultValueEditor, CreateInPlaceTextEditor);
  RegisterEditor(cDefaultResourceEditor, CreateResourceDisplay);
  RegisterEditor(cDefaultIconEditor, CreateIconDisplay);
  RegisterEditor(cDefaultBooleanEditor, CreateBooleanEditor);
}

void ValueEditorFactory::RegisterEditor(StringParam type, ValueEditorCreator creator)
{
  mRegisteredEditors.Insert(type, creator);
}

ValueEditor* ValueEditorFactory::GetEditor(StringParam type, Composite* parent, 
                                           AnyParam data, u32 flags)
{
  ValueEditorCreator creator = mRegisteredEditors.FindValue(type, nullptr);
  if(creator)
    return creator(parent, data, flags);
  return nullptr;
}

}//namespace Zero
