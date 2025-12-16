#pragma GCC optimize("O3")
#include "FileManager.h"

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>

#include "../setup/sd_setup.h"

#ifdef __linux__
#include <unistd.h>
#endif

#define IDLE_WD_GUARD_TIME 250U

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

  void FileManager::makeFullPath(String& out_path, const char* path)
  {
    out_path = SD_MOUNTPOINT;
#ifdef __linux__
    out_path += "/";
    out_path += getlogin();
#endif
    out_path += path;
  }

  uint8_t FileManager::getEntryType(const char* path, dirent* entry)
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
    String full_path;
    makeFullPath(full_path, path);

    struct stat st;

#ifdef _WIN32
    String ansi_str = utf8ToAnsi(full_path.c_str());
    if (stat(ansi_str.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
      return 0;
#else
    if (stat(full_path.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
      return 0;
#endif

    return static_cast<size_t>(st.st_size);
  }

  bool FileManager::readStat(struct stat& out_stat, const char* path)
  {
    String full_path;
    makeFullPath(full_path, path);

#ifdef _WIN32
    String ansi_str = utf8ToAnsi(full_path.c_str());
    if (stat(ansi_str.c_str(), &out_stat) != 0)
      return false;
#else
    if (stat(full_path.c_str(), &out_stat) != 0)
      return false;
#endif

    return true;
  }

  bool FileManager::fileExist(const char* path, bool silently)
  {
    String full_path;
    makeFullPath(full_path, path);

    bool result = getEntryType(full_path.c_str()) == DT_REG;

    if (!result && !silently)
      log_e("File %s not found", full_path.c_str());

    return result;
  }

  bool FileManager::dirExist(const char* path, bool silently)
  {
    String full_path;
    makeFullPath(full_path, path);

    bool result = getEntryType(full_path.c_str()) == DT_DIR;

    if (!result && !silently)
      log_e("Dir %s not found", full_path.c_str());

    return result;
  }

  bool FileManager::exists(const char* path, bool silently)
  {
    String full_path;
    makeFullPath(full_path, path);

    uint8_t type = getEntryType(full_path.c_str());

    if (type == DT_REG || type == DT_DIR)
      return true;

    log_e("[ %s ] not exist", full_path.c_str());
    return false;
  }

  bool FileManager::createDir(const char* path)
  {
    String full_path;
    makeFullPath(full_path, path);

    errno = 0;

    bool result = false;

#ifdef _WIN32
    String ansi_str = utf8ToAnsi(full_path.c_str());
    result = !mkdir(ansi_str.c_str());
#else
    result = !mkdir(full_path.c_str(), 0777);
#endif

    if (!result)
    {
      log_e("Помилка створення директорії: %s", full_path.c_str());
      if (errno == EEXIST)
        log_e("Директорія існує");
    }

    return result;
  }

  size_t FileManager::readFile(const char* path, void* out_buffer, size_t len, int32_t seek_pos)
  {
    String full_path;
    makeFullPath(full_path, path);

#ifdef _WIN32
    String ansi_str = utf8ToAnsi(full_path.c_str());
    FILE* f = fopen(ansi_str.c_str(), "rb");
#else
    FILE* f = fopen(full_path.c_str(), "rb");
#endif

    if (!f)
    {
      log_e("Помилка відкриття файлу: %s", path);
      return 0;
    }

    if (seek_pos > 0 && fseek(f, seek_pos, SEEK_SET))
    {
      log_e("Помилка встановлення позиції(%d) у файлі %s", seek_pos, path);
      fclose(f);
      return 0;
    }

    size_t bytes_read = fread(out_buffer, 1, len, f);

    if (bytes_read != len)
      log_e("Прочитано: [ %zu ]  Очікувалося: [ %zu ]", bytes_read, len);

    fclose(f);
    return bytes_read;
  }

  bool FileManager::readChunkFromFile(FILE* file, void* out_buffer, size_t len, size_t seek_pos)
  {
    if (len == 0)
      return false;

    if (!file)
    {
      log_e("FILE* не повинен бути null");
      return false;
    }

    if (seek_pos > 0 && fseek(file, seek_pos, SEEK_SET))
    {
      log_e("Помилка встановлення позиції: %zu", seek_pos);
      return false;
    }

    size_t section_read = fread(out_buffer, len, 1, file);

    if (section_read == 0)
    {
      log_e("Не вдалося прочитати всі %zu байтів", len);
      return false;
    }

    return true;
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

    if (seek_pos > 0 && fseek(file, seek_pos, SEEK_SET))
      return 0;

    size_t bytes_read = fread(out_buffer, 1, len, file);

    if (bytes_read != len)
      log_e("Прочитано: [ %zu ]  Очікувалося: [ %zu ]", bytes_read, len);

    return bytes_read;
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

    String full_path;
    makeFullPath(full_path, path);

#ifdef _WIN32
    String ansi_str = utf8ToAnsi(full_path.c_str());
    FILE* f = fopen(ansi_str.c_str(), "wb");
#else
    FILE* f = fopen(full_path.c_str(), "wb");
#endif

    if (!f)
    {
      log_e("Помилка відркиття файлу: %s", path);
      return 0;
    }

    size_t written = writeOptimal(f, buffer, len);

    fclose(f);

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

    return writeOptimal(file, buffer, len);
  }

  size_t FileManager::writeOptimal(FILE* file, const void* buffer, size_t len)
  {
    size_t opt_size = 256;
    size_t total_written = 0;

    size_t full_blocks = len / opt_size;
    size_t remaining_bytes = len % opt_size;

    unsigned long ts = millis();
    for (size_t i = 0; i < full_blocks; ++i)
    {
      total_written += fwrite(static_cast<const uint8_t*>(buffer) + total_written, opt_size, 1, file) * opt_size;
      if (millis() - ts > IDLE_WD_GUARD_TIME)
      {
        delay(1);
        ts = millis();
      }
    }

    if (remaining_bytes > 0)
      total_written += fwrite(static_cast<const uint8_t*>(buffer) + total_written, remaining_bytes, 1, file) * remaining_bytes;

    fflush(file);

    if (total_written != len)
      log_e("Записано: [ %zu ]  Очікувалося: [ %zu ]", total_written, len);

    return total_written;
  }

  FILE* FileManager::openFile(const char* path, const char* mode)
  {
    String full_path;
    makeFullPath(full_path, path);

#ifdef _WIN32
    String ansi_str = utf8ToAnsi(full_path.c_str());
    FILE* f = fopen(ansi_str.c_str(), mode);
#else
    FILE* f = fopen(full_path.c_str(), mode);
#endif

    if (!f)
      log_e("Помилка взяття дескриптора для %s", full_path.c_str());

    return f;
  }

  void FileManager::closeFile(FILE*& file)
  {
    if (file)
    {
      fclose(file);
      file = nullptr;
    }
  }

  bool FileManager::seekPos(FILE* file, int32_t pos, uint8_t mode)
  {
    if (!file)
      return false;

    if (fseek(file, pos, mode))
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

    return ftell(file);
  }

  size_t FileManager::available(FILE* file, size_t file_size)
  {
    if (!file || feof(file))
      return 0;

    long tell = ftell(file);

    if (file_size < tell)
      return 0;

    return file_size - tell;
  }

  void FileManager::rm()
  {
    bool result = false;

    String full_path;
    makeFullPath(full_path, _rm_path.c_str());

    bool is_dir = dirExist(_rm_path.c_str(), true);

    if (!is_dir)
      result = rmFile(full_path.c_str());
    else
      result = rmDir(full_path.c_str());

    if (result)
      log_i("Успішно видалено: %s", full_path.c_str());

    taskDone(result);
  }

  bool FileManager::rmFile(const char* path, bool make_full)
  {
    bool result;

    if (make_full)
    {
      String full_path;
      makeFullPath(full_path, path);

#ifdef _WIN32
      String ansi_str = utf8ToAnsi(full_path.c_str());
      result = !remove(ansi_str.c_str());
#else
      result = !remove(full_path.c_str());
#endif
    }
    else
    {
#ifdef _WIN32
      String ansi_str = utf8ToAnsi(path);
      result = !remove(ansi_str.c_str());
#else
      result = !remove(path);
#endif
    }

    if (!result)
      log_e("Помилка видалення файлу: %s", path);

    return result;
  }

  bool FileManager::rmDir(const char* path, bool make_full)
  {
    bool result = false;

    dirent* dir_entry{nullptr};
    DIR* dir;

    if (make_full)
    {
      String full_path;
      makeFullPath(full_path, path);

#ifdef _WIN32
      String ansi_str = utf8ToAnsi(full_path.c_str());
      dir = opendir(ansi_str.c_str());
#else
      dir = opendir(full_path.c_str());
#endif
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

      uint8_t entr_type = getEntryType(full_path.c_str(), dir_entry);

      if (entr_type == DT_REG)
      {
        result = rmFile(full_path.c_str());
        if (!result)
          goto exit;
      }
      else if (entr_type == DT_DIR)
      {
        result = rmDir(full_path.c_str());

        if (!result)
          goto exit;
      }
      else
      {
        log_e("Невідомий тип або не вдалося прочитати: %s", path);
        goto exit;
      }

      delay(1);
    }

  exit:
    if (dir)
      closedir(dir);

    if (result)
      result = !rmdir(path);
    else
      log_e("Помилка під час видалення: %s", path);

    return result;
  }

  void FileManager::rmTask(void* params)
  {
    FileManager* instance = static_cast<FileManager*>(params);
    instance->_is_working = true;
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

    String old_n;
    makeFullPath(old_n, old_name);
    String new_n;
    makeFullPath(new_n, new_name);

    if (new_n.length() >= old_n.length() &&
        (new_n.c_str()[old_n.length()] == '/' || new_n.c_str()[old_n.length()] == '\0') &&
        strncmp(old_n.c_str(), new_n.c_str(), old_n.length()) == 0)
    {
      log_e("Старе і нове ім'я збігаються або спроба переміщення каталогу до самого себе");
      return false;
    }

#ifdef _WIN32
    String old_ansi_str = utf8ToAnsi(old_n.c_str());
    String new_ansi_str = utf8ToAnsi(new_n.c_str());
    return !::rename(old_ansi_str.c_str(), new_ansi_str.c_str());
#else
    return !::rename(old_n.c_str(), new_n.c_str());
#endif
  }

  void FileManager::copyFile()
  {
    uint16_t counter = 1;
    String to_temp = _copy_to_path;
    while (_copy_from_path.equals(_copy_to_path) || fileExist(_copy_to_path.c_str(), true))
    {
      _copy_to_path = to_temp.substring(0, to_temp.lastIndexOf("."));
      _copy_to_path += "(";
      _copy_to_path += counter;
      _copy_to_path += ")";
      _copy_to_path += to_temp.substring(to_temp.lastIndexOf("."));
      ++counter;
    }

    String from;
    String to;

    makeFullPath(from, _copy_from_path.c_str());
    makeFullPath(to, _copy_to_path.c_str());

#ifdef _WIN32
    String to_ansi_str = utf8ToAnsi(to.c_str());
    FILE* n_f = fopen(to_ansi_str.c_str(), "ab");
#else
    FILE* n_f = fopen(to.c_str(), "ab");
#endif

    if (!n_f)
    {
      log_e("Помилка створення файлу: %s", to.c_str());
      taskDone(false);
      return;
    }

#ifdef _WIN32
    String from_ansi_str = utf8ToAnsi(from.c_str());
    FILE* o_f = fopen(from_ansi_str.c_str(), "rb");
#else
    FILE* o_f = fopen(from.c_str(), "rb");
#endif

    if (!o_f)
    {
      log_e("Помилка читання файлу: %s", from.c_str());
      fclose(n_f);
      taskDone(false);
      return;
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
      fclose(n_f);
      fclose(o_f);

      log_e("Помилка виділення пам'яті: %zu b", buf_size);
      esp_restart();
    }

    size_t file_size = getFileSize(_copy_from_path.c_str());

    if (file_size > 0)
    {
      log_i("Починаю копіювання");
      log_i("Із: %s", from.c_str());
      log_i("До: %s", to.c_str());

      size_t writed_bytes_counter{0};
      size_t bytes_read;
      size_t byte_aval = 0;

      while (!_is_canceled && (byte_aval = available(o_f, file_size)) > 0)
      {
        if (byte_aval < buf_size)
          bytes_read = fread(buffer, byte_aval, 1, o_f) * byte_aval;
        else
          bytes_read = fread(buffer, buf_size, 1, o_f) * buf_size;
        //
        writed_bytes_counter += writeOptimal(n_f, buffer, bytes_read);
        _copy_progress = ((float)writed_bytes_counter / file_size) * 100;
      }
    }
    else
    {
      _copy_progress = 100;
      delay(50);
    }

    free(buffer);

    fclose(n_f);
    fclose(o_f);

    if (_is_canceled)
    {
      rmFile(to.c_str());
      taskDone(false);
    }
    else
    {
      taskDone(true);
    }
  }

  void FileManager::copyFileTask(void* params)
  {
    FileManager* instance = static_cast<FileManager*>(params);
    instance->_is_working = true;
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
      return true;
    }
    else
    {
      log_e("copyFileTask was not running");
      return false;
    }
  }

  void FileManager::startIndex(std::vector<FileInfo>& out_vec, const char* dir_path, IndexMode mode, const char* file_ext)
  {
    if (!dirExist(dir_path))
      return;

    String full_path;
    makeFullPath(full_path, dir_path);

    DIR* dir = opendir(full_path.c_str());
    if (!dir)
    {
      log_e("Помилка відкриття директорії %s", full_path.c_str());
      taskDone(false);
      return;
    }

    out_vec.clear();
    out_vec.reserve(40);

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

      uint8_t entr_type = getEntryType(full_name.c_str(), dir_entry);

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
    return startIndex(out_vec, dir_path, INDX_MODE_FILES_EXT, file_ext);
  }

  void FileManager::indexFiles(std::vector<FileInfo>& out_vec, const char* dir_path)
  {
    return startIndex(out_vec, dir_path, INDX_MODE_FILES);
  }

  void FileManager::indexDirs(std::vector<FileInfo>& out_vec, const char* dir_path)
  {
    return startIndex(out_vec, dir_path, INDX_MODE_DIR);
  }

  void FileManager::indexAll(std::vector<FileInfo>& out_vec, const char* dir_path)
  {
    return startIndex(out_vec, dir_path, INDX_MODE_ALL);
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

  //------------------------------------------------------------------------------------------------------------------------

  FileInfo::FileInfo(const String& name, bool is_dir) : _name_len{name.length()}, _is_dir{is_dir}
  {
    if (psramInit())
      _name = static_cast<char*>(ps_malloc(_name_len + 1));
    else
      _name = static_cast<char*>(malloc(_name_len + 1));

    if (!_name)
    {
      log_e("Помилка виділення буферу");
      esp_restart();
    }

    std::memcpy(_name, name.c_str(), _name_len);
    _name[_name_len] = '\0';
  }

  FileInfo::~FileInfo()
  {
    free(_name);
  }

  bool FileInfo::nameEndsWith(const char* suffix) const
  {
    if (!suffix)
      return false;

    size_t suffix_len = strlen(suffix);

    if (_name_len < suffix_len)
      return false;

    return strcmp(_name + _name_len - suffix_len, suffix) == 0;
  }

  bool FileInfo::operator<(const FileInfo& other) const
  {
    if (_is_dir != other._is_dir)
      return _is_dir;

    const char* lhs = _name;
    const char* rhs = other._name;

    while (*lhs && *rhs)
    {
      if (std::isdigit(*lhs) && std::isdigit(*rhs))
      {
        char* end_lhs;
        char* end_rhs;
        long num_lhs = std::strtol(lhs, &end_lhs, 10);
        long num_rhs = std::strtol(rhs, &end_rhs, 10);

        if (num_lhs != num_rhs)
        {
          return num_lhs < num_rhs;
        }
        lhs = end_lhs;
        rhs = end_rhs;
      }
      else
      {
        if (*lhs != *rhs)
        {
          return *lhs < *rhs;
        }
        ++lhs;
        ++rhs;
      }
    }

    return std::strcmp(lhs, rhs) < 0;
  }

  FileInfo::FileInfo(FileInfo&& other) noexcept : _name(other._name), _is_dir(other._is_dir), _name_len{other._name_len}
  {
    other._name = nullptr;
  }

  FileInfo& FileInfo::operator=(FileInfo&& other) noexcept
  {
    if (this != &other)
    {
      free(_name);
      _name = other._name;
      _is_dir = other._is_dir;
      _name_len = other._name_len;
      other._name = nullptr;
    }
    return *this;
  }

  //------------------------------------------------------------------------------------------------------------------------

  FileManager _fs;
}  // namespace pixeler