# SVG2DXF

This program parses SVG files and converts them into DXF files that can be used in CAD programs. It was mainly written for simple, one pattern files and hasn't been tested with any extensive SVG files.

As an example, this tiled SVG file

<img src="https://user-images.githubusercontent.com/43048565/138568238-e4ee97b8-9051-43eb-878c-b2e3aa31d6b7.png" alt="drawing" width="80%"/>

produces this output DXF file (displayed in EzCad 2)

![image](https://user-images.githubusercontent.com/43048565/138568253-4ab13dcd-6043-4e3b-b326-a2a1a6c7975a.png)

## Usage
`./svg2dxf [-s SIZE] Input.svg > Output.dxf`

The optional size parameter is used to fill patterns in a desired NxN space.

## Building

You can use the provided make file to build this project. You will have to manually configure the binary's output directory.

#### Dependencies
* make
* C++ compiler, default is gcc
