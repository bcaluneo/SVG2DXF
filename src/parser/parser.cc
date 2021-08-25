#include "parser.hh"

SVG parsePattern(const std::string &filename) {
  Polygon polygons;
  Line lines;
  float width = 0, height = 0;

  std::ifstream infile(filename);
  std::string line, currentLine;
  std::vector<std::string> linesToParse;

  bool multiline = false;
  std::string currentKey = "";
  while (std::getline(infile, line)) {
    trim(line);
    if (line.size() == 0) continue;

    auto key = line.substr(1, line.find(" "));
    if (key[0] == '!') continue; // ignore comments
    if (currentKey != "") {
      linesToParse.back() += " " + line;
      currentKey = line.find(">") == std::string::npos ? key : "";
      continue;
    }

    currentKey = line.find(">") == std::string::npos ? key : "";
    linesToParse.push_back(line);
  }

  for (auto line : linesToParse) {
    auto map = parse(line);
    auto key = line.substr(1, line.find(" ") - 1);
    if (key == "pattern" && isValid(map, PATTERN)) {
      if (map["width"].size() < 0 || map["height"].size() < 0) {
        width = height = 100;
      } else {
        width = std::atof(map["width"][0].c_str());
        height = std::atof(map["height"][0].c_str());
      }
    } else if (key == "polygon" && isValid(map, POLYGON)) {
      auto points = strVec2FVec(map["points"]);
      std::vector<Point> p;
      while (points.size() > 0) {
        p.push_back({ points[0], points[1] });
        points.erase(points.begin());
        points.erase(points.begin());
      }

      polygons.push_back(p);
    } else if (key == "line" && isValid(map, LINE)) {
      lines.push_back({
        { std::atof(map["x1"][0].c_str()), std::atof(map["y1"][0].c_str())},
        { std::atof(map["x2"][0].c_str()), std::atof(map["y2"][0].c_str())}
      });
    } else if (key == "rect" && isValid(map, RECT)) {
      float x = std::atof(map["x"][0].c_str());
      float y = std::atof(map["y"][0].c_str());
      float rWidth = std::atof(map["width"][0].c_str());
      float rHeight = std::atof(map["height"][0].c_str());

      std::vector<Point> points {
        {x, y},
        {x + rWidth, y},
        {x + rWidth, y + rHeight},
        {x, y + rHeight}
      };

      // TODO: Add support for other kinds of transforms. Right now this just handles matrix transform.
      if (map["transform"].size() > 0) {
        map["transform"][0].erase(0, 7);
        map["transform"][5].pop_back();

        auto matrix = strVec2FVec(map["transform"], ' ');
        for (Point &p : points) {
          geo::transform(p.x, p.y, matrix);
        }
      }

      polygons.push_back(points);
    } else if (key == "path" && isValid(map, PATH)) {
      parsePaths(map["d"], lines);
    }
  }

  return { width, height, polygons, lines };
}

void parsePaths(const std::vector<std::string> &lines, Line &svgLines) {
  Point start, current;
  bool prevCommandCurve = false; Point prevControlPoint;
  for (auto s : lines) {
    auto v = pathVec2FVec(s);
    std::reverse(v.begin(), v.end());
    std::stack<float, std::vector<float>> stack(v);
    std::string command = onlyAlpha(s);
    bool absolute = false;

    for (char c : command) {
      absolute = std::isupper(c);
      if (c == 'M') {
        auto x = stack.top(); stack.pop();
        auto y = stack.top(); stack.pop();
        Point p { x, y };
        if (absolute) start = current = p;
        else { current.x += x; current.y += y; }
        prevCommandCurve = false;
      } else if (c == 'l' || c == 'L') {
        auto x = stack.top(); stack.pop();
        auto y = stack.top(); stack.pop();
        Point end;
        if (absolute) {
          end.x = x; end.y = y;
        } else {
          end = current;
          end.x += x; end.y += y;
        }

        svgLines.push_back({
          { current.x, current.y },
          { end.x, end.y }
        });

        current = end;
        prevCommandCurve = false;
      } else if (c == 'h' || c == 'H') {
        float x = stack.top(); stack.pop();
        Point end;
        if (absolute) { end.x = x; end.y = current.y; }
        else { end.x = current.x + x; end.y = current.y; }
        svgLines.push_back({
          { current.x, current.y },
          { end.x, end.y }
        });
        current = end;
        prevCommandCurve = false;
      } else if (c == 'v' || c == 'V') {
        float y = stack.top(); stack.pop();
        Point end;
        if (absolute) { end.x = current.x; end.y = y; }
        else { end.x = current.x; end.y = current.y + y; }
        svgLines.push_back({
          { current.x, current.y },
          { end.x, end.y }
        });
        current = end;
        prevCommandCurve = false;
      } else if (c == 'c' || c == 'C') {
        auto p1x = stack.top(); stack.pop();
        auto p1y = stack.top(); stack.pop();
        auto p2x = stack.top(); stack.pop();
        auto p2y = stack.top(); stack.pop();
        auto p3x = stack.top(); stack.pop();
        auto p3y = stack.top(); stack.pop();
        Point end, cp0, cp1;
        if (absolute) {
          cp0.x = p1x; cp0.y = p1y;
          cp1.x = p2x; cp1.y = p2y;
          end.x = p3x; end.y = p3y;
        } else {
          cp0.x = current.x + p1x; cp0.y = current.y + p1y;
          cp1.x = current.x + p2x; cp1.y = current.y + p2y;
          end.x = current.x + p3x; end.y = current.y + p3y;
        }

        auto points = getBezierPoints(current, cp0, cp1, end);
        for (size_t i = 0; i < points.size() - 1; ++i) {
          auto p0 = points[i];
          auto p1 = points[i + 1];
          svgLines.push_back({
            { p0.x, p0.y },
            { p1.x, p1.y }
          });
        }

        current = end;
        prevCommandCurve = true; prevControlPoint = cp1;
      } else if (c == 's' || c == 'S') {
        auto p1x = stack.top(); stack.pop();
        auto p1y = stack.top(); stack.pop();
        auto p2x = stack.top(); stack.pop();
        auto p2y = stack.top(); stack.pop();
        Point end, cp0, cp1;
        if (absolute) {
          cp1.x = p1x; cp1.y = p1y;
          end.x = p2x; end.y = p2y;
        } else {
          cp1.x = current.x + p1x; cp1.y = current.y + p1y;
          end.x = current.x + p2x; end.y = current.y + p2y;
        }

        if (prevCommandCurve) {
          cp0 = current;
          cp0 *= 3;
          cp0 -= prevControlPoint;
        } else { cp0 = current; }

        auto points = getBezierPoints(current, cp0, cp1, end);
        for (size_t i = 0; i < points.size() - 1; ++i) {
          auto p0 = points[i];
          auto p1 = points[i + 1];
          svgLines.push_back({
            { p0.x, p0.y },
            { p1.x, p1.y }
          });
        }

        current = end;
        prevCommandCurve = true; prevControlPoint = cp1;
      } else if (c == 'z') {
        svgLines.push_back({
          { current.x, current.y },
          { start.x, start.y }
        });

        prevCommandCurve = false;
      }
    }
  }
}

