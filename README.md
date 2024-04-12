# Telegram bot

<!-- TODO: Описание бота -->

## Настройка окружения

Лучше всего использовать Linux. OSX тоже будет нормально работать.

Можно использовать Windows, но для компиляции и запуска потребуется WSL (Windows Subsystem for Linux). В этом случае сначала установите WSL, а дальшейшие инструкции выполняйте уже в Linux окружении.

Рекомендуется использовать Ubuntu версии **22.04**. Минимальные версии поддерживаемых компиляторов и способ их установки в Ubuntu:

- **g++-12**
```bash
$ sudo apt-get install g++-12
```
- **clang++-16**
```bash
$ wget https://apt.llvm.org/llvm.sh
$ chmod +x llvm.sh
$ sudo ./llvm.sh 16 all
```

## Установка CMake

* На **Ubuntu** `sudo apt-get -y install cmake`
* На **MacOS** `brew install cmake`

## Установка зависимостей

 * На **Ubuntu** `sudo apt-get install libpoco-dev`
 * На **MacOS** `brew install poco --build-from-source --cc=gcc-8`

 ## Запуск бота

 ```bash
horo-tgbot-cpp$ mkdir ./build
horo-tgbot-cpp$ cd ./build
horo-tgbot-cpp/build$ cmake ./..
horo-tgbot-cpp/build$ make
horo-tgbot-cpp/build$ ./bot-run $(< ./../token)
 ```
