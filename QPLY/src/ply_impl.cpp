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

VertexColor::VertexColor() : Vertex(), value_r(1), value_g(1), value_b(1) {}
VertexColor::VertexColor(float x, float y, float z): Vertex(x, y, z), value_r(1), value_g(1), value_b(1) {}
VertexColor::VertexColor(float x, float y, float z, unsigned char r, unsigned char g, unsigned char b)
    : Vertex(x, y, z), value_r(r), value_g(g), value_b(b) {}

Value* VertexColor::get_value(const Element& elem, const Property& prop) {
    Value* result = Vertex::get_value(elem, prop);
    if (result == 0) {
        if (prop.name.compare(prop_r.name.c_str()) == 0)		return &value_r;
        else if (prop.name.compare(prop_g.name.c_str()) == 0)	return &value_g;
        else if (prop.name.compare(prop_b.name.c_str()) == 0)	return &value_b;
    }
    return result;
}

// Construct an Element describing this Object.
bool VertexColor::make_element(Element& elem) const {
    bool result = Vertex::make_element(elem);
    elem.props.push_back(prop_r);
    elem.props.push_back(prop_g);
    elem.props.push_back(prop_b);
    return result;
}

unsigned char VertexColor::r() const {
    double dval;
    value_r.get_scalar(prop_r, dval);
    return (unsigned char)dval;
}

unsigned char VertexColor::g() const {
    double dval;
    value_g.get_scalar(prop_g, dval);
    return (unsigned char)dval;
}

unsigned char VertexColor::b() const {
    double dval;
    value_b.get_scalar(prop_b, dval);
    return (unsigned char)dval;
}

void VertexColor::r(unsigned char coord) { value_r.set_scalar(prop_r, coord); }
void VertexColor::g(unsigned char coord) { value_g.set_scalar(prop_g, coord); }
void VertexColor::b(unsigned char coord) { value_b.set_scalar(prop_b, coord); }

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
template struct ObjArray<Vertex>;
template struct ObjArray<FaceTex>;
template struct ObjArray<VertexColor>;

template struct ObjExternal<Face>;
template struct ObjExternal<Vertex>;
template struct ObjExternal<FaceTex>;
template struct ObjExternal<VertexColor>;
} // namespace PLY
