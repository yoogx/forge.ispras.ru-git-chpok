# Eclipse
На предоставляемую Вам виртуальную машину установлена интегрированная среда разработки Eclipse Mars. Для всех приложений, присутствующих в JetOS в папке *examples/*, созданы проекты в Eclipse.

## Создание нового проекта в Eclipse
1. В Eclipse выберите File->New->C Project
2. Снимите отметку с параметра Use default location и укажите путь к папке проекта, например, `/home/user/workspace/chpok/examples/pure-arinc653-buffer-2-partitions`
3. Укажите имя проекта в поле Project name
4. Убедитесь, что параметр *Project type* имеет значение Executable->Empty Project
5. Убедитесь, что параметр *Toolchains* имеет значение *Cross GCC*
6. Нажмите Next
7. В меню Select Configurations отметьте пункты Debug и Release
8. Нажмите Next
9. В поле Cross compiler prefix укажите powerpc-elf-
10. В поле Cross compiler path укажите путь к кросс-компилятору, например, `/home/user/workspace/powerpc-elf-4.9.1-Linux-x86_64`
11. Нажмите Finish
12. Щёлкните правой клавишей мыши на только что созданном проекте в Project Explorer
13. Выберите SCons->Use self-provided SCons build
14. В меню Edit SCons Options нажмите OK

## Сборка проекта в Eclipse
1. Щёлкните правой клавишей мыши на нужном проекте в Project Explorer
2. Нажмите Build Project

## Запуск системы в Eclipse
Нажмите на стрелку справа от значка Run и выберите нужную конфигурацию запуска.

## Отладка системы в Eclipse
### Отладка в QEMU
1. Выберите нужный проект в Project Explorer
2. Нажмите на стрелку справа от значка Run и выберите конфигурацию `rundbg`.
3. Нажмите на стрелку справа от значка Debug и выберите соответствующую конфигурацию, например, `Debug pure-arinc653-buffer`.

### Использование встроенного отладчика
В настоящее время встроенный отладчик JetOS можно использовать в Eclipse только для приложений с одним разделом.

1. Выберите нужный проект в Project Explorer
2. Нажмите на стрелку справа от значка Run и выберите конфигурацию `Run with 2 ports`.
3. Нажмите на стрелку справа от значка Debug и выберите соответствующую конфигурацию, например, `ISP Debug pure-arinc653-buffer`.
