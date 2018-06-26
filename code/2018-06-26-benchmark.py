import json
import matplotlib.pyplot as plt
import subprocess
import time

def generate_file(f, strategy, n):
    print >>f, '#include <string>'
    print >>f, '#include <vector>'

    if strategy == 0:
        for i in xrange(n):
            print >>f, 'const char *s%d = "s%d";' % (i, i)
    elif strategy == 1:
        for i in xrange(n):
            print >>f, 'std::string s%d = "s%d";' % (i, i)
    elif strategy == 2:
        for i in xrange(n):
            print >>f, 'std::string f%d() { return "s%d"; }' % (i, i)
    elif strategy == 3:
        print >>f, 'std::vector<std::string> foo() {'
        print >>f, '    std::vector<std::string> result;'
        for i in xrange(n):
            print >>f, '    result.emplace_back("s%d");' % i
        print >>f, '    return result;'
        print >>f, '}'
    elif strategy == 4:
        print >>f, 'std::vector<std::string> foo() {'
        print >>f, '    return {'
        for i in xrange(n):
            print >>f, '        "s%d",' % i
        print >>f, '    };'
        print >>f, '}'
    elif strategy == 5:
        print >>f, 'std::vector<std::string> foo() {'
        print >>f, '    static std::string a[%d] = {' % n
        for i in xrange(n):
            print >>f, '        "s%d",' % i
        print >>f, '    };'
        print >>f, '    return std::vector<std::string>(a, a+%d);' % n
        print >>f, '}'
    elif strategy == 6:
        print >>f, 'std::vector<std::string> foo() {'
        print >>f, '    static const char *a[%d] = {' % n
        for i in xrange(n):
            print >>f, '        "s%d",' % i
        print >>f, '    };'
        print >>f, '    return std::vector<std::string>(a, a+%d);' % n
        print >>f, '}'

def generate_ns():
    yield 10
    for i in xrange(100, 1000, 100):
        yield i
    for i in xrange(1000, 20000, 1000):
        yield i
    for i in xrange(10000, 100000, 10000):
        yield i

data = []
for n in generate_ns():
    for strategy in range(7):
        with open("x.cc", "w") as f:
            generate_file(f, strategy, n)
        start = time.time()
        subprocess.call(['clang++', '-std=c++11', '-c', 'x.cc'])
        elapsed = (time.time() - start)
        datapoint = {"s": strategy, "n": n, "t": elapsed}
        print '    %s,' % json.dumps(datapoint)
        data.append(datapoint)

lines = []
data = sorted(data, key=lambda d: d["n"])
for strategy in range(7):
    xs = [d["n"] for d in data if d["s"] == strategy]
    ys = [d["t"] for d in data if d["s"] == strategy]
    color = {
        0: '#ff0000',
        1: '#00ff00',
        2: '#0000ff',
        3: '#ff00ff',
        4: '#ffff00',
        5: '#00ffff',
        6: '#000000',
    }[strategy]
    line = plt.plot(xs, ys, color)
    lines.append(line[0])

plt.xlabel('vector size')
plt.ylabel('compile time (seconds)')
plt.legend(lines, (
    'char* variables',
    'string variables',
    'string functions',
    'repeated emplace',
    'initializer_list',
    'static array of string',
    'static array of char*',
))
plt.show()
