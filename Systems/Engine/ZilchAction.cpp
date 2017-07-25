///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Ease using the shared easing functions and ease type enum
struct EaseTypeFunc
{
  Easer mEaser;

  EaseTypeFunc() {}

  EaseTypeFunc(Easer easer) :
    mEaser(easer)
  {
  }

  static void AddParam(LibraryBuilder& library, ParameterArray& params)
  {
    DelegateParameter& p4 = params.PushBack();
    p4.ParameterType = ZilchTypeId(EaseType::Enum);
    p4.Name = "ease";
  }

  static EaseTypeFunc Build(Call& call, int index)
  {
    return EaseTypeFunc(GetEaser(call.Get<int>(index)));
  }

  float operator()(float t)
  {
    return mEaser(t);
  }
};

// Ease using a sample curve
struct EaseTypeSampleCurve
{
  HandleOf<SampleCurve> mSampleCurve;
  
  EaseTypeSampleCurve(){};

  EaseTypeSampleCurve(SampleCurve* curve) :
    mSampleCurve(curve)
  {
  }

  static void AddParam(LibraryBuilder& library, ParameterArray& params)
  {
    DelegateParameter& p4 = params.PushBack();
    p4.ParameterType = ZilchTypeId(SampleCurve);
    p4.Name = "ease";
  }

  static EaseTypeSampleCurve Build(Call& call, int index)
  {
    Handle handle = call.GetHandle(index);
    SampleCurve* sampleCurve = (SampleCurve*)handle.Dereference();
    return EaseTypeSampleCurve(sampleCurve);
  }

  float operator()(float t)
  {
    // Safeguard against null
    SampleCurve* sampleCurve = mSampleCurve;
    if(sampleCurve == nullptr)
      return t;
    return sampleCurve->Sample(t);
  }
};


// Action that animates a zilch property
template<typename PropertyType, typename EaserType>
class ZilchPropertyAction : public Action
{
public:
  EaserType mEaser;
  float mDuration;
  PropertyType mStarting;
  PropertyType mEnding;
  float mTime;
  Handle mPropertyDelegate;

  ZilchPropertyAction()
  {
  }

  ~ZilchPropertyAction()
  {
  }

  ActionState::Enum Update(float dt) override
  {
    // Check Null
    if(mPropertyDelegate.IsNull())
      return ActionState::Completed;

    PropertyDelegateTemplate* propertyDelegate = (PropertyDelegateTemplate*)mPropertyDelegate.Dereference();

    // Check Null
    if(propertyDelegate->Set.ThisHandle.IsNull())
      return ActionState::Completed;

    if(!mFlags.IsSet(ActionFlag::Started))
    {
      // Get the current value
      Call propertyCall(propertyDelegate->Get);
      ExceptionReport report;
      propertyCall.Invoke(report);
      mStarting = propertyCall.Get<PropertyType>(Call::Return);

      mFlags.SetFlag(ActionFlag::Started);
    }

    // Update time
    mTime += dt;
    float t = mDuration > 0.0f ? mTime / mDuration : 1.0f;
      
    // Interpolate the value
    PropertyType newValue = mEnding;
      
    // If the value has passed the end
    // clamp to the end
    if(t < 1.0f)
    {
      // Compute eased t
      float easedT = mEaser(t);

      // Interpolate the value
      newValue = Interpolation::Lerp<PropertyType>::Interpolate(mStarting, mEnding, easedT);
    }

    // Call the setter
    ExceptionReport report;
    Call propertyCall(propertyDelegate->Set);
    propertyCall.Set(0, newValue);
    propertyCall.Invoke(report);

    // Check for completion
    if(t < 1.0f)
      return ActionState::Running;
    else
      return ActionState::Completed;
  }
};

// Create the Zilch property action.
template<typename PropertyType, typename EaserType>
void CreateZilchPropertyAction(Call& call, ExceptionReport& report)
{
  Handle actionSetHandle = call.GetHandle(0);
  Handle propertyDelegate = call.GetHandle(1);
  // Make sure the delegate isn't null
  if(propertyDelegate.IsNull())
  {
    DoNotifyException("Invalid Property Action", "Cannot form a property action on a null delegate.");
    return;
  }

  // Make sure the property isn't read-only
  PropertyDelegateTemplate* propertyDelegateTemplate = (PropertyDelegateTemplate*)propertyDelegate.Dereference();
  if(propertyDelegateTemplate->Set.BoundFunction == nullptr)
  {
    DoNotifyException("Invalid Property Action", "Cannot form a property action on a read-only property.");
    return;
  }
  // Make sure the property isn't read-only
  if(propertyDelegateTemplate->Get.BoundFunction == nullptr)
  {
    DoNotifyException("Invalid Property Action", "Cannot form a property action on a write-only property.");
    return;
  }

  ZilchPropertyAction<PropertyType, EaserType>* action = new ZilchPropertyAction<PropertyType, EaserType>();
  action->mEnding = call.Get<PropertyType>(2);
  action->mDuration = call.Get<float>(3);
  action->mEaser = EaserType::Build(call, 4);
  action->mStarting = PropertyType();
  action->mTime = 0;
  action->mPropertyDelegate = propertyDelegate;
  ActionSet* actionSet = (ActionSet*)actionSetHandle.Dereference();
  if(actionSet == nullptr)
  {
    call.GetState()->ThrowNullReferenceException(report, "Cannot create an action on a null ActionSet.");
    return;
  }

  actionSet->Add(action);
  call.Set(Call::Return, (Action*)action);
}

