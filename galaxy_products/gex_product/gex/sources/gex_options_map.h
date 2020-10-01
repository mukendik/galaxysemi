#ifndef GEX_OPTIONS_MAP_H
#define GEX_OPTIONS_MAP_H

#include <QMap>
#include <QVariant>

namespace GS
{
namespace Gex
{

class OptionsMap
{
public:

    OptionsMap();
    ~OptionsMap();

    const QMap<QString, QMap<QString, QString> >& map() const;

    void			clear();

    // Request an option value. Return empty QVariant if the option does not exist !
    QVariant		GetOption(const QString& section,const QString& option_name) const;

    friend class	OptionsHandler;

private:

    // Members
    QMap<QString, QMap<QString, QString> >	m_map;					// maps stocking options
};

}   // namespace Gex
}   // namespase GS

#endif // GEX_OPTIONS_MAP_H
