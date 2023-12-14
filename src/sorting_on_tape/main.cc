#include <tape_sorter.h>
#include <temp_file_tape_provider.h>

#include <iostream>

#include "file_tape.h"
#include "tape.h"

using namespace sot;

using TapeValue = char;
using Tape = Tape<TapeValue>;
using MutableFileTape = FileTape<TapeValue, true>;
using ImmutableFileTape = FileTape<TapeValue, false>;

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cout << "Error: expected input file to sort and output file to print result.\n"
              << "Example: ./sorting-on-tape-runnable input output" << std::endl;
    return 1;
  }
  try {
    const Configuration config;
    ImmutableFileTape input_file_tape(config, argv[1]);
    MutableFileTape output_file_tape(config, argv[2]);
    const auto temp_tape_provider = std::make_shared<TempFileTapeProvider<TapeValue>>(config);
    TapeSorter<TapeValue>(config, temp_tape_provider).Sort(input_file_tape, output_file_tape);
    std::cout << "The data has been successfully sorted!";
  } catch (std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  } catch (...) {
    std::cout << "Unknown error..." << std::endl;
  }
  return 0;
}
