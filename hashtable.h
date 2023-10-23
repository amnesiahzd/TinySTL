#pragma once

#include <initializer_list>

#include "algo.h"
#include "exceptdef.h"
#include "functional.h"
#include "vector"
#include "util.h"

namespace TinySTL {
template <class T>
struct hashtable_node {
    hashtable_node* next;
    T               value;

    hashtable_node() = default;
    hashtable_node(const T& v) : next(nullptr), value(n) {}

    hashtable_node(const hashtable_node& node) : next(node.next), value(node.value) {}
    hashtable_node(hashtable_node&& node) : next(node.next), value(TinySTL::move(node.value)) {
        node.next = nullptr;
    }
};

template <class T, bool>
struct ht_value_traits_imp {
    template <class Ty>
    static const T& get_key(const Ty& value) {
        return value;
    }

    template <class Ty>
    static const T& get_value(const Ty& value) {
        return value;
    }
};

template <class T>
struct ht_value_traits_imp<class T, true> {
    typedef typename std::remove_cv<typename T::first_type>::type key_type;
    typedef typename T::second_type                               mapped_type;
    typedef T                                                     value_type;

    template <class Ty>
    static const key_type& get_key(const Ty& value) {
        return value.first;
    }

    template <class Ty>
    static const value_type& get_value(const Ty& value) {
        return value;
    }
};

template <class T>
struct ht_value_traits {
    static constexpr bool is_map = TinySTL::is_pair<T>::value;
    
    using value_traits_type = ht_value_traits_imp<T, is_map>;

    using key_type = value_traits_type::key_type;
    using mapped_type = value_traits_type::mapped_type;
    using value_type= value_traits_type::value_type;

    template <class Ty>
    static const key_type& get_key(const Ty& value) {
        return value_traits_type::get_key(value);
    }

    template <class Ty>
    static const value_type& get_value(const Ty& value) {
        return value_traits_type::get_value(value);
    }
};

// forward declaration
template <class T, class HashFun, class KeyEqual>
class hashtable;

template <class T, class HashFun, class KeyEqual>
struct ht_iterator;

template <class T, class HashFun, class KeyEqual>
struct ht_const_iterator;

template <class T>
struct ht_local_iterator;

template <class T>
struct ht_const_local_iterator;

// ht_iterator
template <class T, class Hash, class KeyEqual>
struct ht_iterator_base : public TinySTL::iterator<TinySTL::forward_iterator_base, T> {
    using hashtable         = TinySTL::hashtable<T, Hash, KeyEqual>;
    using base              = ht_iterator_base<T, Hash, KeyEqual>;
    using iterator          = TinySTL::ht_iterator<T, Hash, KeyEqual>;
    using const_iterator    = TinySTL::ht_const_iterator<T, Hash, KeyEqual>;
    using node_ptr          = hashtable_node<T>*;
    using contain_ptr       = hashtable*;
    using const_node_ptr    = const node_ptr;
    using const_contain_ptr = const contain_ptr;
    using size_type         = size_t;
    using difference_type   = ptrdiff_t;

    node_ptr     node;   // current node
    contain_ptr  ht; // link to container

    bool operator==(const base& rhs) const {
        return node == rhs.node;
    }

    bool operator!=(const base& rhs) const {
        return node != rhs.node;
    }
};

/**
 * When typename depends on the template type, 
 * it is recommended to use typename to qualify it.
*/

template <class T, class Hash, class KeyEqual>
struct ht_iterator : public ht_iterator_base<T, Hash, KeyEqual> {
    using base           = ht_iterator_base<T, Hash, KeyEqual>;
    using hashtable      = typename base::hashtable;
    using iterator       = typename base::iterator;
    using const_iterator = typename base::const_iterator;
    using node_ptr       = typename base::node_ptr;
    using contain_ptr    = typename base::contain_ptr;

    using value_traits = ht_value_traits<T>;
    using value_type   = T;
    using pointer      = value_type*;
    using reference    = value_type&;

    using base::node;
    using base::ht;

    ht_iterator() = default;
    ht_iterator(node_ptr n, contain_ptr t) {
        node = n;
        ht = t;
    }
    ht_iterator(const iterator& rhs) {
        node = rhs.node;
        ht = rhs.ht;
    }
    ht_iterator(const const_iterator& rhs) {
        node = rhs.node;
        ht = rhs.ht;
    }
    iterator& operator=(const iterator& rhs) {
        if (this != &rhs) {
            node = rhs.node;
            ht = rhs.ht;
        }
        return *this;
    }
    iterator& operator=(const const_iterator& rhs) {
        if (this != &rhs) {
            node = rhs.node;
            ht = rhs.ht;
        }
        return *this;
    }

    reference operator*() const {
        return node->value;
    }

    pointer operator->() const {
        return &(operator*());
    }

    iterator& operator++() {
        MYSTL_DEBUG(node != nullptr);
        const node_ptr old = node;

        node = node->next;
        if (node == nullptr) { // go to next bucket
            auto index = ht->hash(value_traits::get_key(old->value));
            while (!node && ++index < ht->_bucket_size) {
                node = ht->_buckets[index];
            }
        }
        return *this;
    }

