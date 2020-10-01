#ifndef WAFERMAPELEMENT_H
#define WAFERMAPELEMENT_H

#include "report_element.h"


class WaferMapElement : public ReportElement
{
public:
    WaferMapElement(const QString &name, const QJsonObject &jsonDescription, Component *parent);

    /**
     * \fn bool DrawSection(QString &imagePath, CTest* sectionTest = NULL);
     * \brief this function draws the wafermap  in the image path given in parameter.
     * \param imagePath the path of the created image
     * \param sectionTest the test comming from the section if exists
     * \return true if the draw has been done with success. Otherwise return false.
     */
    virtual bool        DrawSection(QString &imagePath, int, CTest* sectionTest = NULL);

private:
    void                        UpdateJson();
    static unsigned int          sCurrentId;     /// \param The id of the current WaferMapElement
};

#endif // HISTOGRAMELEMENT_H
