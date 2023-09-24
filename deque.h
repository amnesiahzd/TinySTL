#pragma once

#include <initializer_list>

#include "allocator.h"
#include "iterator.h"
#include "memory.h"
#include "util.h"
#include "exceptdef.h"

#ifdef max
#pragma message("#undefing marco max")
#undef max
#endif // max

#ifdef min
#pragma message("#undefing marco min")
#undef min
#endif // min

const static size_t DEQUE_MAP_INIT_SIZE = 8;

template <class T>
struct deque_buf_size {
    static constexpr size_t value = sizeof(T) < 256 ? 4096 / sizeof(T) : 16;
};

// deque iterator

template <class T, class Ref, class Ptr>
struct deque_iterator : public iterator<random_access_iterator_tag, T> {
    typedef deque_iterator<T, T&, T*>             iterator;
    typedef deque_iterator<T, const T&, const T*> const_iterator;
    typedef deque_iterator                        self;

    typedef T            value_type;
    typedef Ptr          pointer;
    typedef Ref          reference;
    typedef size_t       size_type;
    typedef ptrdiff_t    difference_type;
    typedef T*           value_pointer;
    typedef T**          map_pointer;

    static const size_type buffer_size = deque_buf_size<T>::value;

    value_pointer cur;    // current element
    value_pointer first;  // first element of the buffer
    value_pointer last;   // last element of the buffer
    map_pointer   node;   

    deque_iterator() noexcept
        : cur(nullptr), first(nullptr), last(nullptr), node(nullptr) {}

    deque_iterator(value_pointer v, map_pointer n)
        : cur(v), first(*n), last(*n + buffer_size), node(n) {}

    deque_iterator(const iterator& rhs)
        : cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node) {}
    
    deque_iterator(iterator&& rhs) noexcept
        :cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node) {
        rhs.cur = nullptr;
        rhs.first = nullptr;
        rhs.last = nullptr;
        rhs.node = nullptr;
    }

    deque_iterator(const const_iterator& rhs)
        :cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node) {}

    self& operator=(const iterator& rhs) {
        if (this != &rhs) {
            cur = rhs.cur;
            first = rhs.first;
            last = rhs.last;
            node = rhs.node;
        }
        return *this;
    }

    void set_node(map_pointer new_node) {
        node = new_node;
        first = *new_node;
        last = first + buffer_size;
    }

    reference operator*()  const { return *cur; }
    pointer   operator->() const { return cur; }

    difference_type operator-(const self& x) const {
        return static_cast<difference_type>(buffer_size) * (node - x.node)
            + (cur - first) - (x.cur - x.first);
    }

    self& operator++() {
        ++cur;
        if (cur == last) {
            set_node(node + 1);
            cur = first;
        }
        return *this;
    }

    self operator++(int) {
        self tmp = *this;
        ++*this;
        return tmp;
    }

    self& operator--() {
        if (cur == first) { 
            set_node(node - 1);
            cur = last;
        }
        
        --cur;
        return *this;
    }

    self operator--(int) {
        self tmp = *this;
        --*this;
        return tmp;
    }

    self& operator+=(difference_type n) {
        const auto offset = n + (cur - first);
        if (offset >= 0 && offset < static_cast<difference_type>(buffer_size)) {
            cur += n;
        }
        else {
            const auto node_offset = offset > 0
                ? offset / static_cast<difference_type>(buffer_size)
                : -static_cast<difference_type>((-offset - 1) / buffer_size) - 1;
            
            set_node(node + node_offset);
            cur = first + (offset - node_offset * static_cast<difference_type>(buffer_size));
        }
        return *this;
    } 

    self operator+(difference_type n) const {
        self tmp = *this;
        return tmp += n;
    }

    self& operator-=(difference_type n) {
        return *this += -n;
    }

    self operator-(difference_type n) const {
        self tmp = *this;
        return tmp -= n;
    }

    reference operator[](difference_type n) const { return *(*this + n); }

    bool operator==(const self& rhs) const { return cur == rhs.cur; }
    bool operator< (const self& rhs) const
    { return node == rhs.node ? (cur < rhs.cur) : (node < rhs.node); }
    bool operator!=(const self& rhs) const { return !(*this == rhs); }
    bool operator> (const self& rhs) const { return rhs < *this; }
    bool operator<=(const self& rhs) const { return !(rhs < *this); }
    bool operator>=(const self& rhs) const { return !(*this < rhs); }
};

template <class T>
class deque {
public:
    using allocator_type = TinySTL::allocator<T>;
    using data_allocator = TinySTL::allocator<T>;
    using map_allocator = TinySTL::allocator<T*>;