    iterator operator++(int) {
        iterator tmp = *this;
        ++*this;
        return tmp;
    }
};

template <class T, class Hash, class KeyEqual>
struct ht_const_iterator : public ht_iterator_base<T, Hash, KeyEqual> {
    using base           = ht_iterator_base<T, Hash, KeyEqual>;
    using hashtable      = typename base::hashtable;
    using iterator       = typename base::iterator;
    using const_iterator = typename base::const_iterator;
    using node_ptr       = typename base::const_node_ptr;
    using contain_ptr    = typename base::const_contain_ptr;

    using value_traits = ht_value_traits<T>;
    using value_type   = T;
    using pointer      = const T*;
    using reference    = const T&;

    using base::node;
    using base::ht;

    ht_const_iterator() = default;
    ht_const_iterator(node_ptr n, contain_ptr t) {
        node = n;
        ht = t;
    }
    ht_const_iterator(const iterator& rhs) {
        node = rhs.node;
        ht = rhs.ht;
    }
    ht_const_iterator(const const_iterator& rhs) {
        node = rhs.node;
        ht = rhs.ht;
    }
    const_iterator& operator=(const iterator& rhs) {
        if (this != &rhs) {
            node = rhs.node;
            ht = rhs.ht;
        }
        return *this;
    }
    const_iterator& operator=(const const_iterator& rhs) {
        if (this != &rhs) {
            node = rhs.node;
            ht = rhs.ht;
        }
        return *this;
    }

    reference operator*() const { 
        return node->value; 
    }
    pointer operator->() const { 
        return &(operator*()); 
    }

    const_iterator& operator++() {
        MYSTL_DEBUG(node != nullptr);
        const node_ptr old = node;
        node = node->next;
        
        if (node == nullptr) { 
            auto index = ht->hash(value_traits::get_key(old->value));
            while (!node && ++index < ht->_bucket_size) {
                node = ht->_buckets[index];
            }
        }
        return *this;
    }

    const_iterator operator++(int) { // Postfix increment operator
        const_iterator tmp = *this;
        ++*this;
        return tmp;
    }
};

template <class T>
struct ht_local_iterator : public TinySTL::iterator<TinySTL::forward_iterator_base, T> {
    using value_type              = T;
    using pointer                 = value_type*;
    using reference               = value_type&;
    using size_type               = size_t;
    using difference_type         = ptrdiff_t;
    using node_ptr                = hashtable_node<T>*;
    using self                    = ht_local_iterator<T>;
    using local_iterator          = ht_local_iterator<T>;
    using const_local_iterator    = ht_const_local_iterator<T>;

    node_ptr node;

    ht_local_iterator(node_ptr n) : node(n) {}
    ht_local_iterator(const local_iterator& rhs) : node(rhs.node) {}

    ht_local_iterator(const const_local_iterator& rhs) : node(rhs.node) {}

    reference operator*() const { 
        return node->value; 
    }
    pointer   operator->() const { 
        return &(operator*()); 
    }

    self& operator++() {
        MYSTL_DEBUG(node != nullptr);
        node = node->next;
        return *this;
    }
    
    self operator++(int) {
        self tmp(*this);
        ++*this;
        return tmp;
    }

    bool operator==(const self& other) const { return node == other.node; }
    bool operator!=(const self& other) const { return node != other.node; }
};

template <class T>
struct ht_const_local_iterator :public TinySTL::iterator<TinySTL::forward_iterator_tag, T> {
    using value_type = T;
    using pointer = const value_type*;
    using reference = const value_type&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    using node_ptr = const hashtable_node<T>*;
    using self = ht_const_local_iterator<T>;
    using local_iterator = ht_local_iterator<T>;
    using const_local_iterator = ht_const_local_iterator<T>;


    node_ptr node;

    ht_const_local_iterator(node_ptr n) : node(n) {}
    ht_const_local_iterator(const local_iterator& rhs) : node(rhs.node) {}
    ht_const_local_iterator(const const_local_iterator& rhs) : node(rhs.node) {}

    reference operator*() const { 
        return node->value; 
    }
    pointer operator->() const { return &(operator*()); }

    self& operator++() {
        MYSTL_DEBUG(node != nullptr);
        node = node->next;
        return *this;
    }

    self operator++(int) {
        self tmp(*this);
        ++*this;
        return tmp;
    }

