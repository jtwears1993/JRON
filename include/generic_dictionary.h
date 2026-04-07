//
// Created by jtwears on 3/26/26.
//

#ifndef JRON_GENERIC_DICTIONARY_H
#define JRON_GENERIC_DICTIONARY_H

#ifndef GENERIC_DICTIONARY_H
#define GENERIC_DICTIONARY_H

#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


#define DECLARE_DICTIONARY(NAME, VALUE_TYPE, MAX_ITEMS, KEY_SIZE)                      \
                                                                                       \
typedef struct {                                                                       \
    pthread_rwlock_t lock;                                                             \
    size_t current_size;                                                               \
    char keys[(MAX_ITEMS)][(KEY_SIZE) + 1];                                            \
    VALUE_TYPE values[(MAX_ITEMS)];                                                    \
} NAME##_dictionary_t;                                                                 \
                                                                                       \
static inline NAME##_dictionary_t *NAME##_dictionary_create(void) {                    \
    NAME##_dictionary_t *dict = malloc(sizeof(NAME##_dictionary_t));                   \
    if (!dict) return NULL;                                                            \
                                                                                       \
    if (pthread_rwlock_init(&dict->lock, NULL) != 0) {                                 \
        free(dict);                                                                    \
        return NULL;                                                                   \
    }                                                                                  \
                                                                                       \
    dict->current_size = 0;                                                            \
    for (size_t i = 0; i < (MAX_ITEMS); i++) {                                         \
        dict->keys[i][0] = '\0';                                                       \
        memset(&dict->values[i], 0, sizeof(dict->values[i]));                          \
    }                                                                                  \
                                                                                       \
    return dict;                                                                       \
}                                                                                      \
                                                                                       \
static inline void NAME##_dictionary_destroy(NAME##_dictionary_t *dict) {              \
    if (!dict) return;                                                                 \
    pthread_rwlock_destroy(&dict->lock);                                               \
    free(dict);                                                                        \
}                                                                                      \
                                                                                       \
static inline int NAME##_dictionary_get_index(                                         \
    const NAME##_dictionary_t *dict,                                                   \
    const char *key                                                                    \
) {                                                                                    \
    if (!dict || !key) return -1;                                                      \
                                                                                       \
    for (size_t i = 0; i < dict->current_size; i++) {                                  \
        if (strcmp(dict->keys[i], key) == 0) {                                         \
            return (int)i;                                                             \
        }                                                                              \
    }                                                                                  \
                                                                                       \
    return -1;                                                                         \
}                                                                                      \
                                                                                       \
static inline int NAME##_dictionary_set(                                               \
    NAME##_dictionary_t *dict,                                                         \
    const char *key,                                                                   \
    VALUE_TYPE value                                                                   \
) {                                                                                    \
    if (!dict || !key) return -1;                                                      \
                                                                                       \
    pthread_rwlock_wrlock(&dict->lock);                                                \
                                                                                       \
    const int index = NAME##_dictionary_get_index(dict, key);                          \
    if (index >= 0) {                                                                  \
        dict->values[index] = value;                                                   \
        pthread_rwlock_unlock(&dict->lock);                                            \
        return 0;                                                                      \
    }                                                                                  \
                                                                                       \
    if (dict->current_size >= (MAX_ITEMS)) {                                           \
        pthread_rwlock_unlock(&dict->lock);                                            \
        return -1;                                                                     \
    }                                                                                  \
                                                                                       \
    strncpy(dict->keys[dict->current_size], key, (KEY_SIZE));                          \
    dict->keys[dict->current_size][(KEY_SIZE)] = '\0';                                 \
    dict->values[dict->current_size] = value;                                          \
    dict->current_size++;                                                              \
                                                                                       \
    pthread_rwlock_unlock(&dict->lock);                                                \
    return 0;                                                                          \
}                                                                                      \
                                                                                       \
static inline int NAME##_dictionary_get(                                               \
    NAME##_dictionary_t *dict,                                                         \
    const char *key,                                                                   \
    VALUE_TYPE *out_value                                                              \
) {                                                                                    \
    if (!dict || !key || !out_value) return -1;                                        \
                                                                                       \
    pthread_rwlock_rdlock(&dict->lock);                                                \
                                                                                       \
    const int index = NAME##_dictionary_get_index(dict, key);                          \
    if (index >= 0) {                                                                  \
        *out_value = dict->values[index];                                              \
        pthread_rwlock_unlock(&dict->lock);                                            \
        return 0;                                                                      \
    }                                                                                  \
                                                                                       \
    pthread_rwlock_unlock(&dict->lock);                                                \
    return -1;                                                                         \
}                                                                                      \
                                                                                       \
static inline int NAME##_dictionary_remove(                                            \
    NAME##_dictionary_t *dict,                                                         \
    const char *key                                                                    \
) {                                                                                    \
    if (!dict || !key) return -1;                                                      \
                                                                                       \
    pthread_rwlock_wrlock(&dict->lock);                                                \
                                                                                       \
    const int index = NAME##_dictionary_get_index(dict, key);                          \
    if (index < 0) {                                                                   \
        pthread_rwlock_unlock(&dict->lock);                                            \
        return -1;                                                                     \
    }                                                                                  \
                                                                                       \
    for (size_t i = (size_t)index; i < dict->current_size - 1; i++) {                  \
        memcpy(dict->keys[i], dict->keys[i + 1], (KEY_SIZE) + 1);                      \
        dict->values[i] = dict->values[i + 1];                                         \
    }                                                                                  \
                                                                                       \
    dict->keys[dict->current_size - 1][0] = '\0';                                      \
    memset(&dict->values[dict->current_size - 1], 0, sizeof(dict->values[0]));         \
    dict->current_size--;                                                              \
                                                                                       \
    pthread_rwlock_unlock(&dict->lock);                                                \
    return 0;                                                                          \
}

#endif

#endif //JRON_GENERIC_DICTIONARY_H