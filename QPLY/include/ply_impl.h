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

// Vertex that also has a normal
struct VertexN: public Vertex {
    VertexN();
    VertexN(float x, float y, float z);
    VertexN(float x, float y, float z,
            float nx, float ny, float nz);

    Value* get_value(const Element& elem, const Property& prop);
    bool make_element(Element& elem) const;

    // Accessors
    float nx() const;
    float ny() const;
    float nz() const;

    // Mutators
    void nx(float value);
    void ny(float value);
    void nz(float value);

    // Value members
    FloatValue value_nx, value_ny, value_nz;

    // Property descriptor members
    static const Property prop_nx;
    static const Property prop_ny;
    static const Property prop_nz;
};

// Vertex that also has a normal and a color
struct VertexNC: public VertexN {
    VertexNC();
    VertexNC(float x, float y, float z);
    VertexNC(float x, float y, float z,
             float nx, float ny, float nz);
    VertexNC(float x, float y, float z,
             float nx, float ny, float nz,
             float r, float g, float b, float a);
    VertexNC(float x, float y, float z,
             float nx, float ny, float nz,
             unsigned char r, unsigned char g, unsigned char b, unsigned char a);

    Value* get_value(const Element& elem, const Property& prop);
    bool make_element(Element& elem) const;

    // Accessors
    unsigned char red() const;
    unsigned char green() const;
    unsigned char blue() const;
    unsigned char alpha() const;

    float redF() const;
    float greenF() const;
    float blueF() const;
    float alphaF() const;

    // Mutators
    void red(unsigned char color);
    void green(unsigned char color);
    void blue(unsigned char color);
    void alpha(unsigned char color);

    void redF(float color);
    void greenF(float color);
    void blueF(float color);
    void alphaF(float color);

    // Value members (always stored as floats)
    FloatValue value_r, value_g, value_b, value_a;

    // Property descriptor members
    static const Property prop_r;
    static const Property prop_g;
    static const Property prop_b;
    static const Property prop_a;
};

// Vertex that also has a normal, a color, and texture coordinates
struct VertexNCT: public VertexNC {
    VertexNCT();
    VertexNCT(float x, float y, float z);
    VertexNCT(float x, float y, float z,
              float nx, float ny, float nz);
    VertexNCT(float x, float y, float z,
              float nx, float ny, float nz,
              float r, float g, float b, float a);
    VertexNCT(float x, float y, float z,
              float nx, float ny, float nz,
              unsigned char r, unsigned char g, unsigned char b, unsigned char a);
    VertexNCT(float x, float y, float z,
              float nx, float ny, float nz,
              float r, float g, float b, float a,
              float tu, float tv, float tn);
    VertexNCT(float x, float y, float z,
              float nx, float ny, float nz,
              unsigned char r, unsigned char g, unsigned char b, unsigned char a,
              float tu, float tv, float tn);

    Value* get_value(const Element& elem, const Property& prop);
    bool make_element(Element& elem) const;

    // Accessors
    float tu() const;
    float tv() const;
    float tn() const;

    // Mutators
    void tu(float value);
    void tv(float value);
    void tn(float value);

    // Value members
    FloatValue value_tu, value_tv, value_tn;

    // Property descriptor members
    static const Property prop_tu;
    static const Property prop_tv;
    static const Property prop_tn;
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
typedef ObjArray<Vertex> VertexArray;
typedef ObjArray<VertexN> VNArray;
typedef ObjArray<VertexNC> VNCArray;
typedef ObjArray<VertexNCT> VNCTArray;

typedef ObjArray<Face> FaceArray;
typedef ObjArray<FaceTex> FTArray;

typedef ObjExternal<Vertex> VertexExternal;
typedef ObjExternal<VertexN> VNExternal;
typedef ObjExternal<VertexNC> VNCExternal;
typedef ObjExternal<VertexNCT> VNCTExternal;

typedef ObjExternal<Face> FaceExternal;
typedef ObjExternal<FaceTex> FTExternal;

} // namespace PLY

#endif // __PLY_IMPL_H__
