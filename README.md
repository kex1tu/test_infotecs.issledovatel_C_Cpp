# infotecs_webservice


[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg?style=flat-square&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/17)
[![Build System](https://img.shields.io/badge/CMake-3.28%2B-064F8C.svg?style=flat-square&logo=cmake)](CMakeLists.txt)
[![Build Tool](https://img.shields.io/badge/Ninja-Supported-black.svg?style=flat-square&logo=ninja)](https://ninja-build.org/)
[![Target OS](https://img.shields.io/badge/OS-Ubuntu%2022.04%2F24.04%20%7C%20Debian-E95420.svg?style=flat-square&logo=ubuntu)](https://ubuntu.com)
[![Database](https://img.shields.io/badge/Database-PostgreSQL%2014%2B-336791.svg?style=flat-square&logo=postgresql)](https://www.postgresql.org/)
[![OpenAPI](https://img.shields.io/badge/OpenAPI-v3.1.0-6BA539.svg?style=flat-square&logo=openapiinitiative)](openapi.yaml)

# [Ссылка на github](https://github.com/kex1tu/test_infotecs.issledovatel_C_Cpp)

Высокопроизводительный асинхронный REST веб-сервис на C++17 для выборки записей из базы данных PostgreSQL с набором данных от 1 000 000 записей. Реализован с использованием Boost.Asio, Boost.Beast, `libpqxx` и пула потокобезопасных соединений.

---

## Архитектура системы

```mermaid
graph TD
    subgraph ClientLayer ["Клиентский уровень"]
        CurlClient["curl / client/requests.sh"]
    end

    subgraph ServerLayer ["infotecs_webservice (C++17 / Boost.Beast)"]
        Acceptor["Acceptor Thread (boost::asio::ip::tcp::acceptor)"]
        WorkerPool["Worker Threads Pool (4 потока std::thread)"]
        HttpSession["Http::HttpSession (Boost.Beast / HTTP Parser)"]
        UrlParser["UrlUtils (URL Decoder & Query Parser)"]
        JsonSerializer["JsonSerializer (Boost.JSON)"]

        Acceptor -->|"async_accept"| WorkerPool
        WorkerPool --> HttpSession
        HttpSession --> UrlParser
        HttpSession --> JsonSerializer
    end

    subgraph DBLayer ["Слой доступа к данным"]
        Repository["Db::JournalRepository"]
        ConnGuard["Db::DbConnectionGuard (RAII)"]
        ConnPool["Db::DbConnectionPool (ThreadSafeQueue)"]
        PgDatabase[("PostgreSQL 14+\ntable: journal (1M+ rows)\nidx_journal_ts_level")]

        HttpSession -->|"get_records(min_level, since, limit)"| Repository
        Repository --> ConnGuard
        ConnGuard -->|"take / return connection"| ConnPool
        ConnPool <-->|"libpqxx::connection"| PgDatabase
    end

    CurlClient ==>|"HTTP GET /journal?since=...&level=..."| Acceptor
    JsonSerializer -->|"HTTP 200 OK (JSON array)"| CurlClient
```

---

## Архитектурные принципы и инварианты

1. **Стандарт C++17 и современный стек**: сервис написан на C++17 с использованием `Boost.Asio` (сетевой I/O и таймеры), `Boost.Beast` (HTTP/REST протокол), `Boost.JSON` (быстрая сериализация) и `libpqxx` (официальный C++ драйвер PostgreSQL).
2. **Параллельная обработка (4 worker-потока)**: архитектура разделяет поток приема входящих TCP-соединений (`acceptor_ioc`) и пул из 4 рабочих потоков (`worker_ioc`), гарантируя параллельную обработку независимых HTTP-запросов.
3. **Пул соединений к БД (Connection Pool)**: потокобезопасный пул `DbConnectionPool` на базе `Helpers::ThreadSafeQueue` предварительно открывает 4 постоянных подключения к PostgreSQL, исключая накладные расходы на TCP-handshake при каждом запросе.
4. **RAII и безопасность ресурсов**: дескрипторы соединений с БД управляются через RAII-страж `DbConnectionGuard`. При выходе из области видимости соединение автоматически возвращается в пул без утечек даже при возникновении исключений.
5. **Индексация и производительность БД**: таблица `journal` снабжена индексом `idx_journal_ts_level (ts, level)` для быстрого выполнения фильтраций по таблице.
6. **Graceful Shutdown**: перехват системных сигналов `SIGINT` и `SIGTERM` через `boost::asio::signal_set` обеспечивает корректную остановку сервера, завершение активных транзакций и закрытие сокетов без обрыва соединений.

---

## Структура репозитория

```text
.
├── CMakeLists.txt                    # Корневой CMake-файл проекта
├── README.md                         # Полная техническая документация
├── openapi.yaml                      # Спецификация REST API в формате OpenAPI v3.1.0
├── .clang-format                     # Правила форматирования кода (Google C++ Style)
├── .clang-tidy                       # Конфигурация статического анализатора
│
├── db/                               # SQL-скрипты развертывания и инициализации БД
│   ├── schema.sql                    # DDL: создание таблицы journal и индекса
│   └── seed.sql                      # DML: генерация 1 000 000 тестовых записей
│
├── client/                           # Демонстрационный клиент
│   └── requests.sh                   # Bash-скрипт с 8 сценариями запросов curl
│
├── src/                              # Исходный код веб-сервиса
│   ├── main.cpp                      # Точка входа, парсинг CLI, сигнал-хэндлер, пул потоков
│   ├── db/                           # Слой взаимодействия с базой данных
│   │   ├── db_connection_guard.hpp   # RAII-обертка возврата соединения в пул
│   │   ├── db_connection_pool.hpp    # Интерфейс пула соединений PostgreSQL
│   │   ├── db_connection_pool.cpp    # Реализация пула на базе ThreadSafeQueue
│   │   ├── i_journal_repository.hpp  # Интерфейс репозитория журнала
│   │   ├── journal_record.hpp        # Структура записи JournalRecord
│   │   ├── journal_repository.hpp    # Заголовочный файл репозитория
│   │   ├── journal_repository.cpp    # Параметризованные SQL-запросы через libpqxx
│   │   └── thread_safe_queue.hpp     # Шаблонная потокобезопасная очередь (MPMC)
│   └── http/                         # Слой HTTP-сервера и REST API
│       ├── http_server.hpp           # TCP-акцептор сетевых соединений
│       ├── http_server.cpp           # Реализация асинхронного акцептора
│       ├── http_session.hpp          # Сессия обработки HTTP-запроса (Beast)
│       ├── http_session.cpp          # Роутинг, валидация параметров, формирование ответов
│       ├── json_serializer.hpp       # Интерфейс сериализатора в JSON
│       ├── json_serializer.cpp       # Сериализация массива записей и ошибок (Boost.JSON)
│       ├── url_utils.hpp             # Утилиты декодирования URL
│       └── url_utils.cpp             # URL-декодер и парсер Query-параметров
│
└── tests/                            # Модульные и интеграционные тесты (CTest)
    ├── CMakeLists.txt                # Сборка тестовых исполняемых файлов
    ├── tests.hpp                     # Легковесный тестовый заголовочный фреймворк
    ├── test_journal_repository.cpp   # Интеграционные тесты слоя БД и пула
    ├── test_json_serializer.cpp      # Тесты сериализации данных и ошибок в JSON
    ├── test_thread_safe_queue.cpp    # Многопоточные тесты очереди и graceful stop
    └── test_url_utils.cpp            # Тесты URL-декодирования и парсинга query-строк
```

---

## Требования к окружению

Для сборки и запуска потребуется Linux-окружение (Ubuntu 22.04/24.04 LTS или Debian 11/12):

```bash
# Обновление списков пакетов
sudo apt-get update

# Базовые инструменты сборки и компилятор C++17
sudo apt-get install -y build-essential cmake ninja-build g++ pkg-config

# Библиотеки Boost и libpqxx для работы с PostgreSQL
sudo apt-get install -y libboost-all-dev libpqxx-dev

# СУБД PostgreSQL и клиентские утилиты
sudo apt-get install -y postgresql postgresql-contrib

# Утилиты тестирования, форматирования и анализа (рекомендуется)
sudo apt-get install -y curl jq clang-format clang-tidy valgrind
```

---

## Сборка проекта

### 1. Настройка и инициализация базы данных PostgreSQL

Запустите службу PostgreSQL, создайте базу данных `journal_db` и примените скрипты схемы и наполнения 1 000 000 записей:

```bash
# Запуск службы PostgreSQL (если не запущена)
sudo systemctl start postgresql

# Создание базы данных и пользователя с правами доступа
sudo -u postgres psql -c "CREATE DATABASE journal_db;"
sudo -u postgres psql -c "ALTER USER postgres PASSWORD 'postgres';"

# Применение схемы (таблица journal и индекс idx_journal_ts_level)
psql "postgresql://postgres:postgres@127.0.0.1:5432/journal_db" -f db/schema.sql

# Генерация 1 000 000 тестовых записей в БД
psql "postgresql://postgres:postgres@127.0.0.1:5432/journal_db" -f db/seed.sql
```

### 2. Сборка всех компонентов через CMake (Ninja / Make)

```bash
# Конфигурация проекта в режиме Release
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Сборка исполняемого файла веб-сервиса и тестов
cmake --build build -j $(nproc)
```

*(Опционально при использовании стандартного генератора Unix Makefiles):*
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j $(nproc)
```

### 3. Таргетная сборка отдельных модулей

```bash
cmake --build build --target infotecs_webservice       # Сборка только основного веб-сервиса
cmake --build build --target test_url_utils            # Сборка тестов URL-парсера
cmake --build build --target test_json_serializer      # Сборка тестов JSON-сериализатора
cmake --build build --target test_thread_safe_queue    # Сборка тестов очереди
cmake --build build --target test_journal_repository   # Сборка интеграционных тестов БД
```

---

## Запуск и использование

### 1. Демонстрационный сценарий

Откройте два окна терминала:

**Терминал 1 — Запуск веб-сервиса (`infotecs_webservice`):**
```bash
# Синтаксис: ./build/infotecs_webservice <host> <port> [<db_conn_str>]
./build/infotecs_webservice 127.0.0.1 8080 "host=127.0.0.1 port=5432 dbname=journal_db user=postgres password=postgres"
```

*Также строка подключения к БД может быть задана через переменную окружения `DB_CONN_STR`:*
```bash
export DB_CONN_STR="host=127.0.0.1 port=5432 dbname=journal_db user=postgres password=postgres"
./build/infotecs_webservice 127.0.0.1 8080
```

**Терминал 2 — Автоматический прогон запросов через `curl`:**
```bash
# Запуск демонстрационного скрипта
chmod +x client/requests.sh
./client/requests.sh 127.0.0.1 8080
```

**Пример ручного HTTP-запроса через `curl`:**
```bash
# Выборка записей с датой позже 2025-01-01 и уровнем важности > 2
curl -s "http://127.0.0.1:8080/journal?since=2025-01-01&level=2" | jq .
```

**Пример ответа сервера (JSON):**
```json
[
  {
    "id": 774190,
    "ts": "2025-08-22 14:13:21.577441+00",
    "level": 4,
    "message": "Log message 774190"
  },
  {
    "id": 619064,
    "ts": "2025-08-22 14:16:26.75275+00",
    "level": 4,
    "message": "Log message 619064"
  }
]
```

---

### 2. Справочник параметров CLI

#### Веб-сервис (`infotecs_webservice`)
```bash
./build/infotecs_webservice <host> <port> [<db_conn_str>]
```
| Параметр | Тип | Описание | Пример |
| :--- | :--- | :--- | :--- |
| `<host>` | IPv4 / String | IP-адрес интерфейса для прослушивания (например, `127.0.0.1` или `0.0.0.0`) | `127.0.0.1` |
| `<port>` | uint16 | Порт TCP-сокета сервиса `[1, 65535]` | `8080` |
| `[<db_conn_str>]` | String *(опционально)* | Строка подключения libpq/PostgreSQL (или через `DB_CONN_STR`) | `"host=127.0.0.1 port=5432 dbname=journal_db user=postgres password=postgres"` |

#### Спецификация REST API (`GET /journal`)

| Параметр Query | Тип | Обязательный | Описание | Пример |
| :--- | :--- | :---: | :--- | :--- |
| `since` | String (ISO-8601) | **Да** | Нижняя граница времени (`ts > since`). Поддерживает даты и timestamp со смещением часового пояса. | `2025-08-22%2014:11:13%2B00` |
| `level` | Integer | Нет *(default: 0)* | Нижняя граница уровня важности (`level > min_level`). | `2` |

**Коды ответов:**
- `200 OK` — Успешная выборка (массив от 0 до 10 объектов JSON, отсортированных по возрастанию `ts ASC`).
- `400 Bad Request` — Отсутствует обязательный параметр `since`, передан некорректный формат `level`, либо неподдерживаемый HTTP-метод.
- `404 Not Found` — Запрос к несуществующему маршруту URL.
- `500 Internal Server Error` — Ошибка взаимодействия с базой данных.

---

## Тестирование

### Запуск тестов через CTest
```bash
ctest --test-dir build --output-on-failure -C Release --parallel $(nproc)
```

### Запуск модулей тестирования напрямую
```bash
./build/tests/test_thread_safe_queue   # 9 тестов: потокобезопасная очередь, MPMC и остановка
./build/tests/test_url_utils           # 12 тестов: URL-декодирование (%20, %2B, +) и парсинг query
./build/tests/test_json_serializer     # 5 тестов: сериализация структур и сообщений об ошибках
./build/tests/test_journal_repository  # 5 интеграционных тестов: выборка из БД, фильтры, лимит <= 10
```

### Анализ памяти (Valgrind Memcheck)
```bash
valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./build/tests/test_thread_safe_queue
valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./build/tests/test_url_utils
valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./build/tests/test_json_serializer
```

---

## Соответствие требованиям технического задания

| Раздел ТЗ | Требование | Статус | Компонент реализации / Примечание |
| :--- | :--- | :---: | :--- |
| **1. Обязательная часть** | | | |
| **1.1. БД** | SQL-скрипты создания и заполнения (1 000 000 строк) |  Выполнено | [db/schema.sql](db/schema.sql), [db/seed.sql](db/seed.sql) |
| **1.2. БД** | Таблица `journal` (`время`, `сообщение`, `уровень важности`) |  Выполнено | [db/schema.sql](db/schema.sql) |
| **1.3. БД** | СУБД PostgreSQL |  Выполнено | PostgreSQL 14+, `libpqxx 7.x` |
| **1.4. Сервис** | Веб-сервис по HTTP/REST (GET-эндпоинт по уровню и времени) |  Выполнено | [src/http/http_session.cpp](src/http/http_session.cpp) |
| **1.5. Сервис** | Фильтрация `level > указанного` и `ts > указанного` |  Выполнено | [src/db/journal_repository.cpp](src/db/journal_repository.cpp) |
| **1.6. Сервис** | Ограничение выдачи (не более 10 строк в ответе) |  Выполнено | `LIMIT 10` в `JournalRepository::get_records` |
| **1.7. Клиент** | Shell-скрипт демонстрационных вызовов через `curl` |  Выполнено | [client/requests.sh](client/requests.sh) (8 тестовых сценариев) |
| **2. Опциональная часть** | | | |
| **2.1. Async DB** | Асинхронная работа с БД (`libpq-async`) |  Не выполнено | Взамен реализован потокобезопасный пул синхронных соединений `libpqxx` в пуле рабочих потоков |
| **2.2. SSE** | Асинхронная выдача результата через Server-Sent Events |  Не выполнено | Реализован стандартный синхронный HTTP/REST JSON-ответ |
| **2.3. Shutdown** | Обработка `SIGINT`/`SIGTERM` и Graceful Shutdown |  Выполнено | `boost::asio::signal_set` в [src/main.cpp](src/main.cpp) |
| **2.4. OpenAPI** | Спецификация API в формате OpenAPI v3.1.0 |  Выполнено | [openapi.yaml](openapi.yaml) |
| **3. Системные требования** | | | |
| **3.1. Concurrency**| Параллельное выполнение 4 запросов / пул потоков |  Выполнено | 4 worker-потока в [src/main.cpp](src/main.cpp), пул из 4 соединений |
| **3.2. Стек** | C++17, GCC, CMake, Linux-окружение |  Выполнено | [CMakeLists.txt](CMakeLists.txt), `src/`, `tests/` |

<details>
<summary><b>Развернуть официальный текст технического задания</b></summary>

### Тестовое задание: Исследователь-системный программист (C/C++)

**Компания:** ИнфоТеКС  
**Направление:** Стажер по направлению «Исследователь-системный программист (C/C++)»  
**Тема:** Веб-сервис на C++ 17/20 с хранением данных в БД  
**Срок выполнения:** 1 неделя  

---

#### 1. Обязательная часть

##### База данных
- В отдельном файле написать SQL-инструкции создания и заполнения БД.
- Должна создаваться таблица `journal` с полями:
  - `время` (timestamp)
  - `текст сообщения` (text / varchar)
  - `уровень важности` (integer / level)
- Таблица `journal` должна заполняться **1 миллионом строк** тестовых данных.
- **СУБД:**
  - *Можно:* использовать СУБД SQLite.
  - *Желательно:* использовать СУБД PostgreSQL.

##### Веб-сервис
- Веб-сервис должен взаимодействовать с клиентом через **HTTP, REST**.
- Нужно реализовать один endpoint, обрабатывающий запрос, содержащий **уровень важности** и **время**.
- В ответе должны возвращаться данные из БД из таблицы `journal`, у которых уровень важности и время больше указанных.
- Нужно возвращать **не больше 10 строк**.

##### Клиент
- Для демонстрации работы веб-сервера использовать `curl`.
- Сделать небольшой shell-скрипт с командами `curl`, посылающими запросы к веб-сервису и выводящими результаты запросов.

---

#### 2. Опциональная часть

В веб-сервис добавить:
1. **Асинхронную работу с БД** ([libpq-async](https://www.postgresql.org/docs/current/libpq-async.html)) или другую технологию.
2. **Асинхронную выдачу результата** при помощи **Server-Sent Events (SSE)**. Можно добавить отдельный endpoint.
3. **Обработку `SIGTERM` и реализацию graceful shutdown**, т.е. корректное завершение с закрытием соединений и освобождением ресурсов.
4. **Спецификацию в формате OpenAPI**: [OpenAPI v3.1.0](https://spec.openapis.org/oas/v3.1.0.html).

---

#### 3. Требования к веб-сервису

- Веб-сервис должен представлять собой **консольное приложение под Linux**.
- Веб-сервис должен допускать **параллельное выполнение 4 запросов**.

##### Технологии:
- **Язык:** C++ 17/20.
- **Компилятор:** GCC.
- **Система сборки:** Make или CMake.
- **Библиотеки / фреймворки (на выбор/допускается):**
  - Boost, Boost.Asio, Boost.Beast
  - POCO
  - [Drogon](https://drogonframework.github.io/drogon-docs)
  - Nginx и libfcgi

---

#### 4. Требования к присылаемым решениям

Для проверки необходимо передать zip-архив, содержащий:
1. Скрипты для создания и наполнения БД.
2. Исходные коды веб-сервиса и скрипты для сборки.
3. Скрипты для обращения к сервису при помощи `curl` с разными параметрами.
4. `Readme.txt` (или `README.md`), в котором пошагово описана последовательность действий для создания БД, запуска и использования веб-сервиса.

> **ПРИМЕЧАНИЕ:** Результаты сборки и саму БД высылать не нужно.

</details>