    typedef typename allocator_type::value_type      value_type;
    typedef typename allocator_type::pointer         pointer;
    typedef typename allocator_type::const_pointer   const_pointer;
    typedef typename allocator_type::reference       reference;
    typedef typename allocator_type::const_reference const_reference;
    typedef typename allocator_type::size_type       size_type;
    typedef typename allocator_type::difference_type difference_type;
    typedef pointer*                                 map_pointer;
    typedef const_pointer*                           const_map_pointer;

    typedef deque_iterator<T, T&, T*>                  iterator;
    typedef deque_iterator<T, const T&, const T*>      const_iterator;
    typedef TinySTL::reverse_iterator<iterator>        reverse_iterator;
    typedef TinySTL::reverse_iterator<const_iterator>  const_reverse_iterator;

    inline get_allocator() {
        return allocator_type();
    }

    static const size_type buffer_size = deque_buf_size<T>::value;

private:
    iterator       begin_;     
    iterator       end_;       
    map_pointer    map_;       
    size_type      map_size_;  

public:
    deque() {
        fill_init(0, value_type());
    }

    explicit deque(size_type n) {
        fill_init(n, value_type());
    }

    template <class IIter, typename std::enable_if<TinySTL::is_input_iterator<IIter>::value, int>::type = 0>
    deque(IIter first, IIter last) { 
        copy_init(first, last, iterator_category(first)); 
    }

    deque(std::initializer_list<value_type> ilist) {
        copy_init(ilist.begin(), ilist.end(), TinySTL::forward_iterator_base());
    }

    deque(const deque& rhs) {
        copy_init(rhs.begin(), rhs.end(), TinySTL::forward_iterator_base());
    }

    deque(deque&& rhs) noexcept :begin_(TinySTL::move(rhs.begin_)),
                                 end_(TinySTL::move(rhs.end_)),
                                 map_(rhs.map_),
                                 map_size_(rhs.map_size_) {
        rhs.map_ = nullptr;
        rhs.map_size_ = 0;
    }

    deque& operator=(const deque& rhs);
    deque& operator=(deque&& rhs);

    deque& operator=(std::initializer_list<value_type> ilist) {
        deque tmp(ilist);
        swap(tmp);
        return *this;
    }

    ~deque() {
        if (map_ != nullptr) {
            clear();
            data_allocator::deallocate(*begin_.node, buffer_size);
            *begin_.node = nullptr;
            map_allocator::deallocate(map_, map_size_);
            map_ = nullptr;
        }
    }

public:
    iterator         begin() noexcept { return begin_; }
    const_iterator   begin() const noexcept { return begin_; }
    iterator         end() noexcept { return end_; }
    const_iterator   end() const noexcept { return end_; }

    reverse_iterator        rbegin() noexcept { return reverse_iterator(end()); }
    const_reverse_iterator  rbegin() const noexcept { return reverse_iterator(end()); }
    reverse_iterator        rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator  rend() const noexcept { return reverse_iterator(begin()); }

    const_iterator         cbegin()  const noexcept { return begin(); }
    const_iterator         cend() const noexcept { return end(); }
    const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    const_reverse_iterator crend() const noexcept { return rend(); }

    bool      empty() const noexcept  { return begin() == end(); }
    size_type size() const noexcept  { return end_ - begin_; }
    size_type max_size() const noexcept  { return static_cast<size_type>(-1); }
    void      resize(size_type new_size) { resize(new_size, value_type()); }
    void      resize(size_type new_size, const value_type& value);
    void      shrink_to_fit() noexcept;

    reference operator[](size_type n) {
        MYSTL_DEBUG(n < size());
        return begin_[n];
    }

    const_reference operator[](size_type n) const {
        MYSTL_DEBUG(n < size());
        return begin_[n];
    }

    reference at(size_type n) { 
        THROW_OUT_OF_RANGE_IF(!(n < size()), "deque<T>::at() subscript out of range");
        return (*this)[n];
    }

    const_reference at(size_type n) const {
        THROW_OUT_OF_RANGE_IF(!(n < size()), "deque<T>::at() subscript out of range");
        return (*this)[n]; 
    }

    reference front() {
        MYSTL_DEBUG(!empty());
        return *begin();
    }

    const_reference front() const {
        MYSTL_DEBUG(!empty());
        return *begin();
    }

    reference back() {
        MYSTL_DEBUG(!empty());
        return *(end() - 1);
    }

    const_reference back() const {
        MYSTL_DEBUG(!empty());
        return *(end() - 1);
    }

    void assign(size_type n, const value_type& value) { fill_assign(n, value); }

    template <class IIter, typename std::enable_if< TinySTL::is_input_iterator<IIter>::value, int>::type = 0>
    void assign(IIter first, IIter last) { copy_assign(first, last, iterator_category(first)); }

