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
    if (map.size() == 0) continue;
    auto key = line.substr(1, line.find(" ") - 1);
    if (key == "pattern") {
      if (map["width"].size() < 0 || map["height"].size() < 0) {
        width = height = 100;
      } else {
        width = std::atof(map["width"][0].c_str());
        height = std::atof(map["height"][0].c_str());
      }
    } else if (key == "polygon") {
      auto points = strVec2FVec(map["points"]);
      std::vector<Point> p;
      while (points.size() > 0) {
        p.push_back({ points[0], points[1] });
        points.erase(points.begin());
        points.erase(points.begin());
      }

      polygons.push_back(p);
    } else if (key == "line") {
      lines.push_back({
        { std::atof(map["x1"][0].c_str()), std::atof(map["y1"][0].c_str())},
        { std::atof(map["x2"][0].c_str()), std::atof(map["y2"][0].c_str())}
      });
    } else if (key == "rect") {
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
    }
  }

  return { width, height, polygons, lines };
}

void parsePaths(const std::vector<std::string> &points, std::vector<Line> &lines) {

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
