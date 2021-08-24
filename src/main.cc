#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <map>

#include "dxf.hh"
#include "geo.hh"
#include "bezier.hh"
#include "parser/parser.hh"

// TODO: Starting from 18 down to 498 all this code needs to find a new home
//       that isn't in the main.cc file. The main parse function is currently
//       moving into the parser.cc file. The structure there should be easy
//       to follow. Everything else can probably move to the parser HEADER
//       and class file as well.

Point parseTwoNumbers(const std::string &s) {
  std::stringstream ss(s);
  std::string num1, num2;

  bool reading = true, n1 = true, n2 = false;
  unsigned pos = 0;
  while (reading) {
    char front = s[pos++];
    if (std::isalpha(front)) {
      reading = false;
      continue;
    }

    if (front == '-' && num1.size() == 0) {
      num1 += front;
      continue;
    } else if (front == '-' && num2.size() == 0 && num1.size() > 0) {
      n2 = true;
      n1 = false;
      num2 += front;
      continue;
    } else if (front == '-' && num2.size() > 0) {
      reading = false;
      continue;
    } else if (front == ',') {
      if (num2.size() > 0) {
        reading = false;
        continue;
      }

      if (num1.size() == 0) continue;

      n1 = false;
      n2 = true;
      continue;
    }

    if (n1) {
      num1 += front;
      continue;
    } else if (n2) {
      num2 += front;
      continue;
    }
  }

  return { std::atof(num1.c_str()), std::atof(num2.c_str()) };
}

float parseOneNumber(const std::string &s) {
  std::stringstream ss(s);
  std::string num1;

  bool reading = true;
  unsigned pos = 0;
  while (reading) {
    char front = s[pos++];
    if (std::isalpha(front)) {
      reading = false;
      continue;
    }

    if (front == '-' && num1.size() == 0) {
      num1 += front;
      continue;
    } else if (front == '-' && num1.size() > 0) {
      reading = false;
      continue;
    }

    num1 += front;
  }

  return std::atof(num1.c_str());
}

auto pairStrLength(Point p) {
  unsigned result = 0;
  std::stringstream stream;

  // Takes a floating point number, cuts off the decimal (cast to int)
  // casts back to a float (adding .0) then subtracts the original number
  // to see if there's anything after the decimal.
  bool xNeedsFraction = std::abs(((float) ((int) std::abs(p.x))) - std::abs(p.x)) > std::numeric_limits<float>::epsilon();
  bool yNeedsFraction = std::abs(((float) ((int) std::abs(p.y))) - std::abs(p.y)) > std::numeric_limits<float>::epsilon();

  stream << std::fixed << std::setprecision(xNeedsFraction ? 1 : 0) << p.x;
  result += stream.str().size();
  stream.str("");
  stream << std::fixed << std::setprecision(yNeedsFraction ? 1 : 0) << p.y;
  result += stream.str().size();

  if (p.y > 0) result += 1;
  return result;
}

