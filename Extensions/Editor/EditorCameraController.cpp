///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorCameraController.cpp
/// Implementation of the EditorCameraController class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const float cMinLookDistance = 0.01f;
const float ModeChangeAnimationTime = 0.15f;
const float ResetVerticalAngle = 10.0f;

namespace Events
{
  DefineEvent(CameraControllerUpdated);
}//namespace Events

ZilchDefineType(EditorCameraController, builder, type)
{
  ZeroBindComponent();
  ZeroBindDependency(Transform);
  ZeroBindDependency(Camera);
  type->AddAttribute(ObjectAttributes::cCore);
  ZeroBindEvent(Events::CameraControllerUpdated, Event);

  ZilchBindFieldProperty(mMoveSensitivity);
  ZilchBindGetterSetterProperty(LookTarget);
  ZilchBindGetterSetterProperty(LookDistance);
  ZilchBindGetterSetterProperty(VerticalAngle);
  ZilchBindGetterSetterProperty(HorizontalAngle);
  ZilchBindGetterSetterProperty(ControlMode);
  ZilchBindMethodProperty(Reset);
}

EditorCameraController::EditorCameraController()
{
  mControlMode = ControlMode::Orbit;
  mControlPressed = false;
  mMoving = false;
  mMouseDragging = false;
  mDragMode = CameraDragMode::NotActive;
  mRotateSensitivity = 1.0f;
  mMoveSensitivity = 1.0f;
  mLookDistance = 40.0f;
  mEnabled = false;
  mMinLookDistance = cMinLookDistance;
  mTimeMoving = 0.0f;
  mMinCameraSize = 0.01f;
  mMaxCameraSize = 10000.0f;
  mVerticalAngle = 0.f;
  mHorizontalAngle = 0.f;
  mLookTarget = Vec3(0.f,0.f,0.f);
}

void EditorCameraController::Initialize(CogInitializer& initializer)
{
  mTransform = GetOwner()->has(Transform);
  mCamera =  GetOwner()->has(Camera);
  Deactivate();
  if (initializer.mSpace->IsEditorMode())
  {
    SetEnabled(true);
  }
}

void EditorCameraController::OnDestroy(uint flags)
{
}

void EditorCameraController::SetEnabled(bool value)
{
  if (mEnabled == value)
    return;

  mEnabled = value;

  if (mEnabled)
  {
    mEnabled = true;
    ConnectThisTo(GetSpace(), Events::FrameUpdate, OnFrameUpdate);
  }
  else
    ErrorIf(true, "No implementation for disabling EditorCameraController.");

}

void EditorCameraController::Serialize(Serializer& stream)
{
  SerializeEnumNameDefault(ControlMode, mControlMode, ControlMode::Orbit);
  SerializeEnumNameDefault(ControlMode, mPrevious3DMode, ControlMode::Orbit);
  SerializeNameDefault(mLookDistance, 50.0f);
  SerializeNameDefault(mVerticalAngle, 0.0f);
  SerializeNameDefault(mHorizontalAngle, 0.0f);
  SerializeNameDefault(mLookTarget, Vec3(0,0,0));
  SerializeNameDefault(mMoveSensitivity, 1.0f);
}

void EditorCameraController::Reset()
{
  mLookDistance = 10.0f;
  mLookTarget = mControlMode != ControlMode::ZPlane ? Vec3(0, 2, 0) : Vec3::cZero;
  mCameraDirection = -Vec3::cZAxis;
  mCameraRight = Vec3::cXAxis;
  mCameraUp = Vec3::cYAxis;
  mHorizontalAngle = 0;
  mVerticalAngle = mControlMode != ControlMode::ZPlane ? Math::DegToRad(ResetVerticalAngle) : 0.0f;
  UpdateTransform();
}

ControlMode::Enum EditorCameraController::GetControlMode()
{
  return mControlMode;
}

