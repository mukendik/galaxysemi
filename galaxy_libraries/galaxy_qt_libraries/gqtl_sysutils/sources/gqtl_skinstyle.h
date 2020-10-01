///////////////////////////////////////////////////////////
// GEX skins style classes: header file
///////////////////////////////////////////////////////////

#ifndef GQTL_SKIN_STYLE_H
#define GQTL_SKIN_STYLE_H

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QString>
#include <QPalette>

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexSkinStyle
//
// Description	:	Base class used to define the skin settings
//
///////////////////////////////////////////////////////////////////////////////////
class CGexSkinStyle
{
protected:

    CGexSkinStyle()				{};

public:

    virtual ~CGexSkinStyle()	{};

    const QString&			name() const						{ return m_strName; }
    const QString&			path() const						{ return m_strPath; }
    const QString&			htmlSectionBackgroundColor() const	{ return m_strHtmlSectionBackgroundColor; }
    const QString&			htmlLabelBackgroundColor() const	{ return m_strHtmlLabelBackgroundColor; }
    const QString&			htmlDataBackgroundColor() const		{ return m_strHtmlDataBackgroundColor; }
    const QString&			htmlSectionTextColor() const		{ return m_strHtmlSectionTextColor; }
    const QString&			htmlLabelTextColor() const			{ return m_strHtmlLabelTextColor; }
    const QString&			htmlDataTextColor() const			{ return m_strHtmlLabelTextColor; }
    const QPalette&			palette() const						{ return m_palette; }

protected:

    QString				m_strName;								// Skin name
    QString				m_strPath;								// Skin folder
    QString				m_strHtmlSectionBackgroundColor;		// Background color for section
    QString				m_strHtmlSectionTextColor;				// Text color for section
    QString				m_strHtmlLabelBackgroundColor;			// Background color for label
    QString				m_strHtmlDataBackgroundColor;			// Text color for data
    QString				m_strHtmlLabelTextColor;				// Background color for data
    QString				m_strHtmlDataTextColor;					// Text color for data

    QPalette			m_palette;								// Skin palette
};

//////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexSkinStyleAqua
//
// Description	:	Class used to define the aqua skin settings
//
///////////////////////////////////////////////////////////////////////////////////
class CGexSkinStyleAqua : public CGexSkinStyle
{
public:

    CGexSkinStyleAqua();
    ~CGexSkinStyleAqua()		{};
};
#endif // GQTL_SKIN_STYLE_H
