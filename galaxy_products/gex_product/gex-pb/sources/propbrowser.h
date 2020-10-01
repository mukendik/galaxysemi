#ifndef GEX_PROPBROWSER_H
#define GEX_PROPBROWSER_H

#include "qttreepropertybrowser.h"
#include "qtvariantproperty.h"
#include <QTextEdit>
#include <QDomDocument>

#include "gex_scriptengine.h"

#include "cgexpublicproperty.h"
#include "cpropertyidmanager.h"

#define HTML_TYPE	QVariant::UserType+1
#define PATH_TYPE	QVariant::UserType+2
#define JS_TYPE		QVariant::UserType+3


class CVariantManager : public QtVariantPropertyManager
{
    Q_OBJECT
public:
    CVariantManager(CPropertyIdManager* cpimPtrIDManager, QObject *parent = 0);
    ~CVariantManager();

    virtual QVariant value(const QtProperty *property) const;
    virtual int valueType(int propertyType) const;
    virtual bool isPropertyTypeSupported(int propertyType) const;

    QString valueText(const QtProperty *property) const;

public slots:
    virtual void setValue(QtProperty *property, const QVariant &val);
protected:
    virtual void initializeProperty(QtProperty *property);
    virtual void uninitializeProperty(QtProperty *property);
private slots:
    void slotValueChanged(QtProperty *property, const QVariant &value);
    void slotPropertyDestroyed(QtProperty *property);
private:
    CPropertyIdManager* m_cpimIdManager;

    /// TO REVIEW : hack of the normal flow
    struct Data {
        QVariant value;
    };
    QMap<const QtProperty *, Data> propertyToData;

};

class PbDirectoryWidget;

class CVariantEditorFactory : public QtVariantEditorFactory
{
    Q_OBJECT

public:
    // explicit  CVariantEditorFactory(QObject *parent = 0);
    explicit  CVariantEditorFactory(CPropertyIdManager* cpimPtrIdManager, QObject *parent = 0);
    ~CVariantEditorFactory();
    QMap<QtProperty*, QTextEdit*> m_PropertyToTextEditMap;
    QMap<PbDirectoryWidget*, QtProperty*>   m_DirectoryEditorToProperty;

public slots:
    void SlotDirectoryEditDestroyed(QObject*);
    void SlotDirectoryEditChanged(const QString &value);
    void SlotTextEditChanged();
protected:
    QWidget *createEditor(QtVariantPropertyManager *manager, QtProperty *property, QWidget *parent);

private:
    CPropertyIdManager* m_cpimPtrIdManager;
};



class CPropBrowser : public QtTreePropertyBrowser
{
    Q_OBJECT

public:
  CPropBrowser(QWidget* qwParentWidget, GexScriptEngine *cgsePtrScriptEngine);
    ~CPropBrowser();

    /////////////////////////////////
    // accessors
    /////////////////////////////////
    // comment : addProperty() and SetPropertyValue() are completing methods !!
    int						AddProperty(QMap< QString, QString > attributes, int nParentId=-1);
    int						AddProperty(CGexPublicProperty& cgppProperty);
    QExplicitlySharedDataPointer<CGexPublicProperty>		GetProperty(const int nPropertyId);
    bool					RemoveProperty(int nPropertyId);

    bool					SetPropertyValue(int nPropertyId,const QVariant& qvNewValue);
    QVariant				GetCurrentValue(const int nPropertyId) const;
    bool					SetPropertyDefaultValue(int nPropertyId);

    ///////////////////////////////////
    // global methods
    ///////////////////////////////////
    bool Clear();
    bool Refresh();
    bool Reset();
    void SetExpandedAllProperties(const bool bIsExpanded);

public slots:
    void SlotPbPropertyValueChanged(int nPropertyId, const QVariant& qvPropertyValue);
    void SlotPbPropertyValueChanged(QtProperty* property, const QVariant& value);

signals:
    void SignalPbPropertyValueChanged(int ,const QVariant& );


private:
    //////////////////////////////
    // class members
    //////////////////////////////
    CPropertyIdManager*		m_cpimPtrPropertyIdManager;
    CVariantManager*		m_cvmPtrVariantManager;
    CVariantEditorFactory*	m_cvefPtrVariantFactory;
  GexScriptEngine*		m_cgsePtrScriptEngine;
    int						m_nPropBrowserLogLevel;

    // function pointers map(s)
    QMap<QString, bool(CPropBrowser::*)(QExplicitlySharedDataPointer<CGexPublicProperty>, QtVariantProperty*) >								m_qmUpdateSpecificPropertyAttributes;
    QMap<QString, QVariant(CPropBrowser::*)(const QExplicitlySharedDataPointer<CGexPublicProperty> ,const QtVariantProperty* )const >		m_qmSpecificGetValue;
    QMap<QString, bool(CPropBrowser::*)(QExplicitlySharedDataPointer<CGexPublicProperty>, QtVariantProperty* , const QVariant) >			m_qmSpecificSetValue;

