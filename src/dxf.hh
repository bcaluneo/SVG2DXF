// Copyright (C) Brendan Caluneo

#ifndef DXF_HH
#define DXF_HH

// Some macros for writing DXF files.
// These are from the spec found here: https://images.autodesk.com/adsk/files/autocad_2012_pdf_dxf-reference_enu.pdf

#define POLYLINE() std::cout << "\t0\nPOLYLINE\n\t8\n0\n\t70\n1\n";
#define VERTEX(x, y) std::cout << "\t0\nVERTEX\n\t10\n" << x << "\n\t20\n" << y << "\n";
#define SEQEND() std::cout << "\t0\nSEQEND\n";
#define HEADER() std::cout << "\t0\nSECTION\n\t2\nHEADER\n\t9\n$ACADVER\n\t1\nAC1006\n\t9\n$EXTMIN\n\t10\n0.000000\n\t20\n0.000000\n\t30\n0.000000\n\t9\n$EXTMAX\n\t10\n800.000000\n\t20\n600.000000\n\t30\n0.000000\n\t0\nENDSEC\n\t0\nSECTION\n\t2\nENTITIES\n";
#define FOOTER() std::cout << "\t0\nENDSEC\n\t0\nEOF";
#define LINE(x1, y1, x2, y2) std::cout << "\t0\nLINE\n\t39\n5\n\t10\n" << x1 << "\n\t20\n" << y1 << "\n\t11\n" << x2 << "\n\t21\n" << y2 << "\n";

#endif
