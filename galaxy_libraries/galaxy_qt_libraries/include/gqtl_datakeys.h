#ifndef GQTL_DATAKEYS_H
#define GQTL_DATAKEYS_H

#include <gqtl_datakeys_global.h>
#include <gqtl_datakeys_file.h>
#include <gqtl_datakeys_data.h>
#include <gqtl_datakeys_loader.h>
#include <gqtl_datakeys_engine.h>
#include <gqtl_datakeys_content.h>
#include <gqtl_datakeys_definition_loader.h>

namespace GS
{
namespace QtLib
{

#define DEFAULT_CONFIG_FILE_NAME    "config.gexdbkeys"
#define REGEXP_TEST_KEY             "test\\[(.*)\\]"
#define REGEXP_TEST_CONDITION_KEY   "testCondition\\[(.*)\\]"
#define REGEXP_ATTR_TEST_CONDITION  "testCondition\\[([\\w\\d\\s]+)\\]"
#define REGEXP_ATTR_TEST            "test\\[(name|number)\\]"

}
}

#endif // GQTL_DATAKEYS_H
