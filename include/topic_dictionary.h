//
// Created by jtwears on 3/26/26.
//

#ifndef JRON_TOPIC_DICTIONARY_H
#define JRON_TOPIC_DICTIONARY_H

#include <pthread.h>
#include "topic.h"
#include "generic_dictionary.h"

#define MAX_NO_OF_ITEMS 100
#define DICTIONARY_KEY_SIZE 100

DECLARE_DICTIONARY(
    topic_t,
    topic_t *,
    MAX_NO_OF_ITEMS,
    DICTIONARY_KEY_SIZE
);

#endif //JRON_TOPIC_DICTIONARY_H