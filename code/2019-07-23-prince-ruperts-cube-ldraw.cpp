#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>

#ifndef SIDE
#define SIDE 13
#endif

using T = int;

struct Point {
    double x, y, z;

    constexpr double mag2() const {
        return x*x + y*y + z*z;
    }

    constexpr double dot(Point rhs) const {
        return x*rhs.x + y*rhs.y + z*rhs.z;
    }

    constexpr Point cross(Point rhs) const {
        return Point{y*rhs.z - z*rhs.y, z*rhs.x - x*rhs.z, x*rhs.y - y*rhs.x};
    }

    friend constexpr Point operator+(Point p, Point q) {
        return Point{p.x + q.x, p.y + q.y, p.z + q.z};
    }

    friend constexpr Point operator-(Point p, Point q) {
        return Point{p.x - q.x, p.y - q.y, p.z - q.z};
    }

    friend constexpr bool operator==(Point a, Point b) {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
};

static_assert(Point{1,0,0}.cross(Point{0,1,0}) == Point{0,0,1});

// These two points define the centerline of the hole.
#ifndef PETER
// Traditional solution, with the hole centered on the space diagonal.
static constexpr Point X1 = {0, 0, 0};
static constexpr Point X2 = {1, 1, 1};
#else
// Pieter Nieuwland's solution.
static constexpr Point X1 = {0.5, 0.5, 0.5};
static constexpr Point X2 = X1 + Point{6, 6, 3};
#endif

inline double signed_distance_to_vertical_plane(Point X0)
{
    // N is the direction of the vertical plane's normal vector.
    static constexpr Point N = {X2.y - X1.y, X1.x - X2.x, 0};
    static constexpr double a = N.x;
    static constexpr double b = N.y;
    static constexpr double c = N.z;
    static constexpr double d = -(a*X1.x + b*X1.y + c*X1.z);
    // The equation of the vertical plane is ax+by+cz+d = 0.
    static const double denominator = sqrt(a*a + b*b + c*c);
    double numerator = a*X0.x + b*X0.y + c*X0.z + d;
    return numerator / denominator;
}

inline double signed_distance_to_horizontal_plane(Point X0)
{
    // dir1 is the direction of the vertical plane's normal vector.
    static constexpr Point dir1 = {X2.y - X1.y, X1.x - X2.x, 0};
    // dir2 is the direction of the hole's center line.
    static constexpr Point dir2 = {X2.x - X1.x, X2.y - X1.y, X2.z - X1.z};
    // N is the direction of the horizontal plane's normal vector.
    static constexpr Point N = dir1.cross(dir2);
    static constexpr double a = N.x;
    static constexpr double b = N.y;
    static constexpr double c = N.z;
    static constexpr double d = -(a*X1.x + b*X1.y + c*X1.z);
    // The equation of the vertical plane is ax+by+cz+d = 0.
    static const double denominator = sqrt(a*a + b*b + c*c);
    double numerator = a*X0.x + b*X0.y + c*X0.z + d;
    return numerator / denominator;
}

inline bool is_in_hole(double x, double y, double z)
{
    Point p = {x, y, z};
    double d1 = signed_distance_to_vertical_plane(p);
    double d2 = signed_distance_to_horizontal_plane(p);
    double F = sqrt(2) / (4 * SIDE);  // "F" for "fudge"
    return (-(0.5+F) <= d1 && d1 < (0.5+F)) && (-(0.5+F/2) <= d2 && d2 < (0.5+F/2));
}

inline bool is_solid_plate(T x, T y, T z)
{
    double xd = double(x) / (SIDE*2);
    double yd = double(y) / (SIDE*2);
    double zd = double(z) / (SIDE*5);
    return !is_in_hole(xd, yd, zd);
}

struct Lego {
    int x, y, z;
    explicit constexpr Lego(int x, int y, int z) : x(x), y(y), z(z) {}
    constexpr Lego operator+(const Lego& rhs) const { return Lego{x+rhs.x, y+rhs.y, z+rhs.z}; }
    constexpr bool operator==(const Lego& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
    constexpr bool operator!=(const Lego& rhs) const { return !(*this == rhs); }
};

struct Piece {
    const char *filename_;
    int x_, y_, z_;
    std::vector<Lego> offsets_;

