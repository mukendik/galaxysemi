#ifndef COMPOSITE_H
#define COMPOSITE_H

#include <QJsonObject>
#include <QList>
#include "component.h"

/**
 * @brief The Composite class is used to described Componenent that will contain several Componenent as child
 */
class Composite : public Component
{
public:
    Composite(const QString& name, const QJsonObject &jsonDescription, Component* parent, T_Component type);

    virtual ~Composite();

    /**
     * \fn bool AddElement(Component* reportElement);
     * \brief this function add a report element to the list of elements (mReportElements).
     * \param reportElement: the element to add to the list.
     * \return true if the insertion has been done with success. Otherwise return false.
     */
    bool        AddElement(Component* /*reportElement*/);

    /**
     * \fn bool AddElement(QJsonObject* settings);
     * \brief this function add a report element to the list of elements (mReportElements).
     * \param settings: the setting of the new element to add to the list.
     * \return true if the insertion has been done with success. Otherwise return false.
     */
    bool        AddElement(const QJsonObject& /*settings*/);

    /**
     * @brief erase a child
     * @param toRemove is the Componenent to remove
     */
    void        EraseChild(Component * toRemove);

    /**
     * @brief BuildElement - build a ReportElement objetc according to the json description;
     * @param jsonDescription
     * @return 0 if the description doesn't correspond to any known type of ReportElement
     */
    Component* BuildReportElement( const QJsonObject &jsonDescription);


    /**
     * @brief reimplementation of the Componenent::Clear function
     */
    void        Clear();

    /**
     * @brief WriteJson -  write the object as a json object
     */
    virtual QJsonObject ToJson();


    /**
     * @brief this function is called when the element added neede to be updated
     * with the current state of the Composite instance. Nothing is done by default
     */
    virtual void UpdateElementsBeforeAdded(Component* ){}

    /**
     * \fn bool DrawSection(QString &imagePath, CTest* sectionTest = NULL);
     * \brief this function draws the report element in the image path given in parameter.
     * \param imagePath the path of the created image
     * \param sectionTest the test comming from the section if exists
     * \return true if the draw has been done with success. Otherwise return false.
     */
    bool        DrawSection(QString &imagePath, int imageSize, CTest* sectionTest = NULL);


    /**
    * @brief return the list of the childs
    */
    const QList<Component*> & GetElements() const;

    /**
     * @brief return the the number of childs
     */
    int GetElementsNumber() const;

    const QList<Component *> &GetElementsSorted() const;
protected:
    // mutuable is only because we have to order the list and nor to change values
    mutable QList<Component*>   mReportElements;        /// \param the list of all report elements
};


#endif // COMPOSITE_H
