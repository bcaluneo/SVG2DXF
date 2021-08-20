#ifndef GEO_HH
#define GEO_HH

struct Point {
  float x, y;

  void operator +=(const Point &p) {
    x += p.x;
    y += p.y;
  }

  bool operator ==(const Point &p) {
    return std::abs(x - p.x) < std::numeric_limits<float>::epsilon()
        && std::abs(y - p.y) < std::numeric_limits<float>::epsilon();
  }

  bool operator !=(const Point &p) {
    return !operator==(p);
  }

  void operator *=(float f) {
    x *= f;
    y *= y;
  }

  friend std::ostream& operator<<(std::ostream &os, const Point &p) {
    os << "(" << p.x << "," << p.y << ")";
    return os;
  }
};

typedef std::vector<std::vector<Point>> Polygon;
typedef std::vector<std::pair<Point, Point>> Line;

#endif
