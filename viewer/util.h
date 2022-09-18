#ifndef __UTIL_H__
#define __UTIL_H__


#ifdef swap
#undef swap
#endif

template<class T>
static inline void swap(T &a, T &b)
{
	T t = a;
	a = b;
	b = t;
}

#define radians(x) (( x * M_PI ) / 180)

static inline bool islittleendian() {
	union {
		int i;
		byte b[sizeof(int)];
	} conv;

	conv.i = 1;
	return conv.b[0] != 0;
}

inline ushort endianswap16(ushort n) {
	return (n<<8) | (n>>8);
}

inline uint endianswap32(uint n) {
	return (n<<24) | (n>>24) | ((n>>8)&0xFF00) | ((n<<8)&0xFF0000);
}

template<class T> inline T endianswap(T n) {
	union {
		T t;
		uint i;
	} conv;

	conv.t = n;
	conv.i = endianswap32(conv.i);
	return conv.t;
}

template<> inline ushort endianswap<ushort>(ushort n) {
	return endianswap16(n);
}

template<> inline short endianswap<short>(short n) {
	return endianswap16(n);
}

template<> inline uint endianswap<uint>(uint n) {
	return endianswap32(n);
}

template<> inline int endianswap<int>(int n) {
	return endianswap32(n);
}

template<class T> inline void endianswap(T *buf, int len) {
	for(T *end = &buf[len]; buf < end; buf++)
		*buf = endianswap(*buf);
}

template<class T> inline T lilswap(T n) {
	return islittleendian() ? n : endianswap(n);
}

template<class T> inline void lilswap(T *buf, int len) {
	if (!islittleendian())
		endianswap(buf, len);
}

template<class T> inline T bigswap(T n) {
	return islittleendian() ? endianswap(n) : n;
}

template<class T> inline void bigswap(T *buf, int len) {
	if (islittleendian())
		endianswap(buf, len);
}

template<class T> T getval(FILE *f) {
	T n;
	return fread(&n, 1, sizeof(n), f) == sizeof(n) ? n : 0;
}

template<class T> T getlil(FILE *f) {
	return lilswap(getval<T>(f));
}

template<class T> T getbig(FILE *f) {
	return bigswap(getval<T>(f));
}

#endif

