#ifndef WIZARDS_HANDLER_H
#define WIZARDS_HANDLER_H

#include "QTitleFor.h"

#include <QString>
#include <interactive_wizard.h>

class GexMainwindow;
class QDetachableTabWindowContainer;
namespace Gex
{
    ///
    /// \brief The WizardHandler class handles the creation, link, find between the wizards as the interactive
    /// table, the wafermap 3D, and the interactive chart. The purpose is to have an unique interface to use the wizard
    /// whatever the specificity
    class WizardHandler
    {
    public:

        WizardHandler();
        ~WizardHandler();

        ///
        /// \brief Clear -  delete all the current wizard and clear the containers
        ///
        void                        Clear                   ();

        ///
        /// \brief For convenient, call the function CreateWizard (see description below)
        ///
        GexWizardTable*             CreateWizardTable       (unsigned int testNumber=0, QString testName="", int pinNumber=-1);
        GexWizardChart*             CreateWizardChart       (unsigned int testNumber=0, QString testName="", int pinNumber=-1);
        DrillDataMining3D*          CreateWizardWaferMap3D  (unsigned int testNumber=0, QString testName="", int pinNumber=-1);

        ///
        /// \brief For convenient, call the function RemoveWizard (see description below)
        ///
        void                        RemoveWizardTable       (GexWizardTable* lWizard) ;
        void                        RemoveWizardChart       (GexWizardChart* lWizard);
        void                        RemoveWizardWaferMap3D  (DrillDataMining3D* lWizard);

        ///Remove the "wizard" in parameter from the list of wizard if found.
        ///

        void RemoveAllWizardSExcept(GexWizardChart* wizard);
        ///
        /// \brief ReloadWizards - Reload all wizard whatever the test
        ///
        void                        ReloadWizards (bool resynchronizedWithData = false);

        ///
        /// \brief ReloadWizardsDisplayingTest  - reload all the wizard displaying a specific test
        /// \param testNumber                   - Test number of the test
        /// \param testName                     - Test name of the test
        /// \param pinNumber                    - Pin number of the test
        ///
        void                        ReloadWizardsDisplayingTest (const QString& testNumber, const QString& testName, const QString& pinNumber);


        void  SetGexMainwindow(GexMainwindow* mainWindow) { mMainWindow = mainWindow; }

        ///
        /// \brief Accesor of the different Wizard containers
        /// \return the container
        ///
        QList<GexWizardChart*>&     ChartWizards  ()  { return mChartWizards;}
        QList<GexWizardTable*>&     TableWizards  ()  { return mTableWizards;}
        QList<DrillDataMining3D*>&  Wafer3DWizards()  { return mWafer3DWizards;}

        void                        SetLastChartViewed(GexWizardChart* lastViewed) { mLastChartViewed = lastViewed;}
        GexWizardChart*             GetLastChartViewed() const { return mLastChartViewed;}

        void                        SetLastTableViewed(GexWizardTable* lastViewed) { mLastTableViewed = lastViewed;}
        GexWizardTable*             GetLastTableViewed() const { return mLastTableViewed;}

        void                        SetLastWafer3DViewed(DrillDataMining3D* lastViewed) { mLastWafer3DViewed = lastViewed;}
        DrillDataMining3D*          GetLastWafer3DViewed() const { return mLastWafer3DViewed;}

        void                        setQDetachableTabWindowContainer(QDetachableTabWindowContainer*  qDetachableTabWindowContainer) { mQDetachableTabWindowContainer = qDetachableTabWindowContainer;}

        void                        SetMainWindowAlreadyReset (bool status) { mMainWindowAlreadyReset = status; }

        /**
         * @brief SetWidgetToSwitch keep in memory the widget that need to be switch
         */
        void SetWidgetToSwitch( GS::Gex::QTitleFor< QDialog >* widget)
        {
            // GCORE-10303
            // if (mWidgetToSwitch)
            // {
            //     delete mWidgetToSwitch;
            // }
            mWidgetToSwitch = widget;
        }


        /**
         * @brief GetWidgetToSwitch return if exists the widget that must be switch.
         */
        GS::Gex::QTitleFor< QDialog >* GetWidgetToSwitch() { return mWidgetToSwitch;}

        QDetachableTabWindowContainer * GetDetachableWindowContainer()
        { return mQDetachableTabWindowContainer; }

    private:

        QDetachableTabWindowContainer* mQDetachableTabWindowContainer;


        typedef QList<IInteractiveWizard* >             IInteractiveWizardContainer;
        typedef IInteractiveWizardContainer::iterator   Iterator;

