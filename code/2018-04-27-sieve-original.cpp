#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <utility>

template<typename Int>
class iotarator {
    Int value = 0;
    Int step = 1;
public:
    explicit iotarator() = default;
    explicit iotarator(Int v) : value(v) {}
    explicit iotarator(Int v, Int s) : value(v), step(s) {}
    Int operator*() const { return value; }
    iotarator& operator++() { value += step; return *this; }
    iotarator operator++(int) { auto ret = *this; ++*this; return ret; }

    bool operator==(const iotarator& rhs) const {
        return value == rhs.value && step == rhs.step;
    }
    bool operator!=(const iotarator& rhs) const { return !(*this == rhs); }
};

template<class Int>
class sieverator {
    struct erased_iterator {
        virtual Int dereference() = 0;
        virtual void increment() = 0;
    };
    template<class It>
    class derived_iterator : public erased_iterator {
        It it;
    public:
        derived_iterator(It it) : it(std::move(it)) {}
        Int dereference() override { return *it; }
        void increment() override { ++it; }
    };

    Int m_current;
    std::unique_ptr<erased_iterator> m_ptr;

    explicit sieverator() {}  // used by .end()
public:
    template<class It>
    explicit sieverator(It it) :
        m_current(*it),
        m_ptr(std::make_unique<derived_iterator<It>>(std::move(it)))
    {}
    sieverator begin() { return std::move(*this); }
    sieverator end() const { return sieverator{}; }
    bool operator==(const sieverator&) const { return false; }
    bool operator!=(const sieverator&) const { return true; }

    Int operator*() const {
        return m_current;
    }

    sieverator& operator++() {
        cross_off_multiples_of_prime(m_current);
        do {
            m_ptr->increment();
            m_current = m_ptr->dereference();
        } while (is_already_crossed_off(m_current));
        return *this;
    }

    sieverator& operator++(int) = delete;

private:
    struct pair {
        Int next_crossed_off_value;
        Int prime_increment;
        explicit pair(Int a, Int b) : next_crossed_off_value(a), prime_increment(b) {}
        bool operator<(const pair& rhs) const {
            return next_crossed_off_value < rhs.next_crossed_off_value ? true
                 : next_crossed_off_value > rhs.next_crossed_off_value ? false
                 : prime_increment < rhs.prime_increment;
        }
        bool operator>(const pair& rhs) const { return rhs < *this; }
    };
    template<class T> using min_heap = std::priority_queue<T, std::vector<T>, std::greater<>>;
    min_heap<pair> m_pq;

    bool is_already_crossed_off(Int value) {
        if (value != m_pq.top().next_crossed_off_value) {
            return false;
        } else {
            do {
                auto x = m_pq.top();
                m_pq.pop();
                m_pq.emplace(x.next_crossed_off_value + x.prime_increment, x.prime_increment);
            } while (value == m_pq.top().next_crossed_off_value);
            return true;
        }
    }

    void cross_off_multiples_of_prime(Int value) {
        m_pq.emplace(value * value, value);
    }
};

int main()
{
    iotarator<__int128_t> iota(2);
    sieverator<__int128_t> sieve(iota);
    for (int p : sieve) {
        std::cout << p << std::endl;
    }
}
