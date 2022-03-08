/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBCWEB_GUI_H
#define IBCWEB_GUI_H

#ifndef IBC_API
#define IBC_API extern
#endif

IBC_API void gui_init();
IBC_API void gui_begin_frame();
IBC_API void gui_end_frame();
IBC_API void gui_finalize();

#endif //IBCWEB_GUI_H
