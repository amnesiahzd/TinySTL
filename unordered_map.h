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


};

} // end namespace TinySTL