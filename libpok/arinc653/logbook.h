/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */

#ifndef __LIBJET_ARINC_LOGBOOK_H__
#define __LIBJET_ARINC_LOGBOOK_H__

#include <config.h>

#include <arinc653/types.h>
#include <arinc653/logbook.h>
#include <types.h>

typedef
    enum {
        ABORTED_ENTRY           = 0,
        IN_PROGRESS_ENTRY       = 1,
        COMPLETE_ENTRY          = 2,
        TRANSPORTING_TO_LOGBOOK = 3,
        TRANSPORTING_TO_BUFFER  = 4
} MESSAGE_STATUS_TYPE;

struct arinc_logbook {
//  const char          *name;          // Area name, Указывается в конфигурации
//  const char          *device_name;   // Указывается в конфигурации
    LOGBOOK_NAME_TYPE   logbook_name;
//  PARTITION_ID_TYPE   partition_id,   // Указывается в конфигурации
    const char          *owner_partition_name; // Указывается в конфигурации
    // Поля name, device_name, logbook_name, partition_id с одной стороны почти бесполезны
    // С другой стороны, их нужно где-то хранить.
    // Есть в конфигурировании в стиле oc2000


//  LOGBOOK_ID_TYPE         logbook_id,                 // В буферах id не хранится
                                                        // Там id переводится в index -- индекс в массиве буферов
                                                        // В портах какое-то неэлегантное решение
                                                        // Но в принципе там тоже id -- индекс в массиве
                                                        // Можно обойтись без ID бортового журнала в структуре
    // Следующие три поля инициализируются при конфигурировании и проверяются при создании:
    MESSAGE_SIZE_TYPE   max_message_size;
    MESSAGE_RANGE_TYPE  max_nb_logged_messages;
    MESSAGE_RANGE_TYPE  max_nb_in_progress_messages;
    
    // Шаг сообщения с учетом выравнивания
    MESSAGE_SIZE_TYPE   message_stride;

    // Флаг, отвечающий за то, создан ли бортовой журнал
    pok_bool_t          is_created;

    // Числа сообщений разного типа:
    // (ABORTED -- которые стерлись из буфера, но не записались в NVM?)
    // (Если учитывать, что:
    // Number of completely engraved messages = NB_LOGGED_MESSAGES - NB_ABORTED_MESSAGES
    // То похоже, что нет
    
    MESSAGE_RANGE_TYPE      nb_logged_messages;
    MESSAGE_RANGE_TYPE      nb_in_progress_messages;
    MESSAGE_RANGE_TYPE      nb_aborted_messages;
    
    // Массив сообщений в буфере и их размеров:
    char* buffer_messages;
    MESSAGE_SIZE_TYPE* buffer_messages_size;
        
    // Массив сообщений в "NVM" и их размеров:
    char* nvm_messages;
    MESSAGE_SIZE_TYPE* nvm_messages_size;
    MESSAGE_STATUS_TYPE* nvm_messages_status;
    // MESSAGE_STATUS_TYPE -- придуманный тип, надо будет описать
    
    // Подумать о массиве статусов сообщений
    // И о защите (запись в уже записываемые ячейки, чтение еще недозаписанных сообщений и т.д.)
};

/* Preallocated array of buffers. */
extern struct arinc_logbook* arinc_logbooks;

#endif /* __LIBJET_ARINC_LOGBOOK_H__ */
