#!/usr/bin/env python

import random
import subprocess
import sys
import time


def enable_if(x):
    if x == 0:
        return """
            template<bool, class T> struct enable_if { using type = T; };
            template<class T> struct enable_if<false, T> {};
            template<bool B, class T = void> using enable_if_t = typename enable_if<B, T>::type;
        """
    elif x == 1:
        return """
            template<bool, class> struct enable_if {};
            template<class T> struct enable_if<true, T> { using type = T; };
            template<bool B, class T = void> using enable_if_t = typename enable_if<B, T>::type;
        """
    elif x == 2:
        return """
            template<bool> struct metabase { template<class T> using type = T; };
            template<> struct metabase<false> {};
            template<bool B, class T = void> using enable_if_t = typename metabase<B>::template type<T>;
        """
    elif x == 3:
        return """
            template<bool> struct metabase {};
            template<> struct metabase<true> { template<class T> using type = T; };
            template<bool B, class T = void> using enable_if_t = typename metabase<B>::template type<T>;
        """


def generate_fs(x, y, overloads):
    common_stuff = """
        template<int N> struct priority_tag : priority_tag<N-1> {};
        template<> struct priority_tag<0> {};
        template<int N> struct A {};
    """
    if y == 0:
        return enable_if(x) + common_stuff + "\n".join([
            "template<int N> auto f(priority_tag<%d>) -> enable_if_t<N==%d, A<%d>>;" % (i, i, i)
            for i in range(overloads)
        ])
    elif y == 1:
        return enable_if(x) + common_stuff + "\n".join([
            "template<int N, class = enable_if_t<N==%d>> auto f(priority_tag<%d>) -> A<%d>;" % (i, i, i)
            for i in range(overloads)
        ])
    elif y == 2:
        return enable_if(x) + common_stuff + "\n".join([
            "template<int N, enable_if_t<N==%d, int> = 0> auto f(priority_tag<%d>) -> A<%d>;" % (i, i, i)
            for i in range(overloads)
        ])
    elif y == 3:
        return common_stuff + "\n".join([
            "template<int N> requires (N==%d) auto f(priority_tag<%d>) -> A<%d>;" % (i, i, i)
            for i in range(overloads)
        ])


def generate_file(x, y, overloads):
    return generate_fs(x, y, overloads) + """
        void test() {
    """ + "\n".join([
        "f<%d>(priority_tag<%d>{});" % (i, overloads)
        for i in range(overloads)
    ]) + """
        }
    """


def benchmark_one_compile(x, y, ovf):
    compiler = 'g++'
    compiler_args = ['-std=c++20', '-O2']
    iterations = 3
    overloads = ovf()
    with open('x.cpp', 'w') as f:
        f.write(generate_file(x, y, overloads))
    elapsed = 0.0
    for i in range(iterations):
        start = time.time()
        subprocess.call([compiler] + compiler_args + ['-c', 'x.cpp'])
        elapsed += (time.time() - start)
    print('%d %d %d %.2f' % (x, y, overloads, elapsed / iterations))
    sys.stdout.flush()


def benchmark_all_compiles(ovf):
    for x in [0, 1, 2, 3]:
        for y in [0, 1, 2]:
            benchmark_one_compile(x, y, ovf)
    benchmark_one_compile(0, 3, ovf)


def scatter_one_plot(plt, x, y, data):
    import numpy as np
    marker = 'o' if y == 3 else '^v<>'[x]
    color = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728'][y]
    overloads = [d[2] for d in data if d[0] == x and d[1] == y]
    timings = [d[3] for d in data if d[0] == x and d[1] == y]
    marker, color, label = {
        (0, 0): ('^', '#1f77b4', 'enable_if, false specialization, return type SFINAE'),
        (1, 0): ('<', '#1f77b4', 'enable_if, true specialization, return type SFINAE'),
        (2, 0): ('v', '#1f77b4', 'SCARY, false specialization, return type SFINAE'),
        (3, 0): ('>', '#1f77b4', 'SCARY, true specialization, return type SFINAE'),
        (0, 1): ('^', '#ff7f0e', 'enable_if, false specialization, type parameter'),
        (1, 1): ('<', '#ff7f0e', 'enable_if, true specialization, type parameter'),
        (2, 1): ('v', '#ff7f0e', 'SCARY, false specialization, type parameter'),
        (3, 1): ('>', '#ff7f0e', 'SCARY, true specialization, type parameter'),
        (0, 2): ('^', '#2ca02c', 'enable_if, false specialization, value parameter'),
        (1, 2): ('<', '#2ca02c', 'enable_if, true specialization, value parameter'),
        (2, 2): ('v', '#2ca02c', 'SCARY, false specialization, value parameter'),
        (3, 2): ('>', '#2ca02c', 'SCARY, true specialization, value parameter'),
        (0, 3): ('o', '#d62728', 'C++20 requires-clause'),
    }[(x, y)]
    plt.scatter(overloads, timings, marker=marker, color=color, label=label, s=10)

    a, b, c, d, e = np.polyfit(overloads, timings, 4)
    xs = list(range(overloads[0], overloads[-1]))
    plt.plot(xs, [a*x*x*x*x + b*x*x*x + c*x*x + d*x + e for x in xs], color=color, linewidth=1)


if sys.argv[1] == '--graph':
    import matplotlib.pyplot as plt
    fields = [line.split(' ') for line in sys.stdin.readlines()]
    data = [(int(f[0]), int(f[1]), int(f[2]), float(f[3])) for f in fields]
    for x in [0, 1, 2, 3]:
        for y in [0, 1, 2]:
            scatter_one_plot(plt, x, y, data)
    scatter_one_plot(plt, 0, 3, data)
    # plt.yscale('log')
    plt.legend()
    plt.show()
elif sys.argv[1] == '--time':
    for i in range(25, 425, 25):
        benchmark_all_compiles(lambda: i + random.randint(-10, 10))
else:
    print('Usage: ./x.py --time | ./x.py --graph')
