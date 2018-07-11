///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2016, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

#define InitializePropertyFilterForType(typeName) \
  ZilchInitializeType(PropertyFilter##typeName)

// Ranges
ZilchDefineRange(EventRange);
ZilchDefineRange(NetUserRange);
ZilchDefineRange(NetHostRange);
ZilchDefineRange(WebServerHeaderRange);

// Enums
ZilchDefineEnum(TcpSocketBind);
ZilchDefineEnum(NetUserAddResponse);
ZilchDefineEnum(Network);
ZilchDefineEnum(NetRefreshResult);
ZilchDefineEnum(Role);
ZilchDefineEnum(Authority);
ZilchDefineEnum(AuthorityMode);
ZilchDefineEnum(DetectionMode);
ZilchDefineEnum(ReliabilityMode);
ZilchDefineEnum(SerializationMode);
ZilchDefineEnum(RouteMode);
ZilchDefineEnum(ReplicationPhase);
ZilchDefineEnum(ConvergenceState);
ZilchDefineEnum(BasicNetType);
ZilchDefineEnum(TransportProtocol);
ZilchDefineEnum(ConnectResponseMode);
ZilchDefineEnum(TransmissionDirection);
ZilchDefineEnum(LinkStatus);
ZilchDefineEnum(LinkState);
ZilchDefineEnum(ConnectResponse);
ZilchDefineEnum(DisconnectReason);
ZilchDefineEnum(UserConnectResponse);
ZilchDefineEnum(TransferMode);
ZilchDefineEnum(Receipt);
ZilchDefineEnum(WebServerRequestMethod);

ZilchDefineExternalBaseType(Os::WebResponseCode::Enum, TypeCopyMode::ValueType, builder, type)
{
  ZilchFullBindEnum(builder, type, SpecialType::Enumeration);
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::Invalid, "Invalid");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::NoServerResponse, "NoServerResponse");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::Continue, "Continue");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::SwitchingProtocols, "SwitchingProtocols");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::OK, "OK");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::Created, "Created");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::Accepted, "Accepted");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::NonauthoritativeInformation, "NonauthoritativeInformation");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::NoContent, "NoContent");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::ResetContent, "ResetContent");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::PartialContent, "PartialContent");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::MovedPermanently, "MovedPermanently");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::ObjectMovedTemporarily, "ObjectMovedTemporarily");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::SeeOther, "SeeOther");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::NotModified, "NotModified");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::TemporaryRedirect, "TemporaryRedirect");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::PermanentRedirect, "PermanentRedirect");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::BadRequest, "BadRequest");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::AccessDenied, "AccessDenied");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::Forbidden, "Forbidden");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::NotFound, "NotFound");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::HTTPVerbNotAllowed, "HTTPVerbNotAllowed");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::ClientBrowserRejectsMIME, "ClientBrowserRejectsMIME");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::ProxyAuthenticationRequired, "ProxyAuthenticationRequired");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::PreconditionFailed, "PreconditionFailed");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::RequestEntityTooLarge, "RequestEntityTooLarge");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::RequestURITooLarge, "RequestURITooLarge");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::UnsupportedMediaType, "UnsupportedMediaType");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::RequestedRangeNotSatisfiable, "RequestedRangeNotSatisfiable");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::ExecutionFailed, "ExecutionFailed");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::LockedError, "LockedError");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::InternalServerError, "InternalServerError");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::UnimplementedHeaderValueUsed, "UnimplementedHeaderValueUsed");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::GatewayProxyReceivedInvalid, "GatewayProxyReceivedInvalid");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::ServiceUnavailable, "ServiceUnavailable");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::GatewayTimedOut, "GatewayTimedOut");
  ZilchFullBindEnumValue(builder, type, Os::WebResponseCode::HTTPVersionNotSupported, "HTTPVersionNotSupported");
}

// Arrays
ZeroDefineArrayType(NetPropertyInfoArray);

