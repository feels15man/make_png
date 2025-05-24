# Makefile and elf types
This is a c program that requires zlib, freetype and libpng. So, program draw a text in a png file using any font. It can be built as static, dynamic or blob elf. If u built it as blob, then u need to 
use an elfloader and pass requirement functions.

## Usage
./elf font_file output_png text
