#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>
#include <algorithm>

// These are hardcoded for now, fix this later.
float w = 80.4;
float h = 139.2;

#define POLYLINE() std::cout << "\t0\nPOLYLINE\n\t8\n0\n\t70\n1\n";
#define VERTEX(x, y) std::cout << "\t0\nVERTEX\n\t10\n" << x << "\n\t20\n" << y << "\n";
#define SEQEND() std::cout << "\t0\nSEQEND\n";

// TODO: Work on tiling (maybe 5 times), invert the y coordinate
//       Write a function to parse SVG files so that these things can be read dynamically.

void parseSVGFile() {
  std::ifstream infile("pattern01-original.svg");
  std::string line;
  bool patternFound = false;
  while (std::getline(infile, line)) {
    if (line.rfind("<pattern", 0) == 0) {
      patternFound = true;
      continue;
    }

    if (patternFound) {
      // Remove leading whitespace from the line.
      line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) {
        return !std::isspace(ch);
      }));

      if (line.rfind("</pattern", 0) == 0) break; // We've reached the end of the pattern.
      if (line.rfind("<rect", 0) == 0) {
        std::cout << line << "\n";
      } else if (line.rfind("<polygon", 0) == 0) {

      }
      // std::cout << line << "\n";
    }
  }
}

int main(int argc, char **argv) {
  parseSVGFile();
  return 0;

  std::vector<std::pair<int, int>> points;
  POLYLINE();
  // std::cout << "\t0\nPOLYLINE\n\t8\n0\n\t70\n1\n";
  while (std::cin.good()) {
    std::string s = "";
    float x = 0, y = 0;
    std::cin >> s;

    int end = s.find(",");
    int start = 0;
    while (end != -1) {
      std::string sx = s.substr(start, end - start);
      x = std::atof(sx.c_str());
      start = end + 1;
      end = s.find(",", start);
    }
    std::string sy = s.substr(start, end - start);
    y = std::atof(sy.c_str());

    if (x == 0 && y == 0) continue;

    points.push_back({x, y});
    VERTEX(x, y);
  }

  SEQEND();
  POLYLINE();

  for (int i = 1; i <= 4; ++i) {
    for (const auto [x, y] : points) {
      int newX = x + w*i;
      VERTEX(newX, y);
    }
  }

  SEQEND();

  return 0;
}
