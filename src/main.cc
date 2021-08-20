#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "dxf.hh"

struct Point {
  float x, y;

  void operator +=(const Point &p) {
    x += p.x;
    y += p.y;
  }

  friend std::ostream& operator<<(std::ostream &os, const Point &p) {
    os << "(" << p.x << "," << p.y << ")";
    return os;
  }
};

typedef std::vector<std::vector<Point>> Polygon;
typedef std::vector<std::pair<Point, Point>> Line;

struct SVG {
  float width, height;
  Polygon polygons;
  Line lines;
  unsigned pathCount;
};

void trim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
    return !std::isspace(ch);
  }));

  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
    return !std::isspace(ch);
  }).base(), s.end());
}

Point parseTwoNumbers(const std::string &s) {
  std::stringstream ss(s);
  std::string num1, num2;

  bool reading = true, n1 = true, n2 = false, n1n = false, n2n = false;
  unsigned pos = 0;
  while (reading) {
    char front = s[pos++];
    if (std::isalpha(front)) {
      reading = false;
      continue;
    }

    if (front == '-' && num1.size() == 0) {
      n1n = true;
      num1 += front;
      continue;
    } else if (front == '-' && num2.size() == 0 && num1.size() > 0) {
      n2 = n2n = true;
      n1 = false;
      num2 += front;
      continue;
    } else if (front == ',') {
      if (num2.size() > 0) {
        reading = false;
        continue;
      }

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

  return { n1n ? -std::atof(num1.c_str()) : std::atof(num1.c_str()), n2n ? -std::atof(num2.c_str()) : std::atof(num2.c_str()) };
}

SVG parseSVGFile(const std::string &filename) {
  std::ifstream infile(filename);
  std::string line;
  bool patternFound = false, largePolygonFound = false, largePathFound = false;
  std::vector<std::string> largePolygonStrings, pathCommands;
  Polygon polygons;
  Line lines;

  float width = 0, height = 0;
  while (std::getline(infile, line)) {
    if (line.rfind("<pattern", 0) == 0) {
      patternFound = true;
      continue;
    }

    if (patternFound) {
      trim(line);

      if (largePolygonFound) {
        largePolygonFound = line.find("\"/>") == std::string::npos;
        if (!largePolygonFound) {
          line.erase(line.end() - line.begin() - 3);
        }

        trim(line); // Trimming again allows us to remove any white space after /">

        largePolygonStrings.back() += " ";
        largePolygonStrings.back() += line;
        continue;
      }

      if (largePathFound) {
        largePathFound = line.find("\"/>") == std::string::npos;
        if (!largePathFound) {
          line.erase(line.end() - line.begin() - 3, 3);
        }

        trim(line);

        pathCommands.back() += line;
        continue;
      }

      if (line.rfind("</pattern", 0) == 0) break; // We've reached the end of the pattern.
      if (line.rfind("<rect", 0) == 0) {
        std::string dimensions = line.substr(line.find("width"), std::distance(line.begin() + line.find("width"), line.end()) - 3);

        std::stringstream ss(dimensions);
        std::string sWidth, sHeight;
        std::getline(ss, sWidth, ' ');
        std::getline(ss, sHeight, ' ');
        sWidth.erase(sWidth.end() - sWidth.begin() - 1, 1); // Remove last "
        sHeight.erase(sHeight.end() - sHeight.begin() - 1, 1);
        sWidth.erase(0, 7); // Remove width/height="
        sHeight.erase(0, 8);

        width = std::atof(sWidth.c_str());
        height = std::atof(sHeight.c_str());
      } else if (line.rfind("<polygon", 0) == 0) {
        line = line.substr(line.find("points="), std::string::npos);
        line.erase(0, 8);
        if (line.find("\"/>") == std::string::npos) {
          largePolygonFound = true;
          largePolygonStrings.push_back(line);
          continue;
        }
        std::vector<Point> polygon;
        line.erase(line.end() - line.begin() - 3);
        trim(line);
        std::string pair;
        std::stringstream ss(line);
        while (std::getline(ss, pair, ' ')) {
          std::string x = pair.substr(0, pair.find(","));
          std::string y = pair.substr(pair.find(",") + 1, std::string::npos);
          polygon.push_back({ std::atof(x.c_str()), -std::atof(y.c_str()) });
        }

        polygons.push_back(polygon);
      } else if (line.rfind("<line", 0) == 0) {
        std::string points = line.substr(line.find("x1"), std::distance(line.begin() + line.find("x1"), line.end()) - 2);
        std::stringstream ss(points);
        std::string x1s, y1s, x2s, y2s;
        std::getline(ss, x1s, ' ');
        std::getline(ss, y1s, ' ');
        std::getline(ss, x2s, ' ');
        std::getline(ss, y2s, ' ');

        x1s.erase(x1s.end() - x1s.begin() - 1, 1);
        y1s.erase(y1s.end() - y1s.begin() - 1, 1);
        x2s.erase(x2s.end() - x2s.begin() - 1, 1);
        y2s.erase(y2s.end() - y2s.begin() - 1, 1);

        x1s.erase(0, 4);
        y1s.erase(0, 4);
        x2s.erase(0, 4);
        y2s.erase(0, 4);

        lines.push_back({
          { std::atof(x1s.c_str()), -std::atof(y1s.c_str())},
          { std::atof(x2s.c_str()), -std::atof(y2s.c_str())}
        });
      } else if (line.rfind("<path", 0) == 0) {
        line = line.substr(line.find("d="), std::string::npos);
        line.erase(0, 3);
        if (line.find("\"/>") == std::string::npos) {
          largePathFound = true;
          pathCommands.push_back(line);
          continue;
        }

        line.erase(line.end() - line.begin() - 3, 3);
        pathCommands.push_back(line);
      }
    }
  }

  for (auto s : largePolygonStrings) {
    std::vector<Point> polygon;
    std::string pair;
    std::stringstream ss(s);
    while (std::getline(ss, s, ' ')) {
      std::string x = s.substr(0, s.find(","));
      std::string y = s.substr(s.find(",") + 1, std::string::npos);
      polygon.push_back({ std::atof(x.c_str()), -std::atof(y.c_str()) });
    }

    polygons.push_back(polygon);
  }

  for (auto p : pathCommands) {
    Point start {-1, -1}, current;
    std::cerr << p << "\n";

    while (p.size() > 0) {
      char control = p[0];
      p.erase(0, 1);

      switch (control) {
        case 'M': {
          Point pair = parseTwoNumbers(p);
          if (start.x == -1 && start.y == -1) {
            start = current = pair;
          } else {
            current = pair;
          }
        }
        break;

        case 'm': {
          Point pair = parseTwoNumbers(p);
          current += pair;
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

        case 'z': {
          lines.push_back({
            { current.x, current.y },
            { start.x, start.y }
          });
        }
        break;
      }
    }
  }

  return { width, height, polygons, lines, pathCommands.size() };
}

int main(int argc, char **argv) {
  // std::cerr << "Parsing SVG file " << argv[1] << "... ";
  auto svg = parseSVGFile(argv[1]);
  // std::cerr << "OK" << "\n";
  std::cerr << "-------------------\n";
  std::cerr << "SVG Pattern Width " << svg.width << "\n";
  std::cerr << "SVG Pattern Height " << svg.height << "\n";
  std::cerr << "SVG Polygon Count " << svg.polygons.size() << "\n";
  std::cerr << "SVG Line Count " << svg.lines.size() << "\n";
  std::cerr << "SVG Path Count " << svg.pathCount << "\n";
  std::cerr << "-------------------\n";

  HEADER();
  for (auto p : svg.polygons) {
    POLYLINE();
    for (const auto [x, y] : p) {
      VERTEX(x, y);
    }

    SEQEND();
  }

  for (const auto [p1, p2] : svg.lines) {
    const auto [x1, y1] = p1;
    const auto [x2, y2] = p2;
    LINE(x1, y1, x2, y2);
    SEQEND();
  }

  unsigned SIZE = 5;

  for (int h = 0; h < SIZE; ++h) {
    for (int w = 0; w < SIZE*2; ++w) {
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
