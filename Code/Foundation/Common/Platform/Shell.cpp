// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
bool gIntelGraphics = false;

cstr KeyNames[Keys::KeyMax + 1] = {0};

#define SetKeyName(value) KeyNames[Keys::value] = #value;
#define SetKeyNameLiteral(value, name) KeyNames[value] = name;

void InitializeKeyboard()
{
  SetKeyName(Unknown);
  SetKeyName(LeftBracket);
  SetKeyName(RightBracket);
  SetKeyName(Comma);
  SetKeyName(Period);
  SetKeyName(Semicolon);
  SetKeyName(Space);
  SetKeyName(Equal);
  SetKeyName(Minus);

  SetKeyName(Apostrophe);

  SetKeyName(Up);
  SetKeyName(Down);
  SetKeyName(Left);
  SetKeyName(Right);

  SetKeyName(F1);
  SetKeyName(F2);
  SetKeyName(F3);
  SetKeyName(F4);
  SetKeyName(F5);
  SetKeyName(F6);
  SetKeyName(F7);
  SetKeyName(F8);
  SetKeyName(F9);
  SetKeyName(F10);
  SetKeyName(F11);
  SetKeyName(F12);
  SetKeyName(Insert);
  SetKeyName(Delete);
  SetKeyName(Back);
  SetKeyName(Home);
  SetKeyName(End);
  SetKeyName(Tilde);
  SetKeyName(Slash);
  SetKeyName(Backslash);
  SetKeyName(Tab);
  SetKeyName(Shift);
  SetKeyName(Alt);
  SetKeyName(Control);
  SetKeyName(Capital);
  SetKeyName(Enter);
  SetKeyName(Escape);
  SetKeyName(PageUp);
  SetKeyName(PageDown);

  SetKeyName(NumPad0);
  SetKeyName(NumPad1);
  SetKeyName(NumPad2);
  SetKeyName(NumPad3);
  SetKeyName(NumPad4);
  SetKeyName(NumPad5);
  SetKeyName(NumPad6);
  SetKeyName(NumPad7);
  SetKeyName(NumPad8);
  SetKeyName(NumPad9);

  SetKeyName(Add);
  SetKeyName(Multiply);
  SetKeyName(Subtract);
  SetKeyName(Divide);
  SetKeyName(Decimal);

  SetKeyName(A);
  SetKeyName(B);
  SetKeyName(C);
  SetKeyName(D);
  SetKeyName(E);
  SetKeyName(F);
  SetKeyName(G);
  SetKeyName(H);
  SetKeyName(I);
  SetKeyName(J);
  SetKeyName(K);
  SetKeyName(L);
  SetKeyName(M);
  SetKeyName(N);
  SetKeyName(O);
  SetKeyName(P);
  SetKeyName(Q);
  SetKeyName(R);
  SetKeyName(S);
  SetKeyName(T);
  SetKeyName(U);
  SetKeyName(V);
  SetKeyName(W);
  SetKeyName(Y);
  SetKeyName(X);
  SetKeyName(Z);

  SetKeyNameLiteral('0', "Zero");
  SetKeyNameLiteral('1', "One");
  SetKeyNameLiteral('2', "Two");
  SetKeyNameLiteral('3', "Three");
  SetKeyNameLiteral('4', "Four");
  SetKeyNameLiteral('5', "Five");
  SetKeyNameLiteral('6', "Six");
  SetKeyNameLiteral('7', "Seven");
  SetKeyNameLiteral('8', "Eight");
  SetKeyNameLiteral('9', "Nine");

  SetKeyName(None);
}

FileDialogFilter::FileDialogFilter()
{
}

FileDialogFilter::FileDialogFilter(StringParam filter) : mDescription(filter), mFilter(filter)
{
}

FileDialogFilter::FileDialogFilter(StringParam description, StringParam filter) :
    mDescription(description),
    mFilter(filter)
{
}

FileDialogInfo::FileDialogInfo() : mCallback(nullptr), mUserData(nullptr), Flags(0)
{
}

void FileDialogInfo::AddFilter(StringParam description, StringParam filter)
{
  mSearchFilters.PushBack(FileDialogFilter(description, filter));
}

IntVec2 ShellWindow::GetMonitorClientPosition()
{
  return GetMonitorClientRectangle().TopLeft();
}

void ShellWindow::SetMonitorClientPosition(Math::IntVec2Param monitorPosition)
{
  IntRect monitorClientRectangle = GetMonitorClientRectangle();
  monitorClientRectangle.X = monitorPosition.x;
  monitorClientRectangle.Y = monitorPosition.y;
  SetMonitorClientRectangle(monitorClientRectangle);
}

IntVec2 ShellWindow::GetClientSize()
{
  return GetMonitorClientRectangle().Size();
}

