// MIT Licensed (see LICENSE.md).

// Include protection
#pragma once
#ifndef ZILCH_PROCESSCLASS_HPP
#  define ZILCH_PROCESSCLASS_HPP

namespace Zilch
{
/// Class to interface with another process. Allows re-directing standard input,
/// output, and error.
class ZeroShared ProcessClass : public Zero::Process
{
public:
  ZilchDeclareType(ProcessClass, TypeCopyMode::ReferenceType);

  ProcessClass();
  ~ProcessClass();

  void Start(Zero::ProcessStartInfo& info);

  // Close the process handle this does not force the process to exit.
  void Close();
  void CloseStreams();

  HandleOf<FileStreamClass> GetStandardError();
  HandleOf<FileStreamClass> GetStandardInput();
  HandleOf<FileStreamClass> GetStandardOutput();

  bool mRedirectStandardError;
  bool mRedirectStandardInput;
  bool mRedirectStandardOutput;

  HandleOf<FileStreamClass> mStandardError;
  HandleOf<FileStreamClass> mStandardInput;
  HandleOf<FileStreamClass> mStandardOutput;
};

} // namespace Zilch

#endif
