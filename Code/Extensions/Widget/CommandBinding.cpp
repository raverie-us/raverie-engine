// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(MetaScriptTagAttribute, builder, type)
{
  RaverieBindField(mTags);
}

MetaScriptTagAttribute::MetaScriptTagAttribute()
{
}

void MetaScriptTagAttribute::PostProcess(Status& status, ReflectionObject* owner)
{
  if (mTags.Empty())
  {
    String message = "Tags are ' ' (space) delimited. Additionally: No tags are specified.";
    status.SetFailed(message);
    return;
  }

  mTagSet.Clear();

  StringTokenRange tokens(mTags.c_str(), ' ');
  for (; !tokens.Empty(); tokens.PopFront())
  {
    String tag = tokens.Front();
    if (tag.Empty())
      continue;

    if (!IsValidFilename(tag, status))
    {
      status.Message = BuildString("Tags are ' ' (space) delimited. Additionally: '", tag, "' ", status.Message);
      return;
    }

    mTagSet.Insert(tag);
  }

  // If the only token(s) found consist(s) of ' ' characters, it'll be caught
  // here.
  if (mTagSet.Empty())
  {
    String message = "Tags are ' ' (space) delimited. Additionally: No tags are specified.";
    status.SetFailed(message);
    return;
  }

  // Now that tags have been verified, cleaned up, and duplicates removed -
  // rebuild the whole tag string.
  mTags.Clear();
  mTags = String::JoinRange(" ", mTagSet.All());
}

RaverieDefineType(MetaScriptShortcutAttribute, builder, type)
{
  RaverieBindField(mKey);
  RaverieBindField(mCtrl)->AddAttribute(PropertyAttributes::cOptional);
  RaverieBindField(mAlt)->AddAttribute(PropertyAttributes::cOptional);
  RaverieBindField(mShift)->AddAttribute(PropertyAttributes::cOptional);
}

MetaScriptShortcutAttribute::MetaScriptShortcutAttribute() : mCtrl(false), mAlt(false), mShift(false), mKey("Unknown")
{
}

void MetaScriptShortcutAttribute::PostProcess(Status& status, ReflectionObject* owner)
{
  if (!owner->HasAttribute("Command"))
  {
    String message = "'Shortcut' attribute is dependent on the 'Command' attribute.";
    status.SetFailed(message);
    return;
  }

  bool hasModifier = false;
  hasModifier |= mCtrl;
  hasModifier |= mAlt;
  hasModifier |= mShift;

  int count = 0;
  StringTokenRange tokens(mKey.c_str(), ' ');

  // Prep for all whitespace detection.
  mKey.Clear();

  // Determine if there is more than one non-whitespace Shortcut main-key
  // specified. Only one is allowed.
  for (; !tokens.Empty(); tokens.PopFront())
  {
    // Skip whitespace.
    String key = tokens.Front();
    if (key.Empty())
      continue;

    ++count;
    if (count > 1)
    {
      String message = "Too many non-modifier keys specified for 'Shortcut' "
                       "attribute. Only one key is allowed.";
      status.SetFailed(message);
      return;
    }

    // Don't need to concatenate, as the check for more than one specified key
    // is handled above.
    mKey = key;
  }

  if (mKey.Empty())
  {
    String message = "Missing 'key' parameter value. See 'Keys' for key names, "
                     "or use the key symbol (NumPad symbols must be named).";
    status.SetFailed(message);
    return;
  }

  // Keep internal key string as the keyboard symbol, rather than the key name.
  mKey = Keyboard::Instance->ToSymbol(mKey);
  // As enum for easier [0-9] checking.
  Keys::Enum keyEnum = Keyboard::Instance->ToKey(mKey);

  if (mKey == "Ctrl" || mKey == "Alt" || mKey == "Shift")
  {
    String message = " is a Shortcut modifier-key and cannot be used for the "
                     "Shortcut main-key.";
    status.SetFailed(BuildString("'", mKey, "'", message));
    return;
  }
  else if ((mKey == "Space" || mKey == "Spacebar"))
  {
    String message = "'Keys.Spacebar' is a reserved Shortcut key.";
    status.SetFailed(message);
    return;
  }
  else if (keyEnum >= Keys::Num0 && keyEnum <= Keys::Num9)
  {
    // Inform the user that they can use the numpad version of a numeric key.
    String message = "' is a reserved Shortcut key. Use 'NumPad";
    status.SetFailed(BuildString("'", mKey, message, mKey, "' instead."));
    return;
  }
  else if (!Keyboard::Instance->Valid(mKey)) // Valid, but reserved, keys have
                                             // been pre-checked at this point.
  {
    String message = "\" for 'key' parameter. See 'Keys' for key names, or use "
                     "the key symbol (NumPad symbols must be named).";
    status.SetFailed(BuildString("Invalid value \"", mKey, message));
    return;
  }
  else
  {
    CommandManager* commands = CommandManager::GetInstance();

    Command* foundCommand;
    if (commands->IsShortcutReserved(mCtrl, mAlt, mShift, mKey, &foundCommand))
    {
      BoundType* type = Type::DynamicCast<BoundType*>(owner);

      // Not an error if the shortcut is already reserved by the command
      // currently being processed.
      if (foundCommand->Name != type->Name)
      {
        String message =
            BuildString(foundCommand->Shortcut, " is already used by the '", foundCommand->Name, "' command.");
        status.SetFailed(message);
        return;
      }
    }
  }
}

} // namespace Raverie
