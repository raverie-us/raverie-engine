///////////////////////////////////////////////////////////////////////////////
///
/// \file BasicEditors.cpp
/// Implementation of the basic editors for the property grid.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace PropertyViewUi
{
const cstr cLocation = "EditorUi/PropertyView/Editors";
Tweakable(Vec4, ColorWidgetHighlight, Vec4(1,1,1,1), cLocation);
Tweakable(float, VectorElementMinSize, 80, cLocation);
Tweakable(bool, ElementLables, false, cLocation);
Tweakable(Vec4, ModifiedTextColor, Vec4(1,1,1,1), cLocation);
}

namespace Events
{
  //The button has been pressed.
  DeclareEvent(NumberValueChanged);
  DeclareEvent(NumberValueCommited);

  DefineEvent(ObjectPoll);
  DefineEvent(NumberValueChanged);
  DefineEvent(NumberValueCommited);
}

ZilchDefineType(ValueEvent, builder, type)
{
}

ZilchDefineType(ObjectPollEvent, builder, type)
{
}

DirectProperty::DirectProperty(PropertyWidgetInitializer& initializer)
  :PropertyWidget(initializer)
{
  mNode = initializer.ObjectNode;
  mProperty = initializer.Property;
  mInstance = initializer.Instance;
  mReadOnly = mProperty->IsReadOnly();
  mLabel->SetText(mProperty->Name);
  ConnectThisTo(mLabel, Events::RightMouseUp, OnRightMouseUpLabel);
}

void DirectProperty::BeginPreviewChanges()
{
  mProp->CaptureState(mCapture, mInstance, mProperty);
}

void DirectProperty::EndPreviewChanges()
{
  if(mCapture.HasCapture())
  {
    mProp->RestoreState(mCapture);
    mCapture.Clear();
  }
}

void DirectProperty::PreviewValue(AnyParam var)
{
  PropertyState state(var);
  PreviewValue(state);
}

void DirectProperty::PreviewValue(PropertyState& state)
{
  ErrorIf(!mCapture.HasCapture(), "Must call BeginPreviewChanges");

  mProp->ChangeProperty(mInstance, mProperty, state, PropertyAction::Preview);
}

void DirectProperty::CommitValue(AnyParam var)
{
  PropertyState state(var);
  CommitValue(state);
}

void DirectProperty::BuildPath(ObjectPropertyNode* node, Handle& rootInstance, PropertyPath& path)
{
  bool foundRoot = false;

  Handle object = node->mObject;
  BoundType* objectType = object.StoredType;

  if(object.IsNotNull())
  {
    foundRoot = objectType->HasAttributeInherited(ObjectAttributes::cStoreLocalModifications);
    if(foundRoot)
    {
      rootInstance = object;
      return;
    }
  }

  if(node->mParent || foundRoot)
  {
    if(!foundRoot)
      BuildPath(node->mParent, rootInstance, path);

    if(node->mProperty)
      path.AddPropertyToPath(node->mProperty);
    else
    {
      if(MetaComposition* parentComposition = node->mParent->mComposition)
      {
        path.AddComponentToPath(objectType->Name);
      }
      else if(MetaArray* parentMetaArray = node->mParent->mMetaArray)
      {
        Handle parent = node->mParent->mObject;
        uint index = parentMetaArray->FindIndex(parent, node->mObject);
        path.AddComponentIndexToPath(index);
      }     
    }
  }
  else
  {
    rootInstance = node->mObject;
  }
}

bool DirectProperty::IsModified()
{
  Handle rootInstance;
  PropertyPath propertyPath;
  BuildPath(mNode, rootInstance, propertyPath);

  LocalModifications* modifications = LocalModifications::GetInstance();
  return modifications->IsPropertyModified(rootInstance, propertyPath);
}

void DirectProperty::CommitValue(PropertyState& state)
{
  if(mReadOnly) return;
  EndPreviewChanges();

  Handle rootInstance;
  PropertyPath propertyPath;
  BuildPath(mNode, rootInstance, propertyPath);

  mProp->ChangeProperty(rootInstance, propertyPath, state, PropertyAction::Commit);
}

PropertyState DirectProperty::GetValue()
{
  return mProp->GetValue(mInstance, mProperty);
}

String DirectProperty::GetToolTip(ToolTipColor::Enum* color)
{
  // Build the tool tip text
  StringBuilder toolTip;

  // Get the property type
  BoundType* propertyType = Type::GetBoundType(mProperty->PropertyType);
  
  if(IsModified())
  {
    toolTip << "Modified from Archetype\n";
    toolTip << "(right click to revert)\n";
    toolTip << "____________________\n\n";
    if(color)
      *color = ToolTipColor::Orange;
  }

  // Add the type name line
  toolTip << mProperty->Name;
  toolTip << " : ";
  toolTip << propertyType->Name;
  toolTip << "\n";
  toolTip << mProperty->Description;

  // Only handling descriptions for enum types.
  if(Type::IsEnumType(propertyType))
  {
    if(EnumDoc* enumDoc = Z::gDocumentation->mEnumAndFlagMap.FindValue(propertyType->Name, nullptr))
    {
      if(!enumDoc->mDescription.Empty( ))
      {
        toolTip << "\n\n";
        toolTip << "Enum description:\n";
        toolTip << enumDoc->mDescription;
      }
    }
  }

  return toolTip.ToString();
}

void DirectProperty::OnRightMouseUpLabel(MouseEvent* event)
{
  ContextMenu* menu = new ContextMenu(this);

  if(IsModified())
  {
    ConnectMenu(menu, "Revert", OnRevert);
  }
  else
  {
    ConnectMenu(menu, "Mark Modified", OnMarkModified);
  }
  // Send an event to let other widgets add items to the context menu
  ContextMenuEvent eventToSend;
  eventToSend.mMenu = menu;
  eventToSend.mProperty = mProperty;
  eventToSend.mInstance = mInstance;
  DispatchBubble(Events::PropertyContextMenu, &eventToSend);

  // If nothing was added, don't display the menu
  if(menu->ItemCount() <= 0)
  {
    menu->Destroy();
  }
  else
  {
    Mouse* mouse = Z::gMouse;
    menu->SetBelowMouse(mouse, Pixels(0,0));
  }
}

void DirectProperty::Refresh()
{
  // We want to visually notify that the property is modified
  String labelText = mProperty->Name;

  mLabel->SetColor(Vec4(1));

  // This should only display serialized properties, but that's on hold until the meta refactor
  if(IsModified())// && mProperty->Flags.IsSet(PropertyFlags::Serialized))
    mLabel->SetColor(PropertyViewUi::ModifiedTextColor);
  
  mLabel->SetText(labelText);
}

