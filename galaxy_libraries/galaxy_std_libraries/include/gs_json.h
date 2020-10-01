/******************************************************************************!
 * \file gs_json.h
 ******************************************************************************/
#ifndef GS_JSON_H
#define GS_JSON_H

#ifdef __cplusplus
extern "C" {
#endif

typedef const char* GsJsonIterator;

void GsJsonIteratorBegin(GsJsonIterator* iter, const char* json);
int GsJsonIteratorEnd(GsJsonIterator* iter);
void GsJsonIteratorNext(GsJsonIterator* iter);
int GsJsonIteratorInt(GsJsonIterator* iter);
char* GsJsonIteratorLabel(GsJsonIterator* iter);
const char* GsJsonIteratorValue(GsJsonIterator* iter);

#ifdef __cplusplus
}
#endif

#endif
