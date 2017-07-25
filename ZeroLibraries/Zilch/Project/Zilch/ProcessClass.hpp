/**************************************************************\
* Author: Joshua T. Fisher, Joshua Davis
* Copyright 2015, DigiPen Institute of Technology
\**************************************************************/

// Include protection
#pragma once
#ifndef ZILCH_PROCESSCLASS_HPP
#define ZILCH_PROCESSCLASS_HPP

namespace Zilch
{  
  /// Class to interface with another process. Allows re-directing standard input, output, and error.
  class ProcessClass : public Zero::Process
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    ProcessClass();
    ~ProcessClass();

    void Start(Zero::ProcessStartInfo& info);

    // Close the process handle this does not force the process to exit.
    void Close();
    void CloseStreams();

    FileStream* GetStandardError();
    FileStream* GetStandardInput();
    FileStream* GetStandardOutput();
    
    bool mRedirectStandardError;
    bool mRedirectStandardInput;
    bool mRedirectStandardOutput;
    
    HandleOf<FileStream> mStandardError;
    HandleOf<FileStream> mStandardInput;
    HandleOf<FileStream> mStandardOutput;
  };

}

#endif
