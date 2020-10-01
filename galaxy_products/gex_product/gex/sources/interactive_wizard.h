#ifndef INTERACTIVE_WIZARD_H
#define INTERACTIVE_WIZARD_H

#include "drill_chart.h"
#include "drill_table.h"
#include "drillDataMining3D.h"


namespace Gex
{

    ///
    /// \brief The T_WIZARD enum describes the different kind of wizard currently handled
    ///
    typedef enum{T_TABLE = 0, T_CHARTS, T_WAFER3D, UNKNOWN } T_WIZARD;

    ///
    /// \brief The InteractiveWizard class describ the interface of the InteractiveWizard
    /// that are managed by the WizardHandler. Those class contains a wizard according to a test
    ///
    class IInteractiveWizard
    {
    public:
        IInteractiveWizard(unsigned int testNumber, QString testName, int pinNumber, T_WIZARD type);
        virtual ~IInteractiveWizard();

        /// -- Getter and setter
        virtual  QString       TestName    () const { return "";}
        virtual  QString       TestNumber  () const { return "";}
        virtual  QString       PinIndex    () const { return "";}
        T_WIZARD               Type        () const { return mType;}

        /// -- Call the close function of the wizard. To be reimplemented in each subclass
        /// -- according to the wizard type
        virtual void           Close       () const = 0 ;

        virtual void           Reload      (bool ) const = 0 ;



         int         mIndex;
    protected:



        /// -- indentifier of the test
        CTest*       mTestCell;
        QString      mTestName;
        unsigned int mTestNumber;
        int          mPinNumber;

        /// -- type of the interactive wizard
        T_WIZARD     mType;
    };

    class InteractiveChartsWizard : public IInteractiveWizard
    {
    public:
        InteractiveChartsWizard(unsigned int testNumber, QString testName, int pinNumber);
        ~InteractiveChartsWizard();


         QString             TestName    () const { return mWizard->ptTestCell->GetTestName();}
         QString             TestNumber  () const { return mWizard->ptTestCell->GetTestNumber();}
         QString             PinIndex    () const { return mWizard->ptTestCell->GetPinIndex();}
         GexWizardChart*     Wizard      () const { return mWizard; }

         void                Close       () const { mWizard->close(); }
         void                Reload      (bool resynchronizedWithData) const {
             if(resynchronizedWithData)
             {
                 mWizard->clear();
                 mWizard->isDataAvailable();
             }
             mWizard->ShowChart("");
             mWizard->onHome();
         }

         void                SetWizardInstance(GexWizardChart* wizardInstance) {mWizard  = static_cast<GexWizardChart*>(wizardInstance);}
    private:

        GexWizardChart* mWizard;
    };


    class InteractiveTableWizard : public IInteractiveWizard
    {
    public:
        InteractiveTableWizard(unsigned int testNumber, QString testName, int pinNumber);
        ~InteractiveTableWizard();

         GexWizardTable*     Wizard      () const { return mWizard; }

         void                Close       () const { mWizard->close(); }
         void                Reload      (bool ) const { mWizard->ShowTable("");}

         void                SetWizardInstance(GexWizardTable* wizardInstance) {mWizard  = static_cast<GexWizardTable*>(wizardInstance);}
    private:

        GexWizardTable* mWizard;
    };

    ///
    /// \brief The InteractiveWafer3DWizard class manage
    ///
    class InteractiveWafer3DWizard : public IInteractiveWizard
    {
    public:
        InteractiveWafer3DWizard(unsigned int testNumber, QString testName, int pinNumber);
        ~InteractiveWafer3DWizard();

         QString             TestNumber  () const { return QString(mWizard->m_drillDataInfo.testNumber());}
         QString             PinIndex    () const { return QString(mWizard->m_drillDataInfo.pinmap());}
         DrillDataMining3D*  Wizard      () const { return mWizard; }

         void                Close       () const { mWizard->close(); }
         void                Reload      (bool ) const { mWizard->ShowChart("");}

         void                SetWizardInstance(DrillDataMining3D* wizardInstance) {mWizard  = static_cast<DrillDataMining3D*>(wizardInstance);}
   private:

        DrillDataMining3D* mWizard;
    };


    ///
    /// Template specialization used for the WizardHandler::CreateWizard and
    /// WizardHandler::RemoveWizard function
    ///
    template<T_WIZARD>
    struct T_InteractiveWizard
    {
    };

    template<>
    struct T_InteractiveWizard<T_TABLE>
    {
        typedef InteractiveTableWizard          WizardInterface;
        typedef GexWizardTable                  WizardInstance;
    };

    template<>
    struct T_InteractiveWizard<T_CHARTS>
    {
        typedef InteractiveChartsWizard         WizardInterface;
        typedef GexWizardChart                  WizardInstance;
    };

    template<>
    struct T_InteractiveWizard<T_WAFER3D>
    {
        typedef InteractiveWafer3DWizard        WizardInterface;
        typedef DrillDataMining3D               WizardInstance;
    };

}
#endif // INTERACTIVE_WIZARD_H

