# Настройка рабочего места

Инструменты разработки предполагают работу в ОС Линукс.
Вы можете настроить самостоятельно или воспользоваться виртуальной машиной где уже все настроено.

## Самостоятельная настройка
1. Установить следующие пакеты:

    - git
    - scons
    - qemu
    - python (version 2)
    - python-jinja2
    - python-yaml


1. Скачать кросс-компилятор, соотвествующей целевой платформе:

    - [powerpc (кроме e500v2)](http://newos.org/toolchains/powerpc-elf-4.9.1-Linux-x86_64.tar.xz)
    - [powerpc e500v2](http://forge.ispras.ru/attachments/download/4983/powerpc-elf-eabispe.tgz)
    - [x86](http://newos.org/toolchains/i386-elf-4.9.1-Linux-x86_64.tar.xz)
    - [arm](http://newos.org/toolchains/arm-eabi-4.9.1-Linux-x86_64.tar.xz)

    Если ссылки не работают, можно попробовать взять с <http://wiki.osdev.org/GCC_Cross-Compiler#Prebuilt_Toolchains>.

    Разархивировать архив.

    Добавить в переменную окружения PATH путь к папке bin кросс-компилятора.

1. Клонировать репозиторий JetOS:

        git clone https://forge.ispras.ru/git/chpok.git jetos

    Добавить в переменную окружения JETOS_HOME путь к jetos.


## Виртуальная машина

Для Вашего удобства нами подготовлена виртуальная машина с настроенным для работы окружением.
В качестве операционной системы используется *Linux Debian 8.5 Jessie* с интерфейсом пользователя *Xfce*.

 - Имя пользователя: user
 - Пароль: password
 - Пароль пользователя root: rootpass

Репозиторий JetOS клонирован в директорию `/home/user/workspace/jetos/`.

Следует упомянуть следующие установленные приложения:

 - geany — легковесная среда разработки;
 - Eclipse CDT Mars — интегрированная среда разработки;
 - git — система контроля версий;
 - vim — текстовый редактор;
 - wireshark — анализатор трафика.
