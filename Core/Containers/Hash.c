/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Hash.h"

#include "Array.h"
#include <string.h> //memcpy

typedef void* kvp_t;
typedef arr_handle bucket_t;

#define REHASH_GLOBAL_LOAD_FACTOR 0.75
#define REHASH_LOCAL_LOAD_FACTOR 0.9

#define MINIMUM_HASH_SIZE 16
#define MINIMUM_BUCKET_SIZE 1
#define MAXIMUM_BUCKET_SIZE 32

#define KEY_HASH(hash_handle, kvp) hash_handle->key_hash_f((void*)(kvp))
#define KEY_EQ(hash_handle, k1, k2) hash_handle->key_eq_f((void*)(k1), (void*)(k2))
#define KEY_PTR(hash_handle, kvp) (void*)(kvp)
#define VAL_PTR(hash_handle, kvp) (void*)((const char*)(kvp) + (hash_handle)->key_size)

typedef struct hash_data{
    int32_t val_size;
    int32_t key_size;
    arr_handle buckets;
    int32_t used_buckets;
    hash_key_func key_hash_f;
    hash_key_eq key_eq_f;
} hash_data;


#ifndef CORE_ASSERT
#ifdef __MINGW32__
#include <assert.h>
#define CORE_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#else
#include "assert.h"
#define CORE_ASSERT(e) assert(e)
#endif
#endif


float get_load_factor(hash_handle handle) {
    if (arr_max_size(handle->buckets) < MINIMUM_HASH_SIZE)
        return 1;
    return (float) handle->used_buckets / arr_max_size(handle->buckets);
}

float get_load_factor_bucket(hash_handle handle, bucket_t bucket) {
    if(bucket == 0) return 0;
    return ((float) arr_size(bucket) / MAXIMUM_BUCKET_SIZE);
}

size_t get_next_size(hash_handle handle)
{
    int32_t capacity = arr_max_size(handle->buckets);
    if(capacity < MINIMUM_HASH_SIZE)
        return MINIMUM_HASH_SIZE;
    return capacity * 2;
}

void hash_add_internal(hash_handle handle, const void *key, const void *value) {
    int32_t len = arr_max_size(handle->buckets);
    size_t index = KEY_HASH(handle, key) % len;
    bucket_t bucket = *(bucket_t *) arr_get(handle->buckets, index);
    int32_t size = arr_size(bucket);
    if (size == 0) handle->used_buckets++;
    memcpy(arr_get(bucket, size), key, handle->key_size);
    memcpy((char *) arr_get(bucket, size) + handle->key_size, value, handle->val_size);
    arr_resize(bucket, size + 1);
}

bucket_t hash_get_bucket_internal(hash_handle handle, const void *key) {
    int32_t len = arr_max_size(handle->buckets);
    size_t index = KEY_HASH(handle, key) % len;
    bucket_t bucket = *(bucket_t *) arr_get(handle->buckets, index);
    return bucket;
}

int32_t hash_get_kvp_internal(hash_handle handle, bucket_t bucket, const void* key)
{
    if(bucket == 0) return -1;
    for (int32_t i = 0; i < arr_size(bucket); ++i) {
        kvp_t kvp = arr_get(bucket, i);
        if (KEY_EQ(handle, kvp, key))
            return i;
    }
    return -1;
}

bool hash_contains(hash_handle handle, const void *key) {
    bucket_t bucket = hash_get_bucket_internal(handle, key);
    return hash_get_kvp_internal(handle, bucket, key) != -1;
}


void rehash(hash_handle handle) {

    hash_handle result_handle = hash_new(get_next_size(handle), handle->key_size, handle->val_size,
                                         handle->key_eq_f,
                                         handle->key_hash_f);

    for (int32_t i = 0; i < arr_size(handle->buckets); ++i) {
        bucket_t bucket = *(bucket_t *) arr_get(handle->buckets, i);
        for (int32_t j = 0; j < arr_size(bucket); ++j) {
            kvp_t kvp = arr_get(bucket, j);
            hash_add_internal(result_handle, KEY_PTR(handle, kvp), VAL_PTR(handle, kvp));
        }
    }
    for (int32_t i = 0; i < arr_size(handle->buckets); ++i) {
        bucket_t bucket = *(bucket_t *) arr_get(handle->buckets, i);
        arr_delete(bucket);
    }
    arr_delete(handle->buckets);

    handle->buckets = result_handle->buckets;
    handle->used_buckets = result_handle->used_buckets;

}

hash_handle hash_new(int32_t initial_capacity, int32_t key_size, int32_t value_size, hash_key_eq key_eq_f,
                     hash_key_func key_hash_f) {

    hash_handle handle = OS_MALLOC(sizeof(struct hash_data));
    handle->key_size= key_size;
    handle->val_size= value_size;
    handle->key_eq_f = key_eq_f;
    handle->key_hash_f = key_hash_f;

    arr_handle buckets_unused = arr_new(sizeof(bucket_t), initial_capacity);
    arr_resize(buckets_unused, initial_capacity);

    for(int32_t i=0; i < arr_size(buckets_unused); ++i) {
        bucket_t new_bucket = arr_new(key_size + value_size, MINIMUM_BUCKET_SIZE);
        arr_set(buckets_unused, i, &new_bucket);
    }

    handle->buckets = buckets_unused;
    handle->used_buckets = 0;
    return handle;
}

void hash_delete(hash_handle handle) {

    for (int32_t i = 0; i < arr_size(handle->buckets); ++i) {
        bucket_t bucket = *(bucket_t *) arr_get(handle->buckets, i);
        arr_delete(bucket);
    }

    arr_delete(handle->buckets);
    OS_FREE(handle);
}

bool hash_add(hash_handle handle, const void *key, const void *value) {
    bucket_t bucket = hash_get_bucket_internal(handle, key);

    if (hash_contains(handle, key))
        return false;

    if (get_load_factor(handle) > REHASH_GLOBAL_LOAD_FACTOR) {
        rehash(handle);
    }

    if (get_load_factor_bucket(handle, bucket) > REHASH_LOCAL_LOAD_FACTOR) {
        rehash(handle);
    }

    hash_add_internal(handle, key, value);

    return true;
}

bool hash_remove(hash_handle handle, const void *key) {
    if (handle->used_buckets == 0)
        return false;

    bucket_t bucket = hash_get_bucket_internal(handle, key);
    int32_t kvp_id = hash_get_kvp_internal(handle, bucket, key);
    if(kvp_id == -1) return false;

    arr_remove_swap(bucket, kvp_id);
    if (arr_size(bucket) == 0)
        handle->used_buckets--;

    return true;
}

void* hash_try_get(hash_handle handle, const void *key) {
    bucket_t bucket = hash_get_bucket_internal(handle, key);
    int32_t kvp_id = hash_get_kvp_internal(handle, bucket, key);
    if(kvp_id == -1) return 0;
    kvp_t kvp = arr_get(bucket, kvp_id);
    return VAL_PTR(handle, kvp);
}