void ShellWindow::SetClientSize(Math::IntVec2Param clientSize)
{
  IntRect monitorClientRectangle = GetMonitorClientRectangle();
  monitorClientRectangle.SizeX = clientSize.x;
  monitorClientRectangle.SizeY = clientSize.y;
  SetMonitorClientRectangle(monitorClientRectangle);
}

IntVec2 ShellWindow::GetMonitorBorderedPosition()
{
  return GetMonitorBorderedRectangle().TopLeft();
}

void ShellWindow::SetMonitorBorderedPosition(Math::IntVec2Param monitorPosition)
{
  IntRect monitorBorderedRectangle = GetMonitorBorderedRectangle();
  monitorBorderedRectangle.X = monitorPosition.x;
  monitorBorderedRectangle.Y = monitorPosition.y;
  SetMonitorBorderedRectangle(monitorBorderedRectangle);
}

IntVec2 ShellWindow::GetBorderedSize()
{
  return GetMonitorBorderedRectangle().Size();
}

void ShellWindow::SetBorderedSize(Math::IntVec2Param borderedSize)
{
  IntRect monitorBorderedRectangle = GetMonitorBorderedRectangle();
  monitorBorderedRectangle.SizeX = borderedSize.x;
  monitorBorderedRectangle.SizeY = borderedSize.y;
  SetMonitorBorderedRectangle(monitorBorderedRectangle);
}

static HashMap<uint, String> InternalGenerateUsageNames()
{
  HashMap<uint, String> names;

  names.Insert(UsbUsage::X, "X");
  names.Insert(UsbUsage::Y, "Y");
  names.Insert(UsbUsage::Z, "Z");
  names.Insert(UsbUsage::Rx, "Rx");
  names.Insert(UsbUsage::Ry, "Ry");
  names.Insert(UsbUsage::Rz, "Rz");
  names.Insert(UsbUsage::Slider, "Slider");
  names.Insert(UsbUsage::Dial, "Dial");
  names.Insert(UsbUsage::Wheel, "Wheel");
  names.Insert(UsbUsage::HatSwitch, "Hat switch");
  names.Insert(UsbUsage::CountedBuffer, "Counted Buffer");
  names.Insert(UsbUsage::ByteCount, "Byte Count");
  names.Insert(UsbUsage::MotionWakeup, "Motion Wakeup");
  names.Insert(UsbUsage::Start, "Start");
  names.Insert(UsbUsage::Select, "Select");
  names.Insert(UsbUsage::Vx, "Vx");
  names.Insert(UsbUsage::Vy, "Vy");
  names.Insert(UsbUsage::Vz, "Vz");
  names.Insert(UsbUsage::Vbrx, "Vbrx");
  names.Insert(UsbUsage::Vbry, "Vbry");
  names.Insert(UsbUsage::Vbrz, "Vbrz");
  names.Insert(UsbUsage::Vno, "Vno");
  names.Insert(UsbUsage::FeatureNotification, "Feature Notification");
  names.Insert(UsbUsage::SystemControl, "System Control");
  names.Insert(UsbUsage::SystemPowerDown, "System Power Down");
  names.Insert(UsbUsage::SystemSleep, "System Sleep");
  names.Insert(UsbUsage::SystemWake, "System Wake");
  names.Insert(UsbUsage::SystemContextMenu, "System Context Menu");
  names.Insert(UsbUsage::SystemMainMenu, "System Main Menu");
  names.Insert(UsbUsage::SystemAppMenu, "System App Menu");
  names.Insert(UsbUsage::SystemMenuHelp, "System Menu Help");
  names.Insert(UsbUsage::SystemMenuExit, "System Menu Exit");
  names.Insert(UsbUsage::SystemMenuSelect, "System Menu Select");
  names.Insert(UsbUsage::SystemMenuRight, "System Menu Right");
  names.Insert(UsbUsage::SystemMenuLeft, "System Menu Left");
  names.Insert(UsbUsage::SystemMenuUp, "System Menu Up");
  names.Insert(UsbUsage::SystemMenuDown, "System Menu Down");
  names.Insert(UsbUsage::SystemColdRestart, "System Cold Restart");
  names.Insert(UsbUsage::SystemWarmRestart, "System Warm Restart");
  names.Insert(UsbUsage::DpadUp, "D-pad Up");
  names.Insert(UsbUsage::DpadDown, "D-pad Down");
  names.Insert(UsbUsage::DpadRight, "D-pad Right");
  names.Insert(UsbUsage::DpadLeft, "D-pad Left");
  return names;
}

HashMap<uint, String>& GetUsageNames()
{
  static HashMap<uint, String> names = InternalGenerateUsageNames();
  return names;
}

} // namespace Zero
