///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorCameraController.hpp
/// Declaration of the EditorCameraController class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
 
//------------------------------------------------------------ Controller Button
DeclareEnum13(ControllerButton, MoveForward,
                                MoveBack,
                                MoveLeft,
                                MoveRight,
                                RotateRight,
                                RotateLeft,
                                PitchUp,
                                PitchDown,
                                MoveUp,
                                MoveDown,
                                OrbitMove,
                                ZoomMove,
                                NumButtons);

class KeyboardEvent;
class UpdateEvent;
class Camera;
class Viewport;

namespace Events
{
  DeclareEvent(CameraControllerUpdated);
}//namespace Events

DeclareEnum3(ControlMode, Orbit, FirstPerson, ZPlane);

DeclareEnum4(CameraDragMode, NotActive, Rotation, Pan, Zoom);

//----------------------------------------------------- Editor Camera Controller
class EditorCameraController : public Component
{
public:
  // Meta Initialization
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  EditorCameraController();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void OnDestroy(uint flags) override;

  // The look target the camera will be orbited around
  Vec3 GetLookTarget();
  void SetLookTarget(Vec3Param newTarget);

  // Distance to the look target
  float GetLookDistance() { return mLookDistance; }
  void SetLookDistance(float v);

  // Angle the camera is rotated horizontally around the look target
  float GetHorizontalAngle(){return  Math::RadToDeg(mHorizontalAngle);}
  void SetHorizontalAngle(float value);

  // Angle the camera is rotated vertically around the look target
  float GetVerticalAngle(){ return Math::RadToDeg(mVerticalAngle);}
  void SetVerticalAngle(float value);

  //Direction of the camera.
  Vec3 GetCameraDirection(){return mCameraDirection;}

  //Right vector of the camera.
  Vec3 GetCameraRight(){return mCameraRight;}

  //Up vector of the camera.
  Vec3 GetCameraUp(){return mCameraUp;}

  //Position of Camera
  Vec3 GetCameraPosition(){return mCameraPosition;}

  //Aligns with a passed in camera
  void AlignToCamera(Cog* cameraCog);

  //How close we can look/zoom in
  float GetMinLookDistance();
  void SetMinLookDistance(float distance);

  //Reset the camera
  void Reset();

  //Drag active
  bool IsActive();
  void SetEnabled(bool value);
  void Deactivate();
  void ProcessKeyboardEvent(KeyboardEvent* event);
  void MouseScroll(Vec2Param scrollMove);

  void MouseDrag(CameraDragMode::Enum drag);
  void DragMovement(Vec2Param movement, Viewport* viewport);
  void EndDrag();

  void ProcessMiddleMouseDown();

  ControlMode::Enum GetControlMode();
  void SetControlMode(ControlMode::Enum mode);
  EditorMode::Enum GetEditMode();
  void SetEditMode(EditorMode::Enum mode);

  real mMinCameraSize;
  real mMaxCameraSize;

private:
  friend class EditorCameraMouseDrag;
  void OnFrameUpdate(UpdateEvent* event);
  void UpdateTransform();
  void SetInput(uint index, bool state) { mMovement[index] = state; }
  void SetDragMode(uint mode) { mDragMode = mode; }
  void ClearMovement();
  void Update(float dt);

  void Draw();
  //Control variables
  Camera* mCamera;
  ControlMode::Enum mControlMode;
  ControlMode::Enum mPrevious3DMode;
  float mMoveSensitivity;
  Vec3 mLookTarget;
  Vec3 mCameraPosition;
  Vec3 mCameraDirection;
  Vec3 mCameraRight;
  Vec3 mCameraUp;
  float mRotateSensitivity;
  float mLookDistance;
  float mMinLookDistance;
  Transform* mTransform;
  float mVerticalAngle;
  float mHorizontalAngle;
  byte mMovement[ControllerButton::NumButtons];
  bool mMoving;
  bool mMouseDragging;
  uint mDragMode;
  bool mControlPressed;
  bool mEnabled;
  float mTimeMoving;
  friend class EditorViewport;
};

}//namespace Zero
