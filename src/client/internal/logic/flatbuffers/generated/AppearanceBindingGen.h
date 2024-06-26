// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_APPEARANCEBINDING_RLOGIC_SERIALIZATION_H_
#define FLATBUFFERS_GENERATED_APPEARANCEBINDING_RLOGIC_SERIALIZATION_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 5 &&
              FLATBUFFERS_VERSION_REVISION == 9,
             "Non-compatible flatbuffers version included");

#include "RamsesBindingGen.h"

namespace rlogic_serialization {

struct ResourceId;

struct AppearanceBinding;
struct AppearanceBindingBuilder;

inline const ::flatbuffers::TypeTable *ResourceIdTypeTable();

inline const ::flatbuffers::TypeTable *AppearanceBindingTypeTable();

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(8) ResourceId FLATBUFFERS_FINAL_CLASS {
 private:
  uint64_t resourceIdLow_;
  uint64_t resourceIdHigh_;

 public:
  struct Traits;
  static const ::flatbuffers::TypeTable *MiniReflectTypeTable() {
    return ResourceIdTypeTable();
  }
  ResourceId()
      : resourceIdLow_(0),
        resourceIdHigh_(0) {
  }
  ResourceId(uint64_t _resourceIdLow, uint64_t _resourceIdHigh)
      : resourceIdLow_(::flatbuffers::EndianScalar(_resourceIdLow)),
        resourceIdHigh_(::flatbuffers::EndianScalar(_resourceIdHigh)) {
  }
  uint64_t resourceIdLow() const {
    return ::flatbuffers::EndianScalar(resourceIdLow_);
  }
  uint64_t resourceIdHigh() const {
    return ::flatbuffers::EndianScalar(resourceIdHigh_);
  }
};
FLATBUFFERS_STRUCT_END(ResourceId, 16);

struct ResourceId::Traits {
  using type = ResourceId;
};

struct AppearanceBinding FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef AppearanceBindingBuilder Builder;
  struct Traits;
  static const ::flatbuffers::TypeTable *MiniReflectTypeTable() {
    return AppearanceBindingTypeTable();
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_BASE = 4,
    VT_PARENTEFFECTID = 6
  };
  const rlogic_serialization::RamsesBinding *base() const {
    return GetPointer<const rlogic_serialization::RamsesBinding *>(VT_BASE);
  }
  const rlogic_serialization::ResourceId *parentEffectId() const {
    return GetStruct<const rlogic_serialization::ResourceId *>(VT_PARENTEFFECTID);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_BASE) &&
           verifier.VerifyTable(base()) &&
           VerifyField<rlogic_serialization::ResourceId>(verifier, VT_PARENTEFFECTID, 8) &&
           verifier.EndTable();
  }
};

struct AppearanceBindingBuilder {
  typedef AppearanceBinding Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_base(::flatbuffers::Offset<rlogic_serialization::RamsesBinding> base) {
    fbb_.AddOffset(AppearanceBinding::VT_BASE, base);
  }
  void add_parentEffectId(const rlogic_serialization::ResourceId *parentEffectId) {
    fbb_.AddStruct(AppearanceBinding::VT_PARENTEFFECTID, parentEffectId);
  }
  explicit AppearanceBindingBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<AppearanceBinding> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<AppearanceBinding>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<AppearanceBinding> CreateAppearanceBinding(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<rlogic_serialization::RamsesBinding> base = 0,
    const rlogic_serialization::ResourceId *parentEffectId = nullptr) {
  AppearanceBindingBuilder builder_(_fbb);
  builder_.add_parentEffectId(parentEffectId);
  builder_.add_base(base);
  return builder_.Finish();
}

struct AppearanceBinding::Traits {
  using type = AppearanceBinding;
  static auto constexpr Create = CreateAppearanceBinding;
};

inline const ::flatbuffers::TypeTable *ResourceIdTypeTable() {
  static const ::flatbuffers::TypeCode type_codes[] = {
    { ::flatbuffers::ET_ULONG, 0, -1 },
    { ::flatbuffers::ET_ULONG, 0, -1 }
  };
  static const int64_t values[] = { 0, 8, 16 };
  static const char * const names[] = {
    "resourceIdLow",
    "resourceIdHigh"
  };
  static const ::flatbuffers::TypeTable tt = {
    ::flatbuffers::ST_STRUCT, 2, type_codes, nullptr, nullptr, values, names
  };
  return &tt;
}

inline const ::flatbuffers::TypeTable *AppearanceBindingTypeTable() {
  static const ::flatbuffers::TypeCode type_codes[] = {
    { ::flatbuffers::ET_SEQUENCE, 0, 0 },
    { ::flatbuffers::ET_SEQUENCE, 0, 1 }
  };
  static const ::flatbuffers::TypeFunction type_refs[] = {
    rlogic_serialization::RamsesBindingTypeTable,
    rlogic_serialization::ResourceIdTypeTable
  };
  static const char * const names[] = {
    "base",
    "parentEffectId"
  };
  static const ::flatbuffers::TypeTable tt = {
    ::flatbuffers::ST_TABLE, 2, type_codes, type_refs, nullptr, nullptr, names
  };
  return &tt;
}

}  // namespace rlogic_serialization

#endif  // FLATBUFFERS_GENERATED_APPEARANCEBINDING_RLOGIC_SERIALIZATION_H_
