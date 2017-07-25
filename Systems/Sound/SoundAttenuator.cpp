///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"
namespace Zero
{
const size_t cSoundCurveSamples = 30;

//---------------------------------------------------------------------------- Sound Attenuator Node

//**************************************************************************************************
SoundAttenuatorNode::~SoundAttenuatorNode()
{
  mNode->DeleteThisNode();
}

//--------------------------------------------------------------------------------- Sound Attenuator 

//**************************************************************************************************
ZilchDefineType(SoundAttenuatorDisplay, builder, type)
{
}

//**************************************************************************************************
String SoundAttenuatorDisplay::GetName(HandleParam object)
{
  SoundAttenuator* soundAtten = object.Get<SoundAttenuator*>(GetOptions::AssertOnNull);
  return BuildString("SoundAttenuator: ", soundAtten->Name);
}

//**************************************************************************************************
String SoundAttenuatorDisplay::GetDebugText(HandleParam object)
{
  return GetName(object);
}

//**************************************************************************************************
String SoundAttenuationToString(const BoundType* meta, const byte* data)
{
  SoundAttenuator* soundAtten = (SoundAttenuator*)data;
  return BuildString("SoundAttenuator: ", soundAtten->Name);
}

//**************************************************************************************************
ZilchDefineType(SoundAttenuator, builder, type)
{
  ZeroBindDocumented();
  type->ToStringFunction = SoundAttenuationToString;

  ZilchBindGetterSetterProperty(StartDistance);
  ZilchBindGetterSetterProperty(StopDistance);
  ZilchBindGetterSetterProperty(MinAttenuatedVolume)->Add(new EditorRange(0.0f, 1.0f, 0.01f));
  ZilchBindGetterSetterProperty(UseLowPassFilter)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindGetterSetterProperty(LowPassStartDistance)->ZeroFilterBool(mUseLowPassFilter);
  ZilchBindGetterSetterProperty(LowPassCutoffFreq)->ZeroFilterBool(mUseLowPassFilter);
  ZilchBindGetterSetterProperty(FalloffCurveType)->AddAttribute(PropertyAttributes::cInvalidatesObject); 
  ZilchBindGetterSetterProperty(FalloffCurve)->ZeroFilterEquality(mFalloffCurveType, 
    FalloffCurveType::Enum, FalloffCurveType::Custom);
}

//**************************************************************************************************
SoundAttenuator::SoundAttenuator() : 
  mStartDistance(1.0f), 
  mStopDistance(70.0f), 
  mMinAttenuatedVolume(0.0f),
  mFalloffCurveType(FalloffCurveType::Log), 
  mCustomFalloffCurve(nullptr), 
  mUseLowPassFilter(true), 
  mLowPassStartDistance(20.0f), 
  mLowPassCutoffFreq(1000.0f)
{

}

//**************************************************************************************************
SoundAttenuator::~SoundAttenuator()
{
  // Delete any existing SoundAttenuatorNode objects
  for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); )
  {
    SoundAttenuatorNode* node = &nodes.Front();
    nodes.PopFront();
    delete node;
  }

}

//**************************************************************************************************
void SoundAttenuator::Serialize(Serializer& stream)
{
  SerializeNameDefault(mStartDistance, 1.0f);
  SerializeNameDefault(mStopDistance, 70.0f);
  SerializeNameDefault(mMinAttenuatedVolume, 0.0f);
  SerializeNameDefault(mUseLowPassFilter, true);
  SerializeNameDefault(mLowPassStartDistance, 20.0f);
  SerializeNameDefault(mLowPassCutoffFreq, 1000.0f);
  SerializeNullableResourceNameDefault(mCustomFalloffCurve, CurveManager, nullptr);
  SerializeEnumNameDefault(FalloffCurveType, mFalloffCurveType, FalloffCurveType::Log);
}

//**************************************************************************************************
void SoundAttenuator::Initialize()
{
  if (mFalloffCurveType == FalloffCurveType::Custom && mCustomFalloffCurve)
  {
    UpdateCurve(nullptr);
    SampleCurve* curve = mCustomFalloffCurve;
    if (curve)
      ConnectThisTo(curve, Events::ObjectModified, UpdateCurve);
  }
}

//**************************************************************************************************
void SoundAttenuator::Unload()
{
  mCustomFalloffCurve = nullptr;
}

//**************************************************************************************************
float SoundAttenuator::GetStartDistance()
{
  return mStartDistance;
}