    bool operator==(const self& other) const { return node == other.node; }
    bool operator!=(const self& other) const { return node != other.node; }
};

#if (_MSC_VER && _WIN64) || ((__GNUC__ || __clang__) &&__SIZEOF_POINTER__ == 8)
#define SYSTEM_64 1
#else
#define SYSTEM_32 1
#endif

#ifdef SYSTEM_64

#define PRIME_NUM 99

static constexpr size_t ht_prime_list[] = {
    101ull, 173ull, 263ull, 397ull, 599ull, 907ull, 1361ull, 2053ull, 3083ull,
    4637ull, 6959ull, 10453ull, 15683ull, 23531ull, 35311ull, 52967ull, 79451ull,
    119179ull, 178781ull, 268189ull, 402299ull, 603457ull, 905189ull, 1357787ull,
    2036687ull, 3055043ull, 4582577ull, 6873871ull, 10310819ull, 15466229ull,
    23199347ull, 34799021ull, 52198537ull, 78297827ull, 117446801ull, 176170229ull,
    264255353ull, 396383041ull, 594574583ull, 891861923ull, 1337792887ull,
    2006689337ull, 3010034021ull, 4515051137ull, 6772576709ull, 10158865069ull,
    15238297621ull, 22857446471ull, 34286169707ull, 51429254599ull, 77143881917ull,
    115715822899ull, 173573734363ull, 260360601547ull, 390540902329ull, 
    585811353559ull, 878717030339ull, 1318075545511ull, 1977113318311ull, 
    2965669977497ull, 4448504966249ull, 6672757449409ull, 10009136174239ull,
    15013704261371ull, 22520556392057ull, 33780834588157ull, 50671251882247ull,
    76006877823377ull, 114010316735089ull, 171015475102649ull, 256523212653977ull,
    384784818980971ull, 577177228471507ull, 865765842707309ull, 1298648764060979ull,
    1947973146091477ull, 2921959719137273ull, 4382939578705967ull, 6574409368058969ull,
    9861614052088471ull, 14792421078132871ull, 22188631617199337ull, 33282947425799017ull,
    49924421138698549ull, 74886631708047827ull, 112329947562071807ull, 168494921343107851ull,
    252742382014661767ull, 379113573021992729ull, 568670359532989111ull, 853005539299483657ull,
    1279508308949225477ull, 1919262463423838231ull, 2878893695135757317ull, 4318340542703636011ull,
    6477510814055453699ull, 9716266221083181299ull, 14574399331624771603ull, 18446744073709551557ull
};

#else

#define PRIME_NUM 44
static constexpr size_t ht_prime_list[] = {
    101u, 173u, 263u, 397u, 599u, 907u, 1361u, 2053u, 3083u, 4637u, 6959u, 
    10453u, 15683u, 23531u, 35311u, 52967u, 79451u, 119179u, 178781u, 268189u,
    402299u, 603457u, 905189u, 1357787u, 2036687u, 3055043u, 4582577u, 6873871u,
    10310819u, 15466229u, 23199347u, 34799021u, 52198537u, 78297827u, 117446801u,
    176170229u, 264255353u, 396383041u, 594574583u, 891861923u, 1337792887u,
    2006689337u, 3010034021u, 4294967291u // bug: origin code here is a comma
};
#endif

inline size_t ht_next_prime (size_t n) {
    const size_t* first = ht_prime_list;
    const size_t* last = ht_prime_list + PRIME_NUM;
    const size_t* pos = TinySTL::lower_bound(first, last, n);
    return pos == last ? *(last - 1) : *pos;
}

template <class T, class Hash, class KeyEqual>
class hashtable {
//simplify the access
friend struct TinySTL::ht_iterator<T, Hash, KeyEqual>;
friend struct TinySTL::ht_const_iterator<T, Hash, KeyEqual>;

public:
    using value_traits = ht_value_traits<T>;
    using key_type     = typename value_traits::key_type;
    using mapped_type  = typename value_traits::mapped_type;
    using value_type   = typename value_traits::value_type;
    using hasher       = Hash;
    using key_equal    = KeyEqual;

    using node_type   = hashtable_node<T>;
    using node_ptr    = node_type*;
    using bucket_type = TinySTL::vector<node_ptr>;

    using allocator_type = TinySTL::allocator<T>;
    using data_allocator = TinySTL::allocator<T>;
    using node_allocator = TinySTL::allocator<node_type>;

    using pointer         = typename allocator_type::pointer;
    using const_pointer   = typename allocator_type::const_pointer;
    using reference       = typename allocator_type::reference;
    using const_reference = typename allocator_type::const_reference;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;

    using iterator             = TinySTL::ht_iterator<T, Hash, KeyEqual>;
    using const_iterator       = TinySTL::ht_const_iterator<T, Hash, KeyEqual>;
    using local_iterator       = TinySTL::ht_local_iterator<T>;
    using const_local_iterator = TinySTL::ht_const_local_iterator<T>;

    allocator_type get_allocator() const {
        return allocator_type();
    }

private:
    bucket_type _buckets;
    size_type   _bucket_size;
    size_type   _size;
    float       _mlf;
    hasher      _hash;
    key_equal   _equal;

private:
    // key_type is the type removed cv

    bool is_equal(const key_type& key1, const key_type key2) {
        return _equal(key1, key2);
    }

    bool is_equal(const key_type& key1, const key_type key2) const { // It seems that it cannot be reloaded
        return _equal(key1, key2);
    }

    const_iterator M_cit(node_ptr node) const noexcept {
        return const_iterator(node, const_cast<hashtable*>(this));
    }

    iterator M_begin() noexcept {
        for (size_type n = 0; n < _bucket_size; ++n) {
            if (_buckets[n]) {
                return iterator(_buckets[n], this);
            }
        }
        return iterator(nullptr, this);
    }

    const_iterator M_begin() const noexcept {
        for (size_type n = 0; n < _bucket_size; ++n) {
            if (_buckets[n]) {
                return M_cit(_buckets[n]);
            }
        }
        return M_cit(nullptr);
    }
public:
    explicit hashtable(size_type bucket_count, 
                       const Hash& hash = Hash(), 
                       const KeyEqual& equal = KeyEqual()) 
                        : _size(0), _mlf(1.0f), _hash(hash), _equal(equal) {
        init(bucket_count);
    }

    template <class Iter, typename std::enable_if<TinySTL::is_input_iterator<Iter>::value, int>::type = 0>
    hashtable(Iter first, 
              Iter last, 
              size_type bucket_count, 
              const Hash& hash = Hash(), 
              const KeyEqual& equal = KeyEqual()) 
                : _size(TinySTL::distance(first, last)), _mlf(1.0f), _hash(hash), equal_(equal) {
        init(TinySTL::max(bucket_count, static_cast<size_type>(TinySTL::distance(first, last))))
    }

