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

  if (xNeedsFraction) {
    stream << std::fixed << std::setprecision(1) << p.x;
  } else {
    stream << std::fixed << std::setprecision(0) << p.x;
  }

  result += stream.str().size();
  stream.str("");

  if (yNeedsFraction) {
    stream << std::fixed << std::setprecision(1) << p.y;
  } else {
    stream << std::fixed << std::setprecision(0) << p.y;
  }

  result += stream.str().size();

  if (p.y > 0) result += 1;
  return result;
}

// TODO: Break this function into smaller functions and package them together into a separate file.
SVG parseSVGFile(const std::string &filename, bool drawCurves) {
  std::ifstream infile(filename);
  std::string line;
  bool patternFound = false, largePolygonFound = false, largePathFound = false;
  std::vector<std::string> largePolygonStrings, pathCommands;
  Polygon polygons;
  Line lines;

  float width = 0, height = 0;
  bool dimensionsSet = false;
  while (std::getline(infile, line)) {
    if (line.rfind("<pattern", 0) == 0 || line.rfind("<g id=\"Layer", 0) == 0) {
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
        largePathFound = line.find("/>") == std::string::npos;
        if (!largePathFound) {
          line.erase(line.end() - line.begin() - 2, 2);
        }

        trim(line);

        pathCommands.back() += line;
        continue;
      }

      if (line.rfind("</pattern", 0) == 0) break; // We've reached the end of the pattern.
      if (line.rfind("<rect", 0) == 0) {
        if (!dimensionsSet) {
          std::string dimensions = line.substr(line.find("width"), std::distance(line.begin() + line.find("width"), line.end()) - 2);
          std::stringstream ss(dimensions);
          std::string sWidth, sHeight;
          std::getline(ss, sWidth, ' ');
          std::getline(ss, sHeight, ' ');
          sWidth.pop_back();
          sHeight.pop_back();
          sWidth.erase(0, 7); // Remove width/height="
          sHeight.erase(0, 8);

          width = std::atof(sWidth.c_str());
          height = std::atof(sHeight.c_str());
          dimensionsSet = true;
        } else {
          // <rect x="60.6" y="-168" transform="matrix(0.8678 -0.4969 0.4969 0.8678 92.9162 15.8377)" width="31.3" height="2.6"/>
          std::string info = line.substr(line.find("x"), std::distance(line.begin() + line.find("x"), line.end()) - 2);

          std::string dimensions = line.substr(line.find("width"), std::distance(line.begin() + line.find("width"), line.end()) - 2);
          // TODO: make this a separate function because it is done twice.
          std::stringstream ss(dimensions);
          std::string sWidth, sHeight;
          std::getline(ss, sWidth, ' ');
          std::getline(ss, sHeight, ' ');
          sWidth.pop_back();
          sHeight.pop_back();
          sWidth.erase(0, 7); // Remove width/height="
          sHeight.erase(0, 8);

          float pW = std::atof(sWidth.c_str());
          float pH = std::atof(sHeight.c_str());

          std::stringstream si(info);
          std::string sX, sY;
          std::getline(si, sX, ' ');
          std::getline(si, sY, ' ');
          sX.pop_back();
          sY.pop_back();
          sX.erase(0, 3); // Remove width/height="
          sY.erase(0, 3);

          float x = std::atof(sX.c_str());
          float y = std::atof(sY.c_str());
          std::vector<Point> points {
            {x, y},
            {x + pW, y},
            {x + pW, y + pH},
            {x, y + pH}
          };

          if (line.find("transform") != std::string::npos) {
            std::string transform = line.substr(line.find("transform"), std::string::npos);
            transform = transform.substr(0, transform.find("width"));
            trim(transform);
            transform.erase(0, std::string("transform=\"matrix(").size());
            transform.pop_back();
            transform.pop_back();

            //a b c d e f
            //0 1 2 3 4 5
            float nums[6];

            std::stringstream ss(transform);
            std::string line;
            for (signed i = 0; i < 6; ++i) {
              std::getline(ss, line, ' ');
              nums[i] = std::atof(line.c_str());
            }
            for (Point &p : points) {
              geo::transform(p.x, p.y, nums);
            }
          }

          polygons.push_back(points);
        }
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
          polygon.push_back({ std::atof(x.c_str()), std::atof(y.c_str()) });
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
          { std::atof(x1s.c_str()), std::atof(y1s.c_str())},
          { std::atof(x2s.c_str()), std::atof(y2s.c_str())}
        });
      } else if (line.rfind("<path", 0) == 0) {
        line = line.substr(line.find("d="), std::string::npos);
        trim(line);
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
      polygon.push_back({ std::atof(x.c_str()), std::atof(y.c_str()) });
    }

    polygons.push_back(polygon);
  }

  std::map<char, unsigned> controlMap;

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

  return { width, height, polygons, lines, pathCommands.size() };
}

int main(int argc, char **argv) {
  bool parseCurves = true;

  if (argc == 3) {
    std::string noc { argv[2] };
    parseCurves = !(noc == "--noc");
  }

  auto svg = parsePattern(argv[1]); /*parseSVGFile(argv[1], parseCurves);*/

  std::cerr << "-------------------\n";
  std::cerr << "SVG Filename " << argv[1] << "\n";
  std::cerr << "SVG Pattern Width " << svg.width << "\n";
  std::cerr << "SVG Pattern Height " << svg.height << "\n";
  std::cerr << "SVG Polygon Count " << svg.polygons.size() << "\n";
  std::cerr << "SVG Line Count " << svg.lines.size() << "\n";
  std::cerr << "SVG Path Count " << svg.pathCount << "\n";
  std::cerr << "-------------------\n";

  return 0;

  HEADER();

  unsigned ph = 3;
  unsigned pw = 3;

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