//**************************************************************************************************
void SoundAttenuator::SetStartDistance(float value)
{
  mStartDistance = value;

  // Update the attenuation information on all existing nodes
  for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
    nodes.Front().mNode->SetAttenuationData(Audio::AttenuationData(mStartDistance, mStopDistance, 
      mMinAttenuatedVolume));
}

//**************************************************************************************************
float SoundAttenuator::GetStopDistance()
{
  return mStopDistance;
}

//**************************************************************************************************
void SoundAttenuator::SetStopDistance(float value)
{
  mStopDistance = value;

  // Update the attenuation information on all existing nodes
  for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
    nodes.Front().mNode->SetAttenuationData(Audio::AttenuationData(mStartDistance, mStopDistance, 
      mMinAttenuatedVolume));
}

//**************************************************************************************************
float SoundAttenuator::GetMinAttenuatedVolume()
{
  return mMinAttenuatedVolume;
}

//**************************************************************************************************
void SoundAttenuator::SetMinAttenuatedVolume(float value)
{
  mMinAttenuatedVolume = value;

  // Update the attenuation information on all existing nodes
  for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
    nodes.Front().mNode->SetAttenuationData(Audio::AttenuationData(mStartDistance, mStopDistance, 
      mMinAttenuatedVolume));
}

//**************************************************************************************************
SampleCurve* SoundAttenuator::GetFalloffCurve()
{
  return mCustomFalloffCurve;
}

//**************************************************************************************************
void SoundAttenuator::SetFalloffCurve(SampleCurve* newCurve)
{
  SampleCurve* curve = mCustomFalloffCurve;
  if (curve)
    curve->GetDispatcher()->DisconnectEvent(Events::ObjectModified, this);

  mCustomFalloffCurve = newCurve;

  UpdateCurve(nullptr);

  curve = mCustomFalloffCurve;

  if (curve)
  {
    ConnectThisTo(curve, Events::ObjectModified, UpdateCurve);

    Array<Vec3> curveData;
    curve->GetCurve(curveData);

    // Send the custom curve data to all existing nodes
    for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
      nodes.Front().mNode->SetCurveType(Audio::CustomCurveType, &curveData);
  }
  else
  {
    // Set the custom curve data to null on all existing nodes
    for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
      nodes.Front().mNode->SetCurveType(Audio::CustomCurveType, nullptr);
  }
}

//**************************************************************************************************
FalloffCurveType::Enum SoundAttenuator::GetFalloffCurveType()
{
  return mFalloffCurveType;
}

//**************************************************************************************************
void SoundAttenuator::SetFalloffCurveType(FalloffCurveType::Enum newtype)
{
  mFalloffCurveType = newtype;

  if (newtype != FalloffCurveType::Custom)
  {
    Audio::CurveTypes curve;
    switch (newtype)
    {
    case FalloffCurveType::Linear:
      curve = Audio::LinearCurveType;
      break;
    case FalloffCurveType::Squared:
      curve = Audio::SquaredCurveType;
      break;
    case FalloffCurveType::Sine:
      curve = Audio::SineCurveType;
      break;
    case FalloffCurveType::SquareRoot:
      curve = Audio::SquareRootCurveType;
      break;
    case FalloffCurveType::Log:
      curve = Audio::LogCurveType;
      break;
    default:
      curve = Audio::LogCurveType;
      break;
    }

    // Set the curve type on all existing nodes
    for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
      nodes.Front().mNode->SetCurveType(curve, nullptr);
  }
}

//**************************************************************************************************
bool SoundAttenuator::GetUseLowPassFilter()
{
  return mUseLowPassFilter;
}

//**************************************************************************************************
void SoundAttenuator::SetUseLowPassFilter(bool useFilter)
{
  mUseLowPassFilter = useFilter;

  // Update all existing nodes
  for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
    nodes.Front().mNode->SetUsingLowPass(useFilter);
}

//**************************************************************************************************
float SoundAttenuator::GetLowPassStartDistance()
{
  return mLowPassStartDistance;
}

//**************************************************************************************************
void SoundAttenuator::SetLowPassStartDistance(float distance)
{
  mLowPassStartDistance = distance;

  // Update all existing nodes
  for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
    nodes.Front().mNode->SetLowPassDistance(distance);
}

//**************************************************************************************************
float SoundAttenuator::GetLowPassCutoffFreq()
{
  return mLowPassCutoffFreq;
}

