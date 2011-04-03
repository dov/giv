//======================================================================
//  EggStringArray.c - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Jul 10 07:57:05 2009
//----------------------------------------------------------------------

#include "EggStringArray.h"

EggStringArray* egg_string_array_new()
{
    return (EggStringArray*)g_ptr_array_new();
}

void egg_string_array_free(EggStringArray *string_array)
{
    g_ptr_array_foreach(string_array,
                        (GFunc)g_free,
                        NULL);
    g_free(string_array);
}

/** 
 * Add a string to a string array. Note that only the pointer is
 * transfered and thus the ownership is passed.
 * 
 * @param string_array 
 * @param s 
 */
void egg_string_array_add(EggStringArray* string_array,
                          char *s)
{
    g_ptr_array_add(string_array,
                    s);
}
