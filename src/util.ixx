//#include "precompiled.h"

#include <vector>
#include <functional>
#include <cinder/Vector.h>
#include <cinder/Surface.h>
#include <cinder/Color.h>
#include "macros.h"
using std::vector;
using std::function;
using namespace glm;

export module util;

//export typedef unsigned char byte;

template< typename T >
struct array_deleter
{
	void operator ()(T const* p)
	{
		delete[] p;
	}
};

template<class T>
class ArrayDeleter
{
	std::shared_ptr<T> sp;
public:
	ArrayDeleter(T* arrayPtr) {
		sp = std::shared_ptr<T>(arrayPtr, array_deleter<T>());
	}
};

export enum nofill {};

export template<class T>
struct Array2D;

export typedef glm::tvec3<unsigned char> bytevec3;

void copyCvtData(ci::Surface8u const& surface, Array2D<bytevec3> dst);
void copyCvtData(ci::Surface8u const& surface, Array2D<vec3> dst);
void copyCvtData(ci::SurfaceT<float> const& surface, Array2D<vec3> dst);
void copyCvtData(ci::SurfaceT<float> const& surface, Array2D<float> dst);
void copyCvtData(ci::ChannelT<float> const& surface, Array2D<float> dst);

export template<class T>
struct Array2D
{
	T* data;
	typedef T value_type;
	int area;
	int w, h;
	int NumBytes() const {
		return area * sizeof(T);
	}
	ci::ivec2 Size() const { return ci::ivec2(w, h); }
	ArrayDeleter<T> deleter;

	Array2D(int w, int h, nofill) : deleter(Init(w, h)) { }
	Array2D(ivec2 s, nofill) : deleter(Init(s.x, s.y)) { }
	Array2D(int w, int h, T const& defaultValue = T()) : deleter(Init(w, h)) { fill(defaultValue); }
	Array2D(ivec2 s, T const& defaultValue = T()) : deleter(Init(s.x, s.y)) { fill(defaultValue); }
	Array2D() : deleter(Init(0, 0)) { }

	Array2D(ci::Surface8u const& surface) : deleter(Init(surface.getWidth(), surface.getHeight()))
	{
		::copyCvtData(surface, *this);
	}

	Array2D(ci::SurfaceT<float> const& surface) : deleter(Init(surface.getWidth(), surface.getHeight()))
	{
		::copyCvtData(surface, *this);
	}

	Array2D(ci::ChannelT<float> const& surface) : deleter(Init(surface.getWidth(), surface.getHeight()))
	{
		::copyCvtData(surface, *this);
	}

#ifdef OPENCV_CORE_HPP
	template<class TSrc>
	Array2D(cv::Mat_<TSrc> const& mat) : deleter(nullptr)
	{
		Init(mat.cols, mat.rows, (T*)mat.data);
	}
#endif

	T* begin() { return data; }
	T* end() { return data + w * h; }
	T const* begin() const { return data; }
	T const* end() const { return data + w * h; }

	T& operator()(int x, int y) { return data[offsetOf(x, y)]; }
	T const& operator()(int x, int y) const { return data[offsetOf(x, y)]; }

	T& operator()(ivec2 const& v) { return data[offsetOf(v.x, v.y)]; }
	T const& operator()(ivec2 const& v) const { return data[offsetOf(v.x, v.y)]; }

	ivec2 wrapPoint(ivec2 p)
	{
		ivec2 wp = p;
		wp.x %= w; if (wp.x < 0) wp.x += w;
		wp.y %= h; if (wp.y < 0) wp.y += h;
		return wp;
	}

	T& wr(int x, int y) { return wr(ivec2(x, y)); }
	T const& wr(int x, int y) const { return wr(ivec2(x, y)); }

	T& wr(ivec2 const& v) { return (*this)(wrapPoint(v)); }
	T const& wr(ivec2 const& v) const { return (*this)(wrapPoint(v)); }

	bool contains(ivec2 const& p) const { return p.x >= 0 && p.y >= 0 && p.x < w&& p.y < h; }

	Array2D clone() const {
		Array2D result(Size());
		std::copy(begin(), end(), result.begin());
		return result;
	}

private:
	int offsetOf(int x, int y) const { return y * w + x; }
	void fill(T const& value)
	{
		std::fill(begin(), end(), value);
	}
	T* Init(int w, int h) {
		// fftwf_malloc so we can use "new-array execute" fftw functions
		//auto data = (T*)fftwf_malloc(w * h * sizeof(T));
		auto data = new T[w * h];
		Init(w, h, data);
		return data;
	}
	void Init(int w, int h, T* data) {
		this->data = data;
		area = w * h;
		this->w = w;
		this->h = h;
	}
};


void copyCvtData(ci::Surface8u const& surface, Array2D<bytevec3> dst)
{
	forxy(dst) {
		ci::ColorAT<uint8_t> inPixel = surface.getPixel(p);
		dst(p) = bytevec3(inPixel.r, inPixel.g, inPixel.b);
	}
}

void copyCvtData(ci::Surface8u const& surface, Array2D<vec3> dst) {
	forxy(dst) {
		ci::ColorAT<uint8_t> inPixel = surface.getPixel(p);
		dst(p) = vec3(inPixel.r, inPixel.g, inPixel.b) / 255.0f;
	}
}
void copyCvtData(ci::SurfaceT<float> const& surface, Array2D<vec3> dst) {
	forxy(dst) {
		ci::ColorAT<float> inPixel = surface.getPixel(p);
		dst(p) = vec3(inPixel.r, inPixel.g, inPixel.b);
	}
}
void copyCvtData(ci::SurfaceT<float> const& surface, Array2D<float> dst) {
	forxy(dst) {
		ci::ColorAT<float> inPixel = surface.getPixel(p);
		dst(p) = inPixel.r;
	}
}
void copyCvtData(ci::ChannelT<float> const& surface, Array2D<float> dst) {
	forxy(dst) {
		float inPixel = surface.getValue(p);
		dst(p) = inPixel;
	}
}


export float randFloat()
{
	return rand() / (float)RAND_MAX;
}

export template<class InputIt, class T>
T accumulate(InputIt begin, InputIt end, T base) {
	T sum = base;
	for (auto it = begin; it != end; it++) {
		sum += *it;
	}
	return sum;
}

export template<class T>
Array2D<T> empty_like(Array2D<T> a) {
	return Array2D<T>(a.Size(), nofill());
}

export template<class T>
Array2D<T> zeros_like(Array2D<T> a) {
	return Array2D<T>(a.Size(), ::zero<T>());
}
export template<class T>
void myRemoveIf(vector<T>& vec, function<bool(T const&)> const& pred) {
	vec.erase(std::remove_if(vec.begin(), vec.end(), pred), vec.end());
}

export const float pi = 3.14159265f;

export void rotate(vec2& p, float angle)
{
	float c = cos(angle), s = sin(angle);
	p = vec2(p.x * c + p.y * (-s), p.x * s + p.y * c);
}