void EditorCameraController::SetControlMode(ControlMode::Enum mode)
{
  if(mode == ControlMode::FirstPerson || mode == ControlMode::Orbit)
  {
    ActionGroup* actionGroup = new ActionGroup(this->GetOwner(), ActionExecuteMode::FrameUpdate);

    float minVertical = Math::DegToRad(ResetVerticalAngle);
    if(Math::Abs(mVerticalAngle) < minVertical)
      actionGroup->Add(AnimateMember(&EditorCameraController::mVerticalAngle, Ease::Quad::InOut, this, ModeChangeAnimationTime, minVertical)); 

  }
  else if(mode == ControlMode::ZPlane)
  {
    ActionGroup* actionGroup = new ActionGroup(this->GetOwner(), ActionExecuteMode::FrameUpdate);
    actionGroup->Add(AnimateMember(&EditorCameraController::mVerticalAngle, Ease::Quad::InOut, this, ModeChangeAnimationTime, 0.0f));
    actionGroup->Add(AnimateMember(&EditorCameraController::mHorizontalAngle, Ease::Quad::InOut, this, ModeChangeAnimationTime, 0.0f));
  }

  mControlMode = mode;
}

EditorMode::Enum EditorCameraController::GetEditMode()
{
  if (mControlMode == ControlMode::ZPlane)
    return EditorMode::Mode2D;

  return EditorMode::Mode3D;
}

void EditorCameraController::SetEditMode(EditorMode::Enum mode)
{
  if (mode == EditorMode::Mode2D)
  {
    if (mControlMode != ControlMode::ZPlane)
      mPrevious3DMode = mControlMode;
    mCamera->SetPerspectiveMode(PerspectiveMode::Orthographic);
    SetControlMode(ControlMode::ZPlane);
  }
  else
  {
    mCamera->SetPerspectiveMode(PerspectiveMode::Perspective);
    SetControlMode(mPrevious3DMode);
  }
}

void EditorCameraController::OnAllObjectsCreated(CogInitializer& initializer)
{
  UpdateTransform();
}

void EditorCameraController::MouseDrag(CameraDragMode::Enum mode)
{
  mDragMode = mode;
  mMouseDragging = true;
}

void EditorCameraController::EndDrag()
{
  mMovement[ControllerButton::ZoomMove] = false;
  mMouseDragging  = false;
  mDragMode = CameraDragMode::NotActive;
}

Vec3 RotatePointAboutPoint(Vec3 point, Vec3 about, Quat rotation)
{
  Vec3 direction = point - about;
  Vec3 rotatedDirection = Math::Multiply(rotation, direction);
  return about + rotatedDirection;
}

void EditorCameraController::DragMovement(Vec2Param movement, Viewport* viewport)
{
  //The mouse is being dragged while in orbit mode
  //moving the mouse orbits around the target.
  if(mDragMode == CameraDragMode::Rotation)
  {
    // Default rotation is 0.3f degrees per pixel
    float rotateSen = mRotateSensitivity * Math::DegToRad(0.3f);
    if(mControlMode == ControlMode::FirstPerson)
      rotateSen *= 0.5f;

    float horizontalChange = movement.x * rotateSen;
    float verticalChange = movement.y * rotateSen;

    if(mControlMode == ControlMode::FirstPerson)
    {
      // Rotate the look at point around the camera in the reverse direction
      Vec3 cameraPosition = mTransform->GetWorldTranslation();
      Quat rotation = Math::ToQuaternion(0,-horizontalChange,0) * Math::ToQuaternion(mCameraRight, -verticalChange);
      mLookTarget = RotatePointAboutPoint(mLookTarget, cameraPosition, rotation);
    }

    mHorizontalAngle += horizontalChange;
    mVerticalAngle += verticalChange;
  }

  if(mDragMode == CameraDragMode::Zoom)
  {
    if(mCamera->mPerspectiveMode == PerspectiveMode::Orthographic)
    {
      // Change size in Orthographic
      float newCameraSize = mCamera->GetSize() + (-movement.y * 0.5f);
      newCameraSize = Math::Clamp(newCameraSize, mMinCameraSize, mMaxCameraSize);
      mCamera->SetSize(newCameraSize);
    }
    else
    {
      mLookTarget += mCameraDirection * movement.y;
    }
  }

  // Pan the camera
  if(mDragMode == CameraDragMode::Pan)
  {
    Vec2 viewportSize = viewport->GetSize();
    Vec2 percentageMoved = movement / viewportSize;
    Vec2 movementScalar = viewport->ViewPlaneSize(mLookDistance) * percentageMoved;

    if(mControlMode == ControlMode::FirstPerson)
    {
      // First person does not really use the look distance souse 
      // a pan speed of 0.02 u/per pixel
      movementScalar = movement * 0.02f * mMoveSensitivity;
    }

    mLookTarget -= mCameraRight * movementScalar.x;
    mLookTarget += mCameraUp * movementScalar.y;
  }
}