void DirectProperty::OnRevert(Event* e)
{
  Handle rootInstance;
  PropertyPath propertyPath;
  BuildPath(mNode, rootInstance, propertyPath);

  OperationQueue* queue = Z::gEditor->GetOperationQueue();
  RevertProperty(queue, rootInstance, propertyPath);
}

void DirectProperty::OnMarkModified(Event* e)
{
  Handle rootInstance;
  PropertyPath propertyPath;
  BuildPath(mNode, rootInstance, propertyPath);

  OperationQueue* queue = Z::gEditor->GetOperationQueue();
  MarkPropertyAsModified(queue, rootInstance, propertyPath);
}

//******************************************************************************
// Indexed string array properties that select
// from a list of strings
class PropertyIndexedStringArray : public DirectProperty
{
public:
  typedef PropertyIndexedStringArray ZilchSelf;
  ComboBox* mSelectBox;
  EditorIndexedStringArray* mMetaEdit;
  StringSource mStrings;

  PropertyIndexedStringArray(PropertyWidgetInitializer& initializer)
    :DirectProperty(initializer)
  {
    mDefSet = initializer.Parent->GetDefinitionSet();
    mSelectBox = new ComboBox(this);
    mMetaEdit = mProperty->HasInherited<EditorIndexedStringArray>();

    // Enumerate all possible string values
    mMetaEdit->Enumerate(initializer.Instance, mProperty, mStrings.Strings);
    mSelectBox->SetListSource(&mStrings);

    // Select the current value
    Refresh();

    ConnectThisTo(mSelectBox, Events::ItemSelected, OnIndexChanged);
  }

  void Refresh() override
  {
    DirectProperty::Refresh();

    // If the box is open do nothing
    if(mSelectBox->IsOpen())
      return;

    PropertyState state = GetValue();

    Any currentValue = state.Value;

    // Can be indexed by string or value
    if(currentValue.Is<uint>())
      mSelectBox->SetSelectedItem(currentValue.Get<uint>(), false);
    else if(currentValue.Is<int>())
      mSelectBox->SetSelectedItem(currentValue.Get<int>(), false);
    else if(currentValue.Is<String>())
      mSelectBox->SetSelectedItem(mStrings.GetIndexOfString(currentValue.Get<String>()), false);
  }

  void UpdateTransform() override
  {
    LayoutResult nameLayout = GetNameLayout();
    LayoutResult contentLayout = GetContentLayout(nameLayout);

    PlaceWithLayout(nameLayout, mLabel);

    PlaceWithLayout(contentLayout, mSelectBox);

    PropertyWidget::UpdateTransform();
  }

  void OnIndexChanged(ObjectEvent* event)
  {
    uint index = mSelectBox->GetSelectedItem();
    if(mProperty->PropertyType == ZilchTypeId(int))
    {
      Any variantValue = (int)index;
      CommitValue(variantValue);
    }
    else if(mProperty->PropertyType == ZilchTypeId(String))
    {
      Any variantValue = mStrings[index];
      CommitValue(variantValue);
    }
  }
};

//******************************************************************************
// Editor for enum types
class PropertyEditorEnum : public DirectProperty
{
public:
  typedef PropertyEditorEnum ZilchSelf;
  ComboBox* mSelectBox;
  SelectorButton* mSelectorButton;
  // Storing a pointer to this is safe because when meta is changed, the entire property grid
  // is torn down and rebuilt. This should never point at a destroyed type.
  BoundType* mEnumType;
  StringSource mStrings;

  // Map each enum index value to a list index
  // needed for pulls downs.
  HashMap<int, int> EnumIndexToListIndex;
  Array<int> EnumIndexes;

  // Most recent tool tip displayed.
  HandleOf<ToolTip> mToolTip;

  PropertyEditorEnum(PropertyWidgetInitializer& initializer)
    :DirectProperty(initializer)
  {
    mDefSet = initializer.Parent->GetDefinitionSet();
    mSelectBox = new ComboBox(this);

    mEnumType = Type::GetBoundType(initializer.Property->PropertyType);

    PropertyArray& allProperties = mEnumType->AllProperties;
    for(size_t i = 0; i < allProperties.Size(); ++i)
    {
      Property* prop = allProperties[i];
      String enumString = prop->Name;
      // This should be a static property (all enum values are) so pass in null for an
      // instance and then convert the any to an integer (not unsigned since negative enum values are valid)
      int enumValue = prop->GetValue(nullptr).Get<int>();
      EnumIndexToListIndex[enumValue] = i;
      EnumIndexes.PushBack(enumValue);
      mStrings.Strings.PushBack(enumString);
    }

    mSelectBox->SetListSource(&mStrings);

    mSelectorButton = new SelectorButton(this);
    mSelectorButton->CreateButtons(&mStrings);
    mSelectorButton->SetActive(false);

    Refresh();

    ConnectThisTo(mSelectBox, Events::ItemSelected, OnIndexChanged);
    ConnectThisTo(mSelectorButton, Events::ItemSelected, OnSelectorIndexChanged);

    // Tooltip handlers to display individual enum value descriptions
    ConnectThisTo(mSelectBox, Events::ListBoxOpened, OnListBoxOpen);
    ConnectThisTo(mSelectorButton, Events::MouseHover, OnButtonMouseHover);
  }

  void Refresh() override
  {
    DirectProperty::Refresh();

    if(mSelectBox->IsOpen())
      return;

    // Get the current state of the property
    PropertyState state = GetValue();

    // If it's a valid value, store it and set the selected item
    // in the select box
    if(state.IsValid())
    {
      // Convert the enum index to a list index
      // guard against the index from being an invalid index
      uint enumIndex = state.Value.Get<uint>();
      uint listIndex = EnumIndexToListIndex.FindValue(enumIndex, 0);
      mSelectBox->SetSelectedItem(listIndex, false);
      mSelectorButton->SetSelectedItem(listIndex, false);
    }
    else
    {
      mSelectBox->SetInvalid();
    }
  }

  void OnIndexChanged(ObjectEvent* event)
  {
    // Select a new index commit the new value
    IndexChanged(mSelectBox->GetSelectedItem());
  }

  void OnSelectorIndexChanged(ObjectEvent* event)
  {
    // Select a new index commit the new value
    IndexChanged(mSelectorButton->GetSelectedItem());
  }

  void IndexChanged(uint selectedIndex)
  {
    //Convert the list index to the enum index value
    uint enumIndex = EnumIndexes[selectedIndex];

    Any newValue = enumIndex;
    CommitValue(newValue);
    if(mProperty->HasAttribute(PropertyAttributes::cInvalidatesObject))
      mGrid->Invalidate();
  }