//**************************************************************************************************
ZilchDefineStaticLibrary(NetworkingLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  ZilchInitializeRange(EventRange);
  ZilchInitializeRange(NetUserRange);
  ZilchInitializeRange(NetHostRange);
  ZilchInitializeRange(WebServerHeaderRange);

  // Enums
  ZilchInitializeEnum(TcpSocketBind);
  ZilchInitializeEnum(NetUserAddResponse);
  ZilchInitializeEnum(Network);
  ZilchInitializeEnum(NetRefreshResult);
  ZilchInitializeEnumAs(Role, "NetRole");
  ZilchInitializeEnum(Authority);
  ZilchInitializeEnum(AuthorityMode);
  ZilchInitializeEnum(DetectionMode);
  ZilchInitializeEnum(ReliabilityMode);
  ZilchInitializeEnum(SerializationMode);
  ZilchInitializeEnum(RouteMode);
  ZilchInitializeEnum(ReplicationPhase);
  ZilchInitializeEnum(ConvergenceState);
  ZilchInitializeEnum(BasicNetType);
  ZilchInitializeEnum(TransportProtocol);
  ZilchInitializeEnum(ConnectResponseMode);
  ZilchInitializeEnum(TransmissionDirection);
  ZilchInitializeEnum(LinkStatus);
  ZilchInitializeEnum(LinkState);
  ZilchInitializeEnum(ConnectResponse);
  ZilchInitializeEnum(DisconnectReason);
  ZilchInitializeEnum(UserConnectResponse);
  ZilchInitializeEnum(TransferMode);
  ZilchInitializeEnum(Receipt);
  ZilchInitializeEnum(WebServerRequestMethod);
  ZilchInitializeEnum(Os::WebResponseCode);

  // Meta Arrays
  ZeroInitializeArrayTypeAs(NetPropertyInfoArray, "NetPropertyInfos");

  // Events
  ZilchInitializeType(ConnectionEvent);
  ZilchInitializeType(ReceivedDataEvent);
  ZilchInitializeType(SendableEvent);
  ZilchInitializeType(WebResponseEvent);
  ZilchInitializeType(WebServerRequestEvent);
  ZilchInitializeType(AcquireNetHostInfo);
  ZilchInitializeType(NetHostUpdate);
  ZilchInitializeType(NetHostListUpdate);
  ZilchInitializeType(NetPeerOpened);
  ZilchInitializeType(NetPeerClosed);
  ZilchInitializeType(NetGameStarted);
  ZilchInitializeType(NetPeerSentConnectRequest);
  ZilchInitializeType(NetPeerReceivedConnectRequest);
  ZilchInitializeType(NetPeerSentConnectResponse);
  ZilchInitializeType(NetPeerReceivedConnectResponse);
  ZilchInitializeType(NetLinkConnected);
  ZilchInitializeType(NetLinkDisconnected);
  ZilchInitializeType(NetLevelStarted);
  ZilchInitializeType(NetPeerSentUserAddRequest);
  ZilchInitializeType(NetPeerReceivedUserAddRequest);
  ZilchInitializeType(NetPeerSentUserAddResponse);
  ZilchInitializeType(NetPeerReceivedUserAddResponse);
  ZilchInitializeType(NetUserLostObjectOwnership);
  ZilchInitializeType(NetUserAcquiredObjectOwnership);
  ZilchInitializeType(RegisterCppNetProperties);
  ZilchInitializeType(NetObjectOnline);
  ZilchInitializeType(NetObjectOffline);
  ZilchInitializeType(NetUserOwnerChanged);
  ZilchInitializeType(NetChannelPropertyChange);
  ZilchInitializeType(NetEventSent);
  ZilchInitializeType(NetEventReceived);
  ZilchInitializeType(NetHostRecordEvent);

  // Meta Components
  ZilchInitializeType(EventBundleMetaComposition);
  ZilchInitializeType(PropertyFilterMultiPrimitiveTypes);
  ZilchInitializeType(PropertyFilterFloatingPointTypes);
  ZilchInitializeType(PropertyFilterArithmeticTypes);
  ZilchInitializeType(EditInGameFilter);
  ZilchInitializeType(MetaNetProperty);

  // Net property filters by type
  InitializePropertyFilterForType(Other);
  InitializePropertyFilterForType(Boolean);
  InitializePropertyFilterForType(Integer);
  InitializePropertyFilterForType(DoubleInteger);
  InitializePropertyFilterForType(Integer2);
  InitializePropertyFilterForType(Integer3);
  InitializePropertyFilterForType(Integer4);
  InitializePropertyFilterForType(Real);
  InitializePropertyFilterForType(DoubleReal);
  InitializePropertyFilterForType(Real2);
  InitializePropertyFilterForType(Real3);
  InitializePropertyFilterForType(Real4);
  InitializePropertyFilterForType(Quaternion);
  InitializePropertyFilterForType(String);

  // Other Networking Type Initialization
  ZilchInitializeType(TcpSocket);
  ZilchInitializeType(ThreadedWebRequest);
  ZilchInitializeType(BlockingWebRequest);
  ZilchInitializeType(ConnectionData);
  ZilchInitializeType(WebServer);

  // NetPeer Type Initialization
  ZilchInitializeTypeAs(BitStreamExtended, "BitStream");
  ZilchInitializeType(EventBundle);
  ZilchInitializeType(NetPropertyInfo);
  ZilchInitializeType(NetPropertyConfig);
  ZilchInitializeType(NetPropertyType);
  ZilchInitializeType(NetProperty);
  ZilchInitializeType(NetChannelConfig);
  ZilchInitializeType(NetChannelType);
  ZilchInitializeType(NetChannel);
  ZilchInitializeType(NetHost);
  ZilchInitializeType(NetHostRecord);
  ZilchInitializeType(NetObject);
  ZilchInitializeType(NetSpace);
  ZilchInitializeType(NetUser);
  ZilchInitializeType(NetPeer);
  ZilchInitializeType(SimpleSocket);
  ZilchInitializeType(WebRequester);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

//**************************************************************************************************
void NetworkingLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  // Resource Managers
  InitializeResourceManager(NetChannelConfigManager);
  InitializeResourceManager(NetPropertyConfigManager);

  RegisterPropertyAttributeType(PropertyAttributes::cNetProperty, MetaNetProperty);
  RegisterPropertyAttribute(PropertyAttributes::cNetPeerId)->TypeMustBe(int);
}

//**************************************************************************************************
void NetworkingLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Zero
