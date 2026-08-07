#pragma once
#include <stdint.h>

#ifdef _WIN32
    #define SD_MOUNTPOINT "D:/"
#else
    #define SD_MOUNTPOINT "/home"
#endif
  // Зміни відповідно до налаштувань системи
#define SD_MAX_FILES 255