  void OnListBoxOpen(Event* event)
  {
    ConnectThisTo(*mSelectBox->mListBox, Events::MouseHover, OnListMouseHover);
  }

  void OnListMouseHover(MouseEvent* event)
  {
    StringBuilder toolTip;
    GetListToolTip(&toolTip);
    CreateToolTip(toolTip.ToString( ), mSelectBox->mListBox);
  }

  void OnButtonMouseHover(MouseEvent* event)
  {
    StringBuilder toolTip;
    GetButtonToolTip(&toolTip);
    CreateToolTip(toolTip.ToString( ), mSelectorButton->GetHoverButton( ));
  }

  void CreateToolTip(StringParam toolTipText, Widget* source)
  {
    if(toolTipText.Empty( ))
      return;

    ToolTipColor::Enum color = ToolTipColor::Default;

    mToolTip.SafeDestroy();

    mToolTip = new ToolTip(source);
    mToolTip->SetText(toolTipText);
    mToolTip->SetColor(color);

    ToolTipPlacement placement;
    placement.SetScreenRect(GetScreenRect( ));
    placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left,
      IndicatorSide::Bottom, IndicatorSide::Top);

    mToolTip->SetArrowTipTranslation(placement);
  }

  void GetEnumToolTip(int enumIndex, StringBuilder* toolTip)
  {
    EnumDoc* enumDoc = Z::gDocumentation->mEnumAndFlagMap.FindValue(mEnumType->Name, nullptr);
    if(enumDoc == nullptr)
      return;

    String enumString = mStrings.Strings[enumIndex];

    String valueDescription = enumDoc->mEnumValues.FindValue(enumString, "");
    if(valueDescription.Empty( ))
      return;

    *toolTip << "Value : ";
    *toolTip << enumString;
    *toolTip << "\n\n";
    *toolTip << valueDescription;
  }

  void GetListToolTip(StringBuilder* toolTip)
  {
    // Make sure the highlighted item is valid
    int index = mSelectBox->mListBox->GetHighlightItem( );
    if(index == -1 || uint(index) > mSelectBox->mDataSource->GetCount( ))
      return;

    GetEnumToolTip(index, toolTip);
  }

  void GetButtonToolTip(StringBuilder* toolTip)
  {
    // Make sure the highlighted item is valid
    int index = mSelectorButton->GetHoverItem();
    if(index == -1)
      return;

    GetEnumToolTip(index, toolTip);
  }

  void UpdateTransform() override
  {
    LayoutResult nameLayout = GetNameLayout();
    LayoutResult contentLayout = GetContentLayout(nameLayout);

    PlaceWithLayout(nameLayout, mLabel);

    // Show the buttons if we have enough room to fit the min size of the buttons
    bool showButtons = (mSelectorButton->GetMinSize().x <= contentLayout.Size.x);
    mSelectorButton->SetActive(showButtons);
    mSelectBox->SetActive(!showButtons);

    PlaceWithLayout(contentLayout, mSelectorButton);
    PlaceWithLayout(contentLayout, mSelectBox);

    PropertyWidget::UpdateTransform();
  }
};

//******************************************************************************

// Edit a slider range
class PropertyEditorRange : public DirectProperty
{
public:
  typedef PropertyEditorRange ZilchSelf;
  Slider* mSlider;

  PropertyEditorRange(PropertyWidgetInitializer& initializer)
    : DirectProperty(initializer)
  {
    mDefSet = initializer.Parent->GetDefinitionSet();
    EditorRange* metaEdit = mProperty->HasInherited<EditorRange>();

    mSlider = new Slider(this, SliderType::Number);
    mSlider->SetRange(metaEdit->MinValue, metaEdit->MaxValue);
    mSlider->SetIncrement(metaEdit->Increment);

    Refresh();

    ConnectThisTo(mSlider, Events::SliderManipulationStarted, OnSliderManipulationStarted);
    ConnectThisTo(mSlider, Events::SliderIncrementalChange, OnSliderIncrementalChange);
    ConnectThisTo(mSlider, Events::SliderChanged, OnSliderChanged);
  }

  void Refresh() override
  {
    DirectProperty::Refresh();

    // Get the current property state
    PropertyState state = GetValue();

    if(state.IsValid())
    {
      float newValue = state.Value.Get<float>();
      mSlider->SetValue(newValue, false);
    }
    // If it's invalid, set the slider to invalid
    else
    {
      mSlider->SetInvalid();
    }
  }

  void OnSliderManipulationStarted(ObjectEvent* event)
  {
    BeginPreviewChanges();
  }

  void OnSliderIncrementalChange(ObjectEvent* event)
  {
    Any newValue = mSlider->GetValue();
    PreviewValue(newValue);
  }

  void OnSliderChanged(ObjectEvent* event)
  {
    Any newValue = mSlider->GetValue();
    CommitValue(newValue);
  }

  void UpdateTransform() override
  {
    float numberDisplaySize = Pixels(0);

    LayoutResult nameLayout = GetNameLayout();
    LayoutResult contentLayout = GetContentLayout(nameLayout);
    contentLayout.Size.x -= numberDisplaySize;

    LayoutResult numLayout;
    numLayout.Size.y = nameLayout.Size.y;
    numLayout.Size.x = numberDisplaySize;
    numLayout.Translation = contentLayout.Translation;
    numLayout.Translation.x += contentLayout.Size.x;

    PlaceWithLayout(nameLayout, mLabel);
    PlaceWithLayout(contentLayout, mSlider);

    PropertyWidget::UpdateTransform();
  }
};

//******************************************************************************
class PropertyEditorString : public DirectProperty
{
public:
  typedef PropertyEditorString ZilchSelf;
  TextBox* mEditText;

  PropertyEditorString(PropertyWidgetInitializer& initializer)
    :DirectProperty(initializer)
  {
    mDefSet = initializer.Parent->GetDefinitionSet();

    mEditText = new TextBox(this);
    mEditText->SetEditable(true);

    Refresh();

    ConnectThisTo(mEditText, Events::TextBoxChanged, OnTextEnter);

    if(mProperty->IsReadOnly())
    {
      mEditText->SetEditable(false);
      mEditText->HideBackground(true);
    }
  }

  void Refresh() override
  {
    DirectProperty::Refresh();

    if(mEditText->HasFocus())
      return;

    // Get the current property state
    PropertyState state = GetValue();

    // If it's invalid, set the text box to invalid
    if(!state.IsValid())
    {
      mEditText->SetInvalid();
    }
    else
    {
      String* strValue = state.Value.Get<String*>();
      if (strValue)
        mEditText->SetText(*strValue);
      else
        mEditText->SetText("");
    }
  }

