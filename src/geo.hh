#ifndef GEO_HH
#define GEO_HH

#include <bitset>
#include <iostream>
#include <vector>

struct Point {
  float x, y;

  void operator +=(const Point &p) {
    x += p.x;
    y += p.y;
  }

  bool operator ==(const Point &p) {
    std::bitset<32> px(x);
    std::bitset<32> py(y);
    std::bitset<32> endx(p.x);
    std::bitset<32> endy(p.y);

    return (px == endx) && (py == endy);
  }

  bool operator !=(const Point &p) {
    return !operator==(p);
  }

  void operator *=(float f) {
    x *= f;
    y *= f;
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

typedef std::vector<std::vector<Point>> Polygon;
typedef std::vector<std::pair<Point, Point>> Line;

namespace geo {
  static void transform(float &x, float &y, const float nums[6]) {
    x = nums[0]*x + nums[2]*y + nums[4];
    y = nums[1]*x + nums[3]*y + nums[5];
  }
}

#endif
