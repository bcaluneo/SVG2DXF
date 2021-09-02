# SVG2DXF

This program parses SVG files and converts them into DXF files that can be used in CAD programs. It was mainly written for simple, one pattern files and hasn't been tested with any extensive SVG files.

## Usage
`./svg2dxf [-s SIZE] Input.svg > Output.dxf`
The optional size parameter is used to fill patterns in a desired NxN space.

## Building

#### Dependencies
* make
* C++ compiler, default is gcc