  void OnTextEnter(ObjectEvent* event)
  {
    String newStringValue = mEditText->GetText();
    CommitValue(newStringValue);
    Refresh();
  }

  void UpdateTransform() override
  {
    LayoutResult nameLayout = GetNameLayout();
    LayoutResult contentLayout = GetContentLayout(nameLayout);
    
    PlaceWithLayout(nameLayout, mLabel);
    PlaceWithLayout(contentLayout, mEditText);

    PropertyWidget::UpdateTransform();
  }
};

class PropertyEditorNumber;

template<typename type>
void ComputeLimits(double& min, double& max)
{
  min = (double)std::numeric_limits<type>::lowest();
  max = (double)std::numeric_limits<type>::max();
}

void ComputeLimits(BoundType* boundType, double& min, double& max)
{
  if(boundType == ZilchTypeId(float)) ComputeLimits<float>(min, max);
  else if(boundType == ZilchTypeId(double)) ComputeLimits<double>(min, max);
  else if(boundType == ZilchTypeId(int)) ComputeLimits<int>(min, max);
}

double ConvertValue(AnyParam startValue)
{
  BoundType* boundType = Type::GetBoundType(startValue.StoredType);
  if(boundType == ZilchTypeId(float)) return (double)startValue.Get<float>();
  else if(boundType == ZilchTypeId(double)) return (double)startValue.Get<double>();
  else if(boundType == ZilchTypeId(int)) return (double)startValue.Get<int>();
  return 0.0;
}

Any ConvertValue(BoundType* boundType, double newValue)
{
  if(boundType == ZilchTypeId(float)) return float(newValue);
  else if(boundType == ZilchTypeId(double)) return double(newValue);
  else if(boundType == ZilchTypeId(int)) return int(newValue);
  return 0;
}

//******************************************************************************
class NumberManipulation : public MouseManipulation
{
public:
  NumberManipulation(Mouse* mouse, Composite* owner, AnyParam startingValue, Vec3 screenPosition,
                     uint userData, EditorRange* range)
    :MouseManipulation(mouse, owner)
  {
    mSpinOverlay = owner->GetRootWidget()->GetPopUp()->CreateAttached<Element>("Spinner");
    mSpinOverlay->SetTranslation(screenPosition);
    mOwner = owner;
    mLocalMouseStart = mOwner->ToLocal(mouse->GetClientPosition());
    mCurrentAngle = 0.0f;
    mType = Type::GetBoundType(startingValue.StoredType);
    mStart = ConvertValue(startingValue);

    if(range)
    {
      mMin = range->MinValue;
      mMax = range->MaxValue;
    }
    else
    {
      ComputeLimits(mType, mMin, mMax);
    }

    mCurrentValue = mStart;
    mCurrentChange = 0.0f;
    mUserData = userData;
  }

  NumberManipulation::~NumberManipulation()
  {
    mSpinOverlay->Destroy();
  }

  Element* mSpinOverlay;
  PropertyEditorNumber* mTarget;
  Composite* mOwner;
  Vec2 mLocalMouseStart;
  float mCurrentAngle;
  // Storing a pointer to this is safe because when meta is changed, the entire property grid
  // is torn down and rebuilt. This should never point at a destroyed type.
  BoundType* mType;

  double mMin;
  double mMax;
  double mStart;

  double mCurrentValue;
  double mCurrentChange;
  uint mUserData;

  void NumberManipulation::OnMouseMove(MouseEvent* event)
  {
    Vec2 mousePos = mOwner->ToLocal(event->Position);

    Vec2 newMouseDirection = mousePos - mLocalMouseStart;
    float distanceToMouse = newMouseDirection.AttemptNormalize();

    // Do not adjust the value if the cursor is too close
    // to prevent rapid changes in value
    const float cMinAdjustmenDistance = Pixels(30);
    if(distanceToMouse < cMinAdjustmenDistance)
      return;

    // Compute the difference between the two dir in radians
    float angleDest = Math::ArcTan2(newMouseDirection.y, newMouseDirection.x);
    float angleStart = mCurrentAngle;
    float angleDelta = angleDest - angleStart;
    mCurrentAngle = angleDest;
    //Set the rotation of the spinner overlay
    mSpinOverlay->SetRotation(angleDest);

    // Correct the wrap around
    if(angleDelta > Math::cPi)
      angleDelta -= Math::cTwoPi;

    if(angleDelta < -Math::cPi)
      angleDelta += Math::cTwoPi;

    // Update the adjustment value
    const float cChangeScalar = 4.0f;
    double previousChange = mCurrentChange;
    double newChange  = previousChange + double(angleDelta * cChangeScalar);
    double newValue = mStart + newChange;

    //Clamp it to range
    if(newValue < mMin)
    {
      newValue = mMin;
      //Remove the last change when clamping
      newChange = previousChange;
    }

    if(newValue > mMax)
    {
      newValue = mMax;
      //Remove the last change when clamping
      newChange = previousChange;
    }

    mCurrentChange = newChange;
    mCurrentValue = newValue;


    ValueEvent valueEvent;
    valueEvent.NewValue = ConvertValue(mType, newValue);
    valueEvent.UserData = mUserData;
    DispatchEvent(Events::NumberValueChanged, &valueEvent);
  }

  void NumberManipulation::OnMouseUp(MouseEvent* event)
  {
    ValueEvent valueEvent;
    valueEvent.NewValue = ConvertValue(mType, mCurrentValue);
    valueEvent.UserData = mUserData;
    DispatchEvent(Events::NumberValueCommited, &valueEvent);
    this->Destroy();
  }

};



//******************************************************************************
class PropertyEditorNumber : public DirectProperty
{
public:
  typedef PropertyEditorNumber ZilchSelf;
  TextBox* mEditText;
  IconButton* mSpinButton;
  EditorRange* mEditRange;

  PropertyEditorNumber(PropertyWidgetInitializer& initializer)
    :DirectProperty(initializer)
  {
    mDefSet = initializer.Parent->GetDefinitionSet();

    mEditText = new TextBox(this);
    mEditText->SetEditable(true);

    mSpinButton = new IconButton(this);
    mSpinButton->SetIcon("Spin");
    mSpinButton->mBackgroundColor = ToByteColor(Vec4::cZero);
    mSpinButton->mBackgroundHoverColor = ToByteColor(Vec4(1, 1, 1, 0.1f));
    mSpinButton->mBackgroundClickedColor = ToByteColor(Vec4(1, 1, 1, 0.1f));
    mSpinButton->SetSize(Pixels(24, 16));
    mSpinButton->SetToolTip("Drag to change value");

    mEditRange = initializer.Property->HasInherited<EditorRange>();

    Refresh();

    ConnectThisTo(mEditText, Events::TextBoxChanged, OnTextEnter);
    ConnectThisTo(mSpinButton, Events::LeftMouseDown, OnDownSpinnner);

    if(mProperty->IsReadOnly())
    {
      mEditText->SetReadOnly(true);
      mSpinButton->SetActive(false);
    }
  }

