///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Globally unique identifier
struct Guid
{
  Guid() { }
  Guid(const u64& val) : mValue(val) {}
  bool operator==(const Guid& rhs) const { return mValue == rhs.mValue; }
  bool operator!=(const Guid& rhs) const { return mValue != rhs.mValue; }
  bool operator<(const Guid& rhs) const { return mValue < rhs.mValue; }
  bool operator>(const Guid& rhs) const { return mValue > rhs.mValue; }
  bool operator<=(const Guid& rhs) const { return mValue <= rhs.mValue; }
  bool operator>=(const Guid& rhs) const { return mValue >= rhs.mValue; }
  bool operator==(const u64& rhs) const { return mValue == rhs; }
  bool operator!=(const u64& rhs) const { return mValue != rhs; }
  bool operator==(const uint& rhs) const { return mValue == rhs; }
  bool operator!=(const uint& rhs) const { return mValue != rhs; }
  bool operator==(const int& rhs) const { return mValue == rhs; }
  bool operator!=(const int& rhs) const { return mValue != rhs; }
  Guid operator+(const u64& rhs) const { return mValue + rhs; }
  Guid operator+(const uint& rhs) const { return mValue + rhs; }
  Guid operator^(const size_t& rhs) const { return mValue ^ rhs; }

  explicit operator bool() const { return mValue; }
  explicit operator u64() const { return mValue; }
  explicit operator size_t() const { return (size_t)mValue; }

  size_t Hash() const;

  u64 mValue;
};

} // namespace Zero
