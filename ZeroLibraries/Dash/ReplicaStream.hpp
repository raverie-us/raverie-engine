///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//---------------------------------------------------------------------------------//
//                               ReplicaStream                                     //
//---------------------------------------------------------------------------------//

/// Replica Stream
/// Facilitates replica serialization
class ReplicaStream
{
public:
  /// Constructor
  ReplicaStream(Replicator* replicator, ReplicatorLink* replicatorLink, BitStream& bitStream, ReplicaStreamMode::Enum replicaStreamMode, TimeMs timestamp);
  ReplicaStream(Replicator* replicator, ReplicatorLink* replicatorLink, const BitStream& bitStream, ReplicaStreamMode::Enum replicaStreamMode, TimeMs timestamp);

  //
  // Operations
  //

  /// Returns the operating replicator
  Replicator* GetReplicator() const;

  /// Returns the operating replicator link
  ReplicatorLink* GetReplicatorLink() const;

  /// Returns the operating bitstream
  BitStream& GetBitStream() const;

  /// Returns the replica stream's serialization mode
  ReplicaStreamMode::Enum GetReplicaStreamMode() const;

  /// Returns the replica stream's message timestamp
  TimeMs GetTimestamp() const;

  /// Writes replica creation information (such as CreateContext and ReplicaType) to the replica stream
  /// Returns true if successful, else false
  bool WriteCreationInfo(const Replica* replica);
  bool WriteCreationInfo(const CreateContext& createContext = CreateContext(), const ReplicaType& replicaType = ReplicaType());
  /// Reads replica creation information (such as CreateContext and ReplicaType) from the replica stream
  /// Returns true if successful, else false
  bool ReadCreationInfo(Replica* replica) const;
  bool ReadCreationInfo(CreateContext& createContext, ReplicaType& replicaType) const;

  /// Writes replica identification information (such as IsAbsent, ReplicaId, IsCloned, IsEmplaced, EmplaceContext, and EmplaceId) to the replica stream
  /// Returns true if successful, else false
  bool WriteIdentificationInfo(bool isAbsent, const Replica* replica);
  bool WriteIdentificationInfo(bool isAbsent, ReplicaId replicaId = 0, bool isEmplaced = false, const EmplaceContext& emplaceContext = EmplaceContext(), EmplaceId emplaceId = 0);
  /// Reads replica identification information (such as IsAbsent, ReplicaId, IsCloned, IsEmplaced, EmplaceContext, and EmplaceId) from the replica stream
  /// Returns true if successful, else false
  bool ReadIdentificationInfo(bool& isAbsent, Replica* replica) const;
  bool ReadIdentificationInfo(bool& isAbsent, ReplicaId& replicaId, bool& isCloned, bool& isEmplaced, EmplaceContext& emplaceContext, EmplaceId& emplaceId) const;

  /// Writes replica message channel data (such as forward and reverse ReplicaChannels) to the replica stream
  /// Returns true if successful, else false
  bool WriteChannelData(const Replica* replica);
  /// Reads replica message channel data (such as forward and reverse ReplicaChannels) from the replica stream
  /// Returns true if successful, else false
  bool ReadChannelData(Replica* replica) const;

private:
  //
  // Internal
  //

  /// Data
  Replicator*             mReplicator;        /// Operating replicator
  ReplicatorLink*         mReplicatorLink;    /// Operating replicator link
  BitStream&              mBitStream;         /// Operating bitstream
  ReplicaStreamMode::Enum mReplicaStreamMode; /// Replica stream serialization mode
  TimeMs                  mTimestamp;         /// Message timestamp

  /// No copy constructor
  ReplicaStream(const ReplicaStream&);
  /// No copy assignment operator
  ReplicaStream& operator=(const ReplicaStream&);
};

} // namespace Zero