  void Refresh() override
  {
    DirectProperty::Refresh();

    if(mEditText->HasFocus())
      return;

    // Get the current property state
    PropertyState state = GetValue();

    if(!state.IsValid())
      mEditText->SetInvalid();
    else
      mEditText->SetText( state.Value.ToString() );
  }

  void OnTextEnter(ObjectEvent* event)
  {
    String newStringValue = mEditText->GetText();

    Type* propertyType = mProperty->PropertyType;
    MetaSerialization* metaSerialization = propertyType->HasInherited<MetaSerialization>();

    ReturnIf(metaSerialization == nullptr, , "Must have MetaSerialization to convert from string");

    Any newValue;
    //newValue.DefaultConstruct(mProperty->PropertyType);
    if(metaSerialization->ConvertFromString(newStringValue, newValue))
    {
      CommitValue(newValue);
      Refresh();
    }
  }

  void OnDownSpinnner(MouseEvent* event)
  {
    BeginPreviewChanges();

    Vec2 size = mSpinButton->GetSize();
    Vec3 screenPosition = mSpinButton->GetScreenPosition() + ToVector3(size) * 0.5f;
    SnapToPixels(screenPosition);

    PropertyState state = GetValue();
    NumberManipulation* manip = new NumberManipulation(event->GetMouse(),
                                                       GetParent(),
                                                       state.Value,
                                                       screenPosition, 0, mEditRange);

    ConnectThisTo(manip, Events::NumberValueChanged, OnNumberValueChange);
    ConnectThisTo(manip, Events::NumberValueCommited, OnNumberValueCommited);
  }

  void OnNumberValueCommited(ValueEvent* event)
  {
    CommitValue(event->NewValue);
  }

  void OnNumberValueChange(ValueEvent* event)
  {
    PreviewValue(event->NewValue);
    Refresh();
  }

  void UpdateTransform() override
  {
    LayoutResult nameLayout = GetNameLayout();
    LayoutResult contentLayout = GetContentLayout(nameLayout);

    if(mSpinButton->GetActive())
      contentLayout.Size.x -= (mSpinButton->GetSize().x - Pixels(1));

    PlaceWithLayout(nameLayout, mLabel);
    PlaceWithLayout(contentLayout, mEditText);

    Vec3 last = Vec3(mSize.x, Pixels(1), 0);
    last.x -= mSpinButton->GetSize().x;
    mSpinButton->SetTranslation(last);

    PropertyWidget::UpdateTransform();
  }
};

//******************************************************************************
/// Property editor for a basic boolean type
class PropertyEditorBool : public DirectProperty
{
public:
  typedef PropertyEditorBool ZilchSelf;
  CheckBox* mCheckBox;

  PropertyEditorBool(PropertyWidgetInitializer& initializer)
    :DirectProperty(initializer)
  {
    mDefSet = initializer.Parent->GetDefinitionSet();

    mCheckBox = new CheckBox(this);
    mCheckBox->SetSize(Pixels(20, 20));

    Refresh();

    ConnectThisTo(mCheckBox, Events::ValueChanged, CheckChanged);
    ConnectThisTo(mLabel, Events::LeftMouseDown, MouseDownOnLabel);
  }

  void Refresh() override
  {
    DirectProperty::Refresh();

    PropertyState state = GetValue();
    if(state.IsValid())
      mCheckBox->SetCheckedDirect( state.Value.Get<bool>() );
    else
      mCheckBox->SetInvalid();
  }

  void MouseDownOnLabel(MouseEvent* event)
  {
    mCheckBox->ToggleChecked();
  }

  void CheckChanged(ObjectEvent* event)
  {
    bool checked = mCheckBox->GetChecked();
    Any newState = checked;
    CommitValue(newState);
    Refresh();

    if(mProperty->HasAttribute(PropertyAttributes::cInvalidatesObject))
      mGrid->Invalidate();
  }

  void UpdateTransform() override
  {
    LayoutResult nameLayout = GetNameLayout();
    PlaceWithLayout(nameLayout, mLabel);

    LayoutResult contentLayout = GetContentLayout(nameLayout);
    mCheckBox->SetTranslation(contentLayout.Translation);
    PropertyWidget::UpdateTransform();
  }
};

//******************************************************************************
DeclareEnum4(AxisLables, X, Y, Z, W);

// Property editor vector create a text box for
// each element of a vector for up to four elements
class PropertyEditVector : public DirectProperty
{
public:
  typedef PropertyEditVector ZilchSelf;
  TextBox* mEditText[4];
  Label* mAxisLabel[4];
  uint mDimension;
  PropertyState mState;

  PropertyEditVector(PropertyWidgetInitializer& initializer, uint dimension)
    :DirectProperty(initializer)
  {
    mDefSet = initializer.Parent->GetDefinitionSet();
    mDimension = dimension;

    for(uint i=0;i<mDimension;++i)
    {
      mAxisLabel[i] = new Label(this);
      mAxisLabel[i]->SetText(AxisLables::Names[i]);
      mAxisLabel[i]->SetActive(false);

      mEditText[i] = new TextBox(this);
      mEditText[i]->SetReadOnly(mReadOnly);
      mEditText[i]->SetActive(true);
    }

    float height = PropertyViewUi::PropertySize * 2.0f - 2.0f;
    if(InlinedValues())
      height = PropertyViewUi::PropertySize;

    SetSize(Vec2(0, PropertyViewUi::PropertySize * height));

    ConnectThisTo(mEditText[0], Events::TextBoxChanged, OnTextEnter0);
    ConnectThisTo(mAxisLabel[0], Events::LeftMouseDown, OnLabel0);

    if(mDimension > 1)
    {
      ConnectThisTo(mEditText[1], Events::TextBoxChanged, OnTextEnter1);
      ConnectThisTo(mAxisLabel[1], Events::LeftMouseDown, OnLabel1);
    }

    if(mDimension > 2)
    {
      ConnectThisTo(mEditText[2], Events::TextBoxChanged, OnTextEnter2);
      ConnectThisTo(mAxisLabel[2], Events::LeftMouseDown, OnLabel2);
    }

    if(mDimension > 3)
    {
      ConnectThisTo(mEditText[3], Events::TextBoxChanged, OnTextEnter3);
      ConnectThisTo(mAxisLabel[3], Events::LeftMouseDown, OnLabel3);
    }
  }