    ////////////////////////////////
    // internal methods
    ////////////////////////////////
    // constructor methods
    void Connections();
    void BuildFunctionPointerMaps();

    // validation methods
    bool IsPropertyBrowserValidProperty(const CGexPublicProperty& cgppProperty) const;
    bool IsPropertyBrowserValidProperty(const int nPropertyID) const;

    //////////////////////////////////////////
    // global property management methods
    bool UpdatePropertyAttributes(const int nPropertyID);
    bool UpdateGenericPropertyAttributes(const int nPropertyID);
    bool UpdateSpecificPropertyAttributes( const int nPropertyID);


    //////////////////////////////////////////
    // specific property management methods

    // specific update properties methods
    bool Group_UpdateSpecificPropertyAttributes(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty);
    bool Int_UpdateSpecificPropertyAttributes(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty);
    bool Float_UpdateSpecificPropertyAttributes(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty);
    bool Enum_UpdateSpecificPropertyAttributes(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty);
    bool Flag_UpdateSpecificPropertyAttributes(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty);
    bool Bool_UpdateSpecificPropertyAttributes(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty);
    bool String_UpdateSpecificPropertyAttributes(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty);
    bool Color_UpdateSpecificPropertyAttributes(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty);
    bool Font_UpdateSpecificPropertyAttributes(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty);
    bool HTML_UpdateSpecificPropertyAttributes(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty);
    bool Path_UpdateSpecificPropertyAttributes(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty);
    bool JavaScript_UpdateSpecificPropertyAttributes(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty);

    // specific get property value methods
    QVariant	Specific_Group_GetCurrentValue(const QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty,const QtVariantProperty* qvpPtrVariantProperty) const;
    QVariant	Specific_Int_GetCurrentValue(const QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty,const QtVariantProperty* qvpPtrVariantProperty) const;
    QVariant	Specific_Float_GetCurrentValue(const QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty,const QtVariantProperty* qvpPtrVariantProperty) const;
    QVariant	Specific_Enum_GetCurrentValue(const QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty,const QtVariantProperty* qvpPtrVariantProperty) const;
    QVariant	Specific_Flag_GetCurrentValue(const QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty,const QtVariantProperty* qvpPtrVariantProperty) const;
    QVariant	Specific_Bool_GetCurrentValue(const QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty,const QtVariantProperty* qvpPtrVariantProperty) const;
    QVariant	Specific_String_GetCurrentValue(const QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty,const QtVariantProperty* qvpPtrVariantProperty) const;
    QVariant	Specific_Color_GetCurrentValue(const QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty,const QtVariantProperty* qvpPtrVariantProperty) const;
    QVariant	Specific_Font_GetCurrentValue(const QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty,const QtVariantProperty* qvpPtrVariantProperty) const;
    QVariant	Specific_HTML_GetCurrentValue(const QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty,const QtVariantProperty* qvpPtrVariantProperty) const;
    QVariant	Specific_Path_GetCurrentValue(const QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty,const QtVariantProperty* qvpPtrVariantProperty) const;
    QVariant	Specific_JavaScript_GetCurrentValue(const QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty,const QtVariantProperty* qvpPtrVariantProperty) const;

    // specific set property value methods
    bool	Specific_Group_SetPropertyValue(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty, const QVariant qvNewValue);
    bool	Specific_Int_SetPropertyValue(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty, const QVariant qvNewValue);
    bool	Specific_Float_SetPropertyValue(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty, const QVariant qvNewValue);
    bool	Specific_Enum_SetPropertyValue(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty, const QVariant qvNewValue);
    bool	Specific_Flag_SetPropertyValue(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty, const QVariant qvNewValue);
    bool	Specific_Bool_SetPropertyValue(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty, const QVariant qvNewValue);
    bool	Specific_String_SetPropertyValue(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty, const QVariant qvNewValue);
    bool	Specific_Color_SetPropertyValue(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty, const QVariant qvNewValue);
    bool	Specific_Font_SetPropertyValue(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty, const QVariant qvNewValue);
    bool	Specific_HTML_SetPropertyValue(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty, const QVariant qvNewValue);
    bool	Specific_Path_SetPropertyValue(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty, const QVariant qvNewValue);
    bool	Specific_JavaScript_SetPropertyValue(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty, const QVariant qvNewValue);


};


#endif // GEX_PROPBROWSER_H