ValueMap parse(std::string &line) {
  ValueMap result;

  std::stringstream ss(line);
  std::string keyString, currentKey = "";

  while (std::getline(ss, keyString, ' ')) {
    trim(currentKey);
    if (keyString.find(">") != std::string::npos) {
      if (keyString[keyString.size() - 2] == '/') keyString.pop_back();
      keyString.pop_back();
    }

    if (keyString.find("=") == std::string::npos) {
      if (currentKey == "") continue;
      if (keyString[keyString.size() - 1] == '\"') {
        keyString.pop_back();
        trim(keyString);
        result[currentKey].push_back(keyString);
        currentKey = "";
        continue;
      } else {
        trim(keyString);
        result[currentKey].push_back(keyString);
        continue;
      }
    }

    if (keyString.find("=") != std::string::npos) {
      auto key = keyString.substr(0, keyString.find("="));
      if (keyString[keyString.size() - 1] != '\"') {
        currentKey = key;
      } else keyString.pop_back();
      keyString.erase(0, key.size() + 2);
      trim(keyString);
      trim(key);
      result[key].push_back(keyString);
    }
  }

  return result;
}

bool isValid(ValueMap map, Type type) {
  bool result = true;
  switch(type) {
    case RECT:
      result = !(map.find("x") == map.end()
              || map.find("y") == map.end()
              || map.find("width") == map.end()
              || map.find("height") == map.end());
      break;
    case POLYGON:
      result = !(map.find("points") == map.end());
      break;
    case LINE:
      result = !(map.find("x1") == map.end()
              || map.find("y1") == map.end()
              || map.find("x2") == map.end()
              || map.find("y2") == map.end());
      break;
    case PATTERN:
      result = !(map.find("width") == map.end()
              || map.find("height") == map.end());
      break;
    case PATH:
      result = !(map.find("d") == map.end());
      break;
  }

  return result;
}

std::vector<float> strVec2FVec(const std::vector<std::string> &vector, char delimiter) {
  std::vector<float> result;
  for (auto s : vector) {
    std::stringstream ss(s);
    while (std::getline(ss, s, delimiter)) {
      result.push_back(std::atof(s.c_str()));
    }
  }

  return result;
}

std::vector<float> pathVec2FVec(std::string command) {
  std::vector<float> result;
  std::string numString;
  while (command.size() > 0) {
    char front = command[0];
    command.erase(0, 1);
    if (std::isalpha(front) && numString.size() == 0) {
      continue;
    }

    if (front == '-' && numString.size() == 0) {
      numString += front;
      continue;
    } else if ((front == '-' || front == ',' || std::isalpha(front)) && numString.size() > 0) {
      result.push_back(std::atof(numString.c_str()));
      numString = front == '-' ? "-" : "";
      continue;
    }

    numString += front;
  }

  if (numString != "") result.push_back(std::atof(numString.c_str()));

  return result;
}

std::string onlyAlpha(const std::string &string) {
  std::string result = "";
  for (size_t i = 0; i < string.size(); ++i) {
    if (std::isdigit(string[i]) || string[i] == '.' || string[i] == ',' || string[i] == '-') continue;
    result += string[i];
  }

  return result;
}
