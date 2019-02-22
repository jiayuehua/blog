#include <memory>

template<class T, int Cap>
class fixed_capacity_vector {
    int size_ = 0;
    alignas(T) char buffer_[Cap][sizeof(T)];
public:
    fixed_capacity_vector() = default;
    explicit fixed_capacity_vector(int initial_size) noexcept {
        for (int i=0; i < initial_size; ++i) {
            this->emplace_back();
        }
    }
    fixed_capacity_vector(const fixed_capacity_vector& rhs) noexcept {
        for (const auto& elt : rhs) {
            this->emplace_back(elt);
        }
    }
    fixed_capacity_vector(fixed_capacity_vector&& rhs) noexcept {
#if USE_RELOCATE_TO_MOVE
        size_ = rhs.size_;
        std::uninitialized_relocate(rhs.begin(), rhs.end(), this->begin());
        rhs.size_ = 0;
#else
        size_ = rhs.size_;
        std::uninitialized_move(rhs.begin(), rhs.end(), this->begin());
#endif
    }
    fixed_capacity_vector& operator=(const fixed_capacity_vector& rhs) = delete;
    ~fixed_capacity_vector() {
        for (int i=0; i < size_; ++i) {
            (*this)[i].~T();
        }
    }

    template<class... Args>
    void emplace_back(Args&&... args) {
        // assert(size_ < Cap);
        new ((void*)buffer_[size_]) T(std::forward<Args>(args)...);
        size_ += 1;
    }

    void erase(T *it) {
        assert(this->begin() <= it && it < this->end());
        size_ -= 1;
        for (int i = (it - begin()); i < size_; ++i) {
            (*this)[i] = std::move((*this)[i+1]);
        }
        (*this)[size_].~T();
    }

    const T& operator[](int i) const { return *(const T*)buffer_[i]; }
    T& operator[](int i) { return *(T*)buffer_[i]; }
    const T& back() const { return *(const T*)buffer_[size_-1]; }
    T& back() { return *(T*)buffer_[size_-1]; }
    const T *begin() const { return (const T*)buffer_[0]; }
    const T *end() const { return (const T*)buffer_[size_]; }
    T *begin() { return (T*)buffer_[0]; }
    T *end() { return (T*)buffer_[size_]; }
    int size() const { return size_; }
    bool empty() const { return size() == 0; }
};

fixed_capacity_vector<TYPE, 100>
test(fixed_capacity_vector<TYPE, 100> vec) {
    vec.emplace_back(nullptr);
    return vec;
}
