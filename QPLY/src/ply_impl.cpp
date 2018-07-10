#include "ply_impl.h"

namespace PLY {

template <class T>
ScalarValue<T>::ScalarValue() : val(0) {}

template <class T>
ScalarValue<T>::ScalarValue(const T& v): val(v) {}
	
template <class T>
bool ScalarValue<T>::get_scalar(const Property& prop, double& value) const {
    if (prop.type != SCALAR) {
        return false;
    }
    value = (double)val;
    return true;
}

template <class T>
bool ScalarValue<T>::set_scalar(const Property& prop, const double& value) {
    if (prop.type != SCALAR) {
        return false;
    }
    val = (T)value;
    return true;
}

Vertex::Vertex() : value_x(0), value_y(0), value_z(0) {}
Vertex::Vertex(float x, float y, float z): value_x(x), value_y(y), value_z(z) {}

// Get a Value.
Value* Vertex::get_value(const Element& elem, const Property& prop) {
    if (prop.name.compare(prop_x.name.c_str()) == 0)		return &value_x;
    else if (prop.name.compare(prop_y.name.c_str()) == 0)	return &value_y;
    else if (prop.name.compare(prop_z.name.c_str()) == 0)	return &value_z;
    return 0;
}

// Construct an Element describing this Object.
bool Vertex::make_element(Element& elem) const {
    elem.name = name;
    elem.props.push_back(prop_x);
    elem.props.push_back(prop_y);
    elem.props.push_back(prop_z);
    return true;
}
		
float Vertex::x() const { double dval; value_x.get_scalar(prop_x, dval); return (float)dval; }
float Vertex::y() const { double dval; value_y.get_scalar(prop_y, dval); return (float)dval; }
float Vertex::z() const { double dval; value_z.get_scalar(prop_z, dval); return (float)dval; }

void Vertex::x(float coord) { value_x.set_scalar(prop_x, coord); }
void Vertex::y(float coord) { value_y.set_scalar(prop_y, coord); }
void Vertex::z(float coord) { value_z.set_scalar(prop_z, coord); }

/// ======================== Vertex w/ Normal ================================ ///

VertexN::VertexN() : Vertex(), value_nx(0.0f), value_ny(1.0f), value_nz(0.0f) {}
VertexN::VertexN(float x, float y, float z): Vertex(x, y, z), value_nx(0.0f), value_ny(1.0f), value_nz(0.0f) {}
VertexN::VertexN(float x, float y, float z, float nx, float ny, float nz)
    : Vertex(x, y, z), value_nx(nx), value_ny(ny), value_nz(nz) {}

Value* VertexN::get_value(const Element& elem, const Property& prop) {
    Value* result = Vertex::get_value(elem, prop);
    if (result == 0) {
        if (prop.name.compare(prop_nx.name.c_str()) == 0)		return &value_nx;
        else if (prop.name.compare(prop_ny.name.c_str()) == 0)	return &value_ny;
        else if (prop.name.compare(prop_nz.name.c_str()) == 0)	return &value_nz;
    }
    return result;
}

// Construct an Element describing this Object.
bool VertexN::make_element(Element& elem) const {
    bool result = Vertex::make_element(elem);
    elem.props.push_back(prop_nx);
    elem.props.push_back(prop_ny);
    elem.props.push_back(prop_nz);
    return result;
}

float VertexN::nx() const {
    double dval;
    value_nx.get_scalar(prop_nx, dval);
    return (unsigned char)dval;
}

float VertexN::ny() const {
    double dval;
    value_ny.get_scalar(prop_ny, dval);
    return (unsigned char)dval;
}

float VertexN::nz() const {
    double dval;
    value_nz.get_scalar(prop_nz, dval);
    return (unsigned char)dval;
}

void VertexN::nx(float coord) { value_nx.set_scalar(prop_nx, coord); }
void VertexN::ny(float coord) { value_ny.set_scalar(prop_ny, coord); }
void VertexN::nz(float coord) { value_nz.set_scalar(prop_nz, coord); }

/// ======================== END Vertex w/ Normal ================================ ///

/// ===================== Vertex w/ Normal and Color ============================= ///

VertexNC::VertexNC() : VertexN(), value_r(1.0f), value_g(1.0f), value_b(1.0f), value_a(1.0f) {}
VertexNC::VertexNC(float x, float y, float z): VertexN(x, y, z), value_r(1.0f), value_g(1.0f), value_b(1.0f), value_a(1.0f) {}

VertexNC::VertexNC(float x, float y, float z, float nx, float ny, float nz)
    : VertexN(x, y, z, nx, ny, nz), value_r(1.0f), value_g(1.0f), value_b(1.0f), value_a(1.0f) {}

VertexNC::VertexNC(float x, float y, float z, float nx, float ny, float nz,
                   float r, float g, float b, float a)
    : VertexN(x, y, z, nx, ny, nz), value_r(r), value_g(g), value_b(b), value_a(a) {}

VertexNC::VertexNC(float x, float y, float z, float nx, float ny, float nz,
                   unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    : VertexN(x, y, z, nx, ny, nz), value_r(r/255.0f), value_g(g/255.0f), value_b(b/255.0f), value_a(a/255.0f) {}

Value* VertexNC::get_value(const Element& elem, const Property& prop) {
    Value* result = VertexN::get_value(elem, prop);
    if (result == 0) {
        if (prop.name.compare(prop_r.name.c_str()) == 0)		return &value_r;
        else if (prop.name.compare(prop_g.name.c_str()) == 0)	return &value_g;
        else if (prop.name.compare(prop_b.name.c_str()) == 0)	return &value_b;
        else if (prop.name.compare(prop_a.name.c_str()) == 0)	return &value_a;
    }
    return result;
}

// Construct an Element describing this Object.
bool VertexNC::make_element(Element& elem) const {
    bool result = VertexN::make_element(elem);
    elem.props.push_back(prop_r);
    elem.props.push_back(prop_g);
    elem.props.push_back(prop_b);
    elem.props.push_back(prop_a);
    return result;
}

unsigned char VertexNC::red() const {
    double dval;
    value_r.get_scalar(prop_r, dval);
    return (unsigned char)floor(dval * 255.0);
}

unsigned char VertexNC::green() const {
    double dval;
    value_g.get_scalar(prop_g, dval);
    return (unsigned char)floor(dval * 255.0);
}

unsigned char VertexNC::blue() const {
    double dval;
    value_b.get_scalar(prop_b, dval);
    return (unsigned char)floor(dval * 255.0);
}

unsigned char VertexNC::alpha() const {
    double dval;
    value_a.get_scalar(prop_a, dval);
    return (unsigned char)floor(dval * 255.0);
}

float VertexNC::redF() const {
    double dval;
    value_r.get_scalar(prop_r, dval);
    return (float)dval;
}

float VertexNC::greenF() const {
    double dval;
    value_g.get_scalar(prop_g, dval);
    return (float)dval;
}

float VertexNC::blueF() const {
    double dval;
    value_b.get_scalar(prop_b, dval);
    return (float)dval;
}

float VertexNC::alphaF() const {
    double dval;
    value_a.get_scalar(prop_a, dval);
    return (float)dval;
}

void VertexNC::red(unsigned char color) { value_r.set_scalar(prop_r, color/255.0); }
void VertexNC::green(unsigned char color) { value_g.set_scalar(prop_g, color/255.0); }
void VertexNC::blue(unsigned char color) { value_b.set_scalar(prop_b, color/255.0); }
void VertexNC::alpha(unsigned char color) { value_a.set_scalar(prop_b, color/255.0); }

void VertexNC::redF(float color) { value_r.set_scalar(prop_r, color); }
void VertexNC::greenF(float color) { value_g.set_scalar(prop_g, color); }
void VertexNC::blueF(float color) { value_b.set_scalar(prop_b, color); }
void VertexNC::alphaF(float color) { value_a.set_scalar(prop_b, color); }

/// ===================== END Vertex w/ Normal and Color ============================= ///

/// ================= Vertex w/ Normal, Color, and Tex Coord ========================= ///

VertexNCT::VertexNCT() : VertexNC(), value_tu(0.0f), value_tv(0.0f), value_tn(0.0f) {}
VertexNCT::VertexNCT(float x, float y, float z): VertexNC(x, y, z), value_tu(0.0f), value_tv(0.0f), value_tn(0.0f) {}

VertexNCT::VertexNCT(float x, float y, float z, float nx, float ny, float nz)
    : VertexNC(x, y, z, nx, ny, nz), value_tu(0.0f), value_tv(0.0f), value_tn(0.0f) {}

VertexNCT::VertexNCT(float x, float y, float z, float nx, float ny, float nz,
                     float r, float g, float b, float a)
    : VertexNC(x, y, z, nx, ny, nz, r, g, b, a), value_tu(0.0f), value_tv(0.0f), value_tn(0.0f) {}

VertexNCT::VertexNCT(float x, float y, float z, float nx, float ny, float nz,
                     unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    : VertexNC(x, y, z, nx, ny, nz, r, g, b, a), value_tu(0.0f), value_tv(0.0f), value_tn(0.0f) {}

VertexNCT::VertexNCT(float x, float y, float z, float nx, float ny, float nz,
                     float r, float g, float b, float a, float tu, float tv, float tn)
    : VertexNC(x, y, z, nx, ny, nz, r, g, b, a), value_tu(tu), value_tv(tv), value_tn(tn) {}

VertexNCT::VertexNCT(float x, float y, float z, float nx, float ny, float nz,
                     unsigned char r, unsigned char g, unsigned char b, unsigned char a, float tu, float tv, float tn)
    : VertexNC(x, y, z, nx, ny, nz, r, g, b, a), value_tu(tu), value_tv(tv), value_tn(tn) {}

Value* VertexNCT::get_value(const Element& elem, const Property& prop) {
    Value* result = VertexNC::get_value(elem, prop);
    if (result == 0) {
        if (prop.name.compare(prop_tu.name.c_str()) == 0)		return &value_tu;
        else if (prop.name.compare(prop_tv.name.c_str()) == 0)	return &value_tv;
        else if (prop.name.compare(prop_tn.name.c_str()) == 0)	return &value_tn;
    }
    return result;
}

// Construct an Element describing this Object.
bool VertexNCT::make_element(Element& elem) const {
    bool result = VertexNC::make_element(elem);
    elem.props.push_back(prop_tu);
    elem.props.push_back(prop_tv);
    elem.props.push_back(prop_tn);
    return result;
}

float VertexNCT::tu() const {
    double dval;
    value_tu.get_scalar(prop_tu, dval);
    return (unsigned char)dval;
}

float VertexNCT::tv() const {
    double dval;
    value_tv.get_scalar(prop_tv, dval);
    return (unsigned char)dval;
}

float VertexNCT::tn() const {
    double dval;
    value_tn.get_scalar(prop_tn, dval);
    return (unsigned char)dval;
}

void VertexNCT::tu(float coord) { value_tu.set_scalar(prop_tu, coord); }
void VertexNCT::tv(float coord) { value_tv.set_scalar(prop_tv, coord); }
void VertexNCT::tn(float coord) { value_tn.set_scalar(prop_tn, coord); }

/// ================= END Vertex w/ Normal, Color, and Tex Coord ========================= ///

Face::Face() {}
Face::Face(const size_t& size) { indices.set_size(prop_ind, size); }
Face::Face(const Face& f) {
    if (f.indices.data != 0) {
        size_t size;
        double val;
        f.indices.get_size(prop_ind, size);
        indices.set_size(prop_ind, size);
        for (size_t n = 0; n < size; ++n) {
            f.indices.get_item(prop_ind, n, val);
            indices.set_item(prop_ind, n, val);
        }
    }
}

// Get a Value.
Value* Face::get_value(const Element& elem, const Property& prop) {
    if (prop.name.compare(prop_ind.name.c_str()) == 0) return &indices;
    return 0;
}

// Construct an Element describing this Object.
bool Face::make_element(Element& elem) const {
    elem.name = name;
    elem.props.push_back(prop_ind);
    return true;
}

size_t Face::size() const {
    size_t s;
    indices.get_size(prop_ind, s);
    return s;
}

size_t Face::vertex(const size_t& num) const {
    double index;
    indices.get_item(prop_ind, num, index);
    return (size_t)index;
}

void Face::size(const size_t& size) {
    indices.set_size(prop_ind, size);
}

void Face::vertex(const size_t& num, const size_t& index) {
    indices.set_item(prop_ind, num, (double)index);
}

FaceTex::FaceTex() : Face() {}
FaceTex::FaceTex(const size_t& size) : Face(size) { texcoords.set_size(prop_ind, size); }
FaceTex::FaceTex(const Face& f) : Face(f) {}
FaceTex::FaceTex(const FaceTex& f) : Face(f) {
    if (f.texcoords.data != 0) {
        size_t size;
        double val;
        f.texcoords.get_size(prop_tex, size);
        texcoords.set_size(prop_tex, size);
        for (size_t n = 0; n < size; ++n) {
            f.texcoords.get_item(prop_tex, n, val);
            texcoords.set_item(prop_tex, n, val);
        }
    }
}

// Get a Value.
Value* FaceTex::get_value(const Element& elem, const Property& prop) {
    Value* result = Face::get_value(elem, prop);
    if (result == 0) {
        if (prop.name.compare(prop_tex.name.c_str()) == 0) return &texcoords;
    }
    return result;
}

// Construct an Element describing this Object.
bool FaceTex::make_element(Element& elem) const {
    bool result = Face::make_element(elem);
    elem.props.push_back(prop_tex);
    return result;
}

size_t FaceTex::size() const {
    return Face::size();
}

void FaceTex::size(const size_t& size) {
    Face::size(size);
    texcoords.set_size(prop_tex, size*3);
}

float FaceTex::texcoord(const size_t& num) const {
    double coord;
    texcoords.get_item(prop_tex, num, coord);
    return (float)coord;
}

void FaceTex::texcoord(const size_t& num, const float& coord) {
    texcoords.set_item(prop_tex, num, (double)coord);
}

template <class T>
ObjArray<T>::ObjArray(): AnyArray() {}
	
template <class T>
Object& ObjArray<T>::next_object() {
    if (objects[incr] == 0)
        objects[incr] = new Vertex;
    return *objects[incr++];
}

template <class T>
ObjExternal<T>::ObjExternal(std::vector<T>& v): Array(), objects(v), incr(0) {}

template <class T>
size_t ObjExternal<T>::size() { return objects.size(); }

template <class T>
void ObjExternal<T>::prepare(const size_t& size) { objects.reserve(size); restart(); }

template <class T>
void ObjExternal<T>::clear() { objects.clear(); }

template <class T>
void ObjExternal<T>::restart() { incr = 0; }

template <class T>
Object& ObjExternal<T>::next_object() {
    if (objects.size() <= incr)
        objects.resize(incr+1);
    return objects[incr++];
}

// Explicit template instantiations to match the typedefs in the header
template struct ObjArray<Face>;
template struct ObjArray<FaceTex>;

template struct ObjArray<Vertex>;
template struct ObjArray<VertexN>;
template struct ObjArray<VertexNC>;
template struct ObjArray<VertexNCT>;

template struct ObjExternal<Face>;
template struct ObjExternal<FaceTex>;

template struct ObjExternal<Vertex>;
template struct ObjExternal<VertexN>;
template struct ObjExternal<VertexNC>;
template struct ObjExternal<VertexNCT>;

} // namespace PLY
