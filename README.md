Завдяки цьому порту досягається двостороння сумісніть коду для GUI між мікроконтролером та Linux-ПК, що допоможе значно пришвидшити навчання, розробку ігор та тестування GUI з використанням Pixeler-фреймворку.

Документацію Pixeler можна знайти в репозиторію фреймворку.

В якості рендера вікна використовується SFML версії 3.0.2. 
Для збірки проєкту використовуйте компілятор, що рекомендується розробником SFML за цим посиланням: [Download SFML 3.0.2](https://www.sfml-dev.org/download/sfml/3.0.2/).

Встановлення SFML 3.0.2 локально та налаштування збірки:
```bash
# 0. Встановлення залежностей
sudo apt update && sudo apt install -y \
    cmake build-essential \
    libfreetype6-dev libx11-dev libxrandr-dev libudev-dev \
    libgl1-mesa-dev libflac-dev libvorbis-dev libopenal-dev \
    libxcursor-dev libxi-dev libxinerama-dev libogg-dev \
    libmbedtls-dev libssh2-1-dev libfreetype-dev libharfbuzz-dev

# 1. Завантаження вихідного коду
git clone -b 3.0.2 https://github.com/SFML/SFML.git sfml-3.0.2
cd sfml-3.0.2

# 2. Конфігурація (встановлюємо локально в /usr/local)
cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DSFML_BUILD_EXAMPLES=OFF

# 3. Компіляція (використовуємо всі ядра процесора)
cmake --build build -j$(nproc)

# 4. Локальне встановлення в систему
sudo cmake --install build

# 5. Зміна налаштувань в CMakeLists
set(EN_GIT_BUILD OFF) # GIT SFML 3.0.2
set(EN_LOCAL_BUILD ON) # Локальний SFML 3.0.2
set(EN_STATIC_SFML_LIB ON) # Локальний SFML 3.0.2. Якщо бібліотеки зібрані статично - ON. Інакше OFF.

```

![alt text](resources/image/sokoban.png)
<br>
![alt text](resources/image/cxxdroid_menu.png)