    hashtable(const hashtable& rhs) 
        : _hash(rhs._hash), equal_(rhs.equal_) {
        copy_init(rhs);
    }

    hashtable(hashtable&& rhs) noexcept
        : _bucket_size(rhs._bucket_size), 
          _size(rhs._size),
          _mlf(rhs._mlf),
          _hash(rhs._hash),
          equal_(rhs.equal_) {
        _buckets = TinySTL::move(rhs._buckets);
        rhs._bucket_size = 0;
        rhs._size = 0;
        rhs._mlf = 0.0f;
    }

    hashtable& operator=(const hashtable& rhs);
    hashtable& operator=(hashtable&& rhs) noexcept;

    ~hashtable() { 
        clear(); 
    }

    iterator begin() noexcept { 
        return M_begin(); 
    }
    const_iterator begin() const noexcept { 
        return M_begin(); 
    }
    iterator end() noexcept { 
        return iterator(nullptr, this); 
    }
    const_iterator end() const noexcept { 
        return M_cit(nullptr); 
    }
    
    const_iterator cbegin() const noexcept { 
        return begin(); 
    }
    const_iterator cend() const noexcept { 
        return end(); 
    }

    bool empty() const noexcept {
        return 0 == _size;
    }

    size_type size() const noexcept {
        return _size;
    }

    size_type max_size() const noexcept { 
        return static_cast<size_type>(-1); 
    }

    template <class ...Args>
    iterator emplace_multi(Args&& ...args);

    template <class ...Args>
    pair<iterator, bool> emplace_unique(Args&& ...args);

    template <class ...Args>
    iterator emplace_multi_use_hint(const_iterator /*hint*/, Args&& ...args) { 
        return emplace_multi(TinySTL::forward<Args>(args)...); 
    }

    template <class ...Args>
    iterator emplace_unique_use_hint(const_iterator /*hint*/, Args&& ...args) { 
        return emplace_unique(TinySTL::forward<Args>(args)...).first; 
    }

    iterator insert_multi_noresize(const value_type& value);
    pair<iterator, bool> insert_unique_noresize(const value_type& value);

    iterator insert_multi(const value_type& value) {
        rehash_if_need(1);
        return insert_multi_noresize(value);
    }

    iterator insert_multi(value_type&& value) { 
        return emplace_multi(TinySTL::move(value)); 
    }

    pair<iterator, bool> insert_unique(const value_type& value) {
        rehash_if_need(1);
        return insert_unique_noresize(value);
    }

    pair<iterator, bool> insert_unique(value_type&& value) { 
        return emplace_unique(TinySTL::move(value)); 
    }

    iterator insert_multi_use_hint(const_iterator /*hint*/, const value_type& value) { 
        return insert_multi(value); 
    }

    iterator insert_multi_use_hint(const_iterator /*hint*/, value_type&& value) { 
        return emplace_multi(TinySTL::move(value)); 
    }

    iterator insert_unique_use_hint(const_iterator /*hint*/, const value_type& value) { 
        return insert_unique(value).first; 
    }
    iterator insert_unique_use_hint(const_iterator /*hint*/, value_type&& value) { 
        return emplace_unique(TinySTL::move(value)); 
    }

    template <class InputIter>
    void insert_multi(InputIter first, InputIter last) { 
        copy_insert_multi(first, last, iterator_category(first)); 
    }

    template <class InputIter>
    void insert_unique(InputIter first, InputIter last) { 
        copy_insert_unique(first, last, iterator_category(first)); 
    }

    void erase(const_iterator position);
    void erase(const_iterator first, const_iterator last);

    size_type erase_multi(const key_type& key);
    size_type erase_unique(const key_type& key);

    void clear();
    void swap(hashtable& rhs) noexcept;

    size_type count(const key_type& key) const;

    iterator find(const key_type& key);
    const_iterator find(const key_type& key) const;

    pair<iterator, iterator> equal_range_multi(const key_type& key);
    pair<const_iterator, const_iterator> equal_range_multi(const key_type& key) const;

    pair<iterator, iterator> equal_range_unique(const key_type& key);
    pair<const_iterator, const_iterator> equal_range_unique(const key_type& key) const;

    // Bucket interface
    local_iterator begin(size_type n) noexcept { 
        MYSTL_DEBUG(n < _size);
        return _buckets[n];
    }
    const_local_iterator begin(size_type n) const noexcept { 
        MYSTL_DEBUG(n < _size);
        return _buckets[n];
    }
    const_local_iterator cbegin(size_type n) const noexcept { 
        MYSTL_DEBUG(n < _size);
        return _buckets[n];
    }

    local_iterator end(size_type n) noexcept { 
        MYSTL_DEBUG(n < _size);
        return nullptr; 
    }
    const_local_iterator end(size_type n) const noexcept { 
        MYSTL_DEBUG(n < _size);
        return nullptr; 
    }
    const_local_iterator cend(size_type n) const noexcept {
        MYSTL_DEBUG(n < _size);
        return nullptr; 
    }

    size_type bucket_count() const noexcept { 
        return _bucket_size; 
    }

    size_type max_bucket_count() const noexcept { 
        return ht_prime_list[PRIME_NUM - 1]; 
    }

    size_type bucket_size(size_type n) const noexcept;
    size_type bucket(const key_type& key) const { 
        return hash(key); 
    }

