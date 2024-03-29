// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_RENDERGROUPBINDING_RLOGIC_SERIALIZATION_H_
#define FLATBUFFERS_GENERATED_RENDERGROUPBINDING_RLOGIC_SERIALIZATION_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 5 &&
              FLATBUFFERS_VERSION_REVISION == 9,
             "Non-compatible flatbuffers version included");

#include "RamsesBindingGen.h"

namespace rlogic_serialization {

struct Element;
struct ElementBuilder;

struct RenderGroupBinding;
struct RenderGroupBindingBuilder;

inline const ::flatbuffers::TypeTable *ElementTypeTable();

inline const ::flatbuffers::TypeTable *RenderGroupBindingTypeTable();

struct Element FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ElementBuilder Builder;
  struct Traits;
  static const ::flatbuffers::TypeTable *MiniReflectTypeTable() {
    return ElementTypeTable();
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_RAMSESOBJECT = 6
  };
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  const rlogic_serialization::RamsesReference *ramsesObject() const {
    return GetPointer<const rlogic_serialization::RamsesReference *>(VT_RAMSESOBJECT);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffset(verifier, VT_RAMSESOBJECT) &&
           verifier.VerifyTable(ramsesObject()) &&
           verifier.EndTable();
  }
};

struct ElementBuilder {
  typedef Element Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(Element::VT_NAME, name);
  }
  void add_ramsesObject(::flatbuffers::Offset<rlogic_serialization::RamsesReference> ramsesObject) {
    fbb_.AddOffset(Element::VT_RAMSESOBJECT, ramsesObject);
  }
  explicit ElementBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Element> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Element>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Element> CreateElement(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0,
    ::flatbuffers::Offset<rlogic_serialization::RamsesReference> ramsesObject = 0) {
  ElementBuilder builder_(_fbb);
  builder_.add_ramsesObject(ramsesObject);
  builder_.add_name(name);
  return builder_.Finish();
}

struct Element::Traits {
  using type = Element;
  static auto constexpr Create = CreateElement;
};

inline ::flatbuffers::Offset<Element> CreateElementDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    ::flatbuffers::Offset<rlogic_serialization::RamsesReference> ramsesObject = 0) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return rlogic_serialization::CreateElement(
      _fbb,
      name__,
      ramsesObject);
}

struct RenderGroupBinding FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef RenderGroupBindingBuilder Builder;
  struct Traits;
  static const ::flatbuffers::TypeTable *MiniReflectTypeTable() {
    return RenderGroupBindingTypeTable();
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_BASE = 4,
    VT_ELEMENTS = 6
  };
  const rlogic_serialization::RamsesBinding *base() const {
    return GetPointer<const rlogic_serialization::RamsesBinding *>(VT_BASE);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::Element>> *elements() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::Element>> *>(VT_ELEMENTS);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_BASE) &&
           verifier.VerifyTable(base()) &&
           VerifyOffset(verifier, VT_ELEMENTS) &&
           verifier.VerifyVector(elements()) &&
           verifier.VerifyVectorOfTables(elements()) &&
           verifier.EndTable();
  }
};

struct RenderGroupBindingBuilder {
  typedef RenderGroupBinding Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_base(::flatbuffers::Offset<rlogic_serialization::RamsesBinding> base) {
    fbb_.AddOffset(RenderGroupBinding::VT_BASE, base);
  }
  void add_elements(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::Element>>> elements) {
    fbb_.AddOffset(RenderGroupBinding::VT_ELEMENTS, elements);
  }
  explicit RenderGroupBindingBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<RenderGroupBinding> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<RenderGroupBinding>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<RenderGroupBinding> CreateRenderGroupBinding(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<rlogic_serialization::RamsesBinding> base = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<rlogic_serialization::Element>>> elements = 0) {
  RenderGroupBindingBuilder builder_(_fbb);
  builder_.add_elements(elements);
  builder_.add_base(base);
  return builder_.Finish();
}

struct RenderGroupBinding::Traits {
  using type = RenderGroupBinding;
  static auto constexpr Create = CreateRenderGroupBinding;
};

inline ::flatbuffers::Offset<RenderGroupBinding> CreateRenderGroupBindingDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<rlogic_serialization::RamsesBinding> base = 0,
    const std::vector<::flatbuffers::Offset<rlogic_serialization::Element>> *elements = nullptr) {
  auto elements__ = elements ? _fbb.CreateVector<::flatbuffers::Offset<rlogic_serialization::Element>>(*elements) : 0;
  return rlogic_serialization::CreateRenderGroupBinding(
      _fbb,
      base,
      elements__);
}

inline const ::flatbuffers::TypeTable *ElementTypeTable() {
  static const ::flatbuffers::TypeCode type_codes[] = {
    { ::flatbuffers::ET_STRING, 0, -1 },
    { ::flatbuffers::ET_SEQUENCE, 0, 0 }
  };
  static const ::flatbuffers::TypeFunction type_refs[] = {
    rlogic_serialization::RamsesReferenceTypeTable
  };
  static const char * const names[] = {
    "name",
    "ramsesObject"
  };
  static const ::flatbuffers::TypeTable tt = {
    ::flatbuffers::ST_TABLE, 2, type_codes, type_refs, nullptr, nullptr, names
  };
  return &tt;
}

inline const ::flatbuffers::TypeTable *RenderGroupBindingTypeTable() {
  static const ::flatbuffers::TypeCode type_codes[] = {
    { ::flatbuffers::ET_SEQUENCE, 0, 0 },
    { ::flatbuffers::ET_SEQUENCE, 1, 1 }
  };
  static const ::flatbuffers::TypeFunction type_refs[] = {
    rlogic_serialization::RamsesBindingTypeTable,
    rlogic_serialization::ElementTypeTable
  };
  static const char * const names[] = {
    "base",
    "elements"
  };
  static const ::flatbuffers::TypeTable tt = {
    ::flatbuffers::ST_TABLE, 2, type_codes, type_refs, nullptr, nullptr, names
  };
  return &tt;
}

}  // namespace rlogic_serialization

#endif  // FLATBUFFERS_GENERATED_RENDERGROUPBINDING_RLOGIC_SERIALIZATION_H_
