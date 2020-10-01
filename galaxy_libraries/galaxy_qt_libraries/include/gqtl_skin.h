///////////////////////////////////////////////////////////
// GEX skins classes: header file
///////////////////////////////////////////////////////////

#ifndef GQTL_SKIN_H
#define GQTL_SKIN_H

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QWidget>

class CGexSkinStyle;

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexSkin
//
// Description	:	Class used to hold the skin
//
///////////////////////////////////////////////////////////////////////////////////
class CGexSkin
{
public:

    CGexSkin();
    CGexSkin(const CGexSkin& gexSkin);
    ~CGexSkin();

    // Skin value supported by Galaxy application
    enum skinStyle
    {
        defaultStyle = -1,
        aquaStyle = 0
    };

    const QString&			name() const;
    const QString&			path() const;
    const QString&			htmlSectionBackgroundColor() const;
    const QString&			htmlLabelBackgroundColor() const;
    const QString&			htmlDataBackgroundColor() const;
    const QString&			htmlSectionTextColor() const;
    const QString&			htmlLabelTextColor() const;
    const QString&			htmlDataTextColor() const;
    const QPalette&			palette() const;
    skinStyle				currentStyle() const					{ return m_eStyle; }

    void					setCurrentStyle(CGexSkin::skinStyle eStyle);		// Set a new skin style

    void					applyPalette(QWidget * pWidget) const;					// Apply skin palette to the widget

    CGexSkin&				operator=(const CGexSkin& gexSkin);					// copy operator

private:

    CGexSkinStyle *			m_pSkinStyle;				// Skin style settings
    skinStyle				m_eStyle;					// Current Skin style

    CGexSkin::skinStyle		defaultSkinStyle();			// Get the default skin style from environment variable
};

#endif // GQTL_SKIN_H
