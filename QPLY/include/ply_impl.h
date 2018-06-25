#ifndef __PLY_IMPL_H__
#define __PLY_IMPL_H__

#include "unknown.h"

namespace PLY {

template <class T>
struct ScalarValue: public Value {
    ScalarValue();
    ScalarValue(const T& v);

    // Accessor and mutator
    bool get_scalar(const Property& prop, double& value) const;
    bool set_scalar(const Property& prop, const double& value);

    // Single value member
    T val;
};

// Template instantations
typedef ScalarValue<unsigned char> UByteValue;
typedef ScalarValue<char> ByteValue;
typedef ScalarValue<unsigned int> UIntValue;
typedef ScalarValue<int> IntValue;
typedef ScalarValue<float> FloatValue;
typedef ScalarValue<double> DoubleValue;

// Vertex with 3d Float32 coordinates (and nothing else)
struct Vertex: public Object {
    Vertex();
    Vertex(float x, float y, float z);

    virtual Value* get_value(const Element& elem, const Property& prop);
    virtual bool make_element(Element& elem) const;

    // Accessors
    float x() const; float y() const; float z() const;

    // Mutators
    void x(float coord); void y(float coord); void z(float coord);

    // Value members
    FloatValue value_x, value_y, value_z;

    // Property descriptor members
    static const char* name;
    static const Property prop_x;
    static const Property prop_y;
    static const Property prop_z;
}; // struct Vertex

struct VertexColor: public Vertex {
    VertexColor();
    VertexColor(float x, float y, float z);
    VertexColor(float x, float y, float z,
                unsigned char r, unsigned char g, unsigned char b);

    Value* get_value(const Element& elem, const Property& prop);
    bool make_element(Element& elem) const;

    // Accessors
    unsigned char r() const;
    unsigned char g() const;
    unsigned char b() const;

    // Mutators
    void r(unsigned char color);
    void g(unsigned char color);
    void b(unsigned char color);

    // Value members
    UByteValue value_r, value_g, value_b;

    // Property descriptor members
    static const Property prop_r;
    static const Property prop_g;
    static const Property prop_b;
};

// Face with a list of vertex indices
struct Face: public Object {
    AnyValue indices;

    Face();
    Face(const size_t& size);
    Face(const Face& f);

    Value* get_value(const Element& elem, const Property& prop);
    bool make_element(Element& elem) const;

    virtual size_t size() const;
    size_t vertex(const size_t& num) const;

    virtual void size(const size_t& size);
    void vertex(const size_t& num, const size_t& index);

    static const char* name;
    static const Property prop_ind;
}; // struct Face

struct FaceTex: public Face {
    AnyValue texcoords;

    FaceTex();
    FaceTex(const size_t& size);
    FaceTex(const Face& f);
    FaceTex(const FaceTex& f);

    size_t size() const;
    void size(const size_t& size);

    Value* get_value(const Element& elem, const Property& prop);
    bool make_element(Element& elem) const;

    float texcoord(const size_t& num) const;
    void texcoord(const size_t& num, const float& coord);

    static const Property prop_tex;
}; // struct Face

// Different lists in template form
template <class T>
struct ObjArray: public AnyArray {
    ObjArray();
    Object& next_object();
};

template <class T>
struct ObjExternal: public Array {
    std::vector<T>& objects;
    size_t incr;

    ObjExternal(std::vector<T>& v);

    size_t size();
    void prepare(const size_t& size);
    void clear();
    void restart();

    // Get the next Object.
    Object& next_object();
};

// Typedefs for some template instantiations
typedef ObjArray<Face> FaceArray;
typedef ObjArray<Vertex> VertexArray;
typedef ObjArray<FaceTex> FTArray;
typedef ObjArray<VertexColor> VCArray;

typedef ObjExternal<Face> FaceExternal;
typedef ObjExternal<Vertex> VertexExternal;
typedef ObjExternal<FaceTex> FTExternal;
typedef ObjExternal<VertexColor> VCExternal;
} // namespace PLY

#endif // __PLY_IMPL_H__