    float load_factor() const noexcept { 
        return _bucket_size != 0 ? (float)_size / _bucket_size : 0.0f; 
    }

    float max_load_factor() const noexcept { 
        return _mlf; 
    }
    void max_load_factor(float ml) {
        THROW_OUT_OF_RANGE_IF(ml != ml || ml < 0, "invalid hash load factor");
        _mlf = ml;
    }

    void rehash(size_type count);

    void reserve(size_type count) { 
        rehash(static_cast<size_type>((float)count / max_load_factor() + 0.5f)); 
    }

    hasher hash_fcn() const { return _hash; }
    key_equal key_eq()   const { return equal_; }

private:
    void init(size_type n);
    void copy_init(const hashtable& ht);

    template  <class ...Args>
    node_ptr  create_node(Args&& ...args);
    void      destroy_node(node_ptr n);

    size_type next_size(size_type n) const;
    size_type hash(const key_type& key, size_type n) const;
    size_type hash(const key_type& key) const;
    void      rehash_if_need(size_type n);

    template <class InputIter>
    void copy_insert_multi(InputIter first, InputIter last, TinySTL::input_iterator_base);
    template <class ForwardIter>
    void copy_insert_multi(ForwardIter first, ForwardIter last, TinySTL::forward_iterator_base);
    template <class InputIter>
    void copy_insert_unique(InputIter first, InputIter last, TinySTL::input_iterator_base);
    template <class ForwardIter>
    void copy_insert_unique(ForwardIter first, ForwardIter last, TinySTL::forward_iterator_base);

    pair<iterator, bool> insert_node_unique(node_ptr np);
    iterator             insert_node_multi(node_ptr np);

    // bucket operator
    void replace_bucket(size_type bucket_count);
    void erase_bucket(size_type n, node_ptr first, node_ptr last);
    void erase_bucket(size_type n, node_ptr last);

