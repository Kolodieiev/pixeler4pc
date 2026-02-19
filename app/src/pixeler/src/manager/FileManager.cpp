#pragma GCC optimize("O3")
#include "FileManager.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <algorithm>
#include <cctype>
#include <cstring>

#include "pixeler/setup/sd_setup.hpp"
#include "pixeler/src/util/AutoLock.h"

#ifdef __linux__
#include <unistd.h>
#endif

#define IDLE_WD_GUARD_TIME 250U
#define OPT_BLOCK_SIZE 16384

namespace pixeler
{
#ifdef _WIN32
#include <windows.h>

  static String ansiToUtf8(const char* ansi_str)
  {
    if (!ansi_str)
      return {};

    int wide_len = MultiByteToWideChar(CP_ACP, 0, ansi_str, -1, nullptr, 0);
    if (wide_len <= 0)
      return {};

    std::wstring wide_str(wide_len - 1, 0);
    MultiByteToWideChar(CP_ACP, 0, ansi_str, -1, wide_str.data(), wide_len);

    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), wide_len - 1, nullptr, 0, nullptr, nullptr);
    if (utf8_len <= 0)
      return {};

    std::string utf8_str(utf8_len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), wide_len - 1, utf8_str.data(), utf8_len, nullptr, nullptr);

    return String(utf8_str.c_str());
  }

  static String utf8ToAnsi(const char* utf8_str)
  {
    if (!utf8_str)
      return {};

    int wide_len = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, nullptr, 0);
    if (wide_len <= 0)
      return {};
    std::wstring wide_str(wide_len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, wide_str.data(), wide_len);

    int ansi_len = WideCharToMultiByte(CP_ACP, 0, wide_str.c_str(), wide_len - 1, nullptr, 0, nullptr, nullptr);
    if (ansi_len <= 0)
      return {};
    std::string ansi_str(ansi_len, 0);
    WideCharToMultiByte(CP_ACP, 0, wide_str.c_str(), wide_len - 1, ansi_str.data(), ansi_len, nullptr, nullptr);

    return String(ansi_str.c_str());
  }
