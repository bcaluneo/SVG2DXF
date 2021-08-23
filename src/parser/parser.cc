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
      width = std::atof(map["width"][0].c_str());
      height = std::atof(map["height"][0].c_str());
    }
  }


  return { width, height, polygons, lines };
}

void parsePolygons(const std::vector<std::string> &points, std::vector<Polygon> &polygons) {
}

void parseLines(const std::vector<std::string> &points, std::vector<Line> &lines) {

}

void parseRects(const std::vector<std::string> &points, std::vector<Polygon> &polygons) {

}

void parsePaths(const std::vector<std::string> &points, std::vector<Line> &lines) {

}

ValueMap parse(std::string &line) {
  ValueMap result;

  std::stringstream ss(line);
  std::string keyString, currentKey = "";

  while (std::getline(ss, keyString, ' ')) {
    if (keyString.find(">") != std::string::npos) {
      if (keyString[keyString.size() - 2] == '/') keyString.pop_back();
      keyString.pop_back();
    }

    if (keyString.find("=") == std::string::npos) {
      if (currentKey == "") continue;
      if (keyString[keyString.size() - 1] == '\"') {
        keyString.pop_back();
        result[currentKey].push_back(keyString);
        currentKey = "";
        continue;
      } else {
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
      result[key].push_back(keyString);
    }
  }

  return result;
}

std::vector<float> strVec2FVec(const std::vector<std::string> &vector) {
  std::vector<float> result;
  for (auto s : vector) {
    std::stringstream ss(s);
    while (std::getline(ss, s, ',')) {
      result.push_back(std::atof(s.c_str()));
    }
  }

  return result;
}
