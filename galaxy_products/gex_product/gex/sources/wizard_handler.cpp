#include "QDetachableTabWindowContainer.h"
#include "browser_dialog.h"
#include "wizards_handler.h"


namespace Gex {



int WizardHandler::mCPtIndexWIzard = 0;

    WizardHandler::WizardHandler(): mQDetachableTabWindowContainer(0),
                                    mLastChartViewed(0),
                                    mLastTableViewed(0),
                                    mLastWafer3DViewed(0),
                                    mMainWindow(0),
                                    mWidgetToSwitch(0),
                                    mMainWindowAlreadyReset(false)
    {

    }

    WizardHandler::~WizardHandler()
    {
    }


    //!-------------------------------------!//
    //!               Clear                 !//
    //! ------------------------------------!//
    void   WizardHandler::Clear()
    {
        Iterator lIterBegin(mWizards.begin()), lIterEnd(mWizards.end());
        for(;lIterBegin != lIterEnd; ++lIterBegin)
        {
            (*lIterBegin)->Close();
        }
    }

    //!-------------------------------------!//
    //!             addInList               !//
    //! ------------------------------------!//
    void  WizardHandler::AddInList(GexWizardChart*      wizard)
    {
        mChartWizards.push_front(wizard);
    }

    void  WizardHandler::AddInList(GexWizardTable*      wizard)
    {
        mTableWizards.push_front(wizard);
    }

    void  WizardHandler::AddInList(DrillDataMining3D*   wizard)
    {
        mWafer3DWizards.push_front(wizard);
    }

    //!-------------------------------------!//
    //!           CreateWizardTable         !//
    //! ------------------------------------!//
    GexWizardTable*      WizardHandler::CreateWizardTable(unsigned int testNumber, QString testName, int pinNumber)
    {
        return  CreateWizard< Gex::T_InteractiveWizard<Gex::T_TABLE> > (testNumber, testName, pinNumber);
    }

    //!-------------------------------------!//
    //!           CreateWizardChart         !//
    //! ------------------------------------!//
    GexWizardChart*     WizardHandler::CreateWizardChart(unsigned int testNumber, QString testName, int pinNumber)
    {
        return  CreateWizard< Gex::T_InteractiveWizard<Gex::T_CHARTS> > (testNumber, testName, pinNumber);
    }

    //!-------------------------------------!//
    //!      CreateWizardWaferMap3D         !//
    //! ------------------------------------!//
    DrillDataMining3D*   WizardHandler::CreateWizardWaferMap3D (unsigned int testNumber, QString testName, int pinNumber)
    {
        return  CreateWizard< Gex::T_InteractiveWizard<Gex::T_WAFER3D> > (testNumber, testName, pinNumber);

    }
    //!------------------------------------------!//
    //!            ProcessNoWizardLeft           !//
    //! -----------------------------------------!//
    void WizardHandler::ProcessNoWizardLeft()
    {
        if( mQDetachableTabWindowContainer->IsFirstWindowEmpty() && mMainWindowAlreadyReset == false)
        {
            mMainWindowAlreadyReset = true;
            if(mMainWindow)
                mMainWindow->ReportLink();
        }
    }

    //!-------------------------------------!//
    //!            RemoveWizardTable        !//
    //! ------------------------------------!//
    void  WizardHandler::RemoveWizardTable(GexWizardTable* lWizard)
    {
        RemoveWizard< Gex::T_InteractiveWizard<Gex::T_TABLE> >(lWizard);
        if(mTableWizards.contains(lWizard))
        {
            mTableWizards.removeAt(mTableWizards.indexOf(lWizard));
            if(lWizard == mLastTableViewed)
                mLastTableViewed = 0;
        }
    }

    //!-------------------------------------!//
    //!            RemoveWizardChart        !//
    //! ------------------------------------!//
    void  WizardHandler::RemoveWizardChart(GexWizardChart* lWizard)
    {
        RemoveWizard< Gex::T_InteractiveWizard<Gex::T_CHARTS> >(lWizard);
        if(mChartWizards.contains(lWizard))
        {
            mChartWizards.removeAt(mChartWizards.indexOf(lWizard));
            if(lWizard == mLastChartViewed)
                mLastChartViewed = 0;
        }
    }

    //!------------------------------------------!//
    //!            RemoveWizardWaferMap3D        !//
    //! -----------------------------------------!//
    void  WizardHandler::RemoveWizardWaferMap3D(DrillDataMining3D* lWizard)
    {
        RemoveWizard< Gex::T_InteractiveWizard<Gex::T_WAFER3D> >(lWizard);
        if(mWafer3DWizards.contains(lWizard))
        {
            mWafer3DWizards.removeAt(mWafer3DWizards.indexOf(lWizard));
            if(lWizard == mLastWafer3DViewed)
                mLastWafer3DViewed = 0;
        }
    }

