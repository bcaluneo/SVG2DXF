// Copyright (C) Brendan Caluneo

#ifndef GEO_HH
#define GEO_HH

#include <bitset>
#include <iostream>
#include <vector>
#include <cmath>

struct Point {
  double x, y;

  void operator +=(const Point &p) {
    x += p.x;
    y += p.y;
  }

  float distance(const Point &p) {
    return std::sqrt(std::pow(p.x - x, 2) + std::pow(p.y - y, 2));
  }

  bool operator ==(const Point &p) {
    return std::abs(x - p.x) < std::numeric_limits<double>::epsilon() &&
           std::abs(y - p.y) < std::numeric_limits<double>::epsilon();
  }

  bool operator !=(const Point &p) {
    return !operator==(p);
  }

  void operator *=(float f) {
    x *= f;
    y *= f;
  }

  void operator -=(const Point &p) {
    x -= p.x;
    y -= p.y;
  }

  bool operator <=(const Point &p) {
    return x <= p.x && y <= p.y;
  }

  bool operator <(const Point &p) {
    return x < p.x && y < p.y;
  }

  bool operator >(const Point &p) {
    return x > p.x && y > p.y;
  }

  friend std::ostream& operator<<(std::ostream &os, const Point &p) {
    os << "(" << p.x << "," << p.y << ")";
    return os;
  }
};

// The naming of these don't really make sense.
typedef std::vector<std::vector<Point>> Polygon;
typedef std::vector<std::pair<Point, Point>> Line;

namespace geo {
  inline void transform(double &x, double &y, const std::vector<float> &matrix) {
    x = matrix[0]*x + matrix[2]*y + matrix[4];
    y = matrix[1]*x + matrix[3]*y + matrix[5];
  }
}

#endif