  // Should the element values be display on the same line
  // or on a separate line?
  bool InlinedValues()
  {
    float valueSize = (1.0f - mGrid->mNamePercent) * mGrid->GetSize().x;
    float perElementSize = valueSize / float(mDimension);
    return perElementSize > PropertyViewUi::VectorElementMinSize;
  }

  void Refresh() override
  {
    DirectProperty::Refresh();

    for(uint i=0;i<mDimension;++i)
    {
      if(mEditText[i]->HasFocus())
        return;
    }

    UpdateDisplay();
  }

  virtual Vec4 GetDisplayValue() = 0;
  virtual void PreviewElementValue(int index, float value) = 0;
  virtual void CommitElementValue(int index, float value) = 0;

  void TextEnter(uint index)
  {
    String valueText = mEditText[index]->GetText();
    float newValue = 0.0f;
    ToValue(valueText.All(), newValue);
    CommitElementValue(index, newValue);
  }

  void UpdateDisplay()
  {
    Vec4 newValue = GetDisplayValue();

    for(uint i=0;i<mDimension;++i)
    {
      // If the sub-state is valid, display the value
      if(mState.PartialState[i] == PropertyState::Valid)
        mEditText[i]->SetText( ToString(newValue[i], true) );
      // Otherwise set the textbox to invalid
      else
        mEditText[i]->SetInvalid();
    }
  }

  void OnLabel0(MouseEvent* event){OnLabelMouseDown(event, 0);}
  void OnLabel1(MouseEvent* event){OnLabelMouseDown(event, 1);}
  void OnLabel2(MouseEvent* event){OnLabelMouseDown(event, 2);}
  void OnLabel3(MouseEvent* event){OnLabelMouseDown(event, 3);}

  void OnLabelMouseDown(MouseEvent* event, uint i)
  {
    if(mProperty->IsReadOnly())
      return;

    BeginPreviewChanges();

    Vec3 screenPosition = mAxisLabel[i]->GetScreenPosition();
    Vec4 currentDisplayValue = GetDisplayValue();
    Any stringElement = currentDisplayValue[i];

    NumberManipulation* manip = new NumberManipulation(event->GetMouse(), this, stringElement, screenPosition, i, NULL);
    ConnectThisTo(manip, Events::NumberValueChanged, OnNumberValueChange);
    ConnectThisTo(manip, Events::NumberValueCommited, OnNumberValueCommited);
  }

  void OnNumberValueChange(ValueEvent* event)
  {
    uint index = event->UserData;
    float newValue = event->NewValue.Get<float>();
    PreviewElementValue(index, newValue);
    Refresh();
  }

  void OnNumberValueCommited(ValueEvent* event)
  {
    uint index = event->UserData;
    float newValue = event->NewValue.Get<float>();
    CommitElementValue(index, newValue);
    Refresh();
  }

  void OnTextEnter0(ObjectEvent* event){TextEnter(0);}
  void OnTextEnter1(ObjectEvent* event){TextEnter(1);}
  void OnTextEnter2(ObjectEvent* event){TextEnter(2);}
  void OnTextEnter3(ObjectEvent* event){TextEnter(3);}

  void UpdateTransform() override
  {
    LayoutResult nameLayout = GetNameLayout();
    LayoutResult contentLayout = GetContentLayout(nameLayout);

    PlaceWithLayout(nameLayout, mLabel);

    float startY = PropertyViewUi::PropertySize;
    float startX = nameLayout.Translation.x;
    float numberSize = mSize.x - nameLayout.Translation.x;

    if(InlinedValues())
    {
      startY = 0;
      startX = nameLayout.Size.x;
      numberSize = contentLayout.Size.x;
      mSize.y = PropertyViewUi::PropertySize;
    }
    else
    {
      mSize.y = PropertyViewUi::PropertySize * 2.0f;
    }

    float cellSize = SnapToPixels( numberSize / float(mDimension) );
    float elementLabelSize = Pixels(2);

    if (PropertyViewUi::ElementLables)
      elementLabelSize = Pixels(12);

    for(uint i=0;i<mDimension;++i)
    {
      Vec3 position = Vec3(startX + cellSize * float(i), startY ,0);
      Vec2 labelSize = Vec2(elementLabelSize, PropertyViewUi::PropertySize - 2.0f);
      Vec2 size = Vec2(cellSize - elementLabelSize, PropertyViewUi::PropertySize - 2.0f);

      Vec3 labelPosition = position;

      position.x += elementLabelSize;
      mEditText[i]->SetTranslation(position);
      mEditText[i]->SetSize(size);

      mAxisLabel[i]->SetTranslation(labelPosition);
      mAxisLabel[i]->SetSize(labelSize);

      // Disabled for now
      mAxisLabel[i]->SetActive(false);
    }

    PropertyWidget::UpdateTransform();
  }
};

//******************************************************************************

template<typename vectorType, typename elementType, uint dimension>
class PropertyEditVectorN : public PropertyEditVector
{
public:
  vectorType mCurrent;

  PropertyEditVectorN(PropertyWidgetInitializer& initializer)
    :PropertyEditVector(initializer, dimension)
  {
    Refresh();
  }

  void PreviewElementValue(int index, float value) override
  {
    mCurrent[index] = (elementType)value;
    PropertyState newState = mState;
    newState.Value = mCurrent;
    newState.PartialState[index] = PropertyState::Valid;
    PreviewValue(newState);
  }

  void CommitElementValue(int index, float value) override
  {
    mCurrent[index] = (elementType)value;
    PropertyState newState = mState;
    newState.Value = mCurrent;
    newState.PartialState[index] = PropertyState::Valid;
    CommitValue(newState);
  }

  Vec4 GetDisplayValue() override
  {
    mState = GetValue();
    vectorType typedValue = mState.Value.Get<vectorType>();
    mCurrent = typedValue;

    Vec4 displayValue = Vec4::cZero;
    for(uint i=0;i<dimension;++i)
      displayValue[i] = (float)typedValue[i];

    return displayValue;
  }
};

//******************************************************************************
class PropertyEditRotation : public PropertyEditVector
{
public:
  Vec3 mEulerCurrent;
  bool mEulerMode;