// TODO: Break this function into smaller functions and package them together into a separate file.
// Update 8/24: Almost done.
SVG parseSVGFile(const std::string &filename, bool drawCurves) {
  for (auto p : pathCommands) {
    std::cerr << p << "\n";
    continue;
    Point start, current;

    while (p.size() > 0) {
      char control = p[0];
      if (control == 's' || control == 'S') std::cerr << p << "\n";
      p.erase(0, 1);

      if (std::isalpha(control)) {
        if (controlMap.find(control) != controlMap.end()) {
          controlMap[control]++;
        } else {
          controlMap.insert( { control, 1} );
        }
      }

      switch (control) {
        case 'M': {
          Point pair = parseTwoNumbers(p);
          start = current = pair;
        }
        break;

        case 'L': {
          Point pair = parseTwoNumbers(p);
          lines.push_back({
            { current.x, current.y },
            { pair.x, pair.y }
          });
          current = pair;
        }
        break;

        case 'l': {
          Point pair = parseTwoNumbers(p);
          lines.push_back({
            { current.x, current.y },
            { current.x + pair.x, current.y + pair.y }
          });
          current += pair;
        }
        break;

        case 'H': {
          float x = parseOneNumber(p);
          Point end { x, current.y };
          lines.push_back({
            { current.x, current.y },
            { end.x, end.y }
          });

          current = end;
        }
        break;

        case 'h': {
          float x = parseOneNumber(p);
          Point end { current.x + x, current.y };
          lines.push_back({
            { current.x, current.y },
            { end.x, end.y }
          });

          current = end;
        }
        break;

        case 'V': {
          float y = parseOneNumber(p);
          Point end { current.x, y };
          lines.push_back({
            { current.x, current.y },
            { end.x, end.y }
          });

          current = end;
        }
        break;

        case 'v': {
          float y = parseOneNumber(p);
          Point end { current.x, current.y + y };
          lines.push_back({
            { current.x, current.y },
            { end.x, end.y }
          });

          current = end;
        }
        break;

        // This is bad
        case 'C': {
          if (!drawCurves) continue;
          Point cp0 = parseTwoNumbers(p);
          auto pairLen = pairStrLength(cp0);
          p = p.substr(pairLen, std::string::npos);
          if (p[0] == ',') p = p.substr(1, std::string::npos);
          Point cp1 = parseTwoNumbers(p);
          pairLen = pairStrLength(cp1);
          p = p.substr(pairLen, std::string::npos);
          if (p[0] == ',') p = p.substr(1, std::string::npos);
          Point end = parseTwoNumbers(p);
          pairLen = pairStrLength(end);

          auto points = getBezierPoints(current, cp0, cp1, end);
          for (size_t i = 0; i < points.size() - 1; ++i) {
            auto p0 = points[i];
            auto p1 = points[i + 1];
            lines.push_back({
              { p0.x, p0.y },
              { p1.x, p1.y }
            });
          }

          current = end;
        }
        break;

        case 'c': {
          if (!drawCurves) continue;
          Point p0 = parseTwoNumbers(p);
          auto pairLen = pairStrLength(p0);
          p = p.substr(pairLen, std::string::npos);
          if (p[0] == ',') p = p.substr(1, std::string::npos);
          Point p1 = parseTwoNumbers(p);
          pairLen = pairStrLength(p1);
          p = p.substr(pairLen, std::string::npos);
          if (p[0] == ',') p = p.substr(1, std::string::npos);
          Point p2 = parseTwoNumbers(p);
          pairLen = pairStrLength(p2);

          Point end { current.x, current.y };
          end += p2;
          Point cp0 { current.x, current.y };
          cp0 += p2;
          Point cp1 { current.x, current.y };
          cp1 += p2;
          auto points = getBezierPoints(current, cp0, cp1, end);
          for (size_t i = 0; i < points.size() - 1; ++i) {
            auto p0 = points[i];
            auto p1 = points[i + 1];
            lines.push_back({
              { p0.x, p0.y },
              { p1.x, p1.y }
            });
          }
          current = end;
        }
        break;

        case 'z': {
          lines.push_back({
            { current.x, current.y },
            { start.x, start.y }
          });

          if (p.size() > 0) {
            char next = p[0];
            if (!(next == 'M' || next == 'm')) {
              current = start;
            }
          }
        }
        break;
      }
    }
  }

  for (const auto [control, value] : controlMap) {
    std::cerr << control << " --- " << value << "\n";
  }

  return { width, height, polygons, lines };
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: svg2dxf [SVG FILE] > [OUTPUT]\n";
    return 0;
  }

  auto svg = parsePattern(argv[1]); /*parseSVGFile(argv[1], parseCurves);*/

  std::cerr << "-------------------\n";
  std::cerr << "SVG Filename " << argv[1] << "\n";
  std::cerr << "SVG Pattern Width " << svg.width << "\n";
  std::cerr << "SVG Pattern Height " << svg.height << "\n";
  std::cerr << "SVG Polygon Count " << svg.polygons.size() << "\n";
  std::cerr << "SVG Line Count " << svg.lines.size() << "\n";
  std::cerr << "-------------------\n";

  HEADER();

  unsigned ph = 1;
  unsigned pw = 1;

  for (int h = 0; h < ph; ++h) {
    for (int w = 0; w < pw; ++w) {
      for (auto p : svg.polygons) {
        POLYLINE();
        for (const auto [x, y] : p) {
          VERTEX(x + (w*svg.width), y + (h*svg.height));
        }

        SEQEND();
      }

      for (const auto [p1, p2] : svg.lines) {
        const auto [x1, y1] = p1;
        const auto [x2, y2] = p2;
        LINE(x1 + (w*svg.width), y1 + (h*svg.height), x2 + (w*svg.width), y2 + (h*svg.height));
        SEQEND();
      }
    }
  }

  FOOTER();

  return 0;
}
