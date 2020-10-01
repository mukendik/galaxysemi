/******************************************************************************!
 * \file gs_json.c
 ******************************************************************************/
#include <stdlib.h>
#include "gs_json.h"

/******************************************************************************!
 * \fn GsJsonIteratorBegin
 ******************************************************************************/
void GsJsonIteratorBegin(GsJsonIterator* iter, const char* json)
{
    *iter = json;
    while (**iter != '[')
    {
        if (**iter == '\0')
        {
            return;
        }
        ++(*iter);
    }
    ++(*iter);
}

/******************************************************************************!
 * \fn GsJsonIteratorEnd
 ******************************************************************************/
int GsJsonIteratorEnd(GsJsonIterator* iter)
{
    return (**iter == '\0' || **iter == ']') ? 1 : 0;
}

/******************************************************************************!
 * \fn GsJsonIteratorNext
 ******************************************************************************/
void GsJsonIteratorNext(GsJsonIterator* iter)
{
    while (**iter != ',')
    {
        if (**iter == '\0' || **iter == ']')
        {
            return;
        }
        if (**iter == '"')
        {
            do
            {
                ++(*iter);
                if (**iter == '\0' || **iter == ']')
                {
                    return;
                }
            } while (**iter != '"');
        }
        ++(*iter);
    }
    ++(*iter);
}

/******************************************************************************!
 * \fn GsJsonIteratorInt
 ******************************************************************************/
int GsJsonIteratorInt(GsJsonIterator* iter)
{
    return atoi(*iter);
}

/******************************************************************************!
 * \fn GsJsonIteratorLabel
 ******************************************************************************/
char* GsJsonIteratorLabel(GsJsonIterator* iter)
{
    static char name[256];
    int i;

    if (**iter == '{')
    {
        ++(*iter);
    }
    if (**iter != '"')
    {
        name[0] = '\0';
        return name;
    }
    ++(*iter);
    i = 0;
    while (**iter != '"')
    {
        if (**iter == '\0')
        {
            name[0] = '\0';
            return name;
        }
        name[i] = **iter;
        ++(*iter);
        ++i;
    }
    name[i] = '\0';
    ++(*iter);

    return name;
}

/******************************************************************************!
 * \fn GsJsonIteratorValue
 ******************************************************************************/
const char* GsJsonIteratorValue(GsJsonIterator* iter)
{
    static char value[256];
    int i;

    while (**iter != ':')
    {
        if (**iter == '\0')
        {
            value[0] = '\0';
            return value;
        }
        ++(*iter);
    }
    ++(*iter);
    i = 0;
    while (**iter != ',' && **iter != '}')
    {
        if (**iter == '\0')
        {
            value[0] = '\0';
            return value;
        }
        if (**iter == '"')
        {
            do
            {
                value[i] = **iter;
                ++(*iter);
                ++i;
                if (**iter == '\0')
                {
                    value[0] = '\0';
                    return value;
                }
            } while (**iter != '"');
        }
        value[i] = **iter;
        ++(*iter);
        ++i;
    }
    value[i] = '\0';

    return value;
}
