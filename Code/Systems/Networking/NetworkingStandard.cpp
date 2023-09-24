// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

#define InitializePropertyFilterForType(typeName) RaverieInitializeType(PropertyFilter##typeName)

// Ranges
RaverieDefineRange(EventRange);
RaverieDefineRange(NetUserRange);
RaverieDefineRange(NetHostRange);

// Enums
RaverieDefineEnum(NetUserAddResponse);
RaverieDefineEnum(Network);
RaverieDefineEnum(NetRefreshResult);
RaverieDefineEnum(Role);
RaverieDefineEnum(Authority);
RaverieDefineEnum(AuthorityMode);
RaverieDefineEnum(DetectionMode);
RaverieDefineEnum(ReliabilityMode);
RaverieDefineEnum(SerializationMode);
RaverieDefineEnum(RouteMode);
RaverieDefineEnum(ReplicationPhase);
RaverieDefineEnum(ConvergenceState);
RaverieDefineEnum(BasicNetType);
RaverieDefineEnum(TransportProtocol);
RaverieDefineEnum(ConnectResponseMode);
RaverieDefineEnum(TransmissionDirection);
RaverieDefineEnum(LinkStatus);
RaverieDefineEnum(LinkState);
RaverieDefineEnum(ConnectResponse);
RaverieDefineEnum(DisconnectReason);
RaverieDefineEnum(UserConnectResponse);
RaverieDefineEnum(TransferMode);
RaverieDefineEnum(Receipt);

RaverieDefineExternalBaseType(WebResponseCode::Enum, TypeCopyMode::ValueType, builder, type)
{
  RaverieFullBindEnum(builder, type, SpecialType::Enumeration);
  RaverieFullBindEnumValue(builder, type, WebResponseCode::Invalid, "Invalid");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::NoServerResponse, "NoServerResponse");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::Continue, "Continue");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::SwitchingProtocols, "SwitchingProtocols");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::OK, "OK");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::Created, "Created");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::Accepted, "Accepted");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::NonauthoritativeInformation, "NonauthoritativeInformation");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::NoContent, "NoContent");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::ResetContent, "ResetContent");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::PartialContent, "PartialContent");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::MovedPermanently, "MovedPermanently");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::ObjectMovedTemporarily, "ObjectMovedTemporarily");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::SeeOther, "SeeOther");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::NotModified, "NotModified");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::TemporaryRedirect, "TemporaryRedirect");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::PermanentRedirect, "PermanentRedirect");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::BadRequest, "BadRequest");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::AccessDenied, "AccessDenied");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::Forbidden, "Forbidden");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::NotFound, "NotFound");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::HTTPVerbNotAllowed, "HTTPVerbNotAllowed");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::ClientBrowserRejectsMIME, "ClientBrowserRejectsMIME");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::ProxyAuthenticationRequired, "ProxyAuthenticationRequired");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::PreconditionFailed, "PreconditionFailed");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::RequestEntityTooLarge, "RequestEntityTooLarge");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::RequestURITooLarge, "RequestURITooLarge");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::UnsupportedMediaType, "UnsupportedMediaType");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::RequestedRangeNotSatisfiable, "RequestedRangeNotSatisfiable");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::ExecutionFailed, "ExecutionFailed");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::LockedError, "LockedError");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::InternalServerError, "InternalServerError");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::UnimplementedHeaderValueUsed, "UnimplementedHeaderValueUsed");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::GatewayProxyReceivedInvalid, "GatewayProxyReceivedInvalid");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::ServiceUnavailable, "ServiceUnavailable");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::GatewayTimedOut, "GatewayTimedOut");
  RaverieFullBindEnumValue(builder, type, WebResponseCode::HTTPVersionNotSupported, "HTTPVersionNotSupported");
}

// Arrays
RaverieDefineArrayType(NetPropertyInfoArray);