  PropertyEditRotation(PropertyWidgetInitializer& initializer)
    :PropertyEditVector(initializer, 3)
  {
    // We have to call refresh again because when in our base classes
    // constructor, it calls Refresh, which calls GetDisplayValue().
    // The problem is that the v-table is not constructed yet, so our
    // overridden version of GetDisplay value is not called.
    // Therefor, we have to refresh again to display the proper data.
    Refresh();
  }

  Vec4 GetDisplayValue() override
  {
    // Display as Euler Angles in Degrees
    mState = GetValue();

    // If the value return is a Vec3 it
    // is edited as a euler angle vector
    mEulerMode = mState.Value.Is<Vec3>();

    if(mEulerMode)
    {
      mEulerCurrent = mState.Value.Get<Vec3>();
    }
    else
    {
      Quat rotation = mState.Value.Get<Quat>();
      mEulerCurrent = Math::QuatToEulerDegrees(rotation);
    }

    return ToVector4(mEulerCurrent);
  }

  void UpdateElement(PropertyState& newState, int index, float value)
  {
    mEulerCurrent[index] = value;
    if(mEulerMode)
    {
      newState.Value = mEulerCurrent;
      newState.PartialState[index] = PropertyState::Valid;
    }
    else
    {
      Quat newRotation = Math::EulerDegreesToQuat(mEulerCurrent);
      newState.Value = newRotation;
    }
  }

  void PreviewElementValue(int index, float value) override
  {
    PropertyState newState = mState;
    UpdateElement(newState, index, value);
    PreviewValue(newState);
  }

  void CommitElementValue(int index, float value) override
  {
    PropertyState newState = mState;
    UpdateElement(newState, index, value);
    CommitValue(newState);
  }

};

//******************************************************************************
class PropertyEditRotationBasis : public PropertyEditRotation
{
public:
  typedef PropertyEditRotationBasis ZilchSelf;

  Vec3 mEulerCurrent;
  bool mEulerMode;

  IconButton* mIconButton;

  PropertyEditRotationBasis(PropertyWidgetInitializer& initializer)
    :PropertyEditRotation(initializer)
  {
    mIconButton = new IconButton(this);
    mIconButton->SetIcon("Transform");
    mIconButton->SizeToContents();

    ConnectThisTo(mIconButton, Events::ButtonPressed, OnEditBasis);
    // We have to call refresh again because when in our base classes
    // constructor, it calls Refresh, which calls GetDisplayValue().
    // The problem is that the v-table is not constructed yet, so our
    // overridden version of GetDisplay value is not called.
    // Therefore, we have to refresh again to display the proper data.
    Refresh();
  }

  void OnEditBasis(Event* e)
  {
    EditorRotationBasis* editor = mProperty->HasInherited<EditorRotationBasis>();

    // Get the actual objects selected (deals with multi-properties
    Array<Handle> objects;
    mProp->GetObjects(mInstance, objects);

    for(size_t i = 0; i < objects.Size(); ++i)
    {
      // Each item seems to either be a cog or component depending on if this was multi-selection or not
      Cog* cog = objects[i].Get<Cog*>();
      if(cog != nullptr)
      {
        EditCog(editor, cog);
        continue;
      }
      
      Component* component = objects[i].Get<Component*>();
      if(component != nullptr)
        EditComponent(editor, component);
    }
  }

  void EditComponent(EditorRotationBasis* editor, Component* component)
  {
    Cog* owner = component->GetOwner();
    if(owner != nullptr)
      EditCog(editor, owner);
  }

  void EditCog(EditorRotationBasis* editor, Cog* cog)
  {
    Space* space = cog->GetSpace();

    String gizmoName = editor->mGizmoName;
    Cog* gizmoCog = space->FindObjectByName(gizmoName);
    // Find the existing gizmo if it already exists in the scene
    if(gizmoCog == nullptr)
    {
      // Otherwise create the gizmo archetype
      Archetype* gizmoArchetype = ArchetypeManager::GetInstance()->FindOrNull(editor->mArchetypeName);
      if(gizmoArchetype == nullptr)
      {
        Error("Archetype '%s' not found.", editor->mArchetypeName.c_str());
        return;
      }
      gizmoCog = space->Create(gizmoArchetype);
      gizmoCog->SetName(gizmoName);
    }
    
    // Add the given cog to the gizmo
    RotationBasisGizmoInitializationEvent toSend;
    toSend.Source = cog;
    toSend.mIntData = editor->mIntData;
    gizmoCog->DispatchEvent(Events::AddRotationBasisGizmoObject, &toSend);
  }

  bool InlinedValues()
  {
    float valueSize = (1.0f - mGrid->mNamePercent) * mGrid->GetSize().x;
    float perElementSize = valueSize / float(mDimension);
    return perElementSize > PropertyViewUi::VectorElementMinSize;
  }

  void UpdateTransform() override
  {
    LayoutResult nameLayout = GetNameLayout();
    LayoutResult contentLayout = GetContentLayout(nameLayout);

    PlaceWithLayout(nameLayout, mLabel);

    float startY = PropertyViewUi::PropertySize;
    float startX = nameLayout.Translation.x;
    float iconSize = mIconButton->GetSize().x;
    float numberSize = mSize.x - nameLayout.Translation.x - iconSize;

    if(InlinedValues())
    {
      startY = 0;
      startX = nameLayout.Size.x;
      numberSize = contentLayout.Size.x - iconSize;
      mSize.y = PropertyViewUi::PropertySize;
    }
    else
    {
      mSize.y = PropertyViewUi::PropertySize * 2.0f;
    }

    mIconButton->SetTranslation(Vec3(startX + numberSize, startY, 0));

    float cellSize = SnapToPixels(numberSize / float(mDimension));
    float elementLabelSize = Pixels(2);

    if(PropertyViewUi::ElementLables)
      elementLabelSize = Pixels(12);

    for(uint i = 0; i < mDimension; ++i)
    {
      Vec3 position = Vec3(startX + cellSize * float(i), startY, 0);
      Vec2 labelSize = Vec2(elementLabelSize, PropertyViewUi::PropertySize);
      Vec2 size = Vec2(cellSize - elementLabelSize, PropertyViewUi::PropertySize);

      Vec3 labelPosition = position;

      position.x += elementLabelSize;
      mEditText[i]->SetTranslation(position);
      mEditText[i]->SetSize(size);

      mAxisLabel[i]->SetTranslation(labelPosition);
      mAxisLabel[i]->SetSize(labelSize);
    }

    PropertyWidget::UpdateTransform();
  }

};

//******************************************************************************

// Color Can edit floating point color vectors and ByteColor uint
// so these functions handle changing the variant to the correct type

