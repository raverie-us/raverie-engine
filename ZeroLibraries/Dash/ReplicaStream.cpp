///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------//
//                               ReplicaStream                                     //
//---------------------------------------------------------------------------------//

ReplicaStream::ReplicaStream(Replicator* replicator, ReplicatorLink* replicatorLink, BitStream& bitStream, ReplicaStreamMode::Enum replicaStreamMode, TimeMs timestamp)
  : mReplicator(replicator),
    mReplicatorLink(replicatorLink),
    mBitStream(bitStream),
    mReplicaStreamMode(replicaStreamMode),
    mTimestamp(timestamp)
{
}
ReplicaStream::ReplicaStream(Replicator* replicator, ReplicatorLink* replicatorLink, const BitStream& bitStream, ReplicaStreamMode::Enum replicaStreamMode, TimeMs timestamp)
  : mReplicator(replicator),
    mReplicatorLink(replicatorLink),
    mBitStream(const_cast<BitStream&>(bitStream)),
    mReplicaStreamMode(replicaStreamMode),
    mTimestamp(timestamp)
{
}

//
// Operations
//

Replicator* ReplicaStream::GetReplicator() const
{
  return mReplicator;
}

ReplicatorLink* ReplicaStream::GetReplicatorLink() const
{
  return mReplicatorLink;
}

BitStream& ReplicaStream::GetBitStream() const
{
  return mBitStream;
}

ReplicaStreamMode::Enum ReplicaStream::GetReplicaStreamMode() const
{
  return mReplicaStreamMode;
}

TimeMs ReplicaStream::GetTimestamp() const
{
  return mTimestamp;
}

bool ReplicaStream::WriteCreationInfo(const Replica* replica)
{
  return WriteCreationInfo(replica->mCreateContext, replica->mReplicaType);
}
bool ReplicaStream::WriteCreationInfo(const CreateContext& createContext, const ReplicaType& replicaType)
{
  // Get item cachers
  Replicator::CreateContextCacher& createContextCacher = GetReplicator()->mCreateContextCacher;
  Replicator::ReplicaTypeCacher&   replicaTypeCacher   = GetReplicator()->mReplicaTypeCacher;

  //
  // Create Context (ID)
  //

  // Write create context ID
  Assert(createContextCacher.IsItemMapped(createContext));
  CreateContextId createContextId = createContextCacher.GetMappedItemId(createContext);
  mBitStream.Write(createContextId);

  //
  // Replica Type (ID)
  //

  // Write replica type ID
  Assert(replicaTypeCacher.IsItemMapped(replicaType));
  ReplicaTypeId replicaTypeId = replicaTypeCacher.GetMappedItemId(replicaType);
  mBitStream.Write(replicaTypeId);

  // Finished writing replica info
  return true;
}
bool ReplicaStream::ReadCreationInfo(Replica* replica) const
{
  return ReadCreationInfo(replica->mCreateContext, replica->mReplicaType);
}
bool ReplicaStream::ReadCreationInfo(CreateContext& createContext, ReplicaType& replicaType) const
{
  // Get item cachers
  Replicator::CreateContextCacher& createContextCacher = GetReplicator()->mCreateContextCacher;
  Replicator::ReplicaTypeCacher&   replicaTypeCacher   = GetReplicator()->mReplicaTypeCacher;

  //
  // Create Context (ID)
  //

  // Read create context ID
  CreateContextId createContextId;
  if(!mBitStream.Read(createContextId)) // Unable?
  {
    // Error reading replica info
    Assert(false);
    return false;
  }
  Assert(createContextId != 0);

  // Unknown create context?
  if(!createContextCacher.IsIdMapped(createContextId))
  {
    // Error reading replica info
    Assert(false);
    return false;
  }

  // Get create context
  createContext = createContextCacher.GetMappedIdItem(createContextId);
  if(createContext.IsEmpty()) // Unable?
  {
    // Error reading replica info
    Assert(false);
    return false;
  }

  //
  // Replica Type (ID)
  //

  // Read replica type ID
  ReplicaTypeId replicaTypeId;
  if(!mBitStream.Read(replicaTypeId)) // Unable?
  {
    // Error reading replica info
    Assert(false);
    return false;
  }
  Assert(replicaTypeId != 0);

  // Unknown replica type?
  if(!replicaTypeCacher.IsIdMapped(replicaTypeId))
  {
    // Error reading replica info
    Assert(false);
    return false;
  }

  // Get replica type
  replicaType = replicaTypeCacher.GetMappedIdItem(replicaTypeId);
  if(replicaType.IsEmpty()) // Unable?
  {
    // Error reading replica info
    Assert(false);
    return false;
  }

  // Finished reading replica info
  return true;
}

