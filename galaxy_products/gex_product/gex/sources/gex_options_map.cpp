#include "gex_options_map.h"
#include <gqtl_log.h>

namespace GS
{
namespace Gex
{

OptionsMap::OptionsMap()
{
}

OptionsMap::~OptionsMap()
{
}

const QMap<QString, QMap<QString, QString> >& OptionsMap::map() const
{
    return m_map;
}

void OptionsMap::clear()
{
    m_map.clear();
}

QVariant OptionsMap::GetOption(const QString& section,const QString& option_name) const
{
    if (section=="")
    {
#ifdef QT_DEBUG
        /*
         warning(NULL,"Warning",
         "Trying to GetOption(...) with an empty section name !");
        */
        // CReportOptions used has global variable.
        // can't create a widget if(!qApp)
        // cf. l.291; qWidget.cpp; qFatal("QWidget: Must
        // construct a QApplication before a QPaintDevice");
        GEX_ASSERT(false);
#endif
        return QVariant();
    }

    if (m_map.find(section) == m_map.end())
    {
#ifdef QT_DEBUG
        /*
          warning(NULL,"Warning",
          QString("Trying to GetOption(...) for unfindable section '%1' !").
          arg(section));
        */
        // SetSpecificFlagOption(...) call GetOption in
        // reset method even if option does not exist yet
#endif
        return QVariant();
    }

    QMap<QString, QString> m = m_map[section];

    if (m.find(option_name) == m.end())
        return QVariant();

    QVariant v(m[option_name]);

    return v;
}

}   // namespace Gex
}   // namespase GS
