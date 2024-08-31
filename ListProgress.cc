#include "ListProgress.hh"

#include <iostream>

void
outputMlistProgress(int completed, int total)
{
  std::cout << "\033[s\033[K" // Save the cursor position & clear following text
    << "(" << completed << "/" << total << ")" << std::flush
    << "\033[u"; // Restore the cursor to the saved position
}