bool ReplicaStream::WriteIdentificationInfo(bool isAbsent, const Replica* replica)
{
  if(!replica)
    return WriteIdentificationInfo(isAbsent);
  else
    return WriteIdentificationInfo(isAbsent, replica->mReplicaId, replica->IsEmplaced(), replica->mEmplaceContext, replica->mEmplaceId);
}
bool ReplicaStream::WriteIdentificationInfo(bool isAbsent, ReplicaId replicaId, bool isEmplaced, const EmplaceContext& emplaceContext, EmplaceId emplaceId)
{
  // Get item cachers
  Replicator::EmplaceContextCacher& emplaceContextCacher = GetReplicator()->mEmplaceContextCacher;

  //
  // Is Absent Flag
  //

  // Write 'Is Absent?' Flag
  mBitStream.Write(isAbsent);

  // Is absent?
  if(isAbsent)
  {
    // Finished writing replica info
    return true;
  }

  //
  // Replica ID
  //

  // Write replica ID
  Assert(replicaId != 0);
  mBitStream.Write(replicaId);

  // Should continue writing replica info?
  // (Based on replica stream serialization mode)
  switch(GetReplicaStreamMode())
  {
  case ReplicaStreamMode::Spawn:
  case ReplicaStreamMode::Clone:
    // Continue writing replica info
    break;

  case ReplicaStreamMode::Forget:
  case ReplicaStreamMode::Destroy:
  case ReplicaStreamMode::ReverseReplicaChannels:
    // Finished writing replica info
    return true;

  default:
    // Error writing replica info
    Assert(false);
    return false;
  }

  //
  // Is Cloned Flag
  //

  // Is this a clone serialization mode?
  bool isCloneSerialization = (GetReplicaStreamMode() == ReplicaStreamMode::Clone);

  // (We intentionally avoid writing the 'Is Cloned?' Flag because it can easily be determined via the ReplicaStreamMode)

  // Is this a clone serialization mode?
  if(isCloneSerialization)
  {
    //
    // Is Emplaced Flag
    //

    // Write 'Is Emplaced?' Flag
    mBitStream.Write(isEmplaced);
  }
  // Is this a spawn serialization mode?
  else
    Assert(isEmplaced == false);

  // Is emplaced?
  if(isEmplaced)
  {
    //
    // Emplace Context (ID)
    //

    // Write emplace context ID
    Assert(emplaceContextCacher.IsItemMapped(emplaceContext));
    EmplaceContextId emplaceContextId = emplaceContextCacher.GetMappedItemId(emplaceContext);
    mBitStream.Write(emplaceContextId);

    //
    // Emplace ID
    //

    // Write emplace ID
    Assert(emplaceId != 0);
    mBitStream.Write(emplaceId);
  }

  // Finished writing replica info
  return true;
}
bool ReplicaStream::ReadIdentificationInfo(bool& isAbsent, Replica* replica) const
{
  if(!replica)
  {
    bool           isAbsent   = false;
    ReplicaId      replicaId  = 0;
    bool           isCloned   = false;
    bool           isEmplaced = false;
    EmplaceContext emplaceContext;
    EmplaceId      emplaceId  = 0;
    return ReadIdentificationInfo(isAbsent, replicaId, isCloned, isEmplaced, emplaceContext, emplaceId);
  }
  else
  {
    bool isEmplaced = false;
    return ReadIdentificationInfo(isAbsent, replica->mReplicaId, replica->mIsCloned, isEmplaced, replica->mEmplaceContext, replica->mEmplaceId);
  }
}
bool ReplicaStream::ReadIdentificationInfo(bool& isAbsent, ReplicaId& replicaId, bool& isCloned, bool& isEmplaced, EmplaceContext& emplaceContext, EmplaceId& emplaceId) const
{
  // Get item cachers
  Replicator::EmplaceContextCacher& emplaceContextCacher = GetReplicator()->mEmplaceContextCacher;

  //
  // Is Absent Flag
  //

  // Read 'Is Absent?' Flag
  if(!mBitStream.Read(isAbsent)) // Unable?
  {
    // Error reading replica info
    Assert(false);
    return false;
  }

  // Is absent?
  if(isAbsent)
  {
    // Finished reading replica info
    return true;
  }

  //
  // Replica ID
  //

  // Read replica ID
  if(!mBitStream.Read(replicaId)) // Unable?
  {
    // Error reading replica info
    Assert(false);
    return false;
  }
  Assert(replicaId != 0);

  // Should continue reading replica info?
  // (Based on replica stream serialization mode)
  switch(GetReplicaStreamMode())
  {
  case ReplicaStreamMode::Spawn:
  case ReplicaStreamMode::Clone:
    // Continue reading replica info
    break;

  case ReplicaStreamMode::Forget:
  case ReplicaStreamMode::Destroy:
  case ReplicaStreamMode::ReverseReplicaChannels:
    // Finished reading replica info
    return true;

  default:
    // Error reading replica info
    Assert(false);
    return false;
  }

  //
  // Is Cloned Flag
  //

  // Is this a clone serialization mode?
  bool isCloneSerialization = (GetReplicaStreamMode() == ReplicaStreamMode::Clone);

  // Set 'Is Cloned?' Flag based on serialization mode
  isCloned = isCloneSerialization;

  // Is this a clone serialization mode?
  isEmplaced = false;
  if(isCloneSerialization)
  {
    //
    // Is Emplaced Flag
    //

    // Read 'Is Emplaced?' Flag
    if(!mBitStream.Read(isEmplaced)) // Unable?
    {
      // Error reading replica info
      Assert(false);
      return false;
    }
  }

  // Is emplaced?
  if(isEmplaced)
  {
    //
    // Emplace Context (ID)
    //

    // Read emplace context ID
    EmplaceContextId emplaceContextId;
    if(!mBitStream.Read(emplaceContextId)) // Unable?
    {
      // Error reading replica info
      Assert(false);
      return false;
    }
    Assert(emplaceContextId != 0);

    // Unknown emplace context?
    if(!emplaceContextCacher.IsIdMapped(emplaceContextId))
    {
      // Error reading replica info
      Assert(false);
      return false;
    }

    // Get emplace context
    emplaceContext = emplaceContextCacher.GetMappedIdItem(emplaceContextId);
    if(emplaceContext.IsEmpty()) // Unable?
    {
      // Error reading replica info
      Assert(false);
      return false;
    }

    //
    // Emplace ID
    //

    // Read emplace ID
    if(!mBitStream.Read(emplaceId)) // Unable?
    {
      // Error reading replica info
      Assert(false);
      return false;
    }
    Assert(emplaceId != 0);
  }

  // Finished reading replica info
  return true;
}