// Convert variant to float color
Vec4 VariantToFloatColor(AnyParam variant)
{
  if(variant.Is<Vec4>())
    return variant.Get<Vec4>();
  else if(variant.Is<uint>())
    return ToFloatColor(variant.Get<uint>());
  else
    return Vec4(1,1,1,1);
}

// Convert float color of variant target type
Any FloatColorToVariant(BoundType* boundType, const Vec4& color)
{
  if(boundType == ZilchTypeId(Vec4))
    return color;
  return Vec4(1,1,1,1);
}

class PropertyEditColor : public DirectProperty
{
public:
  typedef PropertyEditColor ZilchSelf;
  Element* mBackground;
  IconButton* mEyeDropper;
  ColorDisplay* mColorDisplay;
  // Storing a pointer to this is safe because when meta is changed, the entire property grid
  // is torn down and rebuilt. This should never point at a destroyed type.
  BoundType* mPropertyType;
  ColorPicker* mColorPicker;

  PropertyEditColor(PropertyWidgetInitializer& initializer) 
    : DirectProperty(initializer), mColorPicker(nullptr)
  {
    // Create the background
    mBackground = CreateAttached<Element>(cWhiteSquareBorder);

    mEyeDropper = new IconButton(this);
    mEyeDropper->SetIcon("EyeDropper");
    mEyeDropper->mBackgroundColor = ToByteColor(Vec4::cZero);
    mEyeDropper->mBackgroundHoverColor = ToByteColor(Vec4(1, 1, 1, 0.1f));
    mEyeDropper->mBackgroundClickedColor = ToByteColor(Vec4(1, 1, 1, 0.1f));
    mEyeDropper->SetSize(Pixels(24, 16));
    mEyeDropper->SetToolTip("Drag to pick color");

    // Create the color buffer
    mColorDisplay = new ColorDisplay(this, 70, 20);

    mPropertyType = Type::GetBoundType(mProperty->PropertyType);

    Refresh();

    ConnectThisTo(this, Events::FinalColorPicked, OnFinalColorPicked);
    ConnectThisTo(this, Events::ColorChanged, OnColorChanged);
    ConnectThisTo(this, Events::ColorPickCancelled, OnColorPickCancelled);
    ConnectThisTo(mColorDisplay, Events::LeftMouseDown, OnMouseDownDisplay);
    ConnectThisTo(mEyeDropper, Events::LeftMouseDown, OnEyeDropper);
  }

  ~PropertyEditColor()
  {
    // this closes the color picker when changing the selection so it doesn't just 
    // stop edit the previously selected object
    if(mColorPicker)
    {
      if(!mColorPicker->IsColorPicked())
        EndPreviewChanges();
      mColorPicker->Close();
    }
  }

  Vec4 RefreshAndGetColor()
  {
    PropertyState state = GetValue();
    if(state.IsValid())
    {
      Vec4 color = VariantToFloatColor(state.Value);
      mColorDisplay->SetColor(color);
      return color;
    }
    else
    {
      mColorDisplay->SetInvalid();
      return Vec4(1,1,1,1);
    }
  }

  void Refresh() override
  {
    DirectProperty::Refresh();

    RefreshAndGetColor();
  }

  void OnMouseDownDisplay(MouseEvent* mouseEvent)
  {
    BeginPreviewChanges();

    Vec4 currentColor = RefreshAndGetColor();

    // Open Color Picker Window
    mColorPicker = ColorPicker::EditColor(this, currentColor);
  }

  void OnEyeDropper(MouseEvent* mouseEvent)
  {
    BeginPreviewChanges();

    // Activate color dropper manipulation
    OpenEyeDropper(mouseEvent->GetMouse(), this);
  }

  void OnFinalColorPicked(ColorEvent* event)
  {
    Vec4 newColor = event->Color;
    Any newColorValue = FloatColorToVariant(mPropertyType, event->Color);
    CommitValue(newColorValue);
    mColorDisplay->SetColor(newColor);
  }

  void OnColorChanged(ColorEvent* event)
  {
    PreviewColor(event->Color);
  }

  void OnColorPickCancelled(ColorEvent* event)
  {
    EndPreviewChanges();
  }

  void PreviewColor(Vec4& colorValue)
  {
    mColorDisplay->SetColor(colorValue);
    Any colorVariant = FloatColorToVariant(mPropertyType, colorValue);
    PreviewValue(colorVariant);
  }

  void UpdateTransform() override
  {
    // Set the label
    LayoutResult nameLayout = GetNameLayout();
    PlaceWithLayout(nameLayout, mLabel);

    LayoutResult outerLayout = GetContentLayout(nameLayout);
    outerLayout.Size.x -= mEyeDropper->GetSize().x - 1.0f;

    Thickness border = Thickness::All(1);
    LayoutResult innerLayout = RemoveThickness(border, outerLayout.Size, outerLayout.Translation);

    PlaceWithLayout(outerLayout, mBackground);
    PlaceWithLayout(innerLayout, mColorDisplay);

    mBackground->SetColor(PropertyViewUi::ColorWidgetHighlight);

    Vec3 last = Vec3(mSize.x, Pixels(1), 0);
    last.x -= mEyeDropper->GetSize().x;
    mEyeDropper->SetTranslation(last);

    DirectProperty::UpdateTransform();
  }
};

void RegisterGeneralEditors()
{
  ZilchTypeId(String)->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditorString>));
  ZilchTypeId(double)->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditorNumber>));
  ZilchTypeId(float)->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditorNumber>));
  ZilchTypeId(int)->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditorNumber>));
  ZilchTypeId(bool)->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditorBool>));

  ZilchTypeId(IntVec2)->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditVectorN<IntVec2, int, 2> >));
  ZilchTypeId(IntVec3)->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditVectorN<IntVec3, int, 3> >));
  ZilchTypeId(IntVec4)->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditVectorN<IntVec4, int, 4> >));
  ZilchTypeId(Vec2   )->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditVectorN<Vec2, float, 2> >));
  ZilchTypeId(Vec3   )->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditVectorN<Vec3, float, 3> >));
  ZilchTypeId(Vec4   )->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditColor>));
  ZilchTypeId(Quat   )->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditRotation>));
  ZilchTypeId(Enum   )->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditorEnum>));

  ZilchTypeId(EditorRange)->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditorRange>));
  ZilchTypeId(EditorRotationBasis)->Add(new MetaPropertyEditor(&CreateProperty<PropertyEditRotationBasis>));
  ZilchTypeId(EditorIndexedStringArray)->Add(new MetaPropertyEditor(&CreateProperty<PropertyIndexedStringArray>));
}

}//namespace Zero

