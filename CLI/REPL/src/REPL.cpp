#include "CLI/REPL.hpp"

#include <algorithm>

namespace TradingEngine::CLI {
REPL::REPL(std::string application_name) : application_name_{std::move(application_name)} {
  auto white_space{std::find_if(application_name_.begin(), application_name_.end(),
                                [](unsigned char ch) { return std::isspace(ch); })};
  if (white_space != application_name_.end()) {
    throw std::invalid_argument("name should not contain white spaces");
  }
  initialize();
}

void REPL::initialize() {
  clean();
  std::cout << "Welcome to " << application_name_
            << "\n you can type help to see list of commands\n";
}

int REPL::run() {
  while (true) {
    std::cout << application_name_ << ">";
    std::string line{};
    if (!std::getline(std::cin, line)) {
      std::cout << '\n';
      return EXIT_SUCCESS;
    }
    std::cout << "\n";
    if (line == "exit") {
      std::cout << "Have a nice day.\n";
      return EXIT_SUCCESS;
    } else if (line == "help") {
      print_help();
      continue;
    } else if (line == "clean") {
      clean();
    }
  }

  return EXIT_FAILURE;
}
void REPL::print_help() {
  std::cout << "Available commands:\n";
  std::cout << "\thelp\t\tprints help\n";
  std::cout << "\tclean\t\tcleans the CLI\n";
  std::cout << "\texit\n";
}

void REPL::clean() {
#if defined _WIN32
  system("cls");
#elif defined(__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
  system("clear");
#elif defined(__APPLE__)
  system("clear");
#endif
}

} // namespace TradingEngine::CLI
