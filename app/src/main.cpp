
#include <cstdint>

#include "pixeler/manager/ContextManager.h"
using namespace pixeler;

#ifdef _WIN32
#include <windows.h>
#define CP_UTF8 65001
void activate_utf8_on_windows()
{
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
}
#endif

int main()
{
#ifdef _WIN32
  activate_utf8_on_windows();
#endif

  ContextManager context_manager;
  context_manager.run();

  return 0;
}
