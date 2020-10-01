#ifndef CHARTELEMENT_H
#define CHARTELEMENT_H

#include <QPixmap>
#include "report_element.h"


class ChartElement : public ReportElement
{
public:

    ChartElement(Component *parent);
    ChartElement(const QString &name, const QJsonObject& description, Component* parent, T_Component type);

    /**
     * \fn bool DrawSection(QString &imagePath, CTest* sectionTest = NULL);
     * \brief this function draws the chart image in the image path given in parameter.
     * \param imagePath the path of the created image
     * \param sectionTest the test comming from the section if exists
     * \return true if the draw has been done with success. Otherwise return false.
     */
    virtual bool        DrawSection(QString &imagePath, int imageSize, CTest* sectionTest = NULL);

    /**
     * \fn QPixmap CreatePixmap()
     * \brief this function draws the chart image.
     * \return the created image. Otherwise returns an empty pixmap.
     */
    QPixmap             CreatePixmap();

    /**
        \brief set the specified panel as parent
     */
    static unsigned int         mCurrentId;

    /**
        \brief update the json object with update with the current element state
     */
    //void                        UpdateJson();
private:
    /**
     * \fn UpdateTestInChartOverlays
     * \brief this function updates the charoverlays using the parameters from the test
     * \param test
     * \param chartOverlays the parameter to change
     * \return the created image. Otherwise returns an empty pixmap.
     */
    void                        UpdateTestInChartOverlays(const Test &test, QJsonObject &chartOverlays);
};

#endif // HISTOGRAMELEMENT_H
