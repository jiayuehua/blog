#include <cassert>
#include <functional>
#include <stdio.h>
#include <queue>
#include <utility>

template<typename Int>
class iotarator {
    Int value = 0;
public:
    explicit iotarator() = default;
    explicit iotarator(Int v) : value(v) {}
    Int operator*() const { return value; }
    iotarator& operator++() { value += 1; return *this; }
    iotarator operator++(int) = delete;

    bool operator==(const iotarator& rhs) const {
        return value == rhs.value;
    }
    bool operator!=(const iotarator& rhs) const { return !(*this == rhs); }
};

template<class Int, class Iter>
class sieverator {
    Int m_current;
    Iter m_iter;

    explicit sieverator() {}  // used by .end()
public:
    explicit sieverator(Iter it) :
        m_current(*it),
        m_iter(it)
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
            ++m_iter;
            m_current = *m_iter;
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

    void cross_off_multiples_of_prime(Int value) {
        m_pq.emplace(value * value, value);
    }

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
};

int main()
{
    iotarator<int64_t> iota(2);
    sieverator<int64_t, decltype(iota)> sieve(iota);
    for (int p : sieve) {
        printf("%d\n", p);
    }
}
