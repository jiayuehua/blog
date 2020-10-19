// Compile with "g++ -std=c++17 -O2 integral-fission.cpp"
// Run with "./a.out"
//
// The program will run forever until you Ctrl+C it, producing terms
// to standard output in the form "index term tree", one per line.
// You can pipe it through `cut -d ' ' -f 2` to extract just the terms.
//
// If you run it as "./a.out terms.txt", it'll remember its progress.
// If the specified file exists, it must contain all previously computed
// terms (starting from index 2). We'll trust whatever it says, parrot
// those lines to stdout, and then resume processing beyond the last
// term in the file.
// Newly found terms will be appended to the file.
// If the specified file doesn't exist, it will be created.

#include <cmath>
#include <fstream>
#include <iostream>
#include <queue>
#include <numeric>
#include <utility>
#include <vector>

using Integer = int;  // to compute term 29729, change this to "long long"

class Primes {
    std::vector<Integer> all_primes_;

    template<class T, class Container, class Compare>
    class PQ : public std::priority_queue<T, Container, Compare>
    {
        int swap_elements(int i, int j) {
            using std::swap;
            swap(this->c[i], this->c[j]);
            return j;
        }

        void sift_down() {
            using std::swap;
            int i = 0;
            int n = this->size();
            while (2*i+1 < n) {
                int lx = 2*i+1;
                int rx = 2*i+2;
                bool i_lessthan_lx = (lx < n && this->comp(this->c[i], this->c[lx]));
                bool i_lessthan_rx = (rx < n && this->comp(this->c[i], this->c[rx]));
                if (i_lessthan_lx && i_lessthan_rx) {
                    i = this->swap_elements(i, (this->comp(this->c[lx], this->c[rx]) ? rx : lx));
                } else if (i_lessthan_lx) {
                    i = this->swap_elements(i, lx);
                } else if (i_lessthan_rx) {
                    i = this->swap_elements(i, rx);
                } else {
                    break;
                }
            }
        }
    public:
        template<class... Args>
        void reemplace_top(Args&&... args) {
            this->c.front() = T(static_cast<Args&&>(args)...);
            this->sift_down();
        }
    };

template<class Int, Int Increment>
class iotarator {
    Int value = 0;
public:
    explicit iotarator() = default;
    explicit iotarator(Int v) : value(v) {}
    Int operator*() const { return value; }
    iotarator& operator++() { value += Increment; return *this; }
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
    template<class T> using min_heap = PQ<T, std::vector<T>, std::greater<>>;
    min_heap<pair> m_pq;

    void cross_off_multiples_of_prime(Int value) {
        if (value < std::numeric_limits<Int>::max() / value) {
            m_pq.emplace(value * value, value);
        }
    }

    bool is_already_crossed_off(Int value) {
        if (value != m_pq.top().next_crossed_off_value) {
            return false;
        } else {
            do {
                auto x = m_pq.top();
                m_pq.reemplace_top(x.next_crossed_off_value + x.prime_increment, x.prime_increment);
            } while (value == m_pq.top().next_crossed_off_value);
            return true;
        }
    }
};

public:
    void build_sieve(Integer limit) {
        if (limit <= 2) return;
        all_primes_.push_back(2);
        iotarator<Integer, 2> iota(3);
        sieverator<Integer, decltype(iota)> sieve(iota);
        Integer next_percentage_mark = limit / 100;
        for (Integer p : sieve) {
             if (p > next_percentage_mark) {
                 std::cerr << "Building prime sieve (" << ((100.0 * p) / limit) << "%)...\r" << std::flush;
                 next_percentage_mark += limit / 100;
            }
            if (p >= limit) break;
            all_primes_.push_back(p);
        }
        std::cerr << "\nDone building sieve\n";
    }
    void add_known_prime(Integer n) {
        assert((all_primes_.empty() && n == 2) || (n > all_primes_.back()));
        all_primes_.push_back(n);
    }
    std::vector<Integer> prime_factorize(Integer n) const {
        std::vector<Integer> factors;
        for (Integer p : all_primes_) {
            while (n % p == 0) {
                factors.push_back(p);
                n /= p;
            }
            if (n == 1) return factors;
            if (p * p > n) break;
        }
        factors.push_back(n);
        return factors;
    }
};
Primes g_primes;

auto factorize(Integer n) -> std::pair<Integer, Integer>
{
    if (n == 2 || n == 3 || n == 5) {
        return {1, n};
    }
    for (Integer f1 = std::sqrt(double(n)); true; --f1) {
        Integer f2 = n / f1;
        if (f1 * f2 == n) {
            return {f1, f2};
        }
    }
}

class Tree {
    int leafcount_ = 0;
    std::vector<bool> occupied_;
    std::vector<Integer> factors_;

    bool is_leaf(int idx) const {
        if (!occupied_[idx]) return false;
        if (2*idx+1 >= occupied_.size()) return true;
        return !occupied_[2*idx+1];
    }

    bool subtree_iso(int lx, const Tree& rhs, int rx) const {
        if (occupied_[lx] != rhs.occupied_[rx]) return false;
        if (is_leaf(lx)) {
            return rhs.is_leaf(rx);
        } else {
            if (rhs.is_leaf(rx)) return false;
            return (subtree_iso(2*lx+1, rhs, 2*rx+1) && subtree_iso(2*lx+2, rhs, 2*rx+2))
                || (subtree_iso(2*lx+1, rhs, 2*rx+2) && subtree_iso(2*lx+2, rhs, 2*rx+1));
        }
    }

