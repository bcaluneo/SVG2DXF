#include "parser.hh"

SVG parsePattern(const std::string &filename) {
  Polygon polygons;
  Line lines;
  float width = 0, height = 0;

  std::ifstream infile(filename);
  std::string line, currentLine;

  while (std::getline(infile, line)) {
    std::cerr << "*********************************\n";
    trim(line);

    if (line.find(">") == std::string::npos) {
      // Handle multi line code.
    } else {
      auto map = parseKeys(line);
      for (const auto [k, v] : map) {
        std::cerr << k << "\n";
        for (auto s : v) {
          std::cerr << "\t" << s << "\n";
        }
        std::cerr << "-------------\n";
      }
    }

    std::cerr << "*********************************\n";
  }

  return { width, height, polygons, lines };
}

void parsePolygons(std::vector<Polygon> &polygons) {

}

void parseLines(std::vector<Line> &lines) {

}

void parseRects(std::vector<Polygon> &polygons) {

}

void parsePaths(std::vector<Line> &lines) {

}

ValueMap parseKeys(std::string &line) {
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