    // comparision
    bool equal_to_multi(const hashtable& other);
    bool equal_to_unique(const hashtable& other);
};

template <class T, class Hash, class KeyEqual>
hashtable<T, Hash, KeyEqual>& hashtable<T, Hash, KeyEqual>::operator=(const hashtable& rhs) {
    if (this != &rhs) {
        hashtable tmp(rhs);
        swap(tmp);
    }

    return *this;
}

template <class T, class Hash, class KeyEqual>
hashtable<T, Hash, KeyEqual>& hashtable<T, Hash, KeyEqual>::operator=(hashtable&& rhs) noexcept {
    hashtable tmp(TinySTL::move(rhs));
    swap(tmp);
    return *this;
}

template <class T, class Hash, class KeyEqual>
template <class ...Args>
typename hashtable<T, Hash, KeyEqual>::iterator hashtable<T, Hash, KeyEqual>::emplace_multi(Args&& ...args) {
    auto np = create_node(TinySTL::forward<Args>(args)...);
    try {
        if ((float)(_size + 1) > (float)_bucket_size * max_load_factor())
        rehash(_size + 1);
    } catch (...) {
        destroy_node(np);
        throw;
    }
    return insert_node_multi(np);
}

template <class T, class Hash, class KeyEqual>
template <class ...Args>
pair<typename hashtable<T, Hash, KeyEqual>::iterator, bool> 
hashtable<T, Hash, KeyEqual>::emplace_unique(Args&& ...args) {
    auto np = create_node(TinySTL::forward<Args>(args)...);
    try {
        if ((float)(_size + 1) > (float)_bucket_size * max_load_factor())
        rehash(_size + 1);
    } catch (...) {
        destroy_node(np);
        throw;
    }

    return insert_node_unique(np);
}

template <class T, class Hash, class KeyEqual>
pair<typename hashtable<T, Hash, KeyEqual>::iterator, bool>
hashtable<T, Hash, KeyEqual>::
insert_unique_noresize(const value_type& value) {
    const auto n = hash(value_traits::get_key(value));
    auto first = _buckets[n];
    for (auto cur = first; cur; cur = cur->next) {
        if (is_equal(value_traits::get_key(cur->value), value_traits::get_key(value)))
        return TinySTL::make_pair(iterator(cur, this), false);
    }
    // 让新节点成为链表的第一个节点
    auto tmp = create_node(value);  
    tmp->next = first;
    _buckets[n] = tmp;
    ++_size;
    return TinySTL::make_pair(iterator(tmp, this), true);
}

template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::iterator
hashtable<T, Hash, KeyEqual>::insert_multi_noresize(const value_type& value) {
    const auto n = hash(value_traits::get_key(value));
    auto first = _bucket[n];
    auto tmp = create_node(value);

    for (auto cur = first; cur; cur = cur->next) {
        if (is_equal(value_traits::get_key(cur->value), value_traits::get_key(value))) {
            tmp->next = cur->next;
            cur->next = tmp;
            ++_size;
            return iterator(tmp, this);
        }
    }

    tmp->next = first;
    _buckets[n] = tmp;
    ++_size;
    return iterator(tmp, this);
}

template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::erase(const_iterator position) {
    auto p = position.node;
    if (p) {
        const auto n = hash(value_traits::get_key(p->value));
        auto cur = _buckets[n];
        if (cur == p) {
            _bucket[n] = cur->next;
            destroy_node(cur);
            --_size;
        } else {
            auto next = cur->next;
            while (next) {
                if (next == p) {
                    cur->next = next->next;
                    destroy_node(next);
                    --_size;
                    break;
                } else {
                    cur = next;
                    next = cur->next;
                }
            }
        }
    }
}

template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::erase(const_iterator first, const_iterator last) {
    if (first.node == last.node){
        return;
    }
    auto first_bucket = first.node ? hash(value_traits::get_key(first.node->value)) : _bucket_size;
    auto last_bucket = last.node ? hash(value_traits::get_key(last.node->value)) : _bucket_size;
    if (first_bucket == last_bucket) { // 如果在 bucket 在同一个位置
        erase_bucket(first_bucket, first.node, last.node);
    }
    else {
        erase_bucket(first_bucket, first.node, nullptr);
        for (auto n = first_bucket + 1; n < last_bucket; ++n) {
            if(_buckets[n] != nullptr) {
                erase_bucket(n, nullptr);
            }
            if (last_bucket != _bucket_size) {
                erase_bucket(last_bucket, last.node);
            }
        }
    }
}

template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::size_type
hashtable<T, Hash, KeyEqual>::erase_multi(const key_type& key) {
    auto p = equal_range_multi(key);
    if (p.first.node != nullptr)
    {
        erase(p.first, p.second);
        return TinySTL::distance(p.first, p.second);
    }
    return 0;
}

template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::size_type
hashtable<T, Hash, KeyEqual>::erase_unique(const key_type& key) {
    const auto n = hash(key);
    auto first = _buckets[n];
    if (first) {
        if (is_equal(value_traits::get_key(first->value), key)) {
            _buckets[n] = first->next;
            destroy_node(first);
            --_size;
            return 1;
        }
        else {
            auto next = first->next;
            while (next) {
                if (is_equal(value_traits::get_key(next->value), key)) {
                    first->next = next->next;
                    destroy_node(next);
                    --_size;
                    return 1;
                }
                first = next;
                next = first->next;
            }
        }
    }
    return 0;
}

template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::clear() { // consider using init empty 
  if (_size != 0) {
        for (size_type i = 0; i < _bucket_size; ++i) {
            node_ptr cur = _buckets[i];
            while (cur != nullptr) {
                node_ptr next = cur->next;
                destroy_node(cur);
                cur = next;
            }
            _buckets[i] = nullptr;
        }
        _size = 0;
    }
}

template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::size_type
hashtable<T, Hash, KeyEqual>::bucket_size(size_type n) const noexcept {
    size_type result = 0;
    for (auto cur = _buckets[n]; cur; cur = cur->next) {
        ++result;
    }
    return result;
}

template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::rehash(size_type count) {
    auto n = ht_next_prime(count);
    if (n > _bucket_size) {
        replace_bucket(n);
    } else {
        if ((float)_size / (float)n < max_load_factor() - 0.25f &&
            (float)n < (float)_bucket_size * 0.75)  // worth rehash
        {
            replace_bucket(n);
        }
    }
}

template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::iterator
hashtable<T, Hash, KeyEqual>::find(const key_type& key) {
    const auto n = hash(key);
    node_ptr first = _buckets[n];
    for (; first && !is_equal(value_traits::get_key(first->value), key); first = first->next) {}
    return iterator(first, this);
}

// cannot overload correctly
template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::const_iterator
hashtable<T, Hash, KeyEqual>::find(const key_type& key) const {
    const auto n = hash(key);
    node_ptr first = _buckets[n];
    for (; first && !is_equal(value_traits::get_key(first->value), key); first = first->next) {}
    return M_cit(first);
}


template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::size_type
hashtable<T, Hash, KeyEqual>::count(const key_type& key) const {
    const auto n = hash(key);
    size_type result = 0;
    for (node_ptr cur = _buckets[n]; cur; cur = cur->next) {
        if (is_equal(value_traits::get_key(cur->value), key))
        ++result;
    }
    return result;
}

template <class T, class Hash, class KeyEqual>
pair<typename hashtable<T, Hash, KeyEqual>::iterator, typename hashtable<T, Hash, KeyEqual>::iterator>
hashtable<T, Hash, KeyEqual>::equal_range_multi(const key_type& key) {
    const auto n = hash(key);
    for (node_ptr first = _buckets[n]; first; first = first->next) {
        if (is_equal(value_traits::get_key(first->value), key)) { 
            for (node_ptr second = first->next; second; second = second->next) {
                if (!is_equal(value_traits::get_key(second->value), key))
                return TinySTL::make_pair(iterator(first, this), iterator(second, this));
            }

            for (auto m = n + 1; m < _bucket_size; ++m) { // if all the list is equal, check the next list
                if (_buckets[m])
                return TinySTL::make_pair(iterator(first, this), iterator(_buckets[m], this));
            }

            return TinySTL::make_pair(iterator(first, this), end());
        }
    }
    return TinySTL::make_pair(end(), end());
}

template <class T, class Hash, class KeyEqual>
pair<typename hashtable<T, Hash, KeyEqual>::const_iterator, 
     typename hashtable<T, Hash, KeyEqual>::const_iterator>
hashtable<T, Hash, KeyEqual>::equal_range_multi(const key_type& key) const {
    const auto n = hash(key);
    for (node_ptr first = _buckets[n]; first; first = first->next) {
        if (is_equal(value_traits::get_key(first->value), key)) {
            for (node_ptr second = first->next; second; second = second->next) {
                if (!is_equal(value_traits::get_key(second->value), key))
                return TinySTL::make_pair(M_cit(first), M_cit(second));
            }
            for (auto m = n + 1; m < _bucket_size; ++m) { // 整个链表都相等，查找下一个链表出现的位置
                if (_buckets[m]) {
                    return TinySTL::make_pair(M_cit(first), M_cit(_buckets[m]));
                }
            }
        return TinySTL::make_pair(M_cit(first), cend());
        }
    }
    return TinySTL::make_pair(cend(), cend());
}

template <class T, class Hash, class KeyEqual>
pair<typename hashtable<T, Hash, KeyEqual>::iterator,
     typename hashtable<T, Hash, KeyEqual>::iterator>
hashtable<T, Hash, KeyEqual>::equal_range_unique(const key_type& key) {
    const auto n = hash(key);
    for (node_ptr first = _buckets[n]; first; first = first->next) {
        if (is_equal(value_traits::get_key(first->value), key)) {
            if (first->next)
                return TinySTL::make_pair(iterator(first, this), iterator(first->next, this));
            for (auto m = n + 1; m < _bucket_size; ++m) { // 整个链表都相等，查找下一个链表出现的位置
                if (_buckets[m])
                return TinySTL::make_pair(iterator(first, this), iterator(_buckets[m], this));
            }
            return TinySTL::make_pair(iterator(first, this), end());
        }
    }
    return TinySTL::make_pair(end(), end());
}

template <class T, class Hash, class KeyEqual>
pair<typename hashtable<T, Hash, KeyEqual>::const_iterator,
     typename hashtable<T, Hash, KeyEqual>::const_iterator>
hashtable<T, Hash, KeyEqual>::equal_range_unique(const key_type& key) const {
    const auto n = hash(key);
    for (node_ptr first = _buckets[n]; first; first = first->next) {
        if (is_equal(value_traits::get_key(first->value), key)) {
            if (first->next)
                return TinySTL::make_pair(M_cit(first), M_cit(first->next));
            for (auto m = n + 1; m < _bucket_size; ++m) { // 整个链表都相等，查找下一个链表出现的位置
                if (_buckets[m])
                    return TinySTL::make_pair(M_cit(first), M_cit(_buckets[m]));
            }
            return TinySTL::make_pair(M_cit(first), cend());
        }
    }
    return TinySTL::make_pair(cend(), cend());
}

template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::swap(hashtable& rhs) noexcept {
    if (this != &rhs) {
        _buckets.swap(rhs._buckets);
        TinySTL::swap(_bucket_size, rhs._bucket_size);
        TinySTL::swap(_size, rhs._size);
        TinySTL::swap(_mlf, rhs._mlf);
        TinySTL::swap(_hash, rhs._hash);
        TinySTL::swap(_equal, rhs._equal);
    }
}

template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::init(size_type n) {
  const auto bucket_nums = next_size(n);
  try {
    _buckets.reserve(bucket_nums);
    _buckets.assign(bucket_nums, nullptr);
  } catch (...) {
    _bucket_size = 0;
    _size = 0;
    throw;
  }
  _bucket_size = _buckets.size();
}

template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::copy_init(const hashtable& ht) {
    _bucket_size = 0;
    _buckets.reserve(ht._bucket_size);
    _buckets.assign(ht._bucket_size, nullptr);
    try {
        for (size_type i = 0; i < ht._bucket_size; ++i) {
            node_ptr cur = ht._buckets[i];
            if (cur) { // 如果某 bucket 存在链表
                auto copy = create_node(cur->value);
                _buckets[i] = copy;
                for (auto next = cur->next; next; cur = next, next = cur->next) {  //复制链表
                    copy->next = create_node(next->value);
                    copy = copy->next;
                }
                copy->next = nullptr;
            }
        }
        _bucket_size = ht._bucket_size;
        _mlf = ht._mlf;
        _size = ht._size;
    } catch (...) {
        clear();
    }
}

template <class T, class Hash, class KeyEqual>
template <class ...Args>
typename hashtable<T, Hash, KeyEqual>::node_ptr
hashtable<T, Hash, KeyEqual>::create_node(Args&& ...args) {
    node_ptr tmp = node_allocator::allocate(1);
    try {
        data_allocator::construct(TinySTL::address_of(tmp->value), TinySTL::forward<Args>(args)...);
        tmp->next = nullptr;
    } catch (...) {
        node_allocator::deallocate(tmp);
        throw;
    }
    return tmp;
}

template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::destroy_node(node_ptr node) {
    data_allocator::destroy(TinySTL::address_of(node->value));
    node_allocator::deallocate(node);
    node = nullptr;
}

template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::size_type
hashtable<T, Hash, KeyEqual>::next_size(size_type n) const
{
  return ht_next_prime(n);
}

// hash 函数
template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::size_type
hashtable<T, Hash, KeyEqual>::
hash(const key_type& key, size_type n) const
{
  return hash_(key) % n;
}

template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::size_type
hashtable<T, Hash, KeyEqual>::hash(const key_type& key) const {
    return hash_(key) % _bucket_size;
}

// rehash_if_need 函数
template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::
rehash_if_need(size_type n)
{
  if (static_cast<float>(_size + n) > (float)_bucket_size * max_load_factor())
    rehash(_size + n);
}

// copy_insert
template <class T, class Hash, class KeyEqual>
template <class InputIter>
void hashtable<T, Hash, KeyEqual>::copy_insert_multi(InputIter first, InputIter last, TinySTL::input_iterator_base) {
    rehash_if_need(TinySTL::distance(first, last));
    for (; first != last; ++first)
        insert_multi_noresize(*first);
}

template <class T, class Hash, class KeyEqual>
template <class InputIter>
void hashtable<T, Hash, KeyEqual>::
copy_insert_unique(InputIter first, InputIter last, TinySTL::input_iterator_base)
{
  rehash_if_need(TinySTL::distance(first, last));
  for (; first != last; ++first)
    insert_unique_noresize(*first);
}

template <class T, class Hash, class KeyEqual>
template <class ForwardIter>
void hashtable<T, Hash, KeyEqual>::
copy_insert_unique(ForwardIter first, ForwardIter last, TinySTL::forward_iterator_base)
{
  size_type n = TinySTL::distance(first, last);
  rehash_if_need(n);
  for (; n > 0; --n, ++first)
    insert_unique_noresize(*first);
}

// insert_node 函数
template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::iterator
hashtable<T, Hash, KeyEqual>::
insert_node_multi(node_ptr np)
{
  const auto n = hash(value_traits::get_key(np->value));
  auto cur = _buckets[n];
  if (cur == nullptr)
  {
    _buckets[n] = np;
    ++_size;
    return iterator(np, this);
  }
  for (; cur; cur = cur->next)
  {
    if (is_equal(value_traits::get_key(cur->value), value_traits::get_key(np->value)))
    {
      np->next = cur->next;
      cur->next = np;
      ++_size;
      return iterator(np, this);
    }
  }
  np->next = _buckets[n];
  _buckets[n] = np;
  ++_size;
  return iterator(np, this);
}

template <class T, class Hash, class KeyEqual>
pair<typename hashtable<T, Hash, KeyEqual>::iterator, bool>
hashtable<T, Hash, KeyEqual>::
insert_node_unique(node_ptr np)
{
  const auto n = hash(value_traits::get_key(np->value));
  auto cur = _buckets[n];
  if (cur == nullptr)
  {
    _buckets[n] = np;
    ++_size;
    return TinySTL::make_pair(iterator(np, this), true);
  }
  for (; cur; cur = cur->next)
  {
    if (is_equal(value_traits::get_key(cur->value), value_traits::get_key(np->value)))
    {
      return TinySTL::make_pair(iterator(cur, this), false);
    }
  }
  np->next = _buckets[n];
  _buckets[n] = np;
  ++_size;
  return TinySTL::make_pair(iterator(np, this), true);
}

template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::
replace_bucket(size_type bucket_count)
{
  bucket_type bucket(bucket_count);
  if (_size != 0)
  {
    for (size_type i = 0; i < _bucket_size; ++i)
    {
      for (auto first = _buckets[i]; first; first = first->next)
      {
        auto tmp = create_node(first->value);
        const auto n = hash(value_traits::get_key(first->value), bucket_count);
        auto f = bucket[n];
        bool is_inserted = false;
        for (auto cur = f; cur; cur = cur->next)
        {
          if (is_equal(value_traits::get_key(cur->value), value_traits::get_key(first->value)))
          {
            tmp->next = cur->next;
            cur->next = tmp;
            is_inserted = true;
            break;
          }
        }
        if (!is_inserted)
        {
          tmp->next = f;
          bucket[n] = tmp;
        }
      }
    }
  }
  _buckets.swap(bucket);
  _bucket_size = _buckets.size();
}

template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::
erase_bucket(size_type n, node_ptr first, node_ptr last)
{
  auto cur = _buckets[n];
  if (cur == first)
  {
    erase_bucket(n, last);
  }
  else
  {
    node_ptr next = cur->next;
    for (; next != first; cur = next, next = cur->next) {}
    while (next != last)
    {
      cur->next = next->next;
      destroy_node(next);
      next = cur->next;
      --_size;
    }
  }
}

template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::
erase_bucket(size_type n, node_ptr last)
{
  auto cur = _buckets[n];
  while (cur != last)
  {
    auto next = cur->next;
    destroy_node(cur);
    cur = next;
    --_size;
  }
  _buckets[n] = last;
}

// equal_to 函数
template <class T, class Hash, class KeyEqual>
bool hashtable<T, Hash, KeyEqual>::equal_to_multi(const hashtable& other)
{
  if (_size != other._size)
    return false;
  for (auto f = begin(), l = end(); f != l;)
  {
    auto p1 = equal_range_multi(value_traits::get_key(*f));
    auto p2 = other.equal_range_multi(value_traits::get_key(*f));
    if (TinySTL::distance(p1.first, p1.last) != TinySTL::distance(p2.first, p2.last) ||
        !TinySTL::is_permutation(p1.first, p2.last, p2.first, p2.last))
      return false;
    f = p1.last;
  }
  return true;
}

template <class T, class Hash, class KeyEqual>
bool hashtable<T, Hash, KeyEqual>::equal_to_unique(const hashtable& other)
{
  if (_size != other._size)
    return false;
  for (auto f = begin(), l = end(); f != l; ++f)
  {
    auto res = other.find(value_traits::get_key(*f));
    if (res.node == nullptr || *res != *f)
      return false;
  }
  return true;
}

// 重载 TinySTL 的 swap
template <class T, class Hash, class KeyEqual>
void swap(hashtable<T, Hash, KeyEqual>& lhs,
          hashtable<T, Hash, KeyEqual>& rhs) noexcept
{
  lhs.swap(rhs);
}



} // end namespace TinySTL 