    void assign(std::initializer_list<value_type> ilist) { 
        copy_assign(ilist.begin(), ilist.end(), TinySTL::forward_iterator_base{}); 
    }

    template <class ...Args>
    void emplace_front(Args&& ...args);

    template <class ...Args>
    void emplace_back(Args&& ...args);

    template <class ...Args>
    iterator emplace(iterator pos, Args&& ...args);

    void push_front(const value_type& value);
    void push_back(const value_type& value);

    void push_front(value_type&& value) { emplace_front(TinySTL::move(value)); }
    void push_back(value_type&& value)  { emplace_back(TinySTL::move(value)); }

    void pop_front();
    void pop_back();

    iterator insert(iterator position, const value_type& value);
    iterator insert(iterator position, value_type&& value);
    void insert(iterator position, size_type n, const value_type& value);

    template <class IIter, typename std::enable_if<TinySTL::is_input_iterator<IIter>::value, int>::type = 0>
    void insert(iterator position, IIter first, IIter last) { 
        insert_dispatch(position, first, last, iterator_category(first)); 
    }

    iterator erase(iterator position);
    iterator erase(iterator first, iterator last);
    void clear();

    void swap(deque& rhs) noexcept;
private:
    map_pointer create_map(size_type size);
    void create_buffer(map_pointer nstart, map_pointer nfinish);
    void destroy_buffer(map_pointer nstart, map_pointer nfinish);
 
    void map_init(size_type nelem);
    void fill_init(size_type n, const value_type& value);

    template <class IIter>
    void copy_init(IIter, IIter, input_iterator_tag);

    template <class FIter>
    void copy_init(FIter, FIter, forward_iterator_tag);

    void fill_assign(size_type n, const value_type& value);

    template <class IIter>
    void copy_assign(IIter first, IIter last, input_iterator_tag);

    template <class FIter>
    void copy_assign(FIter first, FIter last, forward_iterator_tag);

    template <class... Args>
    iterator insert_aux(iterator position, Args&& ...args);

    void fill_insert(iterator position, size_type n, const value_type& x);

    template <class FIter>
    void copy_insert(iterator, FIter, FIter, size_type);

    template <class IIter>
    void insert_dispatch(iterator, IIter, IIter, input_iterator_tag);

    template <class FIter>
    void insert_dispatch(iterator, FIter, FIter, forward_iterator_tag);

    void require_capacity(size_type n, bool front);
    void reallocate_map_at_front(size_type need);
    void reallocate_map_at_back(size_type need);
};

/*****************************************************************/

template <class T>
deque<T>& deque<T>::operator=(const deque& rhs) {
    if (this != &rhs) {
        const auto len = size();
        if (len >= rhs.size()) {
            erase(TinySTL::copy(rhs.begin_, rhs.end_, begin_), end_);
        } else {
            iterator mid = rhs.begin() + static_cast<difference_type>(len);
            TinySTL::copy(rhs.begin_, mid, begin_);
            insert(end_, mid, rhs.end_);
        }
    }
    return *this;
}

template <class T>
deque<T>& deque<T>::operator=(deque&& rhs) {
    clear();
    begin_ = TinySTL::move(rhs.begin_);
    end_ = TinySTL::move(rhs.end_);
    map_ = rhs.map_;
    map_size_ = rhs.map_size_;
    rhs.map_ = nullptr;
    rhs.map_size_ = 0;
    return *this;
}

template <class T>
void deque<T>::resize(size_type new_size, const value_type& value) {
    const auto len = size();
    if (new_size < len) {
        erase(begin_ + new_size, end_);
    } else {
        insert(end_, new_size - len, value);
    }
}

template <class T>
void deque<T>::shrink_to_fit() noexcept {
    for (auto cur = map_; cur < begin_.node; ++cur) {
        data_allocator::deallocate(*cur, buffer_size);
        *cur = nullptr;
    }
    for (auto cur = end_.node + 1; cur < map_ + map_size_; ++cur)
    {
        data_allocator::deallocate(*cur, buffer_size);
        *cur = nullptr;
    }
}

template <class T>
template <class ...Args>
void deque<T>::emplace_front(Args&& ...args) {
    if (begin_.cur != begin_.first) {
        data_allocator::construct(begin_.cur - 1, TinySTL::forward<Args>(args)...);
        --begin_.cur;
    } else {
        require_capacity(1, true);
        try {
            --begin_;
            data_allocator::construct(begin_.cur, TinySTL::forward<Args>(args)...);
        } catch (...) {
            ++begin_;
            throw;
        }
    }
}

