// MIT Licensed (see LICENSE.md).
#pragma once
#include "Systems/Engine/EngineEvents.hpp"

namespace Raverie
{

// Support Functions

// Sanitizes the name of the content file to be compatible with cog, archetype,
// and resource names
String SanitizeContentFilename(StringParam filename);

DeclareEnum2(ImageImport, Textures, Sprites);

class ImageOptions : public Object
{
public:
  RaverieDeclareType(ImageOptions, TypeCopyMode::ReferenceType);

  ImageOptions(ImportOptions* owner);

  ImportOptions* mOwner;
  ImageImport::Enum mImportImages;
};

DeclareEnum3(MeshImport, NoMesh, SingleMesh, MultiMesh);
DeclareEnum3(PhysicsImport, NoMesh, StaticMesh, ConvexMesh);
DeclareEnum7(ScaleConversion,
             Custom,
             CentimeterToInches,
             CentimeterToMeter,
             InchesToCentimenters,
             InchesToMeters,
             MetersToCentimeters,
             MetersToInches);
DeclareEnum6(BasisType, PositiveX, PositiveY, PositiveZ, NegativeX, NegativeY, NegativeZ);

const float cCentimetersToInchesScaleFactor = 0.3937f;
const float cCentimetersToMetersScaleFactor = 0.01f;
const float cInchesToCentimetersScaleFactor = 2.54f;
const float cInchesToMetersScaleFactor = 0.0254f;
const float cMetersToCentimetersScaleFactor = 100.f;
const float cMetersToInchesScaleFactor = 39.37f;

class GeometryOptions : public Object
{
public:
  RaverieDeclareType(GeometryOptions, TypeCopyMode::ReferenceType);

  GeometryOptions(ImportOptions* owner);
  ImportOptions* mOwner;

  bool mImportMeshes;
  // sub mesh import options only visible if importing a mesh
  bool mCombineMeshes;
  // overwrites imported normals with generated ones, generates normals if none
  // are present either way
  bool mGenerateSmoothNormals;
  // the angle at or under which two normals are averaged together
  float mSmoothingAngleDegreesThreshold;
  // overwrites imported tangents and bitangents with generated ones, generates
  // tangents and bitangents if none are present either way
  bool mGenerateTangentSpace;
  // flips the y-axis and adjusts tangent space as needed
  bool mInvertUvYAxis;
  bool mFlipWindingOrder;
  PhysicsImport::Enum mPhysicsImport;
  // end sub mesh import options

  /// Warning: When collapsing pivots on models non-embedded animations may not
  /// remain compatible
  bool mCollapsePivots;
  bool mImportAnimations;
  bool mCreateArchetype;
  bool mImportTextures;

  Vec3 mOriginOffset;
  ScaleConversion::Enum mScaleConversion;
  // scale factors only writable when set to custom scaling
  float mScaleFactor;
  bool mChangeBasis;
  // basis options only visible if change of basis is true
  BasisType::Enum mXBasisTo;
  BasisType::Enum mYBasisTo;
  BasisType::Enum mZBasisTo;
};

// ShowNormalGenerationOptionsFilter
class ShowNormalGenerationOptionsFilter : public MetaPropertyFilter
{
public:
  RaverieDeclareType(ShowNormalGenerationOptionsFilter, TypeCopyMode::ReferenceType);
  bool Filter(Member* prop, HandleParam instance) override;
};

DeclareEnum3(AudioCueImport, None, PerSound, Grouped);
class AudioOptions : public Object
{
public:
  RaverieDeclareType(AudioOptions, TypeCopyMode::ReferenceType);

  AudioOptions(ImportOptions* owner);

  AudioCueImport::Enum mGenerateCue;
  String mGroupCueName;
  AudioFileLoadType::Enum mStreamingMode;
  ImportOptions* mOwner;
};

DeclareEnum2(ConflictAction, Skip, Replace);
class ConflictOptions : public Object
{
public:
  RaverieDeclareType(ConflictOptions, TypeCopyMode::ReferenceType);

  ConflictOptions(ImportOptions* owner);

  void SetAction(ConflictAction::Enum action);
  ConflictAction::Enum GetAction();

  ConflictAction::Enum mAction;
  ImportOptions* mOwner;
};

namespace Events
{
DeclareEvent(ImportOptionsModified);
}

class ImportOptions : public EventObject
{
public:
  RaverieDeclareType(ImportOptions, TypeCopyMode::ReferenceType);

  ImportOptions();
  ~ImportOptions();

  void Initialize(Array<String>& files, ContentLibrary* library);
  void BuildOptions();
  bool ShouldAutoImport();

  ImageOptions* mImageOptions;
  GeometryOptions* mGeometryOptions;
  AudioOptions* mAudioOptions;
  ConflictOptions* mConflictOptions;

  // Destination Library
  ContentLibrary* mLibrary;

  // File to be imported
  Array<String> mFiles;
  Array<String> mConflictedFiles;
};

} // namespace Raverie
