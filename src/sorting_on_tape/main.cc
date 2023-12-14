#include <tape_sorter.h>
#include <temp_file_tape_provider.h>

#include <iostream>

#include "file_tape.h"
#include "tape.h"

using namespace sot;

using TapeValue = std::int32_t;
using Comparator = std::function<bool(const TapeValue &, const TapeValue &)>;
using Tape = Tape<TapeValue>;
using MutableFileTape = FileTape<TapeValue, true>;
using ImmutableFileTape = FileTape<TapeValue, false>;

Comparator ParseSortingOrder(const std::string &order_str) {
  if (order_str == "asc") {
    return std::less();
  }
  if (order_str == "desc") {
    return std::greater();
  }
  throw std::invalid_argument(
      "Invalid sorting order! Use 'asc' for ascending order or 'desc' for descending order."
  );
}

template <typename Comparator>
std::string SortingOrderToString() {
  if (typeid(Comparator) == typeid(std::less<TapeValue>)) {
    return "ascending";
  } else {
    return "descending";
  }
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cout << "Error: expected input file to sort and output file to print result.\n"
              << "Example: ./sorting-on-tape-runnable input output" << std::endl;
    return 1;
  }
  try {
    const Configuration config;
    const auto temp_tape_provider = std::make_shared<TempFileTapeProvider<TapeValue>>(config);
    const auto input_file_name = argv[1];
    const auto output_file_name = argv[2];
    const auto comparator = argc >= 4 ? ParseSortingOrder(argv[3]) : std::less();

    ImmutableFileTape input_file_tape(config, input_file_name);
    MutableFileTape output_file_tape(config, output_file_name);

    std::cout << "Start sorting data from '" << input_file_name << "' to '" << output_file_name
              << "' in " << SortingOrderToString<Comparator>() << " order." << std::endl;
    TapeSorter<TapeValue, decltype(comparator)>(config, temp_tape_provider, comparator)
        .Sort(input_file_tape, output_file_tape);
    std::cout << "The data has been successfully sorted!" << std::endl;
  } catch (std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  } catch (...) {
    std::cout << "Unknown error..." << std::endl;
  }
  return 0;
}
