// Copyright (C) Brendan Caluneo

#include <iostream>

#include "dxf.hh"
#include "geo.hh"
#include "bezier.hh"
#include "parser/parser.hh"

int main(int argc, char **argv) {
  if (argc < 2 || argc == 3) {
    std::cerr << "Usage: svg2dxf -s [SIZE] [SVG FILE] > [OUTPUT]\n";
    return 0;
  }

  unsigned size = 0;
  std::string fileName = "";
  if (argc == 4) {
    size = std::atof(argv[2]);
    fileName = argv[3];
  } else {
    size = 1;
    fileName = argv[1];
  }

  std::cerr << "Parsing " << fileName << "... ";
  // This doesn't actually return any errors, so having it print OK afterwards
  // doesn't really make sense.
  auto svg = parseFile(argv[1]); /*parseSVGFile(argv[1], parseCurves);*/
  std::cerr << "OK\n";

  // std::cerr << "-------------------\n";
  // std::cerr << "SVG Filename " << argv[1] << "\n";
  // std::cerr << "SVG Pattern Width " << svg.width << "\n";
  // std::cerr << "SVG Pattern Height " << svg.height << "\n";
  // std::cerr << "SVG Polygon Count " << svg.polygons.size() << "\n";
  // std::cerr << "SVG Line Count " << svg.lines.size() << "\n";
  // std::cerr << "-------------------\n";

  HEADER();

  for (unsigned h = 0; h < size; ++h) {
    for (unsigned w = 0; w < size; ++w) {
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