//**************************************************************************************************
void SoundAttenuator::SetLowPassCutoffFreq(float frequency)
{
  mLowPassCutoffFreq = frequency;

  // Update all existing nodes
  for (AttenuatorListType::range nodes = mNodeList.All(); !nodes.Empty(); nodes.PopFront())
    nodes.Front().mNode->SetLowPassCutoffFreq(frequency);
}

//**************************************************************************************************
void SoundAttenuator::UpdateCurve(Event* event)
{
  // We don't ever use the fallback or default resources... just roll back to 'none'
  SampleCurve* curve = mCustomFalloffCurve;
  if (curve)
  {
    CurveManager* manager = CurveManager::GetInstance();
    String& name = curve->Name;
    if (name == manager->DefaultResourceName || name == manager->FallbackResourceName)
      mCustomFalloffCurve = nullptr;
  }

}

//**************************************************************************************************
SoundAttenuatorNode* SoundAttenuator::GetAttenuationNode(StringParam name, unsigned ID)
{
  // Get the enum value for the curve type
  Audio::CurveTypes curveType;
  switch (mFalloffCurveType)
  {
  case FalloffCurveType::Linear:
    curveType = Audio::LinearCurveType;
    break;
  case FalloffCurveType::Squared:
    curveType = Audio::SquaredCurveType;
    break;
  case FalloffCurveType::Sine:
    curveType = Audio::SineCurveType;
    break;
  case FalloffCurveType::SquareRoot:
    curveType = Audio::SquareRootCurveType;
    break;
  case FalloffCurveType::Log:
    curveType = Audio::LogCurveType;
    break;
  case FalloffCurveType::Custom:
    curveType = Audio::CustomCurveType;
    break;
  default:
    curveType = Audio::LogCurveType;
    break;
  }

  SoundAttenuatorNode* node;
  Zero::Status status;
  // If using a custom curve, create the SoundAttenuatorNode with that curve
  if (mFalloffCurveType == FalloffCurveType::Custom && mCustomFalloffCurve)
  {
    Array<Vec3> curve;
    mCustomFalloffCurve->GetCurve(curve);

    node = new SoundAttenuatorNode(new Audio::AttenuatorNode(status, name, ID, Math::Vec3(0, 0, 0), 
      Audio::AttenuationData(mStartDistance, mStopDistance, mMinAttenuatedVolume), curveType, &curve, this));
  }
  // Otherwise create it for the specified curve type
  else
    node = new SoundAttenuatorNode(new Audio::AttenuatorNode(status, name, ID, Math::Vec3(0, 0, 0), 
      Audio::AttenuationData(mStartDistance, mStopDistance, mMinAttenuatedVolume), curveType, nullptr, this));

  if (status.Failed())
  {
    DoNotifyWarning("Audio Error", status.Message);
    node->mNode->DeleteThisNode();
    delete node;
    return nullptr;
  }

  node->mNode->SetUsingLowPass(mUseLowPassFilter);
  if (mUseLowPassFilter)
  {
    node->mNode->SetLowPassDistance(mLowPassStartDistance);
    node->mNode->SetLowPassCutoffFreq(mLowPassCutoffFreq);
  }

  mNodeList.PushBack(node);

  return node;
}

//**************************************************************************************************
void SoundAttenuator::RemoveAttenuationNode(SoundAttenuatorNode* node)
{
  mNodeList.Erase(node);
  delete node;
}

//**************************************************************************************************
bool SoundAttenuator::HasInput()
{
  // If any of the SoundAttenuatorNodes has input, return true
  forRange(SoundAttenuatorNode& node, mNodeList.All())
  {
    if (node.mNode->HasInputs())
      return true;
  }

  return false;
}

//------------------------------------------------------------------------- Sound Attenuator Manager

ImplementResourceManager(SoundAttenuatorManager, SoundAttenuator);

//**************************************************************************************************
SoundAttenuatorManager::SoundAttenuatorManager(BoundType* resourceType) : ResourceManager(resourceType)
{
  AddLoader("SoundAttenuator", new TextDataFileLoader<SoundAttenuatorManager>());
  DefaultResourceName = "DefaultNoAttenuation";
  mCategory = "Sound";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.SoundAttenuator.data"));
  mCanCreateNew = true;
  mCanDuplicate = true;
  mExtension = DataResourceExtension;
}

}