void EditorCameraController::OnFrameUpdate(UpdateEvent* event)
{
  Update(event->RealDt);
  Draw();
}

bool EditorCameraController::IsActive()
{
  return mDragMode != CameraDragMode::NotActive;
}

void EditorCameraController::SetVerticalAngle(float value)
{
  mVerticalAngle = Math::DegToRad(value);
  UpdateTransform();
}

void EditorCameraController::SetHorizontalAngle(float value)
{
  mHorizontalAngle = Math::DegToRad(value);
  UpdateTransform();
}

void EditorCameraController::SetLookTarget(Vec3Param newTarget)
{
  mLookTarget = newTarget;
  UpdateTransform();
}

Vec3 EditorCameraController::GetLookTarget()
{
  return mLookTarget;
}

void EditorCameraController::SetLookDistance(float newLookDis)
{
  mLookDistance = Math::Clamp(newLookDis, mMinLookDistance, mCamera->mFarPlane);
  UpdateTransform();
}

void EditorCameraController::AlignToCamera(Cog* cameraCog)
{
  if(Camera* camera = cameraCog->has(Camera))
  {
    // compute the look at target we want to set
    Vec3 camPos = camera->GetWorldTranslation();
    Vec3 camDir = camera->GetWorldDirection();
    Vec3 lookAtTarget = camPos + camDir * mLookDistance;

    // get the camera we are aligning withs vertical and horizontal angle
    // we directly set the angle values here over using the setters to save on
    // converting this to degrees to just be converted back to radians
    camDir.Normalize();
    mVerticalAngle = -Math::Sin(camDir.y);
    mHorizontalAngle = Math::ArcTan2(camDir.x, -camDir.z);

    // Set look at target calls update transform for us and uses our new
    // vertical and horizontal angle values
    SetLookTarget(lookAtTarget);
  }
}

float EditorCameraController::GetMinLookDistance()
{
  return mMinLookDistance;
}

void EditorCameraController::SetMinLookDistance(float distance)
{
  mMinLookDistance = distance;
  UpdateTransform();
}

void EditorCameraController::Draw()
{
  if(mControlMode == ControlMode::Orbit)
  {
    if(mDragMode == CameraDragMode::Rotation)
    {
      gDebugDraw->Add(Debug::Sphere(mLookTarget, 1.0f).BackShade(true).Colored(true));
    }
    else if(mDragMode == CameraDragMode::Pan)
    {
      gDebugDraw->Add(Debug::LineCross(mLookTarget, 1.0f));
    }
  }
}

