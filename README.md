# Soring on tape

Реализация алгоритма сортировки на устройствах типа лента (tape).

## Описание задачи

Пусть имеется входная лента длины `N` (где `N` – велико), содержащая элементы типа `int32_t`.
Имеется выходная лента такой же длины. После сортировки на выходную ленту записываются
отсортированные по возрастанию элементы с входной ленты.

Есть ограничение по использованию оперативной памяти – не более `M` байт (`M` может быть < `N`, т.е. загрузить все
данные с
ленты в оперативную память не получится). Для реализации алгоритма можно использовать разумное количество временных
лент, т.е. лент, на которых можно хранить какую-то временную информацию, необходимую в процессе
работы алгоритма.

## Описание реализации

В данной реализации определен [интерфейс](src/sorting_on_tape/tape.h) для устройства типа лента.
При этом основными операциями являются:

- чтение одного или нескольких элементов;
- запись одного или нескольких элементов;
- сдвиг ленты на одну позицию вперед или назад;
- перемотка ленты в начало или в конец.

Кроме того, представлена [реализация](src/sorting_on_tape/file_tape.h) данного интерфейса, эмулирующая работу с лентой
посредством файла.

Сама реализация алгоритма сортировки представлена классом [TapeSorter](src/sorting_on_tape/tape_sorter.h), в котором
есть метод для сортировки данных с входной ленты на выходную.
При этом данному классу необходимо где-то сохранять временные ленты при сортировке, для этого в него передается
также [провайдер](src/sorting_on_tape/temp_tape_provider.h) временных устройств.

Классы [FileTape](src/sorting_on_tape/file_tape.h) и [TapeSorter](src/sorting_on_tape/tape_sorter.h) можно
конфигурировать, для этого [специальный класс](src/sorting_on_tape/configuration.h) считывает из файла config.properties
свойства. Пример этого файла можно посмотреть [здесь](config.properties), в нем указаны все возможные конфигурируемые
параметры.

| Параметр                     | Значение по умолчанию               | Описание                                                        |
|------------------------------|-------------------------------------|-----------------------------------------------------------------|
| `read_duration`              | 7                                   | Задержка чтения с устройства в мкс (us)                         |
| `write_duration`             | 7                                   | Задержка записи на устройство в мкс (us)                        |
| `move_duration`              | 1                                   | Задержка сдвига лента на одну позицию в мкс (us)                |
| `rewind_duration`            | 100                                 | Задержка перемотки ленты в мкс (us)                             |
| `memory_limit`               | 1073741824 B = 1 GiB                | Ограничение используемой памяти при сортировке в байтах (B)     |
| `max_thread_count`           | std::thread::hardware_concurrency() | Максимальное количество потоков                                 |
| `max_value_count_per_thread` | 1000000                             | Максимальное количество элементов, обрабатываемых одним потоком |

Сама сортировка построена следующим образом: для начала данные из входного устройства считываются поблочно, сортируются
и записываются на временные устройства.
Это сделано ввиду ограничения на объем используемой памяти. После этого происходит попарное слияние отсортированных
блоков и запись в новый временный блок.
Попарное слияние происходит до того момента, когда останется только один отсортированный блок, который в конце
записывается на выходное устройство.

Кроме того, так как нет возможности прочитать сразу все данные, а задержки записи на устройство и чтения с него в тысячи
раз превышают задержки оперативной памяти, имеет смысл организовать параллельную сортировку, запись и слияние временных
блоков.

В проекте используется `Google Test`, `Google Benchmark` для написания модульных тестов и тестов производительности
соответственно.

## Сборка и запуск

Можно воспользоваться предварительно собранными [файлами](https://github.com/stolex1y/sorting-on-tape/releases/latest).
Там также можно найти документацию и результаты бенчмарков.

Для сборки в ручную можно использовать следующие команды:

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j`nproc --all` -t sorting-on-tape-runnable
./build/bin/sorting_on_tape/sorting-on-tape-runnable input output desc 
```

Можно заметить, что при запуске программы в аргументах передаются три аргумента.

1. Входной файл, содержащий данные для сортировки.
2. Выходной файл.
3. Порядок сортировки: `asc`, `desc` (по умолчанию используется `asc`).

Кроме того, как было описано выше, можно задать конфигурационный файл `config.properties`, который должен лежать в одной
директории с исполняемым файлом.

Для запуска тестов можно использовать следующие команды:

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cd build
cmake --build . -j`nproc --all`
ctest --timeout 10 --output-on-failure --schedule-random -j`nproc --all` # unit tests
cmake --build . -j`nproc --all` -t sorting-on-tape-test-runnable-memcheck # valgrind
```

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j`nproc --all`
./build/test/benchmark/sorting_on_tape/sorting-on-tape-benchmark --benchmark_time_unit=s # benchmarks
```