    void subtree_print(std::ostream& os, int ix) const {
        if (is_leaf(ix)) {
            os << factors_[ix];
        } else {
            os << '(';
            this->subtree_print(os, 2*ix+1);
            os << ',';
            this->subtree_print(os, 2*ix+2);
            os << ')';
        }
    }

    void subtree_scan(std::istream& is, int ix) {
        while (std::isspace(is.peek())) {
            is.get();
        }
        if (is.peek() == '(') {
            is.get();
            subtree_scan(is, 2*ix+1);
            is.get();  // ','
            subtree_scan(is, 2*ix+2);
            is.get();  // ')'
            occupied_[ix] = true;
            factors_[ix] = factors_[2*ix+1] * factors_[2*ix+2];
        } else {
            if (ix >= occupied_.size()) {
                occupied_.resize(ix+1);
                factors_.resize(ix+1);
            }
            occupied_[ix] = true;
            is >> factors_[ix];
            leafcount_ += 1;
        }
    }

public:
    explicit Tree(Integer n) {
        occupied_.reserve(40);
        occupied_.push_back(true);
        factors_.reserve(40);
        factors_.push_back(n);
        for (int i=0; i < factors_.size(); ++i) {
            if (occupied_[i]) {
                auto [f1, f2] = factorize(factors_[i]);
                if (f1 != 1) {
                    occupied_.resize(2*i+3);
                    factors_.resize(2*i+3);
                    occupied_[2*i+1] = true;
                    occupied_[2*i+2] = true;
                    factors_[2*i+1] = f1;
                    factors_[2*i+2] = f2;
                } else {
                    leafcount_ += 1;
                }
            }
        }
    }

    Integer root() const {
        return factors_[0];
    }

    int leafcount() const {
        return leafcount_;
    }

    template<class F>
    void for_each_prime_leaf(F f) const {
        for (int i=0; i < occupied_.size(); ++i) {
            if (this->is_leaf(i)) {
                f(factors_[i]);
            }
        }
    }

    bool identical_shape(const Tree& rhs) const {
        return occupied_ == rhs.occupied_;
    }

    bool isomorphic_shape(const Tree& rhs) const {
        if (leafcount_ != rhs.leafcount_) {
            return false;
        }
        return subtree_iso(0, rhs, 0);
    }

    friend std::ostream& operator<<(std::ostream& os, const Tree& rhs) {
        rhs.subtree_print(os, 0);
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Tree& rhs) {
        rhs.leafcount_ = 0;
        rhs.subtree_scan(is, 0);
        return is;
    }
};

int main(int argc, char **argv) {
    int idx = 1;  // the OEIS sequence starts "1 2 4 8..." but we won't print the "1"
    Integer n = 1;

    std::vector<Tree> seen_of_leafcount[64];  // this is good up to 2^64
    int unseen_of_leafcount[] = {
        0, 1, 1, 2, // two distinct trees with 3 factors
        5, 14, 42, 132, 429, 1430, 4862,  // with 10 factors
        16796, 58786, 208012, 742900, 2674440, 9694845,
        35357670, 129644790, 477638700, 1767263190,
    };

    std::ofstream outfile;

    if (argc == 2) {
        std::ifstream input(argv[1]);
        auto tree = Tree(2);
        idx = 1;
        n = 1;
        while (input >> idx >> n >> tree) {
            std::cout << idx << " " << n << " " << tree << "\n";
            int lc = tree.leafcount();
            assert(lc < std::size(seen_of_leafcount));
            seen_of_leafcount[lc].push_back(std::move(tree));
            if (lc < std::size(unseen_of_leafcount)) {
                assert(unseen_of_leafcount[lc] > 0);
                unseen_of_leafcount[lc] -= 1;
            }
        }
        // Record all primes, up to and including n.
        g_primes.build_sieve(n+1);

        // Pick up where we left off.
        ++idx;
        ++n;
        std::cout << std::flush;
        outfile = std::ofstream(argv[1], std::ios_base::app);
    } else {
        // Start at the beginning.
        idx = 2;
        n = 2;
    }

    for ( ; true; ++n) {
        auto prime_factors = g_primes.prime_factorize(n);
        int lc = prime_factors.size();
        assert(lc >= 1);
        if (lc == 1) {
            g_primes.add_known_prime(n);
        }
        assert(lc < std::size(seen_of_leafcount));
        if (lc >= std::size(unseen_of_leafcount) || unseen_of_leafcount[lc] != 0) {
            auto tree = Tree(n);
            assert(tree.leafcount() == lc);
            bool is_novel = std::none_of(
                seen_of_leafcount[lc].begin(), seen_of_leafcount[lc].end(),
                [&](const Tree& t) { return t.identical_shape(tree); }
            );
            if (is_novel) {
                if (outfile) {
                    outfile << idx << " " << n << " " << tree << std::endl;
                }
                std::cout << idx << " " << n << " " << tree << std::endl;
                ++idx;
                seen_of_leafcount[lc].push_back(std::move(tree));
                if (lc < std::size(unseen_of_leafcount)) {
                    assert(unseen_of_leafcount[lc] > 0);
                    unseen_of_leafcount[lc] -= 1;
                }
            }
        }
    }
}