void EditorCameraController::Update(float dt)
{
  Keyboard* keyboard = Keyboard::Instance;

  if(!(keyboard->KeyIsDown(Keys::W) || keyboard->KeyIsDown(Keys::Up)))
    mMovement[ControllerButton::MoveForward] = false;
  if(!(keyboard->KeyIsDown(Keys::A) || keyboard->KeyIsDown(Keys::Left)))
    mMovement[ControllerButton::MoveLeft] = false;
  if(!(keyboard->KeyIsDown(Keys::S) || keyboard->KeyIsDown(Keys::Down)))
    mMovement[ControllerButton::MoveBack] = false;
  if(!(keyboard->KeyIsDown(Keys::D) || keyboard->KeyIsDown(Keys::Right)))
    mMovement[ControllerButton::MoveRight] = false;

  if(!keyboard->KeyIsDown(Keys::Control))
    mControlPressed = false;
  if(keyboard->KeyIsDown(Keys::E))
    mDragMode = CameraDragMode::Pan;
  else if(keyboard->KeyIsDown(Keys::Q) || keyboard->KeyIsDown(Keys::Alt))
    mDragMode = CameraDragMode::Rotation;
  else if(!mMouseDragging)
    mDragMode = CameraDragMode::NotActive;

  // Keyboard rotate is one degree per second
  float rotateSpeed = Math::DegToRad(1.0f);

  // Default move speed is 10 u/s
  float moveSpeed = 10;

  if(mControlMode == ControlMode::FirstPerson)
    rotateSpeed *= 0.5f;

  rotateSpeed *= mRotateSensitivity;
  moveSpeed *= mMoveSensitivity;

  // Move speed increase while holding
  // down button
  float extraMove = mTimeMoving - 2.0f;
  if(extraMove > 0.0f)
  {
    extraMove = Math::Clamp(extraMove, 0.0f, 20.0f);
    moveSpeed += moveSpeed * extraMove;
  }

  rotateSpeed *= dt;
  moveSpeed *= dt;

  Vec3 direction = mCameraDirection;
  Vec3 right = mCameraRight;
  Vec3 up = Cross(right, direction);

  if(mControlMode == ControlMode::ZPlane)
  {
    direction = -Vec3::cZAxis;
    right = Vec3::cXAxis;
  }

  if(mControlMode != ControlMode::ZPlane)
  {
    if(mMovement[ControllerButton::MoveForward])
      mLookTarget += direction * moveSpeed;
    if(mMovement[ControllerButton::MoveBack])
      mLookTarget -= direction * moveSpeed;
  }
  else
  {
    if(mMovement[ControllerButton::MoveForward])
      mLookTarget += up * moveSpeed;
    if(mMovement[ControllerButton::MoveBack])
      mLookTarget -= up * moveSpeed;
  }

  if(mMovement[ControllerButton::MoveLeft])
    mLookTarget -= right * moveSpeed;
  if(mMovement[ControllerButton::MoveRight])
    mLookTarget += right * moveSpeed;

  //Not used
  if(mMovement[ControllerButton::RotateRight])
    mHorizontalAngle += rotateSpeed;
  if(mMovement[ControllerButton::RotateLeft])
    mHorizontalAngle -= rotateSpeed;

  if(mMovement[ControllerButton::PitchUp])
    mVerticalAngle += rotateSpeed;
  if(mMovement[ControllerButton::PitchDown])
    mVerticalAngle -= rotateSpeed;

  if(mMovement[ControllerButton::MoveUp])
    mLookTarget += mCameraUp * moveSpeed;
  if(mMovement[ControllerButton::MoveDown])
    mLookTarget -= mCameraUp * moveSpeed;

  mMoving = false;
  for(uint i = 0; i < ControllerButton::NumButtons; ++i)
  {
    if(mMovement[i])
      mMoving = true;
  }

  if(mMoving)
    mTimeMoving += dt;
  else
    mTimeMoving = 0.0;

  UpdateTransform();
}

void EditorCameraController::ProcessMiddleMouseDown(void)
{
   Keyboard* keyboard = Keyboard::Instance;

   if(keyboard->KeyIsDown(Keys::Shift))
   {
     MouseDrag(CameraDragMode::Zoom);
   }
   else
   {
     MouseDrag(CameraDragMode::Pan);
   }
}

void EditorCameraController::MouseScroll(Vec2Param scrollMove)
{
  // Update mouse scrolling
  if(mControlMode == ControlMode::FirstPerson)
  {
    mLookTarget += GetCameraDirection() * scrollMove.y;
  }
  else
  {
    // How many steps it takes to jump a level
    const float cScrollExponentScalar = 0.1f;

    //if orthographic, we have to adjust the size not the look distance
    if(mCamera->mPerspectiveMode == PerspectiveMode::Orthographic)
    {
      float cameraSizeExponent = Math::Log(mCamera->GetSize());
      float newSizeExponent = cameraSizeExponent + (-scrollMove.y * cScrollExponentScalar);

      float newCameraSize = Math::Exp(newSizeExponent);
      newCameraSize = Math::Clamp(newCameraSize, mMinCameraSize, mMaxCameraSize);
      mCamera->SetSize(newCameraSize);
    }
    //otherwise, dolly the camera based on the mouse scroll wheel
    else
    {
      float expLookDistance = Math::Log(mLookDistance);
      expLookDistance += -scrollMove.y * cScrollExponentScalar;
      mLookDistance = Math::Exp(expLookDistance);
    }
  }

  UpdateTransform();
}

