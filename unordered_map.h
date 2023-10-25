#pragma once
#include "functional.h"
#include "algo.h"
namespace TinySTL {

template <class Key, class Value, class Hash = TinySTL::hash<Key>, class Equal = TinySTL::equal_to<Key>>
class unordered_map {
private:
    using base_type = TinySTL::hashtable<TinySTL::pair<const Key, value>, Hash, KeyEqual>;
    base_type ht_;

public:
    using allocator_type = typename base_type::allocator_type;
    using key_type = typename base_type::key_type;
    using mapped_type = typename base_type::mapped_type;
    using value_type = typename base_type::value_type;
    using hasher = typename base_type::hasher;
    using key_equal = typename base_type::key_equal;

    using size_type = typename base_type::size_type;
    using difference_type = typename base_type::difference_type;
    using pointer = typename base_type::pointer;
    using const_pointer = typename base_type::const_pointer;
    using reference = typename base_type::reference;
    using const_reference = typename base_type::const_reference;

    using iterator = typename base_type::iterator;
    using const_iterator = typename base_type::const_iterator;
    using local_iterator = typename base_type::local_iterator;
    using const_local_iterator = typename base_type::const_local_iterator;

    allocator_type get_allocator() const { return ht_.get_allocator(); }

public:
    unordered_map() : ht_(100, Hash(), KeyEqual());
    explicit unordered_map(size_type bucket_count,
                           const Hash& hash = Hash(),
                           const KeyEqual& equal = KeyEqual())
        :ht_(bucket_count, hash, equal) {}

    template <class InputIterator>
    unordered_map(InputIterator first, InputIterator last,
                    const size_type bucket_count = 100,
                    const Hash& hash = Hash(),
                    const KeyEqual& equal = KeyEqual())
        : ht_(TinySTL::max(bucket_count, static_cast<size_type>(TinySTL::distance(first, last))), hash, equal) {
        for (; first != last; ++first)
            ht_.insert_unique_noresize(*first);
    }

    unordered_map(const unordered_map& rhs) 
        :ht_(rhs.ht_)  {}

    unordered_map(unordered_map&& rhs) noexcept
        :ht_(TinySTL::move(rhs.ht_)) {}

    unordered_map& operator=(const unordered_map& rhs) { 
        ht_ = rhs.ht_;
        return *this; 
    }

    unordered_map& operator=(unordered_map&& rhs) { 
        ht_ = TinySTL::move(rhs.ht_);
        return *this;
    }

    unordered_map& operator=(std::initializer_list<value_type> ilist) {
        ht_.clear();
        ht_.reserve(ilist.size());
        for (auto first = ilist.begin(), last = ilist.end(); first != last; ++first)
            ht_.insert_unique_noresize(*first);
        return *this;
    }

    ~unordered_map() = default;

    // code 122
    iterator begin() noexcept {
        return ht_.begin();
    }

    const_iterator begin() const noexcept {
        return ht_.begin();
    }

    iterator end() noexcept {
        return ht_.end();
    }

    const_iterator end() const noexcept {
        return ht_.end();
    }

    const_iterator cbegin() const noexcept { 
        return ht_.cbegin(); 
    }

    const_iterator cend() const noexcept { 
        return ht_.cend(); 
    }

    bool empty() const noexcept {
        return ht_.empty();
    }

    size_type size() const noexcept {
        return ht_.size();
    }

    size_type max_size() const noexcept {
        return ht_.max_size();
    }

    template <class ...Args>
    pair<iterator, bool> emplace(Args&& ...args) {
        return ht_.emplace_unique(TinySTL::forward<Args>(args)...);
    }

    template <class ...Args>
    iterator emplace_hint(const_iterator hint, Args&& ...args) {
        return ht_.emplace_unique_use_hint(hint, TinySTL::forward(args)...);
    }

    pair<iterator, bool> insert(const value_type& value) {
        return ht_.insert_unique(value);
    }

    pair<iterator, bool> insert(value_type&& value) { 
        return ht_.emplace_unique(TinySTL::move(value)); 
    }