    bool doit(Lego mainguy, std::vector<Lego>& v) const {
        std::vector<int> to_remove;
        for (const Lego& offset : offsets_) {
            Lego lookingfor = mainguy + offset;
            auto it = std::find(v.begin(), v.end(), lookingfor);
            if (it == v.end()) return false;
            to_remove.push_back(it - v.begin());
        }
        std::sort(to_remove.begin(), to_remove.end(), std::greater<int>());
        for (int i : to_remove) {
            v.erase(v.begin() + i);
        }
        return true;
    }
};

int main()
{
    printf("0 STEP\n");

    std::vector<Lego> all_plates;
    std::vector<Lego> all_tiles;
    for (T z = 0; z < SIDE*5; ++z) {
        for (T y = 0; y < SIDE*2; ++y) {
            for (T x = 0; x < SIDE*2; ++x) {
                if (is_solid_plate(x, y, z)) {
                    if (z == SIDE*5 - 1 || !is_solid_plate(x, y, z+1)) {
                        all_tiles.emplace_back(x,y,z);
                    } else {
                        all_plates.emplace_back(x,y,z);
                    }
                }
            }
        }
    }
    std::mt19937 g;
    std::shuffle(all_plates.begin(), all_plates.end(), g);
    std::shuffle(all_tiles.begin(), all_tiles.end(), g);
    std::partition(all_plates.begin(), all_plates.end(), [](const Lego& g){ return g.z % 3 != 0; });

    if (true) {
        const Piece poss[] = {
            Piece{ "3001.dat",  +30, +10, -24, {
                Lego{0,0,0}, Lego{0,0,1}, Lego{0,0,2}, Lego{1,0,0}, Lego{1,0,1}, Lego{1,0,2},
                Lego{0,1,0}, Lego{0,1,1}, Lego{0,1,2}, Lego{1,1,0}, Lego{1,1,1}, Lego{1,1,2},
                Lego{2,0,0}, Lego{2,0,1}, Lego{2,0,2}, Lego{3,0,0}, Lego{3,0,1}, Lego{3,0,2},
                Lego{2,1,0}, Lego{2,1,1}, Lego{2,1,2}, Lego{3,1,0}, Lego{3,1,1}, Lego{3,1,2},
            } },
            Piece{ "6223.dat",  +10, +10, -24, {Lego{0,0,0}, Lego{0,0,1}, Lego{0,0,2}, Lego{1,0,0}, Lego{1,0,1}, Lego{1,0,2}, Lego{0,1,0}, Lego{0,1,1}, Lego{0,1,2}, Lego{1,1,0}, Lego{1,1,1}, Lego{1,1,2}} },
            Piece{ "3004.dat", +10,  +0, -24, {Lego{0,0,0}, Lego{0,0,1}, Lego{0,0,2}, Lego{1,0,0}, Lego{1,0,1}, Lego{1,0,2}} },
            Piece{ "30071.dat",  +0,  +0, -24, {Lego{0,0,0}, Lego{0,0,1}, Lego{0,0,2}} },
            Piece{ "30008.dat",  +0,  +0,  -8, {Lego{0,0,0}} },
        };
        while (!all_plates.empty()) {
            Lego mainguy = all_plates.back();
            for (const Piece& p : poss) {
                if (p.doit(mainguy, all_plates)) {
                    printf("1 7 %d %d %d 1 0 0 0 1 0 0 0 1 %s\n", mainguy.x*20 + p.x_, mainguy.z*-8 + p.z_, mainguy.y*20 + p.y_, p.filename_);
                    break;
                }
            }
        }
    }

    if (true) {
        const Piece poss[] = {
            Piece{ "63327.dat", +10, +10,  -8, {Lego{0,0,0}, Lego{1,0,0}, Lego{0,1,0}, Lego{1,1,0}} },
            Piece{ "63864.dat", +20,  +0,  -8, {Lego{0,0,0}, Lego{1,0,0}, Lego{2,0,0}} },
            Piece{ "30070.dat", +10,  +0,  -8, {Lego{0,0,0}, Lego{1,0,0}} },
            Piece{ "30039.dat",  +0,  +0,  -8, {Lego{0,0,0}} },
        };
        while (!all_tiles.empty()) {
            Lego mainguy = all_tiles.back();
            for (const Piece& p : poss) {
                if (p.doit(mainguy, all_tiles)) {
                    printf("1 7 %d %d %d 1 0 0 0 1 0 0 0 1 %s\n", mainguy.x*20 + p.x_, mainguy.z*-8 + p.z_, mainguy.y*20 + p.y_, p.filename_);
                    break;
                }
            }
        }
    }
}
