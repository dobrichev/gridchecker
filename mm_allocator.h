#ifndef MM_ALLOCATOR_H_INCLUDED

#define MM_ALLOCATOR_H_INCLUDED

#ifndef _MSC_VER
#include <mm_malloc.h>
#endif

//code inspired by http://www.drdobbs.com/cpp/184403759
template <class T> class mm_allocator {
public:
	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	template <class U> struct rebind { typedef mm_allocator<U> other; };
	mm_allocator() {}
	mm_allocator(const mm_allocator&) {}
	template <class U> mm_allocator(const mm_allocator<U>&) {}
	~mm_allocator() {}
	pointer address(reference x) const { return &x; }
	const_pointer address(const_reference x) const { return x; }
	pointer allocate(size_type n, const_pointer = 0) {
		void* p = _mm_malloc(n * sizeof(T), 16);
		if (!p)	throw std::bad_alloc();
		return static_cast<pointer>(p);
	}
	void deallocate(pointer p, size_type) { _mm_free(p); }
	size_type max_size() const { return static_cast<size_type>(-1) / sizeof(T);	}
	void construct(pointer p, const value_type& x) { new(p) value_type(x); }
	void destroy(pointer p) { p->~value_type(); }
private:
	void operator=(const mm_allocator&);
};

template<> class mm_allocator<void> {
	typedef void value_type;
	typedef void* pointer;
	typedef const void* const_pointer;
	template <class U> struct rebind { typedef mm_allocator<U> other; };
};

template <class T> inline bool operator==(const mm_allocator<T>&, const mm_allocator<T>&) {
	return true;
}

template <class T> inline bool operator!=(const mm_allocator<T>&, const mm_allocator<T>&) {
	return false;
}

#endif // MM_ALLOCATOR_H_INCLUDED
