// This file is licensed under the Elastic License 2.0. Copyright 2021 StarRocks Limited.

#ifndef STARROCKS_BE_UTIL_RAW_CONTAINER_H
#define STARROCKS_BE_UTIL_RAW_CONTAINER_H

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace starrocks {
namespace raw {

// RawAllocator allocates `trailing` more object(not bytes) than caller required,
// to avoid overflow when the memory is operated with 128-bit aligned instructions,
// such as `_mm_testc_si128`.
//
// Also it does not initiate allocated object to zero-value.
// This behavior is shipped from raw::RawAllocator.
//
// C++ Reference recommend to use this allocator implementation to
// prevent containers resize invocation from initializing the allocated
// memory space unnecessarily.
//
//   https://stackoverflow.com/questions/21028299/is-this-behavior-of-vectorresizesize-type-n-under-c11-and-boost-container/21028912#21028912
//
// Allocator adaptor that interposes construct() calls to
// convert value initialization into default initialization.
template <typename T, size_t trailing = 16, typename A = std::allocator<T>>
class RawAllocator : public A {
    static_assert(std::is_trivially_destructible_v<T>, "not trivially destructible type");
    typedef std::allocator_traits<A> a_t;

public:
    template <typename U>
    struct rebind {
        using other = RawAllocator<U, trailing, typename a_t::template rebind_alloc<U>>;
    };

    using A::A;

    // allocate more than caller required
    T* allocate(size_t n) {
        T* x = A::allocate(n + RawAllocator::_trailing);
        return x;
    }
    T* allocate(size_t n, const void* hint) {
        T* x = A::allocate(n + RawAllocator::_trailing, hint);
        return x;
    }

    // deallocate the storage referenced by the pointer p
    void deallocate(T* p, size_t n) { A::deallocate(p, n + RawAllocator::_trailing); }

    // do not initialized allocated.
    template <typename U>
    void construct(U* ptr) noexcept(std::is_nothrow_default_constructible<U>::value) {
        ::new (static_cast<void*>(ptr)) U;
    }
    template <typename U, typename... Args>
    void construct(U* ptr, Args&&... args) {
        a_t::construct(static_cast<A&>(*this), ptr, std::forward<Args>(args)...);
    }

private:
    static const size_t _trailing = trailing;
};

using RawString = std::basic_string<char, std::char_traits<char>, RawAllocator<char, 0>>;

using RawStringPad16 = std::basic_string<char, std::char_traits<char>, RawAllocator<char, 16>>;

// From cpp reference: "A trivial destructor is a destructor that performs no action. Objects with
// trivial destructors don't require a delete-expression and may be disposed of by simply
// deallocating their storage. All data types compatible with the C language (POD types)
// are trivially destructible."
// Types with trivial destructors is safe when when move content from a RawVectorPad16<T> into
// a std::vector<U> and both T and U the same bit width, i.e.
// starrocks::raw::RawVectorPad16<int8_t> a;
// a.resize(100);
// std::vector<uint8_t> b = std::move(reinterpret_cast<std::vector<uint8_t>&>(a));
template <class T>
using RawVector = std::vector<T, RawAllocator<T, 0>>;

template <class T>
using RawVectorPad16 = std::vector<T, RawAllocator<T, 16>>;

template <class T>
static inline void make_room(std::vector<T>* v, size_t n) {
    RawVector<T> rv;
    rv.resize(n);
    v->swap(reinterpret_cast<std::vector<T>&>(rv));
}

static inline void make_room(std::string* s, size_t n) {
    RawStringPad16 rs;
    rs.resize(n);
    s->swap(reinterpret_cast<std::string&>(rs));
}

template <typename T>
inline void stl_vector_resize_uninitialized(std::vector<T>* vec, size_t new_size) {
    ((RawVector<T>*)vec)->resize(new_size);
}

inline void stl_string_resize_uninitialized(std::string* str, size_t new_size) {
    ((RawString*)str)->resize(new_size);
}

} // namespace raw
} //namespace starrocks

#endif //STARROCKS_BE_UTIL_RAW_CONTAINER_H