// Add the Action creation function
template<typename PropertyType, typename EaserType>
void AddZilchActionProperty(LibraryBuilder& library)
{
  Array<Constant> typeParameters;
  typeParameters.PushBack(Constant(ZilchTypeId(PropertyType)));
  String sig = String::Format("Property[%s]", typeParameters.Front().TypeValue->ToString().c_str());
  LibraryArray libs;
  libs.PushBack(Core::GetInstance().GetLibrary());
  BoundType* propertyType = library.InstantiateTemplate("Property", typeParameters, libs).Type;
  ParameterArray params;
  DelegateParameter& p0 = params.PushBack();
  p0.ParameterType = ZilchTypeId(ActionSet);
  p0.Name = "actionSet";
  DelegateParameter& p1 = params.PushBack();
  p1.ParameterType = propertyType;
  p1.Name = "prop";
  DelegateParameter& p2 = params.PushBack();
  p2.ParameterType = ZilchTypeId(PropertyType);
  p2.Name = "endValue";
  DelegateParameter& p3 = params.PushBack();
  p3.ParameterType = ZilchTypeId(float);
  p3.Name = "duration";

  EaserType::AddParam(library, params);

  library.AddExtensionFunction(ZilchTypeId(Action), "Property", CreateZilchPropertyAction<PropertyType, EaserType>, params, ZilchTypeId(Action), FunctionOptions::Static);
}

class ZilchCallAction : public Action
{
public:
  Delegate mDelegate;
  Array<Any> mArguments;

  ActionState::Enum Update(float dt) override
  {
    // Check Null
    if(mDelegate.BoundFunction == nullptr)
      return ActionState::Completed;

    // The object that the handle was on died
    if(mDelegate.ThisHandle.IsNull())
      return ActionState::Completed;

    ExceptionReport report;
    mDelegate.Invoke(mArguments);
    return ActionState::Completed;
  }
};

void CreateCallAction(Call& call, ExceptionReport& report)
{
  ActionSet* actionSet = call.Get<ActionSet*>(0);
  if(actionSet == nullptr)
  {
    call.GetState()->ThrowNullReferenceException(report, "Cannot create an action on a null ActionSet.");
    return;
  }

  ZilchCallAction* callAction = new ZilchCallAction();
  callAction->mDelegate = call.GetDelegate(1);

  // This 'CreateCallAction' is bound multiple times to support "variadic anys"
  // By default, the base function just takes 2 arguments (ActionSet, Delegate)
  // The variadic forms will take (ActionSet, Delegate, Any, Any...) which starts on parameter 2
  size_t anyStartParameter = 2;
  DelegateType* createCallFunctionType = call.GetFunction()->FunctionType;

  for (size_t i = anyStartParameter; i < createCallFunctionType->Parameters.Size(); ++i)
  {
    ErrorIf(createCallFunctionType->Parameters[i].ParameterType != ZilchTypeId(Any),
      "All parameters afterward must be of type Any");

    Any& arg = call.Get<Any&>(i);
    callAction->mArguments.PushBack(arg);
  }

  actionSet->Add(callAction);
  call.Set(Call::Return, (Action*)callAction);
}

void AddCall(LibraryBuilder& library)
{
  ParameterArray params;
  DelegateParameter& p0 = params.PushBack();
  p0.ParameterType = ZilchTypeId(ActionSet);

  // Accept any delegate, regardless of return type or parameters
  DelegateParameter& p1 = params.PushBack();
  p1.ParameterType = ZilchTypeId(Delegate);

  library.AddExtensionFunction(ZilchTypeId(Action), "Call", CreateCallAction, params, ZilchTypeId(Action), FunctionOptions::Static);

  // Now make up to a certain number of parameters overloads
  // (a better way to do this would be to support variadic params...)
  static const size_t VariadicParameterCount = 5;
  for (size_t i = 1; i <= VariadicParameterCount; ++i)
  {
    // Add extra call arguments for anys
    DelegateParameter& nextParam = params.PushBack();
    nextParam.ParameterType = ZilchTypeId(Any);

    library.AddExtensionFunction(ZilchTypeId(Action), "Call", CreateCallAction, params, ZilchTypeId(Action), FunctionOptions::Static);
  }
}

