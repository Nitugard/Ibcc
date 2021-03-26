/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef FIXEDPHYSICS_ASSET_H
#define FIXEDPHYSICS_ASSET_H

#ifndef API
#define API extern
#endif

typedef int asset_type;
typedef void asset;
typedef void file_hndl;

typedef struct {
    char const* path;
    asset* asset;
    unsigned int timestamp;
} asset_hndl;

typedef struct {

} asset_init_desc;

API asset_hndl asset_hndl_null(void);
API asset_hndl asset_hndl_new(char const*);
API asset_hndl asset_hndl_new_load(char const*);
API asset_hndl asset_hdnl_new_ptr(asset*);

API int asset_hndl_valid(asset const*);
API const char* asset_hndl_path(asset const*);
API asset* asset_hndl_ptr(asset const*);
API int asset_hndl_equal(asset const*);

API void asset_init(asset_init_desc const*);
API void asset_finish(void);
API void asset_cache_flush(void);
API void asset_register(asset_type, char const*, asset*(*)(file_hndl const*), void(*)(asset const*));

API void file_load(char const*);
API void file_unload(char const*);
API void file_reload(char const*);
API int file_exists(char const*);
API int file_is_loaded(char const*);

API void folder_load(char const*);
API void folder_unload(char const*);
API void folder_reload(char const*);
API void folder_reload_recursive(char const*);

API asset* asset_get(char const*);
API asset* asset_get_load(char const*);
API asset* asset_get_load_as_type(asset_type, char const*);

API char const* asset_get_path(asset const*);
API char const* asset_get_extension(asset const*);
API char const* asset_get_typename(asset const*);

#endif //FIXEDPHYSICS_ASSET_H
