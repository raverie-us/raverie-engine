// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
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

RaverieDefineType(Mouse, builder, type)
{
  type->HandleManager = RaverieManagerId(PointerManager);

  RaverieBindDocumented();
  RaverieBindGetterSetterProperty(Cursor);
  RaverieBindMethod(IsButtonDown);

  RaverieBindGetterProperty(ClientPosition);
  RaverieBindGetterProperty(CursorMovement);
  RaverieBindGetterSetterProperty(Trapped);
  RaverieBindMethod(ToggleTrapped);

  RaverieBindFieldProperty(mRawMovement);
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

} // namespace Raverie
