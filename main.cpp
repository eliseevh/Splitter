#include <cxxopts.hpp>
#include <fstream>
#include <iostream>

namespace {
constexpr std::streamsize BUFFER_SIZE = 4096;

void help(cxxopts::Options const& options, cxxopts::ParseResult const& result) {
  std::cout << options.help() << std::endl;
  std::cout
      << R"(In split mode output parts filenames will be "<full>_1.prt", "<full>_2.prt" and so on)"
      << std::endl;
  if (result["help"].count() != 0) {
    return;
  }
  if (result["split"].count() + result["merge"].count() != 1) {
    std::cerr << R"(Only one of "split" and "merge" options must be provided)"
              << std::endl;
  }
  if (result["full"].count() != 1) {
    std::cerr << "Full file name must be provided" << std::endl;
  }
  if (result["parts_size"].count() > 1) {
    std::cerr << "Parts size must be provided only once" << std::endl;
  }
  if (result["split"].count() + result["parts"].count() != 1) {
    std::cerr << "Parts file names cannot be provided in split mode"
              << std::endl;
  }
  if (!result.unmatched().empty()) {
    std::cerr << "Unknown options:";
    for (std::string const& option : result.unmatched()) {
      std::cerr << " " << option;
    }
    std::cerr << std::endl;
  }
}
void move(std::istream& input, std::ostream& output, std::string& buffer,
          std::streamsize buffer_size) {
  input.read(buffer.data(), buffer_size);
  output.write(buffer.data(), input.gcount());
}
} // namespace
int main(int argc, char** argv) {
  cxxopts::Options options(
      "Splitter", "Tool for splitting files or merging some files in one");
  options.add_options()
      ("s,split", "split mode")
      ("m,merge", "merge mode")
      ("f,full", "full file name", cxxopts::value<std::string>(),
      "filename")
      ("p,parts", "parts file names",
      cxxopts::value<std::vector<std::string>>(), "filename1,filename2...")
      ("parts_size", "parts size in bytes",
      cxxopts::value<std::streamsize>()
      ->default_value("4096"), "size")
      ("h,help", "show help message");

  try {
    auto result = options.parse(argc, argv);
    if (result["help"].count() != 0) {
      help(options, result);
      return 0;
    } else if (!(result["split"].count() + result["merge"].count() == 1 &&
          result["full"].count() == 1 && result["parts_size"].count() <= 1 &&
          result["split"].count() + result["parts"].count() == 1)) {
      help(options, result);
      return 1;
    }

    if (result["split"].count() != 0) {
      std::streamsize output_size = result["parts_size"].as<std::streamsize>();
      std::string input_filename = result["full"].as<std::string>();
      std::ifstream input(input_filename);
      size_t count_outputs = 0;
      std::string buffer;
      buffer.reserve(output_size);
      while (!input.eof()) {
        ++count_outputs;
        std::string output_filename =
            input_filename + "_" + std::to_string(count_outputs) + ".prt";
        std::ofstream output(output_filename);
        move(input, output, buffer, output_size);
      }
      std::cout << "Total created files: " << count_outputs << std::endl;
    } else {
      std::vector<std::string> inputs =
          result["parts"].as<std::vector<std::string>>();
      std::ofstream output(result["full"].as<std::string>());
      std::string buffer;
      buffer.reserve(BUFFER_SIZE);
      for (auto const& input_filename : inputs) {
        std::ifstream input(input_filename);
        while (!input.eof()) {
          move(input, output, buffer, BUFFER_SIZE);
        }
      }
    }
  } catch (cxxopts::OptionParseException const& e) {
    std::cerr << "Options parsing error: " << e.what() << std::endl;
    return 1;
  } catch (std::fstream::failure const& e) {
    std::cerr << "I/O error: " << e.what() << std::endl;
    return 1;
  }
}
