/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef HASH_H
#define HASH_H

#include <stdbool.h>
#include <stdint.h>

#ifndef API
#define API
#endif

typedef void* hash_iterator;
typedef struct hash_data* hash_handle;

/*!
 * Function definition for key comparison.
 */
typedef bool(*hash_key_eq)(void const* key_a, void const* key_b);

/*!
 * Function definition for key hash retrieval.
 */
typedef int32_t(*hash_key_func)(void const* key);

/*!
 * Creates a new hash with specified initial capacity, stored key size, stored value size both
 * in bytes and two function pointers that operate on the key.
 */
API hash_handle hash_new(int32_t initial_capacity, int32_t key_size, int32_t value_size,
                        hash_key_eq key_eq_f, hash_key_func key_hash_f);

/*!
 * Deletes a hash and all its associated data.
 */
API void hash_delete(hash_handle handle);

/*!
 * Adds key that should be unique and associated value in the hash table.
 * Return true on success.
 */
API bool hash_add(hash_handle handle, void const* key, void const* value);

/*!
 * Removes key and associated value from the hash table.
 * Return true on success.
 */
API bool hash_remove(hash_handle handle, void const* key);

/*!
 * Returns true if key is present in the hash table.
 */
API bool hash_contains(hash_handle handle, void const* key);

/*!
 * Return valid pointer on success, otherwise 0.
 */
API void* hash_try_get(hash_handle handle, void const* key);

API hash_iterator hash_it_begin(hash_handle handle);
API hash_iterator hash_it_next(hash_handle handle, hash_iterator iterator);
API void hash_it_dereference(hash_handle handle, hash_iterator iterator, void const** key, void const** value);

#endif //HASH_H