template <class T>
template <class ...Args>
void deque<T>::emplace_back(Args&& ...args) {
    if (end_.cur != end_.last - 1) {
        data_allocator::construct(end_.cur, TinySTL::forward<Args>(args)...);
        ++end_.cur;
    } else {
        require_capacity(1, false);
        data_allocator::construct(end_.cur, TinySTL::forward<Args>(args)...);
        ++end_;
    }
}

template <class T>
void deque<T>::push_front(const value_type& value) {
    if (begin_.cur != begin_.first) {
        data_allocator::construct(begin_.cur - 1, value);
        --begin_.cur;
    } else {
        require_capacity(1, true);
        try {
            --begin_;
            data_allocator::construct(begin_.cur, value);
        } catch (...) {
            ++begin_;
            throw;
        }
    }
}

template <class T>
void deque<T>::push_back(const value_type& value) {
    if (end_.cur != end_.last - 1) {
        data_allocator::construct(end_.cur, value);
        ++end_.cur;
    } else {
        require_capacity(1, false);
        data_allocator::construct(end_.cur, value);
        ++end_;
    }
}

template <class T>
void deque<T>::pop_front() {
    MYSTL_DEBUG(!empty());
    if (begin_.cur != begin_.last - 1) {
        data_allocator::destroy(begin_.cur);
        ++begin_.cur;
    } else {
        data_allocator::destroy(begin_.cur);
        ++begin_;
        destroy_buffer(begin_.node - 1, begin_.node - 1);
    }
}

template <class T>
void deque<T>::pop_back() {
    MYSTL_DEBUG(!empty());
    if (end_.cur != end_.first) {
        --end_.cur;
        data_allocator::destroy(end_.cur);
    } else {
        --end_;
        data_allocator::destroy(end_.cur);
        destroy_buffer(end_.node + 1, end_.node + 1);
    }
}

template <class T>
typename deque<T>::iterator
deque<T>::insert(iterator position, const value_type& value) {
    if (position.cur == begin_.cur) {
        push_front(value);
        return begin_;
    } else if (position.cur == end_.cur) {
        push_back(value);
        auto tmp = end_;
        --tmp;
        return tmp;
    } else {
        return insert_aux(position, value);
    }
}

template <class T>
typename deque<T>::iterator
deque<T>::insert(iterator position, value_type&& value) {
    if (position.cur == begin_.cur) {
        emplace_front(TinySTL::move(value));
        return begin_;
    } else if (position.cur == end_.cur) {
        emplace_back(TinySTL::move(value));
        auto tmp = end_;
        --tmp;
        return tmp;
    } else {
        return insert_aux(position, TinySTL::move(value));
    }
}

template <class T>
void deque<T>::insert(iterator position, size_type n, const value_type& value) {
    if (position.cur == begin_.cur) {
        require_capacity(n, true);
        auto new_begin = begin_ - n;
        TinySTL::uninitialized_fill_n(new_begin, n, value);
        begin_ = new_begin;
    } else if (position.cur == end_.cur) {
        require_capacity(n, false);
        auto new_end = end_ + n;
        TinySTL::uninitialized_fill_n(end_, n, value);
        end_ = new_end;
    } else {
        fill_insert(position, n, value);
    }
}

template <class T>
typename deque<T>::iterator
deque<T>::erase(iterator position) {
    auto next = position;
    ++next;
    const size_type elems_before = position - begin_;
    if (elems_before < (size() / 2)) {
        TinySTL::copy_backward(begin_, position, next);
        pop_front();
    } else {
        TinySTL::copy(next, end_, position);
        pop_back();
    }
    return begin_ + elems_before;
}

template <class T>
typename deque<T>::iterator
deque<T>::erase(iterator first, iterator last)
{
    if (first == begin_ && last == end_) {
        clear();
        return end_;
    } else {
        const size_type len = last - first;
        const size_type elems_before = first - begin_;

        if (elems_before < ((size() - len) / 2)) {
            TinySTL::copy_backward(begin_, first, last);
            auto new_begin = begin_ + len;
            data_allocator::destroy(begin_.cur, new_begin.cur);
            begin_ = new_begin;
        } else {
            TinySTL::copy(last, end_, first);
            auto new_end = end_ - len;
            data_allocator::destroy(new_end.cur, end_.cur);
            end_ = new_end;
        }
        return begin_ + elems_before;
    }
}

template <class T>
void deque<T>::clear() {
  for (map_pointer cur = begin_.node + 1; cur < end_.node; ++cur) {
    data_allocator::destroy(*cur, *cur + buffer_size);
  }

  if (begin_.node != end_.node) { 
    TinySTL::destroy(begin_.cur, begin_.last);
    TinySTL::destroy(end_.first, end_.cur);
  } else {
    TinySTL::destroy(begin_.cur, end_.cur);
  }
  
  shrink_to_fit();
  end_ = begin_;
}