void EditorCameraController::ClearMovement()
{
  for(uint i = 0; i < ControllerButton::NumButtons; ++i)
    mMovement[i] = false;
}

void EditorCameraController::Deactivate()
{
  mMouseDragging = false;
  mDragMode = CameraDragMode::NotActive;
  ClearMovement();
}

void EditorCameraController::ProcessKeyboardEvent(KeyboardEvent* keyEvent)
{
  //Check for control state
  if(keyEvent->Key == Keys::Control)
    mControlPressed = keyEvent->State != 0;

  //No camera controls while control is pressed.
  if(mControlPressed)
    return;

  //Do not process repeats
  if(keyEvent->State == KeyState::Repeated)
    return;

  // Skip alt for key states
  if(keyEvent->AltPressed)
    return;

  switch(keyEvent->Key)
  {
    //--------------------------------------------------------------------------
    case Keys::Up:
    case Keys::W:
    {
      mMovement[ControllerButton::MoveForward] = keyEvent->State;
    }
    break;

    //--------------------------------------------------------------------------
    case Keys::Left:
    case Keys::A:
    {
      mMovement[ControllerButton::MoveLeft] = keyEvent->State;
    }
    break;

    //--------------------------------------------------------------------------
    case Keys::Down:
    case Keys::S:
    {
      mMovement[ControllerButton::MoveBack] = keyEvent->State;
    }
    break;

    //--------------------------------------------------------------------------
    case Keys::Right:
    case Keys::D:
    {
      mMovement[ControllerButton::MoveRight] = keyEvent->State;
    }
    break;

    //--------------------------------------------------------------------------
    case Keys::Q:
    {
      if(keyEvent->State)
        mDragMode = CameraDragMode::Rotation;
      else
        mDragMode = CameraDragMode::NotActive;
    }
    break;

    //--------------------------------------------------------------------------
    case Keys::E:
    {
      if(keyEvent->State)
        mDragMode = CameraDragMode::Pan;
      else
        mDragMode = CameraDragMode::NotActive;
    }
    break;
  }//end switch key event
}


void EditorCameraController::UpdateTransform()
{
  mVerticalAngle = Math::Clamp(mVerticalAngle, -Math::cPi * 0.5f, Math::cPi * 0.5f);
  mLookDistance = Math::Clamp(mLookDistance, mMinLookDistance, mCamera->mFarPlane);

  mCameraDirection.x = Math::Sin(mHorizontalAngle) * Math::Cos(mVerticalAngle);
  mCameraDirection.y = -Math::Sin(mVerticalAngle);
  mCameraDirection.z = -Math::Cos(mHorizontalAngle) * Math::Cos(mVerticalAngle);

  mCameraRight.x = Math::Sin(mHorizontalAngle + Math::cPi * 0.5f);
  mCameraRight.y = 0.0f;
  mCameraRight.z = -Math::Cos(mHorizontalAngle + Math::cPi * 0.5f);

  mCameraUp = Cross(mCameraRight, mCameraDirection).Normalized();

  // Compute camera position
  mCameraPosition = mLookTarget - mCameraDirection * mLookDistance;

  Quat rotation = Math::ToQuaternion(mCameraDirection, mCameraUp, mCameraRight);

  mTransform->SetRotation(rotation);
  mTransform->SetTranslation(mCameraPosition);

  mTransform->UpdateAll();

  Event e;
  GetOwner()->DispatchEvent(Events::CameraControllerUpdated, &e);
}

}//namespace Zero
