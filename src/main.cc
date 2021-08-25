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

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: svg2dxf [SVG FILE] > [OUTPUT]\n";
    return 0;
  }

  std::cerr << "Parsing " << argv[1] << "... ";
  auto svg = parsePattern(argv[1]); /*parseSVGFile(argv[1], parseCurves);*/
  std::cerr << "OK\n";

  // std::cerr << "-------------------\n";
  // std::cerr << "SVG Filename " << argv[1] << "\n";
  // std::cerr << "SVG Pattern Width " << svg.width << "\n";
  // std::cerr << "SVG Pattern Height " << svg.height << "\n";
  // std::cerr << "SVG Polygon Count " << svg.polygons.size() << "\n";
  // std::cerr << "SVG Line Count " << svg.lines.size() << "\n";
  // std::cerr << "-------------------\n";

  HEADER();

  unsigned ph = 1;
  unsigned pw = 1;

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
