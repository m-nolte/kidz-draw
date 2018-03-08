// Cursor Format:
//
// Each character represents a pixel of the cursor:
// ' ' transparent
// '.' white
// 'x' black
// '^' transparent hot spot
// ':' white hot spot
// 'X' black hot spot
//
// All characters after a '#' are ignored as comments. Any spaces before the '#'
// are removed. If '#' is the first character on the line (no leading spaces),
// the line is ignored completely.

extern const char arrow_cursor[];
extern const char empty_cursor[];

const char arrow_cursor[]
  = "X\n"
    "xx\n"
    "x.x\n"
    "x..x\n"
    "x...x\n"
    "x....x\n"
    "x.....x\n"
    "x......x\n"
    "x.......x\n"
    "x........x\n"
    "x.....xxxxx\n"
    "x..x..x\n"
    "xx x..x\n"
    "x   x..x\n"
    "    x..x\n"
    "     x..x\n"
    "     x..x\n"
    "      xx\n";

const char empty_cursor[] = "^\n";
