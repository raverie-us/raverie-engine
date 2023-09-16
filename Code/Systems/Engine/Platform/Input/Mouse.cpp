// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Z
{
Mouse* gMouse = nullptr;
}

Mouse::Mouse()
{
  mPlatform = Z::gEngine->has(OsShell);
  mCursorMovement = Vec2::cZero;
  mClientPosition = Vec2::cZero;
  mRawMovement = Vec2::cZero;
  for (uint i = 0; i < MouseButtons::Size; ++i)
    mButtonDown[i] = false;
  Z::gMouse = this;
}

ZilchDefineType(Mouse, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);

  ZeroBindDocumented();
  ZilchBindGetterSetterProperty(Cursor);
  ZilchBindMethod(IsButtonDown);

  ZilchBindGetterProperty(ClientPosition);
  ZilchBindGetterProperty(CursorMovement);
  ZilchBindGetterSetterProperty(Trapped);
  ZilchBindMethod(ToggleTrapped);

  ZilchBindFieldProperty(mRawMovement);
}

bool Mouse::IsButtonDown(MouseButtons::Enum button)
{
  return mButtonDown[button] != 0;
}

Cursor::Enum Mouse::GetCursor()
{
  return Shell::sInstance->GetMouseCursor();
}

void Mouse::SetCursor(Cursor::Enum cursor)
{
  Shell::sInstance->SetMouseCursor(cursor);
}

bool Mouse::GetTrapped()
{
  return Shell::sInstance->GetMouseTrap();
}

void Mouse::SetTrapped(bool state)
{
  Shell::sInstance->SetMouseTrap(state);
}

void Mouse::ToggleTrapped()
{
  Shell::sInstance->SetMouseTrap(!Shell::sInstance->GetMouseTrap());
}

} // namespace Zero