RaverieDefineStaticLibrary(NetworkingLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  RaverieInitializeRange(EventRange);
  RaverieInitializeRange(NetUserRange);
  RaverieInitializeRange(NetHostRange);

  // Enums
  RaverieInitializeEnum(NetUserAddResponse);
  RaverieInitializeEnum(Network);
  RaverieInitializeEnum(NetRefreshResult);
  RaverieInitializeEnumAs(Role, "NetRole");
  RaverieInitializeEnum(Authority);
  RaverieInitializeEnum(AuthorityMode);
  RaverieInitializeEnum(DetectionMode);
  RaverieInitializeEnum(ReliabilityMode);
  RaverieInitializeEnum(SerializationMode);
  RaverieInitializeEnum(RouteMode);
  RaverieInitializeEnum(ReplicationPhase);
  RaverieInitializeEnum(ConvergenceState);
  RaverieInitializeEnum(BasicNetType);
  RaverieInitializeEnum(TransportProtocol);
  RaverieInitializeEnum(ConnectResponseMode);
  RaverieInitializeEnum(TransmissionDirection);
  RaverieInitializeEnum(LinkStatus);
  RaverieInitializeEnum(LinkState);
  RaverieInitializeEnum(ConnectResponse);
  RaverieInitializeEnum(DisconnectReason);
  RaverieInitializeEnum(UserConnectResponse);
  RaverieInitializeEnum(TransferMode);
  RaverieInitializeEnum(Receipt);
  RaverieInitializeEnum(WebResponseCode);

  // Meta Arrays
  RaverieInitializeArrayTypeAs(NetPropertyInfoArray, "NetPropertyInfos");

  // Events
  RaverieInitializeType(WebResponseEvent);
  RaverieInitializeType(AcquireNetHostInfo);
  RaverieInitializeType(NetHostUpdate);
  RaverieInitializeType(NetHostListUpdate);
  RaverieInitializeType(NetPeerOpened);
  RaverieInitializeType(NetPeerClosed);
  RaverieInitializeType(NetGameStarted);
  RaverieInitializeType(NetPeerSentConnectRequest);
  RaverieInitializeType(NetPeerReceivedConnectRequest);
  RaverieInitializeType(NetPeerSentConnectResponse);
  RaverieInitializeType(NetPeerReceivedConnectResponse);
  RaverieInitializeType(NetLinkConnected);
  RaverieInitializeType(NetLinkDisconnected);
  RaverieInitializeType(NetLevelStarted);
  RaverieInitializeType(NetPeerSentUserAddRequest);
  RaverieInitializeType(NetPeerReceivedUserAddRequest);
  RaverieInitializeType(NetPeerSentUserAddResponse);
  RaverieInitializeType(NetPeerReceivedUserAddResponse);
  RaverieInitializeType(NetUserLostObjectOwnership);
  RaverieInitializeType(NetUserAcquiredObjectOwnership);
  RaverieInitializeType(RegisterCppNetProperties);
  RaverieInitializeType(NetObjectOnline);
  RaverieInitializeType(NetObjectOffline);
  RaverieInitializeType(NetUserOwnerChanged);
  RaverieInitializeType(NetChannelPropertyChange);
  RaverieInitializeType(NetEventSent);
  RaverieInitializeType(NetEventReceived);
  RaverieInitializeType(NetHostRecordEvent);

  // Meta Components
  RaverieInitializeType(EventBundleMetaComposition);
  RaverieInitializeType(PropertyFilterMultiPrimitiveTypes);
  RaverieInitializeType(PropertyFilterFloatingPointTypes);
  RaverieInitializeType(PropertyFilterArithmeticTypes);
  RaverieInitializeType(EditInGameFilter);
  RaverieInitializeType(MetaNetProperty);

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
  RaverieInitializeType(AsyncWebRequest);
  RaverieInitializeType(WebRequester);

  // NetPeer Type Initialization
  RaverieInitializeTypeAs(BitStreamExtended, "BitStream");
  RaverieInitializeType(EventBundle);
  RaverieInitializeType(NetPropertyInfo);
  RaverieInitializeType(NetPropertyConfig);
  RaverieInitializeType(NetPropertyType);
  RaverieInitializeType(NetProperty);
  RaverieInitializeType(NetChannelConfig);
  RaverieInitializeType(NetChannelType);
  RaverieInitializeType(NetChannel);
  RaverieInitializeType(NetHost);
  RaverieInitializeType(NetHostRecord);
  RaverieInitializeType(NetObject);
  RaverieInitializeType(NetSpace);
  RaverieInitializeType(NetUser);
  RaverieInitializeType(NetPeer);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

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

void NetworkingLibrary::Shutdown()
{
  AsyncWebRequest::CancelAllActiveRequests();

  GetLibrary()->ClearComponents();
}

} // namespace Raverie
