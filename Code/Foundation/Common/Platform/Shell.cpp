// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
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

FileDialogInfo::FileDialogInfo() : mCallback(nullptr), mUserData(nullptr)
{
}

void FileDialogInfo::AddFilter(StringParam description, StringParam filter)
{
  mSearchFilters.PushBack(FileDialogFilter(description, filter));
}

} // namespace Zero