    iterator insert(const_iterator hint, const value_type& value) { 
        return ht_.insert_unique_use_hint(hint, value); 
    }

    iterator insert(const_iterator hint, value_type&& value) { 
        return ht_.emplace_unique_use_hint(hint, TinySTL::move(value)); 
    }

    template <class InputIterator>
    void insert(InputIterator first, InputIterator last) { 
        sht_.insert_unique(first, last); 
    }

    void erase(iterator it) { 
        ht_.erase(it); 
    }
    void erase(iterator first, iterator last) { 
        ht_.erase(first, last); 
    }

    size_type erase(const key_type& key) { 
        return ht_.erase_unique(key); 
    }

    void clear() { 
        ht_.clear(); 
    }

    void swap(unordered_map& other) noexcept { 
        ht_.swap(other.ht_); 
    }

    // line 190
    mapped_type& at(const key_type& key) {
        iterator it = ht_.find(key);
        THROW_OUT_OF_RANGE_IF(it.node == nullptr, "unordered_map<Key, T> no such element exists");
        return it->second;
    }

    const mapped_type& at(const key_type& key) const {
        iterator it = ht_.find(key);
        THROW_OUT_OF_RANGE_IF(it.node == nullptr, "unordered_map<Key, T> no such element exists");
        return it->second;
    }

    mapped_type& operator[](const key_type& key) {
        iterator it = ht_.find(key);
        if (it.node == nullptr)
            it = ht_.emplace_unique(key, T{}).first;
        return it->second;
    }

    mapped_type& operator[](key_type&& key) {
        iterator it = ht_.find(key);
        if (it.node == nullptr)
            it = ht_.emplace_unique(mystl::move(key), T{}).first;
        return it->second;
    }

    size_type count(const key_type& key) const { 
        return ht_.count(key); 
    }

    iterator find(const key_type& key) { 
        return ht_.find(key); 
    }

    const_iterator find(const key_type& key) const { 
        return ht_.find(key); 
    }

    pair<iterator, iterator> equal_range(const key_type& key) { 
        return ht_.equal_range_unique(key); 
    }

    pair<const_iterator, const_iterator> equal_range(const key_type& key) const { 
        return ht_.equal_range_unique(key); 
    }

    local_iterator begin(size_type n) noexcept { 
        return ht_.begin(n); 
    }

    const_local_iterator begin(size_type n) const noexcept { 
        return ht_.begin(n); 
    }

    const_local_iterator cbegin(size_type n) const noexcept { 
        return ht_.cbegin(n); 
    }

    local_iterator end(size_type n) noexcept { 
        return ht_.end(n); 
    }

    const_local_iterator end(size_type n) const noexcept { 
        return ht_.end(n); 
    }
    const_local_iterator cend(size_type n) const noexcept { 
        return ht_.cend(n); 
    }

    size_type bucket_count() const noexcept { 
        return ht_.bucket_count(); 
    }

    size_type max_bucket_count() const noexcept { 
        return ht_.max_bucket_count(); 
    }

    size_type bucket_size(size_type n) const noexcept { 
        return ht_.bucket_size(n); 
    }

    size_type bucket(const key_type& key) const { 
        return ht_.bucket(key); 
    }

    float load_factor() const noexcept {
        return ht_.load_factor();
    }

    float max_load_factor() const noexcept {
        return ht_.max_load_factor();
    }

    void max_load_factor(float ml) { 
        ht_.max_load_factor(ml); 
    }

    void rehash(size_type count) { 
        ht_.rehash(count); 
    }

    void reserve(size_type count) { 
        ht_.reserve(count); 
    }

    hasher hash_fcn() const { 
        return ht_.hash_fcn(); 
    }
    key_equal key_eq() const { 
        return ht_.key_eq(); 
    }
public:
    friend bool operator==(const unordered_map& lhs, const unordered_map& rhs) {
        return lhs.ht_.equal_range_unique(rhs.ht_);
    }
    friend bool operator!=(const unordered_map& lhs, const unordered_map& rhs) {
        return !lhs.ht_.equal_range_unique(rhs.ht_);
    }


};

} // end namespace TinySTL