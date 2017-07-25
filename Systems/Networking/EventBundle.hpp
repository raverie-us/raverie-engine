///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Typedefs.
typedef Array< HandleOf<Event> > EventArray;

//---------------------------------------------------------------------------------//
//                                  EventRange                                     //
//---------------------------------------------------------------------------------//

/// Event Range.
typedef EventArray::range EventRange;

//---------------------------------------------------------------------------------//
//                           EventBundleMetaComposition                            //
//---------------------------------------------------------------------------------//

/// Event Bundle Meta Composition.
class EventBundleMetaComposition : public MetaComposition
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  EventBundleMetaComposition();

  //
  // MetaComposition Interface
  //

  /// "Component" Instance Management
  uint GetComponentCount(HandleParam instance) override;
  Handle GetComponentAt(HandleParam instance, uint index) override;
  Handle GetComponent(HandleParam instance, BoundType* boundType) override;
  uint GetComponentIndex(HandleParam instance, BoundType* boundType) override;
  void AddComponent(HandleParam instance, HandleParam subObject, int index, bool ignoreDependencies) override;
  void RemoveComponent(HandleParam instance, HandleParam subObject, bool ignoreDependencies) override;
};

//---------------------------------------------------------------------------------//
//                                 EventBundle                                     //
//---------------------------------------------------------------------------------//

/// Event Bundle.
/// Serialized event storage container.
class EventBundle : public Object
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructors.
  EventBundle();
  EventBundle(Event* event);
  EventBundle(GameSession* gameSession);
  EventBundle(GameSession* gameSession, Event* event);
  EventBundle(const EventBundle& rhs);

  /// Destructor.
  ~EventBundle();

  /// Assignment Operators.
  EventBundle& operator =(const EventBundle& rhs);
  EventBundle& operator =(const BitStream& rhs);
  EventBundle& operator =(MoveReference<BitStream> rhs);

  //
  // Interface
  //

  /// Sets the game session.
  void SetGameSession(GameSession* gameSession);
  /// Returns the game session.
  GameSession* GetGameSession();

  /// Returns true if the event bundle is empty (doesn't contain any events), else false.
  bool IsEmpty();

  /// Adds the event to back of the event bundle.
  /// Returns true if successful, else false (an event of that type has already been added).
  bool AddEvent(Event* event);

  /// Returns the event specified if it has been added to the event bundle, else nullptr.
  Event* GetEventByTypeName(StringParam eventTypeName);
  Event* GetEventByType(BoundType* eventType);
  Event* GetEventByIndex(uint index);

  /// Returns the event index specified if it has been added to the event bundle, else nullptr.
  uint GetEventIndexByType(BoundType* eventType);

  /// Returns all the events that have been added to the event bundle.
  EventRange GetEvents();
  /// Returns the number of events that have been added to the event bundle.
  uint GetEventCount();

  /// Removes the event specified from the event bundle.
  /// Returns true if successful, else false (an event of that type has already been added).
  bool RemoveEvent(Event* event);
  bool RemoveEventByTypeName(StringParam eventTypeName);
  bool RemoveEventByType(BoundType* eventType);
  bool RemoveEventByIndex(uint index);

  /// Clears the event bundle.
  void Clear();

  /// Returns the event bundle as bitstream.
  BitStream& GetBitStream();

private:
  //
  // Internal
  //

  /// Serialize our events to our bitstream.
  /// Returns true if successful, else false.
  bool SerializeEventsToBitStream();
  /// Deserialize our bitstream to our events.
  /// Returns true if successful, else false.
  bool DeserializeBitStreamToEvents();

  // Data
  GameSession*      mGameSession;       ///< Operating game session (needed to create events).
  BitStreamExtended mBitStream;         ///< Serialized event objects.
  EventArray        mEvents;            ///< Deserialized event objects (Note: We own the memory for these events).
  bool              mNeedToSerialize;   ///< We need to serialize our modified events to update our bitstream.
  bool              mNeedToDeserialize; ///< We need to deserialize our modified bitstream to update our events.
};

} // namespace Zero
