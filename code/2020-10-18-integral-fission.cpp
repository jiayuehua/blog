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
#include <utility>
#include <vector>

using Integer = int;  // make this "long long" if your numbers get too big

auto factorize(Integer n) -> std::pair<Integer, Integer>
{
    if (n == 2 || n == 3 || n == 5 || n == 7 || n == 11 || n == 13 || n == 17) {
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
    int leafcount_;
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
    explicit Tree(Integer n) : leafcount_(0) {
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
    std::vector<Tree> seen;
    std::ofstream outfile;

    if (argc == 2) {
        std::ifstream input(argv[1]);
        auto tree = Tree(2);
        idx = 1;
        n = 1;
        while (input >> idx >> n >> tree) {
            std::cout << idx << " " << n << " " << tree << "\n";
            seen.push_back(std::move(tree));
        }
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
        auto tree = Tree(n);
        bool is_novel = std::none_of(
            seen.begin(), seen.end(),
            [&](const Tree& t) { return t.identical_shape(tree); }
        );
        if (is_novel) {
            if (outfile) {
                outfile << idx << " " << n << " " << tree << std::endl;
            }
            std::cout << idx << " " << n << " " << tree << std::endl;
            ++idx;
            seen.push_back(std::move(tree));
        }
    }
}
