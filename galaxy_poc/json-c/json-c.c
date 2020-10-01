/******************************************************************************!
 * \file json-c.c
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json.h>
#include <json_object_iterator.h>

#define ERROR(f, ...) fprintf(stderr, "error: "f"\n", ## __VA_ARGS__)
#define DEBUG(f, ...) fprintf(stdout, "debug: "f"\n", ## __VA_ARGS__)

/******************************************************************************!
 * \fn getValue
 ******************************************************************************/
enum json_type getValue(json_object* obj, const char* key, json_object** value)
{
    if (! json_object_object_get_ex(obj, key, value))
    {
        ERROR("%s not found", key);
        exit(EXIT_FAILURE);
    }
    return json_object_get_type(*value);
}

/******************************************************************************!
 * \fn printIter
 ******************************************************************************/
void printIter(struct json_object_iterator* iter)
{
    json_object* obj;
    const char* name;
    enum json_type type;

    obj = json_object_iter_peek_value(iter);
    name = json_object_iter_peek_name(iter);
    type = json_object_get_type(obj);
    DEBUG("%s", json_type_to_name(type));
    switch (type)
    {
    case json_type_int:
        DEBUG("  %s = %d", name, json_object_get_int(obj)); break;
    case json_type_string:
        DEBUG("  %s = %s", name, json_object_get_string(obj)); break;
    case json_type_double:
        DEBUG("  %s = %f", name, json_object_get_double(obj)); break;
    default:
        break;
    }
}

/******************************************************************************!
 * \fn main
 ******************************************************************************/
int main()
{
    json_object* obj;
    json_object* cur;
    json_object* item;
    json_object* ft_ptest_info;
    json_object* ft_ptest_info_array;
    struct json_object_iterator iter;
    struct json_object_iterator iEnd;
    const char* str;
    int i;

    // ------------------------
    // Build
    // ------------------------
    obj = json_object_new_object();
    json_object_object_add(obj, "splitlot_id", json_object_new_int(1));

    ft_ptest_info_array = json_object_new_array();
    for (i = 0; i < 2; ++i)
    {
        ft_ptest_info = json_object_new_object();
        json_object_object_add(ft_ptest_info,
                               "ptest_info_id", json_object_new_int(123));
        json_object_object_add(ft_ptest_info,
                               "units", json_object_new_string("volt"));
        json_object_object_add(ft_ptest_info,
                               "spec_ll", json_object_new_double(0.123));
        json_object_array_add(ft_ptest_info_array, ft_ptest_info);
    }

    json_object_object_add(obj, "ft_ptest_info", ft_ptest_info_array);

    // ------------------------
    // Parse
    // ------------------------
    str = strdup(json_object_to_json_string(obj));
    DEBUG("%s", str);

    json_object_put(obj);
    obj = json_tokener_parse(str);
    if (obj == NULL)
    {
        ERROR("json_tokener_parse returns NULL");
        return EXIT_FAILURE;
    }

    // ------------------------
    // Use
    // ------------------------
    if (getValue(obj, "splitlot_id", &cur) == json_type_int)
    {
        DEBUG("splitlot_id = %d", json_object_get_int(cur));
    }

    if (getValue(obj, "ft_ptest_info", &cur) == json_type_array)
    {
        for (i = 0; i < json_object_array_length(cur); ++i)
        {
            DEBUG("------------------------ %d", i);
            item = json_object_array_get_idx(cur, i);
            iter = json_object_iter_begin(item);
            iEnd = json_object_iter_end(item);
            for (; ! json_object_iter_equal(&iter, &iEnd);
                 json_object_iter_next(&iter))
            {
                printIter(&iter);
            }
        }
    }

    return EXIT_SUCCESS;
}