    //!---------------------------------!//
    //!            ReloadWizards        !//
    //! --------------------------------!//
    void     WizardHandler::ReloadWizards(bool resynchronizedWithData)
    {
        Iterator lIterBegin(mWizards.begin()), lIterEnd(mWizards.end());

        for(;lIterBegin != lIterEnd; ++lIterBegin )
        {
            (*lIterBegin)->Reload(resynchronizedWithData);
        }
    }

    //!-----------------------------------------!//
    //!      ReloadWizardsDisplayingTest        !//
    //! ----------------------------------------!//
    void     WizardHandler::ReloadWizardsDisplayingTest (const QString& testNumber, const QString& testName, const QString& pinNumber)
    {
        QList<IInteractiveWizard*> lWizardsToBeUpdated = FindWizardsOnSameTest(testNumber, testName, pinNumber);

        Iterator lIterBegin(lWizardsToBeUpdated.begin()), lIterEnd(lWizardsToBeUpdated.end());

        for(;lIterBegin != lIterEnd; ++lIterBegin )
        {
            (*lIterBegin)->Reload(false);
        }

    }

    //!-----------------------------------------------------!//
    //!               FindWizardsOnSameTest                 !//
    //! ----------------------------------------------------!//
    QList<IInteractiveWizard*> WizardHandler::FindWizardsOnSameTest(const QString& testNumber, const QString& testName, const QString& pinIndex)
    {
        QList<IInteractiveWizard*> lWizardsOnSameTest;
        Iterator lIterBegin(mWizards.begin()), lIterEnd(mWizards.end());

        for(;lIterBegin != lIterEnd; ++lIterBegin )
        {
            if( (*lIterBegin)->TestNumber() ==  testNumber   &&
                (*lIterBegin)->TestName()   ==  testName     &&
                (*lIterBegin)->PinIndex()   ==  pinIndex
               )
               lWizardsOnSameTest.push_back(*lIterBegin);
        }

        return lWizardsOnSameTest;
    }

    //!-------------------------------------!//
    //!               FindWizard            !//
    //! ------------------------------------!//
    IInteractiveWizard* WizardHandler::FindWizard(T_WIZARD typeOfWizard, const QString& testNumber, const QString& testName, const QString& pinIndex)
    {
        Iterator lIterBegin(mWizards.begin()), lIterEnd(mWizards.end());

        for(;lIterBegin != lIterEnd; ++lIterBegin )
        {
            if( (*lIterBegin)->Type()       ==  typeOfWizard    &&
                (*lIterBegin)->TestNumber() ==  testNumber     &&
                ( (*lIterBegin)->TestName() == "" ||  (*lIterBegin)->TestName()   ==  testName  )          &&
                (*lIterBegin)->PinIndex()   ==  pinIndex
               )
               return (*lIterBegin);
        }

        return 0;
    }

    //!-------------------------------------!//
    //!         RemoveAllWizardSExcept      !//
    //! ------------------------------------!//
    void WizardHandler::RemoveAllWizardSExcept(GexWizardChart* wizard)
    {
        QList<GexWizardChart*>::iterator lIterChartBegin(mChartWizards.begin()),  lIterChartEnd(mChartWizards.end());
        for(; lIterChartBegin != lIterChartEnd; ++lIterChartBegin)
        {
            if( wizard== 0 || (*lIterChartBegin)->mIndex != wizard->mIndex)
            {
                mQDetachableTabWindowContainer->removeTabWindowContaining(*lIterChartBegin);
            }
        }

        QList<GexWizardTable*>::iterator lIterTableBegin(mTableWizards.begin()),  lIterTableEnd(mTableWizards.end());
        for(; lIterTableBegin != lIterTableEnd; ++lIterTableBegin)
        {
            if( wizard== 0 || (*lIterTableBegin)->mIndex != wizard->mIndex)
            {
                mQDetachableTabWindowContainer->removeTabWindowContaining(*lIterTableBegin);
            }
        }

        QList<DrillDataMining3D*>::iterator lIterWaferBegin(mWafer3DWizards.begin()),  lIterWaferEnd(mWafer3DWizards.end());
        for(; lIterWaferBegin != lIterWaferEnd; ++lIterWaferBegin)
        {
            if( wizard== 0 || (*lIterWaferBegin)->mIndex != wizard->mIndex)
            {
                mQDetachableTabWindowContainer->removeTabWindowContaining(*lIterWaferBegin);
            }
        }
    }

}
