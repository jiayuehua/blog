#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

using Element = int;

int g_total_swaps = 0;
int g_total_comparisons = 0;
int g_elements_that_moved_down_by_one = 0;
int g_elements_that_stayed_still = 0;

bool ElementIsLess(Element a, Element b) {
    g_total_comparisons += 1;
    return (a < b);
}

void SwapElements(Element& a, Element& b) {
    g_total_swaps += 1;
    std::swap(a, b);
}

void naive_bubble_sort(Element *a, int n)
{
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n-i-1; ++j) {
            if (ElementIsLess(a[j+1], a[j])) {
                SwapElements(a[j], a[j+1]);
            }
        }
    }
}

void knuth_bubble_sort(Element *a, int n)
{
    int water_level = n;
    while (water_level >= 2) {
        int t = 0;
        for (int j=0; j+1 < water_level; ++j) {
            if (ElementIsLess(a[j+1], a[j])) {
                SwapElements(a[j], a[j+1]);
                t = j+1;
            }
        }
        water_level = t;
    }
}

void cocktail_shaker_sort(Element *a, int n)
{
    int water_level = n;
    int rocks_level = 0;
    while (water_level > rocks_level) {
        int t = rocks_level;
        for (int j = rocks_level; j+1 < water_level; ++j) {
            if (ElementIsLess(a[j+1], a[j])) {
                SwapElements(a[j], a[j+1]);
                g_elements_that_moved_down_by_one += 1;
                t = j+1;
            } else {
                g_elements_that_stayed_still += 1;
            }
        }
        water_level = t;
        for (int j = water_level-1; j-1 >= rocks_level; --j) {
            if (ElementIsLess(a[j], a[j-1])) {
                SwapElements(a[j], a[j-1]);
                t = j;
            }
        }
        rocks_level = t;
    }
}

void shifting_shaker_sort(Element *a, int n)
{
    int water_level = n;
    int rocks_level = 0;
    int base = 0;
    Element pullout = 9999;
    while (water_level > rocks_level) {
        // Do the "bubble" phase. In this phase, the majority of elements
        // still in the water (i.e. neither in the airy top nor in the
        // rocky bottom) end up shifting down by 1.
        // The "in the water" elements are `a[rocks_level..water_level)`.
        // We're going to pretend that we are "moving" elements from `a`
        // into `b[rocks_level..water_level)`. Typically, a[i] moves to b[i-1].
        // So let's actually 1-index `b`; let's make its indices
        // `b[rocks_level+1..water_level+1)`. Now a[i] usually moves to b[i].
        // Now we overlay b[1..n] on a[1..n].
        // So a[rocks_level] gets read but never written.
        // And at the end, b[water_level] needs to get written.
        int t = rocks_level;
        SwapElements(pullout, a[rocks_level]);
        for (int j = rocks_level; j+1 < water_level; ++j) {
            if (ElementIsLess(a[j+1], pullout)) {
                t = j+1;
            } else {
                SwapElements(pullout, a[j+1]);
            }
        }
        int old_water_level = water_level;
        water_level = t;
        // Do the "rocks" phase. In this phase, the majority of elements
        // still in the water end up shifting up by 1.
        // So for example we're going to put b[water_level] into a[water_level-1],
        // which is convenient!
        for (int j = old_water_level-1; j >= water_level; --j) {
            SwapElements(pullout, a[j]);
        }

        for (int j = water_level-1; j >= rocks_level; --j) {
            if (j != rocks_level && ElementIsLess(pullout, a[j])) {
                t = j;
            } else {
                SwapElements(pullout, a[j]);
            }
        }
        rocks_level = t;
    }
}

struct Wrapped {
    Element e;
    //Wrapped(Wrapped&&) noexcept = default;
    //Wrapped& operator=(Wrapped&&) noexcept = default;
    friend void swap(Wrapped& a, Wrapped& b) {
        SwapElements(a.e, b.e);
    }
    friend bool operator<(const Wrapped& a, const Wrapped& b) {
        return ElementIsLess(a.e, b.e);
    }
};
void library_sort(Element *a, int n)
{
    std::sort((Wrapped*)a, (Wrapped*)a+n);
}

int main() {
    std::mt19937 g;
    std::uniform_int_distribution<int> dist(10, 99);
    for (int i=0; i < 100; ++i) {
        std::vector<int> v(100);
        std::generate(v.begin(), v.end(), [&](){ return dist(g); });
        {
            auto s = v;
            g_total_comparisons = 0;
            g_total_swaps = 0;
            naive_bubble_sort(s.data(), s.size());
            assert(std::is_sorted(s.begin(), s.end()) && std::is_permutation(s.begin(), s.end(), v.begin(), v.end()));
            std::cout << "Naïve: " << g_total_comparisons << "c " << g_total_swaps << "s - ";
        }
        {
            auto s = v;
            g_total_comparisons = 0;
            g_total_swaps = 0;
            knuth_bubble_sort(s.data(), s.size());
            assert(std::is_sorted(s.begin(), s.end()) && std::is_permutation(s.begin(), s.end(), v.begin(), v.end()));
            std::cout << "Bubble: " << g_total_comparisons << "c " << g_total_swaps << "s - ";
        }
        {
            auto s = v;
            g_total_comparisons = 0;
            g_total_swaps = 0;
            cocktail_shaker_sort(s.data(), s.size());
            assert(std::is_sorted(s.begin(), s.end()) && std::is_permutation(s.begin(), s.end(), v.begin(), v.end()));
            std::cout << "Shake: " << g_total_comparisons << "c " << g_total_swaps << "s - ";
        }
        {
            auto s = v;
            g_total_comparisons = 0;
            g_total_swaps = 0;
            shifting_shaker_sort(s.data(), s.size());
            assert(std::is_sorted(s.begin(), s.end()) && std::is_permutation(s.begin(), s.end(), v.begin(), v.end()));
            std::cout << "Shift: " << g_total_comparisons << "c " << g_total_swaps << "s - ";
        }
        {
            auto s = v;
            g_total_comparisons = 0;
            g_total_swaps = 0;
            library_sort(s.data(), s.size());
            assert(std::is_sorted(s.begin(), s.end()) && std::is_permutation(s.begin(), s.end(), v.begin(), v.end()));
            std::cout << "Std: " << g_total_comparisons << "c " << g_total_swaps << "s\n";
        }
    }
}
