#include <iostream>

#include "file_tape.h"
#include "tape.h"

using namespace sot;

using Tape = Tape<char>;
using MutableFileTape = FileTape<char, true>;
using ImmutableFileTape = FileTape<char, false>;

int main() {
  const Configuration config;
  MutableFileTape mutable_file_tape(config, "mutable");
  return 0;
}
