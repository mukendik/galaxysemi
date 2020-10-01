///////////////////////////////////////////////////////////
// GEX skin style classes: implementation file
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gqtl_skinstyle.h"

///////////////////////////////////////////////////////////////////////////////////
// Class CGexSkinStyleAqua - class which holds skin aqua
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexSkinStyleAqua::CGexSkinStyleAqua()
{
    m_strName						= "AQUA";
    m_strPath						= "/skins/aqua/";
    m_strHtmlSectionBackgroundColor	= "\"#FFFFFF\"";
    m_strHtmlSectionTextColor		= "\"#006699\"";
    m_strHtmlLabelBackgroundColor	= "\"#CCECFF\"";
    m_strHtmlDataBackgroundColor	= "\"#F8F8F8\"";
    m_strHtmlLabelTextColor			= "\"#000000\"";
    m_strHtmlDataTextColor			= "\"#000000\"";

    QBrush brushGex;

    brushGex  = QBrush(Qt::white);
    brushGex.setStyle(Qt::SolidPattern);

    m_palette.setBrush(QPalette::Active,	QPalette::Window, brushGex);
    m_palette.setBrush(QPalette::Inactive,	QPalette::Window, brushGex);
    m_palette.setBrush(QPalette::Disabled,	QPalette::Window, brushGex);
}
