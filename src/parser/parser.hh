#ifndef PARSER_HH
#define PARSER_HH

#include "../geo.hh"
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>

typedef std::map<std::string, std::vector<std::string>> ValueMap;

struct SVG {
  float width, height;
  Polygon polygons;
  Line lines;
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
void parsePolygons(const std::string &line, std::vector<Polygon> &polygons);
void parseLines(const std::string &line, std::vector<Line> &lines);
void parseRects(const std::string &line, std::vector<Polygon> &polygons);
void parsePaths(const std::string &line, std::vector<Line> &lines);
ValueMap parse(std::string &line);
std::vector<float> strVec2FVec(const std::vector<std::string> &vector);

#endif
