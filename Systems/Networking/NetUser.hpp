///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                   NetUser                                       //
//---------------------------------------------------------------------------------//

/// Network User.
/// Manages the replication of a single negotiated user on the network.
class NetUser : public NetObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetUser();

  /// Comparison Operators (compares network user IDs).
  bool operator ==(const NetUser& rhs) const;
  bool operator !=(const NetUser& rhs) const;
  bool operator  <(const NetUser& rhs) const;
  bool operator ==(const NetUserId& rhs) const;
  bool operator !=(const NetUserId& rhs) const;
  bool operator  <(const NetUserId& rhs) const;

  //
  // Component Interface
  //

  /// Initializes the component.
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;

  /// Adds C++ component net properties to this net object.
  void OnRegisterCppNetProperties(RegisterCppNetProperties* event);

  /// Uninitializes the component.
  void OnDestroy(uint flags) override;

  //
  // User Interface
  //

  /// Returns true if the user was added by our local peer, else false.
  bool AddedByMyPeer() const;
  /// Returns true if the user was added by the specified peer, else false.
  bool AddedByPeer(NetPeerId netPeerId) const;

  //
  // Object Interface
  //

  /// Handles behavior when the net object is brought online, dispatches events accordingly.
  const String& GetNetObjectOnlineEventId() const override;
  void HandleNetObjectOnlinePreDispatch(NetObjectOnline* event) override;
  /// Handles behavior when the net object is taken offline, dispatches events accordingly.
  const String& GetNetObjectOfflineEventId() const override;
  void HandleNetObjectOfflinePostDispatch(NetObjectOffline* event) override;

  //
  // Ownership Interface
  //

  /// Finds a net object with the given name owned by this user in the specified space, else nullptr.
  Cog* FindOwnedNetObjectByNameInSpace(StringParam name, Space* space) const;
  /// Finds a net object with the given name owned by this user in any space, else nullptr.
  Cog* FindOwnedNetObjectByName(StringParam name) const;
  /// Returns all net objects owned by this user in all spaces.
  CogHashSetRange GetOwnedNetObjects() const;
  /// Returns the number of net objects owned by this user in all spaces.
  uint GetOwnedNetObjectCount() const;

  /// [Server/Offline] Releases ownership of all net objects owned by this user in all spaces.
  void ReleaseOwnedNetObjects();

  // Data
  NetPeerId   mNetPeerId;       ///< Adding network peer identifier.
  NetUserId   mNetUserId;       ///< Network user identifier.
  CogHashSet  mOwnedNetObjects; ///< Owned network objects.
  EventBundle mRequestBundle;   ///< [Server/Offline] Bundled request event data.
  EventBundle mResponseBundle;  ///< [Server/Offline] Bundled response event data.
};

//---------------------------------------------------------------------------------//
//                               NetUserSortPolicy                                 //
//---------------------------------------------------------------------------------//

/// Defines the sorting policy for NetUser containers.
struct NetUserSortPolicy : public SortPolicy<CogId>
{
  typedef SortPolicy<CogId> base_type;
  typedef base_type::value_type value_type;

  /// Dereferences to NetUser component before comparing with another value.
  template<typename CompareType>
  bool operator()(const value_type& lhs, const CompareType& rhs) const // Intended for: CogId < NetUserId
  {
    // Get net user
    NetUser* lhsNetUser = static_cast<Cog*>(lhs)->has(NetUser);
    if(!lhsNetUser)
    {
      DoNotifyException("NetUserSortPolicy", "Unable to compare Cogs - Missing NetUser component");
      return false;
    }

    // Compare
    return *lhsNetUser < rhs;
  }
  bool operator()(const value_type& lhs, const value_type& rhs) const // Intended for: CogId < CogId
  {
    // Get net users
    NetUser* lhsNetUser = static_cast<Cog*>(lhs)->has(NetUser);
    NetUser* rhsNetUser = static_cast<Cog*>(rhs)->has(NetUser);
    if(!lhsNetUser || !rhsNetUser)
    {
      DoNotifyException("NetUserSortPolicy", "Unable to compare Cogs - Missing NetUser component");
      return false;
    }

    // Compare
    return *lhsNetUser < *rhsNetUser;
  }
  bool operator()(const value_type& lhs, const Cog*& rhs) const // Intended for: CogId < Cog*
  {
    // Get net users
    NetUser* lhsNetUser = static_cast<Cog*>(lhs)->has(NetUser);
    NetUser* rhsNetUser = const_cast<Cog*>(rhs)->has(NetUser);
    if(!lhsNetUser || !rhsNetUser)
    {
      DoNotifyException("NetUserSortPolicy", "Unable to compare Cogs - Missing NetUser component");
      return false;
    }

    // Compare
    return *lhsNetUser < *rhsNetUser;
  }

  template<typename CompareType>
  bool Equal(const value_type& lhs, const CompareType& rhs) const // Intended for: CogId == NetUserId
  {
    // Get net user
    NetUser* lhsNetUser = static_cast<Cog*>(lhs)->has(NetUser);
    if(!lhsNetUser)
    {
      DoNotifyException("NetUserSortPolicy", "Unable to compare Cogs - Missing NetUser component");
      return false;
    }

    // Compare
    return *lhsNetUser == rhs;
  }
  bool Equal(const value_type& lhs, const value_type& rhs) const // Intended for: CogId == CogId
  {
    // Get net users
    NetUser* lhsNetUser = static_cast<Cog*>(lhs)->has(NetUser);
    NetUser* rhsNetUser = static_cast<Cog*>(rhs)->has(NetUser);
    if(!lhsNetUser || !rhsNetUser)
    {
      DoNotifyException("NetUserSortPolicy", "Unable to compare Cogs - Missing NetUser component");
      return false;
    }

    // Compare
    return *lhsNetUser == *rhsNetUser;
  }
  bool Equal(const value_type& lhs, const Cog*& rhs) const // Intended for: CogId == Cog*
  {
    // Get net users
    NetUser* lhsNetUser = static_cast<Cog*>(lhs)->has(NetUser);
    NetUser* rhsNetUser = const_cast<Cog*>(rhs)->has(NetUser);
    if(!lhsNetUser || !rhsNetUser)
    {
      DoNotifyException("NetUserSortPolicy", "Unable to compare Cogs - Missing NetUser component");
      return false;
    }

    // Compare
    return *lhsNetUser == *rhsNetUser;
  }
};

/// Typedefs.
typedef ArraySet<CogId, NetUserSortPolicy> NetUserSet;

//---------------------------------------------------------------------------------//
//                                 NetUserRange                                    //
//---------------------------------------------------------------------------------//

/// Network User Range.
struct NetUserRange : public NetUserSet::range
{
  /// Typedefs.
  typedef Cog* FrontResult;

  /// Constructors.
  NetUserRange();
  NetUserRange(const NetUserSet::range& rhs);
};

//---------------------------------------------------------------------------------//
//                                PendingNetUser                                   //
//---------------------------------------------------------------------------------//

/// Pending outgoing net user request.
struct PendingNetUser
{
  /// Constructor.
  PendingNetUser();
  PendingNetUser(GameSession* gameSession);

  // Data
  EventBundle mOurRequestBundle; ///< Our bundled request event data.
};

/// Typedefs.
typedef Array<PendingNetUser> PendingNetUserArray;

} // namespace Zero