        static int mCPtIndexWIzard;
        ///
        /// \brief mWizardCharts, contains the list of IInteractiveWizard
        /// that have been created ordered by T_WIZARD type
        ///
        IInteractiveWizardContainer  mWizards;

        QList<GexWizardChart*>          mChartWizards;   // -- contains the list of the Chart Wizard
        QList<GexWizardTable*>          mTableWizards;   // -- contains the list of the Table Wizard
        QList<DrillDataMining3D*>       mWafer3DWizards; // -- contains the list of the Wafer 3D Wizard


        GexWizardChart*                 mLastChartViewed;
        GexWizardTable*                 mLastTableViewed;
        DrillDataMining3D*              mLastWafer3DViewed;
        GexMainwindow*                  mMainWindow;

        GS::Gex::QTitleFor< QDialog >*  mWidgetToSwitch;

        bool                            mMainWindowAlreadyReset;

        ///
        /// \brief addInList - update the dedicated container
        /// \param wizard    the new wizard that must be added
        ///
        void AddInList(GexWizardChart*      wizard);
        void AddInList(GexWizardTable*      wizard);
        void AddInList(DrillDataMining3D*   wizard);

        ///
        /// \brief ProcessNoWizardLeft - if all the wizard have been deleted, process a special
        /// behaviour as displaying the page report
        ///
        void ProcessNoWizardLeft();

        ///
        /// Create a IInteractiveWizard according to the type of interactive object. This IInteractiveWizard will contain
        /// the corresponding wizard QObject that will be return
        /// See the functions CreateWizardTable, CreateWizardChart, CreateWizardWaferMap3D for more convenient
        /// @param QWidget* widgetParent, widget that will contain the wizard
        /// @param testNumber is test number of the test currently displayed in the wizard
        /// @param testName is the test name of the test currently displayed in the wizard
        /// @param pinNumber is the pin number of the test currently displayed in the wizard
        /// @return the wizard QObject (either GexWizardTable, GexWizardChart or DrillDataMining3D)
        template<typename T_Wizard>
        typename T_Wizard::WizardInstance* CreateWizard( unsigned int testNumber, QString testName, int pinNumber)
        {
            typename T_Wizard::WizardInterface* lWizardInterface =  new typename T_Wizard::WizardInterface (testNumber, testName, pinNumber);
            mWizards.push_back(lWizardInterface);

            typename T_Wizard::WizardInstance* lWizardInstance =  new typename T_Wizard::WizardInstance (0);

            lWizardInterface->SetWizardInstance(lWizardInstance);
            lWizardInterface->mIndex = mCPtIndexWIzard;
            lWizardInstance->mIndex = mCPtIndexWIzard;
            ++mCPtIndexWIzard;
            AddInList(lWizardInterface->Wizard());
            return lWizardInterface->Wizard();
        }


        ///
        ///Remove the "wizard" in parameter from the list of wizard if found.
        ///
        template<typename T_Wizard>
        void RemoveWizard(typename T_Wizard::WizardInstance* wizard)
        {
            Iterator lIterBegin(mWizards.begin()), lIterEnd(mWizards.end());
            for(; lIterBegin != lIterEnd; ++lIterBegin)
            {
                //if( static_cast< typename T_Wizard::WizardInterface*> ((*lIterBegin))->Wizard() == wizard)
                if( (*lIterBegin)->mIndex == wizard->mIndex)
                {
                    delete (*lIterBegin);
                    mWizards.erase(lIterBegin);
                    break;
                }
            }
            ProcessNoWizardLeft();
        }


        ///
        /// \brief FindWizardsOnSameTest - Find all the wizard on the same test
        /// \param testNumber            - Test number of the test
        /// \param testName              - Test name of the test
        /// \param pinNumber             - Pin number of the test
        /// \return                      - the list of the interactiveWizard currently displaying the test
        ///
        QList<IInteractiveWizard*>  FindWizardsOnSameTest   (const QString &testNumber, const QString &testName, const QString &pinNumber);

        ///
        /// \brief FindWizard       - Find a type of wizard that is currently display data of a test
        /// \param typeOfWizard     - Type of the wizard researched. (Table, Wafer3D or Charts)
        /// \param testNumber       - Test number of the test
        /// \param testName         - Test name of the test
        /// \param pinNumber        - Pin number of the test
        /// \return
        ///
        IInteractiveWizard*         FindWizard              (T_WIZARD typeOfWizard, const QString &testNumber, const QString &testName, const QString &pinIndex);


    };
}

#endif // WIZARDS_HANDLER_H