bool ReplicaStream::WriteChannelData(const Replica* replica)
{
  // Not the reverse replica channels serialization mode?
  if(GetReplicaStreamMode() != ReplicaStreamMode::ReverseReplicaChannels)
  {
    // Is this the replication initialization phase? (We are creating an object?)
    bool isInitializationPhase = (GetReplicaStreamMode() == ReplicaStreamMode::Spawn
                               || GetReplicaStreamMode() == ReplicaStreamMode::Clone);
    if(!isInitializationPhase)
    {
      // (If this is not the initialization phase, it should be the uninitialization phase)
      Assert(GetReplicaStreamMode() == ReplicaStreamMode::Forget
          || GetReplicaStreamMode() == ReplicaStreamMode::Destroy);
    }

    // Is this the replication initialization phase? (We are creating an object?)
    if(isInitializationPhase)
    {
      //
      // Forward Replica-Message-Channels
      //

      // Open and write forward message channels corresponding to our applicable replica channels
      if(!GetReplicatorLink()->OpenAndSerializeForwardReplicaChannels(replica, mBitStream)) // Unable?
      {
        // Error writing replica data
        Assert(false);
        return false;
      }
    }

    //
    // Replica Channel Property Values
    //

    // For all replica channels
    forRange(ReplicaChannel* replicaChannel, replica->GetReplicaChannels().All())
    {
      // Get replica channel type
      ReplicaChannelType* replicaChannelType = replicaChannel->GetReplicaChannelType();

      // Should serialize replica channel?
      // (Based on replica stream serialization mode)
      switch(GetReplicaStreamMode())
      {
      case ReplicaStreamMode::Spawn:
        {
          // Replica channel type not configured to serialize on spawn?
          if(!(replicaChannelType->GetSerializationFlags() & SerializationFlags::OnSpawn))
            continue; // Skip channel
        }
        break;

      case ReplicaStreamMode::Clone:
        {
          // Replica channel type not configured to serialize on clone-from-emplace/clone-from-spawn?
          if(!(replica->IsEmplaced() ? (replicaChannelType->GetSerializationFlags() & SerializationFlags::OnCloneEmplace)
                                     : (replicaChannelType->GetSerializationFlags() & SerializationFlags::OnCloneSpawn)))
            continue; // Skip channel
        }
        break;

      case ReplicaStreamMode::Forget:
        {
          // Replica channel type not configured to serialize on forget?
          if(!(replicaChannelType->GetSerializationFlags() & SerializationFlags::OnForget))
            continue; // Skip channel
        }
        break;

      case ReplicaStreamMode::Destroy:
        {
          // Replica channel type not configured to serialize on destroy?
          if(!(replicaChannelType->GetSerializationFlags() & SerializationFlags::OnDestroy))
            continue; // Skip channel
        }
        break;

      default:
        // Error writing replica data
        Assert(false);
        return false;
      }

      // Write replica channel
      bool result = replicaChannel->Serialize(mBitStream, (isInitializationPhase ? ReplicationPhase::Initialization : ReplicationPhase::Uninitialization), GetTimestamp());
      if(!result) // Unable?
      {
        // Error writing replica data
        Assert(false);
        return false;
      }
    }
  }
  // Is the reverse replica channels serialization mode?
  else
  {
    //
    // Reverse Replica-Message-Channels
    //

    // (Replica should use reverse replica channels)
    Assert(replica->UsesReverseReplicaChannels());

    // Open and write reverse message channels corresponding to our applicable replica channels
    if(!GetReplicatorLink()->OpenAndSerializeReverseReplicaChannels(replica, mBitStream)) // Unable?
    {
      // Error writing replica data
      Assert(false);
      return false;
    }
  }

  // Finished writing replica data
  return true;
}
bool ReplicaStream::ReadChannelData(Replica* replica) const
{
  // Not the reverse replica channels serialization mode?
  if(GetReplicaStreamMode() != ReplicaStreamMode::ReverseReplicaChannels)
  {
    // Is this the replication initialization phase? (We are creating an object?)
    bool isInitializationPhase = (GetReplicaStreamMode() == ReplicaStreamMode::Spawn
                               || GetReplicaStreamMode() == ReplicaStreamMode::Clone);
    if(!isInitializationPhase)
    {
      // (If this is not the initialization phase, it should be the uninitialization phase)
      Assert(GetReplicaStreamMode() == ReplicaStreamMode::Forget
          || GetReplicaStreamMode() == ReplicaStreamMode::Destroy);
    }

    // Is this the replication initialization phase? (We are creating an object?)
    if(isInitializationPhase)
    {
      //
      // Forward Replica-Message-Channels
      //

      // Read and set forward message channels corresponding to our applicable replica channels
      if(!GetReplicatorLink()->DeserializeAndSetForwardReplicaChannels(replica, mBitStream)) // Unable?
      {
        // Error reading replica data
        Assert(false);
        return false;
      }
    }

    //
    // Replica Channel Property Values
    //

    // For all replica channels
    forRange(ReplicaChannel* replicaChannel, replica->GetReplicaChannels().All())
    {
      // Get replica channel type
      ReplicaChannelType* replicaChannelType = replicaChannel->GetReplicaChannelType();

      // Should deserialize replica channel?
      // (Based on replica stream serialization mode)
      switch(GetReplicaStreamMode())
      {
      case ReplicaStreamMode::Spawn:
        {
          // Replica channel type not configured to serialize on spawn?
          if(!(replicaChannelType->GetSerializationFlags() & SerializationFlags::OnSpawn))
            continue; // Skip channel
        }
        break;

      case ReplicaStreamMode::Clone:
        {
          // Replica channel type not configured to serialize on clone-from-emplace/clone-from-spawn?
          if(!(replica->IsEmplaced() ? (replicaChannelType->GetSerializationFlags() & SerializationFlags::OnCloneEmplace)
                                     : (replicaChannelType->GetSerializationFlags() & SerializationFlags::OnCloneSpawn)))
            continue; // Skip channel
        }
        break;

      case ReplicaStreamMode::Forget:
        {
          // Replica channel type not configured to serialize on forget?
          if(!(replicaChannelType->GetSerializationFlags() & SerializationFlags::OnForget))
            continue; // Skip channel
        }
        break;

      case ReplicaStreamMode::Destroy:
        {
          // Replica channel type not configured to serialize on destroy?
          if(!(replicaChannelType->GetSerializationFlags() & SerializationFlags::OnDestroy))
            continue; // Skip channel
        }
        break;

      default:
        // Error reading replica data
        Assert(false);
        return false;
      }

      // Read replica channel
      bool result = replicaChannel->Deserialize(mBitStream, (isInitializationPhase ? ReplicationPhase::Initialization : ReplicationPhase::Uninitialization), GetTimestamp());
      if(!result) // Unable?
      {
        // Error reading replica data
        Assert(false);
        return false;
      }
    }
  }
  // Is the reverse replica channels serialization mode?
  else
  {
    //
    // Reverse Replica-Message-Channels
    //

    // (Replica should use reverse replica channels)
    Assert(replica->UsesReverseReplicaChannels());

    // Read and set reverse message channels corresponding to our applicable replica channels
    if(!GetReplicatorLink()->DeserializeAndSetReverseReplicaChannels(replica, mBitStream)) // Unable?
    {
      // Error reading replica data
      Assert(false);
      return false;
    }
  }

  // Finished reading replica data
  return true;
}

} // namespace Zero