void CreateDelayAction(Call& call, ExceptionReport& report)
{
  ActionDelay* action = new ActionDelay(call.Get<float>(1));

  ActionSet* actionSet = call.Get<ActionSet*>(0);
  if(actionSet == NULL)
  {
    call.GetState()->ThrowNullReferenceException(report, "Cannot create an action on a null ActionSet.");
    return;
  }
  actionSet->Add(action);
  call.Set(Call::Return, (Action*)action);
}

void AddDelay(LibraryBuilder& library)
{
  ParameterArray params;
  DelegateParameter& p0 = params.PushBack();
  p0.ParameterType = ZilchTypeId(ActionSet);
  DelegateParameter& p1 = params.PushBack();
  p1.ParameterType = ZilchTypeId(float);
  library.AddExtensionFunction(ZilchTypeId(Action), "Delay", CreateDelayAction, params, ZilchTypeId(ActionDelay), FunctionOptions::Static);
}

void CreateEventAction(Call& call, ExceptionReport& report)
{
  ActionSet* actionSet = call.Get<ActionSet*>(0);
  Object* object = call.Get<Object*>(1);
  Handle eventIdHandle = call.GetHandle(2);
  String eventId = eventIdHandle.Get<String>();
  Event* eventToSend = call.Get<Event*>(3);

  if (actionSet == nullptr)
  {
    call.GetState()->ThrowNullReferenceException(report, "Cannot create an action on a null ActionSet.");
    return;
  }

  if(object == nullptr)
  {
    call.GetState()->ThrowNullReferenceException(report, "Cannot dispatch an Event on a null Object.");
    return;
  }

  if(eventId.Empty())
  {
    call.GetState()->ThrowNullReferenceException(report, "Cannot dispatch an Event with an empty event id.");
    return;
  }

  if (eventToSend == nullptr)
  {
    call.GetState()->ThrowNullReferenceException(report, "Cannot dispatch a null Event.");
    return;
  }

  ActionEvent* action = new ActionEvent(object, eventId, eventToSend);
  actionSet->Add(action);
  call.Set(Call::Return, (Action*)action);
}

void AddEvent(LibraryBuilder& library)
{
  ParameterArray params;
  DelegateParameter& p0 = params.PushBack();
  p0.ParameterType = ZilchTypeId(ActionSet);
  DelegateParameter& p1 = params.PushBack();
  p1.ParameterType = ZilchTypeId(Object);
  DelegateParameter& p2 = params.PushBack();
  p2.ParameterType = ZilchTypeId(String);
  DelegateParameter& p3 = params.PushBack();
  p3.ParameterType = ZilchTypeId(Event);
  library.AddExtensionFunction(ZilchTypeId(Action), "Event", CreateEventAction, params, ZilchTypeId(Action), FunctionOptions::Static);
}

template<typename ActionType>
void CreateActionType(Call& call, ExceptionReport& report)
{
  ActionSet* actionSet = call.Get<ActionSet*>(0);
  if(actionSet == nullptr)
  {
    call.GetState()->ThrowNullReferenceException(report, "Cannot create an action on a null ActionSet.");
    return;
  }
  ActionType* action = new ActionType();
  actionSet->Add(action);
  call.Set(Call::Return, action);
}

template<typename ActionType>
void AddActionSet(cstr name, LibraryBuilder& library)
{
  // Set the ActionSet type
  ParameterArray params;
  DelegateParameter& p0 = params.PushBack();
  p0.ParameterType = ZilchTypeId(ActionSet);
  library.AddExtensionFunction(ZilchTypeId(Action), name, CreateActionType<ActionType>, params, ZilchTypeId(ActionType), FunctionOptions::Static);
}

void BindActionFunctions(LibraryBuilder& library)
{
  AddZilchActionProperty<float, EaseTypeFunc>(library);
  AddZilchActionProperty<Vec2, EaseTypeFunc>(library);
  AddZilchActionProperty<Vec3, EaseTypeFunc>(library);
  AddZilchActionProperty<Vec4, EaseTypeFunc>(library);
  AddZilchActionProperty<Quat, EaseTypeFunc>(library);
  AddZilchActionProperty<int, EaseTypeFunc>(library);
  AddZilchActionProperty<bool, EaseTypeFunc>(library);
  AddZilchActionProperty<String, EaseTypeFunc>(library);

  AddZilchActionProperty<float, EaseTypeSampleCurve>(library);
  AddZilchActionProperty<Vec2, EaseTypeSampleCurve>(library);
  AddZilchActionProperty<Vec3, EaseTypeSampleCurve>(library);
  AddZilchActionProperty<Vec4, EaseTypeSampleCurve>(library);
  AddZilchActionProperty<Quat, EaseTypeSampleCurve>(library);
  AddZilchActionProperty<int, EaseTypeSampleCurve>(library);
  AddZilchActionProperty<bool, EaseTypeSampleCurve>(library);
  AddZilchActionProperty<String, EaseTypeSampleCurve>(library);

  AddDelay(library);
  AddCall(library);
  AddEvent(library);

  AddActionSet<ActionSequence>("Sequence", library);
  AddActionSet<ActionGroup>("Group", library);
}

}//namespace Zero