#endif

  String FileManager::makeFullPath(const char* path)
  {
    String full_path = SD_MOUNTPOINT;
#ifdef __linux__
    full_path += "/";
    full_path += getlogin();
#endif
    full_path += path;
    return full_path;
  }

  String FileManager::makeUniqueFilename(const String& file_path)
  {
    uint16_t counter = 1;
    String temp_path = file_path;
    String unique_filename = file_path;
    while (fileExist(unique_filename.c_str(), true))
    {
      unique_filename = temp_path.substring(0, temp_path.lastIndexOf("."));
      unique_filename += "(";
      unique_filename += counter;
      unique_filename += ")";
      unique_filename += temp_path.substring(temp_path.lastIndexOf("."));
      ++counter;
    }

    return unique_filename;
  }

  FileManager::FileManager() : _sd_mutex{xSemaphoreCreateMutex()}
  {
  }

  uint8_t FileManager::getEntryTypeUnlocked(const char* path, dirent* entry)
  {
    struct stat st;

#ifdef _WIN32
    String ansi_str = utf8ToAnsi(path);

    if (stat(ansi_str.c_str(), &st) == 0)
    {
      if (S_ISREG(st.st_mode))
        return DT_REG;
      if (S_ISDIR(st.st_mode))
        return DT_DIR;
    }
#else
    if (entry && entry->d_type != DT_UNKNOWN)
      return entry->d_type;

    if (stat(path, &st) == 0)

    {
      if (S_ISREG(st.st_mode))
        return DT_REG;
      if (S_ISDIR(st.st_mode))
        return DT_DIR;
    }
#endif

    return DT_UNKNOWN;
  }

  size_t FileManager::getFileSize(const char* path)
  {
    AutoLock lock(_sd_mutex);
    return getFileSizeUnlocked(path);
  }

  size_t FileManager::getFileSizeUnlocked(const char* path)
  {
    String full_path = makeFullPath(path);
    struct stat st;

#ifdef _WIN32
    full_path = utf8ToAnsi(full_path.c_str());
#endif

    if (stat(full_path.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
      return 0;

    return static_cast<size_t>(st.st_size);
  }

  bool FileManager::readStat(struct stat& out_stat, const char* path)
  {
    String full_path = makeFullPath(path);
#ifdef _WIN32
    full_path = utf8ToAnsi(full_path.c_str());
#endif

    AutoLock lock(_sd_mutex);

    if (stat(full_path.c_str(), &out_stat) != 0)
      return false;

    return true;
  }

  bool FileManager::fileExist(const char* path, bool silently)
  {
    String full_path = makeFullPath(path);

    AutoLock lock(_sd_mutex);
    bool result = getEntryTypeUnlocked(full_path.c_str()) == DT_REG;

    if (!result && !silently)
      log_e("File %s not found", full_path.c_str());

    return result;
  }

  bool FileManager::dirExist(const char* path, bool silently)
  {
    String full_path = makeFullPath(path);

    AutoLock lock(_sd_mutex);
    bool result = getEntryTypeUnlocked(full_path.c_str()) == DT_DIR;

    if (!result && !silently)
      log_e("Dir %s not found", full_path.c_str());

    return result;
  }

  bool FileManager::exists(const char* path, bool silently)
  {
    String full_path = makeFullPath(path);
    AutoLock lock(_sd_mutex);
    uint8_t type = getEntryTypeUnlocked(full_path.c_str());

    if (type == DT_REG || type == DT_DIR)
      return true;

    log_e("[ %s ] not exist", full_path.c_str());
    return false;
  }

  bool FileManager::createDir(const char* path)
  {
    String full_path = makeFullPath(path);
#ifdef _WIN32
    full_path = utf8ToAnsi(full_path.c_str());
#endif
    errno = 0;

    AutoLock lock(_sd_mutex);

#ifdef _WIN32
    if (mkdir(full_path.c_str()) != 0)
#else
    if (mkdir(full_path.c_str(), 0777) != 0)
#endif
    {
      log_e("Помилка створення директорії: %s", full_path.c_str());
      if (errno == EEXIST)
        log_e("Директорія існує");

      return false;
    }

    return true;
  }

  size_t FileManager::readFile(const char* path, void* out_buffer, size_t len, int32_t seek_pos)
  {
    String full_path = makeFullPath(path);
#ifdef _WIN32
    full_path = utf8ToAnsi(full_path.c_str());
#endif

    AutoLock lock(_sd_mutex);

    int fd = open(full_path.c_str(), O_RDONLY);

    if (seek_pos > 0)
    {
      off_t result = lseek(fd, seek_pos, SEEK_SET);
      if (result == -1)
      {
        log_e("Помилка встановлення позиції(%d) у файлі %s", seek_pos, full_path.c_str());
        close(fd);
        return 0;
      }
    }

    ssize_t bytes_read = read(fd, out_buffer, len);
    if (bytes_read < 0)
    {
      log_e("Помилка читання файлу %s", full_path.c_str());
      close(fd);
      return 0;
    }

    if (bytes_read != static_cast<ssize_t>(len))
      log_e("Прочитано: [ %zd ]  Очікувалося: [ %zu ]", bytes_read, len);

    close(fd);
    return bytes_read;
  }

  size_t FileManager::readFromFile(FILE* file, void* out_buffer, size_t len, size_t seek_pos)
  {
    if (!file)
    {
      log_e("FILE* не повинен бути null");
      return 0;
    }

    if (len == 0)
      return 0;

    int fd = fileno(file);

    AutoLock lock(_sd_mutex);

    if (seek_pos > 0)
    {
      off_t result = lseek(fd, seek_pos, SEEK_SET);
      if (result == -1)
      {
        log_e("Помилка встановлення позиції(%zu) у файлі", seek_pos);
        return 0;
      }
    }

    ssize_t bytes_read = read(fd, out_buffer, len);
    if (bytes_read < 0)
    {
      log_e("Помилка читання з файлу");
      return 0;
    }

    if (bytes_read != static_cast<ssize_t>(len))
      log_e("Прочитано: [ %zd ]  Очікувалося: [ %zu ]", bytes_read, len);

    return bytes_read;
  }

  bool FileManager::readFromFileExact(FILE* file, void* out_buffer, size_t len, size_t seek_pos)
  {
    return readFromFile(file, out_buffer, len, seek_pos) == static_cast<ssize_t>(len);
  }

  size_t FileManager::writeFile(const char* path, const void* buffer, size_t len)
  {
    if (len == 0)
      return 0;

    if (!path || !buffer)
    {
      log_e("Bad arguments");
      return 0;
    }

    String full_path = makeFullPath(path);
#ifdef _WIN32
    full_path = utf8ToAnsi(full_path.c_str());
#endif

    AutoLock lock(_sd_mutex);

    int fd = open(full_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0)
    {
      log_e("Помилка відкриття файлу: %s", full_path.c_str());
      return 0;
    }

    size_t written = writeOptimalUnlocked(fd, buffer, len);

    fsync(fd);
    close(fd);

    return written;
  }

  size_t FileManager::writeToFile(FILE* file, const void* buffer, size_t len)
  {
    if (len == 0)
      return 0;

    if (!file || !buffer)
    {
      log_e("Bad arguments");
      return 0;
    }

    AutoLock lock(_sd_mutex);
    int fd = fileno(file);
    return writeOptimalUnlocked(fd, buffer, len);
  }

  size_t FileManager::writeOptimalUnlocked(int file_desc, const void* buffer, size_t len)
  {
    size_t total_written = 0;
    const uint8_t* ptr = static_cast<const uint8_t*>(buffer);

    size_t full_blocks = len / OPT_BLOCK_SIZE;
    size_t remaining_bytes = len % OPT_BLOCK_SIZE;

    unsigned long ts = millis();
    for (size_t i = 0; i < full_blocks; ++i)
    {
      ssize_t res = write(file_desc, ptr + total_written, OPT_BLOCK_SIZE);
      if (res == -1)
        break;

      total_written += res;

      if (millis() - ts > IDLE_WD_GUARD_TIME)
      {
        delay(1);
        ts = millis();
      }
    }

    if (total_written == (full_blocks * OPT_BLOCK_SIZE) && remaining_bytes > 0)
    {
      ssize_t res = write(file_desc, ptr + total_written, remaining_bytes);
      if (res != -1)
        total_written += res;
    }

    if (total_written != len)
      log_e("Записано: [ %zu ]  Очікувалося: [ %zu ]", total_written, len);

    return total_written;
  }

  FILE* FileManager::openFile(const char* path, const char* mode)
  {
    String full_path = makeFullPath(path);
#ifdef _WIN32
    full_path = utf8ToAnsi(full_path.c_str());
#endif

    AutoLock lock(_sd_mutex);
    FILE* f = fopen(full_path.c_str(), mode);

    if (!f)
      log_e("Помилка взяття дескриптора для %s", full_path.c_str());

    return f;
  }

  void FileManager::closeFile(FILE*& file)
  {
    if (file)
    {
      AutoLock lock(_sd_mutex);
      closeFileUnlocked(file);
    }
  }

  void FileManager::closeFileUnlocked(FILE*& file)
  {
    int fd = fileno(file);
    fsync(fd);
    fclose(file);
    file = nullptr;
  }

  bool FileManager::seekPos(FILE* file, int32_t pos, uint8_t mode)
  {
    if (!file)
      return false;

    int fd = fileno(file);

    AutoLock lock(_sd_mutex);

    off_t result = lseek(fd, pos, mode);
    if (result == -1)
    {
      log_e("Помилка встановлення позиції [%d]", pos);
      return false;
    }

    return true;
  }

  size_t FileManager::getPos(FILE* file)
  {
    if (!file)
      return 0;

    int fd = fileno(file);

    AutoLock lock(_sd_mutex);

    off_t pos = lseek(fd, 0, SEEK_CUR);
    if (pos == -1)
      return 0;

    return pos;
  }

  size_t FileManager::available(FILE* file, size_t file_size)
  {
    AutoLock lock(_sd_mutex);
    return availableUnlocked(file, file_size);
  }

  uint8_t FileManager::getCopyProgress() const
  {
    return _copy_progress;
  }

  size_t FileManager::availableUnlocked(FILE* file, size_t file_size)
  {
    if (!file)
      return 0;

    int fd = fileno(file);

    off_t current_pos = lseek(fd, 0, SEEK_CUR);
    if (current_pos == -1)
      return 0;

    if (file_size < static_cast<size_t>(current_pos))
      return 0;

    return file_size - current_pos;
  }

  void FileManager::rm()
  {
    bool result = false;

    String full_path = makeFullPath(_rm_path.c_str());

    bool is_dir = dirExist(_rm_path.c_str(), true);

    {
      AutoLock lock(_sd_mutex);
      if (!is_dir)
        result = rmFileUnlocked(full_path.c_str());
      else
        result = rmDirUnlocked(full_path.c_str());
    }

    if (result)
      log_i("Успішно видалено: %s", full_path.c_str());

    taskDone(result);
  }

  bool FileManager::rmFileUnlocked(const char* path, bool make_full)
  {
    bool result;

    if (make_full)
    {
      String full_path = makeFullPath(path);
#ifdef _WIN32
      full_path = utf8ToAnsi(full_path.c_str());
#endif

      result = !remove(full_path.c_str());

      if (!result)
        log_e("Помилка видалення файлу: %s", full_path.c_str());
    }
    else
    {
#ifdef _WIN32
      String ansi_str = utf8ToAnsi(path);
      result = !remove(ansi_str.c_str());
#else
      result = !remove(path);
#endif

      if (!result)
        log_e("Помилка видалення файлу: %s", path);
    }

    return result;
  }

  bool FileManager::rmDirUnlocked(const char* path, bool make_full)
  {
    bool result = false;

    dirent* dir_entry{nullptr};
    DIR* dir;

    if (make_full)
    {
      String full_path = makeFullPath(path);
#ifdef _WIN32
      full_path = utf8ToAnsi(full_path.c_str());
#endif

      dir = opendir(full_path.c_str());
    }
    else
    {
#ifdef _WIN32
      String ansi_str = utf8ToAnsi(path);
      dir = opendir(ansi_str.c_str());
#else
      dir = opendir(path);
#endif
    }

    if (!dir)
      goto exit;

    errno = 0;

    while (!_is_canceled)
    {
      dir_entry = readdir(dir);
      if (!dir_entry)
      {
        if (!errno)
          result = true;
        break;
      }

      if (std::strcmp(dir_entry->d_name, ".") == 0 || std::strcmp(dir_entry->d_name, "..") == 0)
        continue;

      String full_path = path;
      full_path += "/";
      full_path += dir_entry->d_name;

      uint8_t entr_type = getEntryTypeUnlocked(full_path.c_str(), dir_entry);

      if (entr_type == DT_REG)
      {
        result = rmFileUnlocked(full_path.c_str());
        if (!result)
          goto exit;
      }
      else if (entr_type == DT_DIR)
      {
        result = rmDirUnlocked(full_path.c_str());

        if (!result)
          goto exit;
      }
      else
      {
        log_e("Невідомий тип або не вдалося прочитати: %s", full_path.c_str());
        goto exit;
      }

      delay(1);
    }

  exit:
    if (dir)
      closedir(dir);

    if (result)
      result = !remove(path);
    else
      log_e("Помилка під час видалення: %s", path);

    return result;
  }

  bool FileManager::rmFile(const char* path)
  {
    AutoLock lock(_sd_mutex);
    return rmFileUnlocked(path, true);
  }

  bool FileManager::rmDir(const char* path)
  {
    AutoLock lock(_sd_mutex);
    _is_canceled = false;
    return rmDirUnlocked(path, true);
  }

  void FileManager::rmTask(void* params)
  {
    FileManager* instance = static_cast<FileManager*>(params);
    instance->rm();
  }

  bool FileManager::startRemoving(const char* path)
  {
    if (_is_working)
    {
      log_e("Вже працює інша задача");
      return false;
    }

    if (!exists(path))
      return false;

    _rm_path = path;
    _is_canceled = false;

    BaseType_t result = xTaskCreatePinnedToCore(rmTask, "rmTask", TASK_SIZE, this, 10, nullptr, 0);

    if (result == pdPASS)
    {
      log_i("rmTask is working now");
      _is_working = true;
      return true;
    }
    else
    {
      log_e("rmTask was not running");
      return false;
    }
  }

  bool FileManager::rename(const char* old_name, const char* new_name)
  {
    if (!exists(old_name))
      return false;

    String old_n = makeFullPath(old_name);
    String new_n = makeFullPath(new_name);

    if (new_n.length() >= old_n.length() &&
        (new_n.c_str()[old_n.length()] == '/' || new_n.c_str()[old_n.length()] == '\0') &&
        strncmp(old_n.c_str(), new_n.c_str(), old_n.length()) == 0)
    {
      log_e("Старе і нове ім'я збігаються або спроба переміщення каталогу до самого себе");
      return false;
    }

#ifdef _WIN32
    old_n = utf8ToAnsi(old_n.c_str());
    new_n = utf8ToAnsi(new_n.c_str());
#endif

    AutoLock lock(_sd_mutex);
    return !::rename(old_n.c_str(), new_n.c_str());
  }

  bool FileManager::copyFileUnlocked(const String& from, const String& to)
  {
#ifdef _WIN32
    String to_ansi_str = utf8ToAnsi(to.c_str());
    int n_fd = open(to_ansi_str.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
#else
    int n_fd = open(to.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);

#endif

    if (n_fd < 0)
    {
      log_e("Помилка створення файлу: %s", to.c_str());
      return false;
    }

#ifdef _WIN32
    String from_ansi_str = utf8ToAnsi(from.c_str());
    int o_fd = open(from_ansi_str.c_str(), O_RDONLY);
#else
    int o_fd = open(from.c_str(), O_RDONLY);
#endif

    if (o_fd < 0)
    {
      log_e("Помилка читання файлу: %s", from.c_str());
      close(n_fd);
      return false;
    }

    size_t buf_size = 1024;
    uint8_t* buffer;

    if (psramInit())
    {
      buf_size *= 160;
      buffer = static_cast<uint8_t*>(ps_malloc(buf_size));
    }
    else
    {
      buf_size *= 16;
      buffer = static_cast<uint8_t*>(malloc(buf_size));
    }

    if (!buffer)
    {
      close(n_fd);
      close(o_fd);

      log_e("Помилка виділення пам'яті: %zu b", buf_size);
      esp_restart();
    }

    size_t file_size = getFileSizeUnlocked(_copy_from_path.c_str());

    if (file_size > 0)
    {
      log_i("Починаю копіювання");
      log_i("З: %s", from.c_str());
      log_i("До: %s", to.c_str());

      size_t writed_bytes_counter{0};
      ssize_t bytes_read;

      off_t current_pos = lseek(o_fd, 0, SEEK_CUR);
      size_t byte_aval = (current_pos != -1 && file_size > static_cast<size_t>(current_pos)) ? file_size - current_pos : 0;

      unsigned long ts = millis();
      while (!_is_canceled && byte_aval > 0)
      {
        size_t to_read = (byte_aval < buf_size) ? byte_aval : buf_size;
        bytes_read = read(o_fd, buffer, to_read);

        if (bytes_read <= 0)
          break;

        writed_bytes_counter += writeOptimalUnlocked(n_fd, buffer, bytes_read);
        _copy_progress = (static_cast<float>(writed_bytes_counter) / file_size) * 100;

        current_pos = lseek(o_fd, 0, SEEK_CUR);
        byte_aval = (current_pos != -1 && file_size > static_cast<size_t>(current_pos)) ? file_size - current_pos : 0;

        if (millis() - ts > IDLE_WD_GUARD_TIME)
        {
          delay(1);
          ts = millis();
        }
      }
    }
    else
    {
      _copy_progress = 100;
      delay(50);
    }

    free(buffer);

    fsync(n_fd);
    close(n_fd);
    close(o_fd);

    return true;
  }

  void FileManager::copyFile()
  {
    _copy_to_path = makeUniqueFilename(_copy_to_path);

    String from = makeFullPath(_copy_from_path.c_str());
    String to = makeFullPath(_copy_to_path.c_str());

    bool result;

    {
      AutoLock lock(_sd_mutex);
      result = copyFileUnlocked(from, to);
    }

    if (_is_canceled)
    {
      log_i("Копіювання скасовано: %s", from.c_str());
      rmFileUnlocked(to.c_str());
      taskDone(false);
    }
    else
    {
      if (result)
        log_i("Успішно скопійовано: %s", from.c_str());
      else
        log_i("Невдача копіювання: %s", from.c_str());

      taskDone(result);
    }
  }

  void FileManager::copyFileTask(void* params)
  {
    FileManager* instance = static_cast<FileManager*>(params);
    instance->copyFile();
  }

  bool FileManager::startCopyFile(const char* from, const char* to)
  {
    if (!from || !to)
    {
      log_e("Bad arguments");
      return false;
    }

    if (_is_working)
    {
      log_e("Вже працює інша задача");
      return false;
    }

    if (!fileExist(from))
      return false;

    _copy_from_path = from;
    _copy_to_path = to;

    _is_canceled = false;
    _copy_progress = 0;

    BaseType_t result = xTaskCreatePinnedToCore(copyFileTask, "copyFileTask", TASK_SIZE, this, 10, nullptr, 0);

    if (result == pdPASS)
    {
      log_i("copyFileTask is working now");
      _is_working = true;
      return true;
    }
    else
    {
      log_e("copyFileTask was not running");
      return false;
    }
  }

  void FileManager::index(std::vector<FileInfo>& out_vec, const char* dir_path, IndexMode mode, const char* file_ext)
  {
    out_vec.clear();
    out_vec.reserve(40);

    if (!dirExist(dir_path))
      return;

    String full_path = makeFullPath(dir_path);

    AutoLock lock(_sd_mutex);
    DIR* dir = opendir(full_path.c_str());
    if (!dir)
    {
      log_e("Помилка відкриття директорії %s", full_path.c_str());
      return;
    }

    dirent* dir_entry{nullptr};
    String filename;
    bool is_dir;

    unsigned long ts = millis();

    while (1)
    {
      dir_entry = readdir(dir);
      if (!dir_entry)
        break;

#ifdef _WIN32
      filename = ansiToUtf8(dir_entry->d_name);
#else
      filename = dir_entry->d_name;
#endif

      if (filename.equals(".") || filename.equals(".."))
        continue;

      String full_name{full_path};
      full_name += "/";
      full_name += filename;

      uint8_t entr_type = getEntryTypeUnlocked(full_name.c_str(), dir_entry);

      if (entr_type == DT_REG)
        is_dir = false;
      else if (entr_type == DT_DIR)
        is_dir = true;
      else
        continue;

      switch (mode)
      {
        case INDX_MODE_DIR:
          if (is_dir)
            out_vec.emplace_back(filename, true);
          break;
        case INDX_MODE_FILES:
          if (!is_dir)
            out_vec.emplace_back(filename, false);
          break;
        case INDX_MODE_FILES_EXT:
          if (!is_dir && filename.endsWith(file_ext))
            out_vec.emplace_back(filename, false);
          break;
        case INDX_MODE_ALL:
          if (is_dir)
            out_vec.emplace_back(filename, true);
          else
            out_vec.emplace_back(filename, false);
          break;
      }

      if (millis() - ts > IDLE_WD_GUARD_TIME)
      {
        delay(1);
        ts = millis();
      }
    }

    out_vec.shrink_to_fit();
    std::sort(out_vec.begin(), out_vec.end());

    if (dir)
      closedir(dir);
  }

  void FileManager::indexFilesExt(std::vector<FileInfo>& out_vec, const char* dir_path, const char* file_ext)
  {
    return index(out_vec, dir_path, INDX_MODE_FILES_EXT, file_ext);
  }

  void FileManager::indexFiles(std::vector<FileInfo>& out_vec, const char* dir_path)
  {
    return index(out_vec, dir_path, INDX_MODE_FILES);
  }

  void FileManager::indexDirs(std::vector<FileInfo>& out_vec, const char* dir_path)
  {
    return index(out_vec, dir_path, INDX_MODE_DIR);
  }

  void FileManager::indexAll(std::vector<FileInfo>& out_vec, const char* dir_path)
  {
    return index(out_vec, dir_path, INDX_MODE_ALL);
  }

  void FileManager::taskDone(bool result)
  {
    _is_working = false;

    _last_task_result = result;

    if (_doneHandler)
      _doneHandler(result, _doneArg);
  }

  void FileManager::cancel()
  {
    _is_canceled = true;
  }

  void FileManager::setTaskDoneHandler(TaskDoneHandler handler, void* arg)
  {
    _doneHandler = handler;
    _doneArg = arg;
  }

  bool FileManager::isWorking() const
  {
    return _is_working;
  }

  bool FileManager::lastTaskResult() const
  {
    return _last_task_result;
  }

  //------------------------------------------------------------------------------------------------------------------------

  bool FileManager::isMounted() const
  {
    if (_pdrv == 0xFF)
    {
      log_e("Карту пам'яті не примонтовано");
      return false;
    }

    AutoLock lock(_sd_mutex);
    return isMountedUnlocked();
  }

  bool FileManager::isMountedUnlocked() const
  {
    String path_to_root = SD_MOUNTPOINT;
    path_to_root += "/";

    struct stat st;
    if (stat(path_to_root.c_str(), &st) != 0)
    {
      log_e("Помилка читання stat під час монтування SD");
      return false;
    }

    return S_ISDIR(st.st_mode);
  }

  bool FileManager::mount()
  {
    return true;
  }

  void FileManager::unmount()
  {
    // stub
  }

  FileManager _fs;
}  // namespace pixeler
