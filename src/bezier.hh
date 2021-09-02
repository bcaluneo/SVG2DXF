#ifndef BEZIER_HH
#define BEZIER_HH

#include <cmath>

// p = (1-t)^3 *P0 + 3*t*(1-t)^2*P1 + 3*t^2*(1-t)*P2 + t^3*P3
inline auto getBezierPoints(Point start, Point cp0, Point cp1, Point end) {
  std::vector<Point> result { start };

  float t = 0.0;
  while (t <= 1) {
    t += 1 / std::abs(cp0.distance(cp1));
    auto term1 = std::pow(1-t, 3);
    auto term2 = 3*t*std::pow(1-t, 2);
    auto term3 = 3*std::pow(t, 2)*(1-t);
    auto term4 = std::pow(t, 3);
    Point p0 = start;
    Point p1 = cp0;
    Point p2 = cp1;
    Point p3 = end;
    p0 *= term1;
    p1 *= term2;
    p2 *= term3;
    p3 *= term4;
    p0 += p1;
    p0 += p2;
    p0 += p3;
    result.push_back(p0);
  }

  return result;
}

#endif
