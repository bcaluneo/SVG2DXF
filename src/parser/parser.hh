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

void trim(std::string &);
SVG parseFile(const std::string &);
void parsePaths(const std::vector<std::string> &, Line &);
ValueMap parse(std::string &);
bool isValid(ValueMap, Type);
std::vector<float> strVec2FVec(const std::vector<std::string> &, char delimiter = ',');
std::vector<float> pathVec2FVec(std::string);
std::string onlyAlpha(const std::string &);

#endif
