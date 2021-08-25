#ifndef PARSER_HH
#define PARSER_HH

#include "../geo.hh"
#include "../bezier.hh"
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <stack>

typedef std::map<std::string, std::vector<std::string>> ValueMap;

struct SVG {
  float width, height;
  Polygon polygons;
  Line lines;
};

enum Type {
  RECT, POLYGON, LINE, PATTERN, PATH
};

static void trim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
    return !std::isspace(ch);
  }));

  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
    return !std::isspace(ch);
  }).base(), s.end());
}

SVG parsePattern(const std::string &filename);
void parsePaths(const std::vector<std::string> &commands, Line &lines);
ValueMap parse(std::string &line);
bool isValid(ValueMap map, Type type);
std::vector<float> strVec2FVec(const std::vector<std::string> &vector, char delimiter = ',');
std::vector<float> pathVec2FVec(std::string command);
std::string onlyAlpha(const std::string &string);

#endif
