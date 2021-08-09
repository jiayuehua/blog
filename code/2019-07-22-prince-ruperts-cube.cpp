#include <iostream>
#include <cmath>

#ifndef SIDE
#define SIDE 300
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

inline double sqr(double x) { return x * x; }

// http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
inline double squared_distance_to_hole_centerline(Point X0)
{
    Point X1_X0 = (X1 - X0);
    static constexpr Point X2_X1 = (X2 - X1);
    double numerator = (X1_X0.mag2() * X2_X1.mag2()) - sqr(X1_X0.dot(X2_X1));
    static constexpr double denominator = X2_X1.mag2();
    return numerator / denominator;
}

inline double distance_to_vertical_plane(Point X0)
{
    // N is the direction of the vertical plane's normal vector.
    static constexpr Point N = {X2.y - X1.y, X1.x - X2.x, 0};
    static constexpr double a = N.x;
    static constexpr double b = N.y;
    static constexpr double c = N.z;
    static constexpr double d = -(a*X1.x + b*X1.y + c*X1.z);
    // The equation of the vertical plane is ax+by+cz+d = 0.
    static const double denominator = sqrt(a*a + b*b + c*c);
    double numerator = fabs(a*X0.x + b*X0.y + c*X0.z + d);
    return numerator / denominator;
}

inline double distance_to_horizontal_plane(Point X0)
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
    double numerator = fabs(a*X0.x + b*X0.y + c*X0.z + d);
    return numerator / denominator;
}

double max_d1 = -1;
double max_d2 = -1;

inline double is_in_hole(double x, double y, double z)
{
    Point p = {x, y, z};
    double d1 = distance_to_vertical_plane(p);
    double d2 = distance_to_horizontal_plane(p);
    assert(d1 >= 0);
    assert(d2 >= 0);
    max_d1 = std::max(d1, max_d1);
    max_d2 = std::max(d2, max_d2);
    static const double half_side_of_hole = 0.5;
    if (d1 < half_side_of_hole && d2 < half_side_of_hole) {
        return 1.0;
    } else if (d1 <= half_side_of_hole && d2 <= half_side_of_hole) {
        return 0.5;
    } else {
        return 0.0;
    }
}

int main()
{
    double hole_volume = 0.0;
    for (T x = 0; x < SIDE; ++x) {
      double xd = double(x) / SIDE;
      for (T y = 0; y < SIDE; ++y) {
        double yd = double(y) / SIDE;
        for (T z = 0; z < SIDE; ++z) {
          double zd = double(z) / SIDE;
          hole_volume += is_in_hole(xd, yd, zd);
        }
      }
    }
    std::cout << "max_d1 = " << max_d1 << "\n";
    std::cout << "max_d2 = " << max_d2 << "\n";
    T total = SIDE * SIDE * SIDE;
    std::cout << hole_volume << " of " << total << " points were inside the hole.\n";
    std::cout << (total - hole_volume) << " of " << total << " points remain.\n";
    std::cout << "The volume of the remaining cube is about " << (double(total - hole_volume) / double(total)) << ".\n";
}
