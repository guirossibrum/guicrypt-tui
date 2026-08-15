#include <iostream>
#include "ui/screen.h"
#include "core/gocryptfs.h"

int main() {
  std::cout << "\033]0;guicrypt-tui\007" << std::flush;

  if (!guicrypt::Gocryptfs::check_installed()) {
    std::cerr << "gocryptfs not found. Attempting to install..." << std::endl;
    if (!guicrypt::Gocryptfs::install()) {
      std::cerr << "Failed to install gocryptfs. Please install manually." << std::endl;
      return 1;
    }
  }

  guicrypt::Screen app;
  app.run();
  return 0;
}
