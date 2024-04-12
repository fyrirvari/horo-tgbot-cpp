# Telegram bot

Телеграмм бот способен по вашему желанию рассказать гороскоп на выбранные дни для любого знака зодиака. Также этот бот умеет делать расклады таро, но пока что эта функция находится в тестовом режиме :)

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

## Документация по методам 

Получение гороскопа через API в зависимости от знака (`sign`) на:
* Вчерашний день
* Сегодняшний день
* Завтрашний день
* Неделю
* Месяц

```cpp
std::string GetDaily(std::string day, std::string sign);
std::string GetMonthly(std::string sign);
std::string GetWeekly(std::string sign);
```

Получение обновлений (новых сообщений), аргументы:
* `offset` - начиная с какого обновления (номер) мы хотим получать
* `timeout` - через какое время хотим получить (частота) 

```cpp
std::vector<Update> GetUpdates(std::optional<int64_t> offset = std::nullopt,
                               std::optional<int64_t> timeout = std::nullopt);
```

Отправка сообщения, аргументы:
* `chat_id` - идентификатор чата, в который мы отправляем сообщение
* `text` - текст сообщения
* `reply_to_message_id` - идентификатор сообщения, на которое мы отвечаем
* `reply_markup` - набор маркап кнопок, пример "♎ Весы ♎"

```cpp
Message SendMessage(int64_t chat_id, std::optional<std::string> text = std::nullopt,
                    std::optional<int64_t> reply_to_message_id = std::nullopt,
                    std::optional<std::vector<std::string>> reply_markup = std::nullopt);
```

Отправка фото, аргументы:
* `chat_id` - идентификатор чата, в который мы отправляем фото
* `file` - относительынй путь до изображения, которые мы хотим отправить

```cpp
Message SendPhoto(int64_t chat_id, std::string file);
```