///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "browser_dialog.h"
#include "gexsinglechartwidget.h"
#include "gexhistogramchart.h"
#include "gextrendchart.h"
#include "gexboxplotchart.h"
#include "gexprobabilityplotchart.h"
#include "gexscatterchart.h"
#include "gex_xbar_r_chart.h"
#include "gex_box_whisker_chart.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "report_options.h"
#include "drill_chart.h"
#include "drillDataMining3D.h"
#include "drill_editchart_options.h"
#include "gexinputdialog.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "customdialogbox.h"
#include "QUndoView"
#include "message.h"
#include "ofr_controller.h"
///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QClipboard>
#include <QPainter>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QCheckBox>
//////////////////////////////////////////////////////////////////////////////////
// External object
///////////////////////////////////////////////////////////////////////////////////
extern CGexReport *		gexReport;
extern GexMainwindow *	pGexMainWindow;
extern double	ScalingPower(int iPower);

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

///////////////////////////////////////////////////////////////////////////////////
// External definitions for pointers to pixmaps.
///////////////////////////////////////////////////////////////////////////////////
#include "gex_pixmap_extern.h"

///////////////////////////////////////////////////////////////////////////////////
// Class GexSingleChartWidget - class which display one chart
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexSingleChartWidget::GexSingleChartWidget(QWidget * parent /*= NULL*/, bool bAutoRepaint /*= false*/)
                        : QWidget( parent), m_bAutoRepaint(bAutoRepaint), mWizardParent(0)
{
    m_eMouseMode		= mouseSelect;
    m_bOutline			= false;
    m_bTracking			= false;
    m_pRubberBand		= NULL;

    m_pAbstractChart	= NULL;
    m_pChartMenuInfo	= NULL;

    m_zoomCursor		= QCursor(QPixmap(QString::fromUtf8(":/gex/icons/zoomcursor.png")), QPixmap(QString::fromUtf8(":/gex/icons/zoomcursormask.png")), 1, 1);

    // Allow to catch all mouse moves
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    m_eMouseStatus = GexSingleChartWidget::unknow;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexSingleChartWidget::~GexSingleChartWidget()
{

}

void   GexSingleChartWidget::init(GexWizardChart*   lWizardParent)
{
    if(lWizardParent)
    {
        mWizardParent = lWizardParent;
    }
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setChartType(int nType)
//
// Description	:	Set the type of the chart
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::setChartType(GexAbstractChart::chartType eChartType, CGexChartOverlays*	lChartsInfo, int nViewportMode, bool bKeepViewport)
{
    if (m_pAbstractChart)
    {
        delete m_pAbstractChart;
        m_pAbstractChart = NULL;
    }

    switch(eChartType)
    {
        case GexAbstractChart::chartTypeHistogram			:	m_pAbstractChart = new GexHistogramChart(false, GEX_CHARTSIZE_AUTO, mWizardParent, lChartsInfo);
                                                                break;

        case GexAbstractChart::chartTypeTrend				:	m_pAbstractChart = new GexTrendChart(false, GEX_CHARTSIZE_AUTO, mWizardParent, lChartsInfo);
                                                                break;

        case GexAbstractChart::chartTypeScatter				:	m_pAbstractChart = new GexScatterChart(GEX_CHARTSIZE_AUTO, mWizardParent, lChartsInfo);
                                                                break;

        case GexAbstractChart::chartTypeBoxPlot				:	m_pAbstractChart = new GexBoxPlotChart(GEX_CHARTSIZE_AUTO, mWizardParent, lChartsInfo);
                                                                break;

        case GexAbstractChart::chartTypeProbabilityPlot     :	m_pAbstractChart = new GexProbabilityPlotChart(GEX_CHARTSIZE_AUTO, mWizardParent, lChartsInfo);
                                                                break;

        case GexAbstractChart::chartTypeXBar				:	m_pAbstractChart = new GexXBarChart(GEX_CHARTSIZE_AUTO, mWizardParent, lChartsInfo);
                                                                break;

        case GexAbstractChart::chartTypeRChart				:	m_pAbstractChart = new GexRChart(GEX_CHARTSIZE_AUTO, mWizardParent, lChartsInfo);
                                                                break;

        case GexAbstractChart::chartTypeBoxWhisker          :	m_pAbstractChart = new GexBoxWhiskerChart(GEX_CHARTSIZE_AUTO, mWizardParent, lChartsInfo);
                                                                break;

        default												:	GSLOG(SYSLOG_SEV_ERROR, QString("Unknown chart type : %1").arg( eChartType).toLatin1().constData());
                                                                break;
    }

    m_pAbstractChart->setViewportMode(nViewportMode);
    m_pAbstractChart->setKeepViewport(bKeepViewport);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setMouseMode(MouseMode mouseMode)
//
// Description	:	Set the mouse mode
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::setMouseMode(MouseMode mouseMode)
{
    m_eMouseMode = mouseMode;

    switch(m_eMouseMode)
    {
        case mouseSelect	:	setCursor(Qt::ArrowCursor);
                                break;

        case mouseDrag		:	setCursor(Qt::OpenHandCursor);
                                break;

        case mouseZoom		:	setCursor(m_zoomCursor);
                                break;

        case mouseZoneSelect    :       setCursor(Qt::CrossCursor);
                                break;

        default				:	break;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setViewportMode(int nViewportMode)
//
// Description	:	sets the viewport mode
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::setViewportMode(int nViewportMode)
{
    if (m_pAbstractChart)
        m_pAbstractChart->setViewportMode(nViewportMode);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	int zoomDirection() const
//
// Description	:	Returns the zoom mode
//
///////////////////////////////////////////////////////////////////////////////////
int GexSingleChartWidget::zoomDirection() const
{
    int	nZoomDirection = Chart::DirectionHorizontalVertical;

    if (abstractChart())
    {
        switch(abstractChart()->type())
        {
            case GexAbstractChart::chartTypeHistogram	:	nZoomDirection = Chart::DirectionHorizontal;
                                                            break;

            case GexAbstractChart::chartTypeBoxPlot		:	if (ReportOptions.GetOption("adv_boxplot_ex","orientation").toString()=="vertical" )	//(ReportOptions.bBoxPlotExVertical)
                                                                nZoomDirection = Chart::DirectionVertical;
                                                            else
                                                                nZoomDirection = Chart::DirectionHorizontal;
                                                            break;

            default										:	break;
        }
    }

    return nZoomDirection;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	int scrollDirection() const
//
// Description	:	Returns the scroll mode
//
///////////////////////////////////////////////////////////////////////////////////
int GexSingleChartWidget::scrollDirection() const
{
    int	nScrollDirection = Chart::DirectionHorizontalVertical;

    if (abstractChart())
    {
        switch(abstractChart()->type())
        {
            case GexAbstractChart::chartTypeHistogram	:	nScrollDirection = Chart::DirectionHorizontal;
                                                            break;

            case GexAbstractChart::chartTypeBoxPlot		:	if (ReportOptions.GetOption("adv_boxplot_ex","orientation").toString()=="vertical")	//bBoxPlotExVertical)
                                                                nScrollDirection = Chart::DirectionVertical;
                                                            else
                                                                nScrollDirection = Chart::DirectionHorizontal;
                                                            break;

            default										:	break;
        }
    }

    return nScrollDirection;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void replotChart()
//
// Description	:	compute data and repaint chart
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::replotChart( CReportOptions* lOption,  CTest* lTestCellX, CTest* lTestCellY)
{
    if (m_pAbstractChart)
    {
        m_pAbstractChart->computeData(lOption , lTestCellX, lTestCellY);

        if (m_bAutoRepaint)
            update();
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void drawOutLine()
//
// Description	:	draw outline when dragging
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::drawOutLine()
{
    if (m_bOutline)
    {
        QRect rcGeometry = QRect(m_ptFirst, m_ptLast).normalized();

        if (m_pRubberBand == NULL && rcGeometry.height() > 2 && rcGeometry.width() > 2)
        {
            m_pRubberBand = new QRubberBand(QRubberBand::Rectangle, this);
            m_pRubberBand->show();
        }

        switch (zoomDirection())
        {
            case Chart::DirectionHorizontal	:	rcGeometry.setTop(m_pAbstractChart->top());
                                                rcGeometry.setBottom(m_pAbstractChart->bottom());
                                                break;

            case Chart::DirectionVertical	:	rcGeometry.setLeft(m_pAbstractChart->left());
                                                rcGeometry.setRight(m_pAbstractChart->right());
                                                break;

            default							:	break;
        }

        if (m_pRubberBand)
            m_pRubberBand->setGeometry(rcGeometry);
        if(m_eMouseMode == mouseZoneSelect)
            update();
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void resetViewPortManager(bool bEraseCustomViewport)
//
// Description	:	reset the view port manager data
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::resetViewPortManager(bool bEraseCustomViewport)
{
    if (m_pAbstractChart)
        m_pAbstractChart->resetViewportManager(bEraseCustomViewport);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void mousePressEvent(QMouseEvent *e)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::mousePressEvent(QMouseEvent *e)
{
    m_eMouseStatus = GexSingleChartWidget::pressed;
    if (m_bTracking == false)
    {
        m_ptFirst = e->pos();
        m_ptLast  = e->pos();

        // Compute values for HotSpot and Cursor
        m_pAbstractChart->computeCursorValue(m_ptLast);

        if (m_eMouseMode == mouseDrag)
            m_pAbstractChart->startDrag();

        // enable outline for zoom mode
        setOutline((bool)(m_eMouseMode == mouseZoom || m_eMouseMode == mouseZoneSelect));

        if (m_eMouseMode == mouseSelect && e->button() == Qt::RightButton)
            contextualMenu();
        else
            // Enable tracking mode
            m_bTracking = true;

        if(m_eMouseMode == mouseZoneSelect && abstractChart() && abstractChart()->type() == GexAbstractChart::chartTypeScatter /*&& e->modifiers() == Qt::ControlModifier*/){
                if(m_pRubberBand)
                    delete m_pRubberBand;
                m_pRubberBand = new QRubberBand(QRubberBand::Rectangle, this);
                m_pRubberBand->setGeometry(QRect());
                //m_pRubberBand->hide();

                update( QRegion (m_pAbstractChart->leftMargin(), m_pAbstractChart->topMargin(),
                                 m_pAbstractChart->width() - m_pAbstractChart->horizontalMargin(),
                                 m_pAbstractChart->height() - m_pAbstractChart->verticalMargin()));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void mouseReleaseEvent(QMouseEvent *e)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////

void GexSingleChartWidget::RemoveZoneFromScatter(){

    CustomDialogBox oCustomBox(this, "Remove data set by selection");
    QLabel *poLabel = new QLabel("Is this the selection you want to remove?\n Warning : All the other windows will be closed !" ,&oCustomBox);
    oCustomBox.addWidget(poLabel);
    QCheckBox *poCheck = new QCheckBox("Remove all matching parts",&oCustomBox);
    oCustomBox.addWidget(poCheck);

    oCustomBox.addButtonBox("Yes, continue", QDialogButtonBox::YesRole);
    oCustomBox.addButtonBox("No, redo selection", QDialogButtonBox::NoRole);
    oCustomBox.addButtonBox("Cancel", QDialogButtonBox::RejectRole);
    oCustomBox.exec();


    if(oCustomBox.clickedButton() && oCustomBox.buttonRole(oCustomBox.clickedButton()) == QDialogButtonBox::NoRole){
        //Redo the selection
        if (m_pRubberBand){
            // Hide and delete rubberband
            delete m_pRubberBand;
            m_pRubberBand = NULL;
        }

        if(m_pRubberBandScaterList.count())
            m_pRubberBandScaterList.clear();
        m_bTracking = false;
        setMouseTracking(false);
        setOutline(false);

        if(m_eMouseMode == mouseZoneSelect && abstractChart() && abstractChart()->type() == GexAbstractChart::chartTypeScatter){
            if(m_pRubberBand)
                delete m_pRubberBand;
            m_pRubberBand = new QRubberBand(QRubberBand::Rectangle, this);
            m_pRubberBand->setGeometry(QRect());
        }
        update();
        return;
    }

    setMouseTracking(false);
    if(oCustomBox.clickedButton() && oCustomBox.buttonRole(oCustomBox.clickedButton())== QDialogButtonBox::YesRole){
        //execute the split
        mWizardParent->comboBoxChartType->setDisabled(true);
        ApplyRemove(poCheck->isChecked());
        mWizardParent->comboBoxChartType->setDisabled(false);
    }

    //cancel the split  Or Restore the widget
    if (m_pRubberBand){
        // Hide and delete rubberband
        m_pRubberBand->hide();
        delete m_pRubberBand;
        m_pRubberBand = NULL;
    }
    if(m_pRubberBandScaterList.count())
        m_pRubberBandScaterList.clear();
    // Disable outline
    setOutline(false);

    // stop mouse tracking
    m_bTracking = false;
    m_eMouseMode = mouseSelect;
    setMouseTracking(true);
    mWizardParent->resetButtonStatus();
    setCursor(Qt::ArrowCursor);

}

void GexSingleChartWidget::ApplyRemove(bool bAllMatchingPart){

    QList <QRectF> oAreasToRemove;
    for(int iIdx=0; iIdx<m_pRubberBandScaterList.count();iIdx++){
        m_pAbstractChart->computeCursorValue(m_pRubberBandScaterList[iIdx].topLeft());
        double dFirstX = m_pAbstractChart->xCursorValue();
        double dFirstY = m_pAbstractChart->yCursorValue();
        m_pAbstractChart->computeCursorValue(m_pRubberBandScaterList[iIdx].bottomRight());
        double dLastX = m_pAbstractChart->xCursorValue();
        double dLastY = m_pAbstractChart->yCursorValue();
        oAreasToRemove.append(QRectF(QPointF(dFirstX,dFirstY),QPointF(dLastX,dLastY)));

    }

    m_pAbstractChart->computeCursorValue(m_ptFirst);
    m_pAbstractChart->computeCursorValue(m_ptLast);

    mWizardParent->onRemoveDatapointScatter(oAreasToRemove,bAllMatchingPart);
}

void GexSingleChartWidget::OnDeselectHighlightedDie()
{
    emit deselectHighlightedDie();
}


void GexSingleChartWidget::processZoneSelection()
{
    // Do not inherit from the parent (button border:none stylesheet)
    // QMessageBox oMsgBox(this);
    QMessageBox oMsgBox(0);
    oMsgBox.setWindowTitle("Split data set by selection");
    oMsgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    oMsgBox.setText("Is this the selection you want?\n Warning : All the other windows will be closed !");
    oMsgBox.addButton("yes, continue", QMessageBox::YesRole);
    oMsgBox.addButton("no, redo selection", QMessageBox::NoRole);
    oMsgBox.addButton("cancel split", QMessageBox::RejectRole);
    oMsgBox.setIcon(QMessageBox::Warning);
    oMsgBox.exec();

    if(oMsgBox.clickedButton() && oMsgBox.buttonRole(oMsgBox.clickedButton()) == QMessageBox::NoRole){
        //Redo the selection
        if (m_pRubberBand){
            // Hide and delete rubberband
            m_bTracking = false;
            delete m_pRubberBand;
            m_pRubberBand = NULL;
            update();
            return;
        }
    }

    setMouseTracking(false);
    if(oMsgBox.clickedButton() && oMsgBox.buttonRole(oMsgBox.clickedButton())== QMessageBox::YesRole){
        //execute the split
        mWizardParent->getWizardHandler()->RemoveAllWizardSExcept(mWizardParent);
         mWizardParent->comboBoxChartType->setDisabled(true);
        onSplitDatasetZone();
       // mWizardParent->comboBoxChartType->setDisabled(false);

    }

    //cancel the split  Or Restor the widget
    if (m_pRubberBand){
        // Hide and delete rubberband
        m_pRubberBand->hide();
        delete m_pRubberBand;
        m_pRubberBand = NULL;
    }
    // Disable outline
    //setOutline(false);

    // stop mouse tracking
    //m_bTracking = false;
    //m_eMouseMode = mouseSelect;
    //setMouseTracking(true);
    //mWizardParent->resetButtonStatus();
    //setCursor(Qt::ArrowCursor);
}

void GexSingleChartWidget::mouseReleaseEvent(QMouseEvent *e)
{
    m_eMouseStatus = GexSingleChartWidget::released;
    m_ptLast  = e->pos();

    // Compute values for HotSpot and Cursor
    m_pAbstractChart->computeCursorValue(m_ptLast);

    if (m_bTracking)
    {
        bool bUpdateViewport = false;

        if (m_eMouseMode == mouseDrag)
            bUpdateViewport = m_pAbstractChart->dragTo(scrollDirection(), m_ptLast.x() - m_ptFirst.x(), m_ptLast.y() - m_ptFirst.y());
        else if (m_eMouseMode == mouseZoom)
        {
            // Delete rubberband if any and apply zoom
            if (m_pRubberBand)
            {
                if (m_pAbstractChart->canZoomIn(zoomDirection()))
                    bUpdateViewport = m_pAbstractChart->zoomTo(zoomDirection(), m_pRubberBand->geometry().topLeft().x(), m_pRubberBand->geometry().topLeft().y(),
                                                                    m_pRubberBand->geometry().bottomRight().x(), m_pRubberBand->geometry().bottomRight().y());

                // Hide and delete rubberband
                m_pRubberBand->hide();
                delete m_pRubberBand;
                m_pRubberBand = NULL;
            }
            else
            {
                // Left button means zoom in
                if (e->button() == Qt::LeftButton && m_pAbstractChart->canZoomIn(zoomDirection()))
                    bUpdateViewport = m_pAbstractChart->zoomAt(zoomDirection(), e->pos().x(), e->pos().y(), 2.0);
                // Right button means zoom out
                else if(e->button() == Qt::RightButton && m_pAbstractChart->canZoomOut(zoomDirection()))
                    bUpdateViewport = m_pAbstractChart->zoomAt(zoomDirection(), e->pos().x(), e->pos().y(), 0.5);
            }

            // Redraw chart if need be
            if (bUpdateViewport)
            {
                repaint();

                m_pAbstractChart->chartOverlays()->getViewportRectangle()[m_pAbstractChart->type()].mChangeViewPort = true;

                emit viewportChanged(this);
            }
        }else if(m_eMouseMode == mouseZoneSelect){

            if(abstractChart()->type() == GexAbstractChart::chartTypeScatter){
                QRect oNewSelectedZone = m_pRubberBand->geometry();
                QRegion oWholeRegion(m_pAbstractChart->leftMargin(), m_pAbstractChart->topMargin(),
                                     m_pAbstractChart->width() - m_pAbstractChart->horizontalMargin(), m_pAbstractChart->height() - m_pAbstractChart->verticalMargin());
                if(oNewSelectedZone.top() < oWholeRegion.boundingRect().top()){
                    oNewSelectedZone.setTop(oWholeRegion.boundingRect().top());
                }
                if(oNewSelectedZone.bottom() > oWholeRegion.boundingRect().bottom()){
                    oNewSelectedZone.setBottom(oWholeRegion.boundingRect().bottom());
                }
                if(oNewSelectedZone.left() < oWholeRegion.boundingRect().left()){
                    oNewSelectedZone.setLeft(oWholeRegion.boundingRect().left());
                }
                if(oNewSelectedZone.right() > oWholeRegion.boundingRect().right()){
                    oNewSelectedZone.setRight(oWholeRegion.boundingRect().right());
                }

                m_pRubberBandScaterList.append(oNewSelectedZone);
                if(e->modifiers() == Qt::ControlModifier){
                    //Selection not finished
                    delete m_pRubberBand;
                    m_pRubberBand = 0;
                    // Disable outline
                    setOutline(false);
                    // stop mouse tracking
                    m_bTracking = false;
                    return;

                }else{
                    delete m_pRubberBand;
                    m_pRubberBand = 0;
                    //Selection finished
                    //apply the Remove
                    RemoveZoneFromScatter();
                    return ;
                }

            }else{
                processZoneSelection();
            }
            return ;
        }

        // Disable outline
        setOutline(false);

        // stop mouse tracking
        m_bTracking = false;

        // Trend Chart mode (ctrl key + left or right click)
        // If wafermap available &, hitting CTRL key then the Interactive Wafermap window is launched with given pointed die Sample highlighted
        if (e->modifiers() == Qt::ControlModifier && abstractChart()->type() == GexAbstractChart::chartTypeTrend)
            selectCurrentDieLocation();
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void mouseMoveEvent(QMouseEvent *e)
//
// Description	:	Set the type of the chart
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::mouseMoveEvent(QMouseEvent *e)
{
    m_ptLast  = e->pos();

    // Compute values for HotSpot and Cursor
    m_pAbstractChart->computeCursorValue(m_ptLast);

    if (m_bTracking)
    {
        if (m_eMouseMode == mouseDrag)
        {
            m_pAbstractChart->dragTo(scrollDirection(), m_ptLast.x() - m_ptFirst.x(), m_ptLast.y() - m_ptFirst.y());

            repaint();

            emit viewportChanged(this);
        }
        else
            // Draw rubber band if need be
            drawOutLine();
    }
    else
        setToolTip(m_pAbstractChart->makeTooltip());
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void paintEvent(QPaintEvent *e)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::paintEvent(QPaintEvent* /*e*/)
{
    if (m_pAbstractChart)
    {
        QImage		cImage;
        MemBlock	memoryBlock;

        // Build chart
        if (isDrawAuthorized())
            m_pAbstractChart->buildChart(width(), height());

        // Convert ChartDirector image to Bitmap
        m_pAbstractChart->drawChart(memoryBlock);
        cImage = QImage::fromData((unsigned char*) memoryBlock.data, memoryBlock.len,"BMP");

        QPoint		pPoint(0,0);
        QPainter	painterObject(this);
        painterObject.setPen(Qt::NoPen);

        if(m_eMouseMode == mouseZoneSelect && ((m_pRubberBand && (m_pRubberBand->geometry().isNull() || (m_pRubberBand->height()>2 && m_pRubberBand->width()>2)))|| m_pRubberBandScaterList.count()) ){

            QImage oImageDup =  cImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            QImage oWholeImage = QImage(width(), height(), QImage::Format_ARGB32_Premultiplied);
            QImage oPlotImage = QImage(width(), height(), QImage::Format_ARGB32_Premultiplied);
            oPlotImage.fill(0);

            QPainter oImagePainter (&oPlotImage);
            oImagePainter.setPen(Qt::NoPen);
            oImagePainter.setCompositionMode(QPainter::CompositionMode_DestinationOver);

            oImagePainter.setPen(Qt::NoPen);
            oImagePainter.setRenderHint(QPainter::SmoothPixmapTransform);
            oImagePainter.drawImage(pPoint, oImageDup/* The FILE*/);
            memcpy(oWholeImage.bits(), oPlotImage.bits(), oWholeImage.byteCount());

            //Paint the mask

            QPainter oMaskPainter(&oWholeImage);
            oMaskPainter.setPen(Qt::NoPen);
            oMaskPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            oMaskPainter.setRenderHint(QPainter::Antialiasing);
            QColor oColor(Qt::lightGray) ;
            oColor.setAlpha(189);
            oMaskPainter.setBrush(oColor);


            QRegion oWholeRegion(m_pAbstractChart->leftMargin(), m_pAbstractChart->topMargin(),
                                 m_pAbstractChart->width() - m_pAbstractChart->horizontalMargin(), m_pAbstractChart->height() - m_pAbstractChart->verticalMargin());

            QList<QRect> oRectList;
            if(m_pRubberBandScaterList.count()){
                oRectList = m_pRubberBandScaterList;
            }
            if(m_pRubberBand){
                QRect oRect (m_pRubberBand->geometry());
                if(oRect.top() < oWholeRegion.boundingRect().top()){
                    oRect.setTop(oWholeRegion.boundingRect().top());
                }
                if(oRect.bottom() > oWholeRegion.boundingRect().bottom()){
                    oRect.setBottom(oWholeRegion.boundingRect().bottom());
                }
                if(oRect.left() < oWholeRegion.boundingRect().left()){
                    oRect.setLeft(oWholeRegion.boundingRect().left());
                }
                if(oRect.right() > oWholeRegion.boundingRect().right()){
                    oRect.setRight(oWholeRegion.boundingRect().right());
                }
                oRectList.append(oRect);
            }

            QRegion oGraysRegion = oWholeRegion;
            for(int iIdx=0; iIdx<oRectList.count();iIdx++){
               oGraysRegion = oGraysRegion.xored(oRectList[iIdx]);

            }

            QVector<QRect> oVectorGraysRegion = oGraysRegion.rects();
            oMaskPainter.drawRects(oVectorGraysRegion);
            painterObject.drawImage(pPoint, oWholeImage);
        }else
            painterObject.drawImage(pPoint, cImage);// Draw image

    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void selectCurrentDieLocation()
//
// Description	:	Wafermap drill-down: show on wafermap current trend point selected
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::selectCurrentDieLocation()
{
    GexTrendChart * pTrendChart = dynamic_cast<GexTrendChart *> (m_pAbstractChart);

    if (pTrendChart && pTrendChart->isValidDie())
    {
        // Get which result occurance is pointed
        CTest *				ptTest = NULL;
        CGexSingleChart	*	pChart = NULL;

        if (mWizardParent->getChartInfo()->chartsList().isEmpty() == false)
            pChart = mWizardParent->getChartInfo()->chartsList().first();

        if(pChart == NULL || pTrendChart->markerFile() == NULL || pTrendChart->markerGroup() == NULL)
            return;

        if(pTrendChart->markerFile()->FindTestCell(pChart->iTestNumberX, pChart->iPinMapX, &ptTest, true, false) != 1)
            return;

        // Launch 3D window...unless already detached (not a child window hidden)
        pGexMainWindow->ShowWizardDialog(GEX_DRILL_3D_WIZARD_P1);

        Gex::DrillDataMining3D* lWafer3DWizard = pGexMainWindow->LastCreatedWizardWafer3D();

        // Make it display the wafermap
        lWafer3DWizard->ShowChart(pTrendChart->markerGroupID(), pTrendChart->markerFile()->lFileID, ptTest->lTestNumber, ptTest->lPinmapIndex, GEX_DRILL_WAFERPROBE);

        // Highlight relevant die
        lWafer3DWizard->SelectDieLocation(pTrendChart->markerFile(), pTrendChart->xDie(), pTrendChart->yDie(), true);

        // redraw the 3D viewer
        lWafer3DWizard->refreshViewer();

        // Detach this Window so we can see side-by-side the Trend window & the Wafer one.
            //lWafer3DWizard->ForceAttachWindow(false);

        emit currentDieSelected();
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void contextualMenu()
//
// Description	:	Show the contextual menu
//
///////////////////////////////////////////////////////////////////////////////////
void GexSingleChartWidget::contextualMenu()
{
    if (abstractChart())
    {
        if (m_pChartMenuInfo)
        {
            delete m_pChartMenuInfo;
            m_pChartMenuInfo = NULL;
        }

        // Variables used when removing N*Sigma or IQR on each layer.
        CGexSingleChart *	pChart;
        CGexGroupOfFiles *	ptGroup = NULL;

        // Get X units
        double	lfCustomScaleFactor;

        QString strUnitsX = m_pAbstractChart->referenceTestX()->GetScaledUnits(&lfCustomScaleFactor,
                            ReportOptions.GetOption("dataprocessing","scaling").toString());
        strUnitsX.truncate(10);


        // Enable/disable some features...
        bool bFullMenu = true;
        switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
        {
            case GS::LPPlugin::LicenseProvider::eLtxcOEM:		// OEM-Examinator for LTXC
                bFullMenu = false;
                break;

            case GS::LPPlugin::LicenseProvider::eSzOEM:			// OEM-Examinator for Credence SZ
                return;
            default:
            break;
        }

        QIcon icSave;
        QIcon icCopy;
        QIcon icRemove;
        QIcon icWafermap;
        QIcon icOutlier;
        QIcon icReport;
        QIcon icReportWeb;
        QIcon icReportWord;
        QIcon icReportPPT;
        QIcon icReportPDF;
        QIcon icReportExcel;

        icSave.addPixmap(*pixSave);
        icCopy.addPixmap(*pixCopy);
        icRemove.addPixmap(*pixRemove);
        icWafermap.addPixmap(*pixWafermap);
        icOutlier.addPixmap(*pixOutlier);
        icReport.addPixmap(*pixTestStatistics);
        icReportWeb.addPixmap(*pixOutput);
        icReportWord.addPixmap(*pixWord);
        icReportPPT.addPixmap(*pixPowerpoint);
        icReportPDF.addPixmap(*pixPdf);
        icReportExcel.addPixmap(*pixCSVSpreadSheet);

        // Show contextual menu to allow Chart capture into the clipboard.
        // Popup menus (if mouse right-click)
        m_pChartMenuInfo = new GexChartMenuInfo();

        GexAbstractChart::chartType ltypeChart= abstractChart()->type();
        if (!GS::LPPlugin::ProductInfo::getInstance()->isExaminator()
            && bFullMenu
            && (ltypeChart == GexAbstractChart::chartTypeHistogram ||
                ltypeChart == GexAbstractChart::chartTypeTrend ||
                ltypeChart == GexAbstractChart::chartTypeProbabilityPlot ||
                ltypeChart ==  GexAbstractChart::chartTypeBoxPlot)
            )
        {
            //-- For now, the add in the OFR report is forbiden for the multichart
            if(qobject_cast<GexMultiChartWidget*>(parent()) == 0)
            {
                if(GS::Gex::OFR_Controller::GetInstance()->IsReportEmpty())
                {
                    m_pChartMenuInfo->menu()->addAction(QIcon(":/gex/icons/ofr_icon.png"),
                                                        "Start a report builder session",
                                                        this,
                                                        SLOT(OnAddNewOFRSection()));
                }
                else
                {

                    GS::Gex::OFR_Controller* lOFRInstance = GS::Gex::OFR_Controller::GetInstance();

                    QAction* lLastUpdateAction = 0;
                    QAction* lNewSection = 0;
                    QList<QAction*> actionSections;
                    QMenu* lMenuSection =lOFRInstance->BuildContextualMenu(m_pChartMenuInfo->menu(),
                                                                           &lLastUpdateAction,
                                                                           &lNewSection,
                                                                           actionSections);
                    if(lMenuSection)
                    {
                        lMenuSection->setIcon(QIcon(":/gex/icons/ofr_icon.png"));
                        if(lLastUpdateAction)
                        {
                            connect(lLastUpdateAction, SIGNAL(triggered(bool)), this, SLOT(OnAddToLastUpdatedOFRSection()));
                            lMenuSection->addAction(lLastUpdateAction);
                        }

                        if(lNewSection)
                        {
                            connect(lNewSection, SIGNAL(triggered(bool)),this,  SLOT(OnAddNewOFRSection()));
                            lMenuSection->addAction(lNewSection);
                        }
                        lMenuSection->addSeparator();
                        QList<QAction*>::iterator lIterBegin(actionSections.begin()), lIterEnd(actionSections.end());
                        for(;lIterBegin != lIterEnd; ++lIterBegin)
                        {
                            lMenuSection->addAction(*lIterBegin);
                        }
                        connect(lMenuSection, SIGNAL(triggered(QAction*)), this, SLOT(OnAddToAnExistingOFRSection(QAction*)));

                        m_pChartMenuInfo->menu()->addMenu(lMenuSection);
                    }
                }
            }
            m_pChartMenuInfo->menu()->addSeparator();
        }
        if(bFullMenu && abstractChart()->type() != GexAbstractChart::chartTypeXBar && abstractChart()->type() != GexAbstractChart::chartTypeRChart)
        {
            m_pChartMenuInfo->menu()->addAction("Edit Legends, Scales...",			this,	SLOT(onEditProperties()));
            m_pChartMenuInfo->menu()->addAction("Edit Drawing style, Colors...",	this,	SLOT(onEditStyles()));
            m_pChartMenuInfo->menu()->addAction("Show/Hide Markers...",				this,	SLOT(onEditMarkers()));
        }

        // Add entry to deselect 'selection' markers
        if (abstractChart()->type() != GexAbstractChart::chartTypeXBar && abstractChart()->type() != GexAbstractChart::chartTypeRChart)
        {
            if (gexReport->HasCustomMarkers("Selection"))
                m_pChartMenuInfo->menu()->addAction("Deselect highlighted die...",  this,	SLOT(OnDeselectHighlightedDie()));

        }

        if(bFullMenu && abstractChart()->type() == GexAbstractChart::chartTypeScatter){
            m_pChartMenuInfo->menu()->addSeparator();
            m_pChartMenuInfo->menu()->addAction("Remove data by selection...", this, SLOT(onSelectScaterArea()));
        }

        QMenu * reportbuild_submenu = NULL;
        if  (bFullMenu
             && (
                 (  abstractChart()->type() == GexAbstractChart::chartTypeHistogram)
                    || abstractChart()->type() == GexAbstractChart::chartTypeTrend
                    || abstractChart()->type() == GexAbstractChart::chartTypeProbabilityPlot // case 7062
                 )
            )
        {
            QMenu * filter_submenu		= NULL;
            QMenu * wafermap_submenu	= NULL;
            QString strText;

            // Show results in device only available if one dataset group only!
            if((abstractChart()->type() == GexAbstractChart::chartTypeTrend) && (gexReport->getGroupsList().count() == 1))
            {
                QString strString = "Show all Tests results for run#: " + QString::number((int)m_pAbstractChart->xCursorValue());
                m_pChartMenuInfo->menu()->addAction(*pixTestStatistics, strString, this, SLOT(onDeviceResults()));
            }

            m_pChartMenuInfo->menu()->addSeparator();

            // Depending of charting type, we either consider the X or Y value as cut-off lmit.
            if (abstractChart()->type() == GexAbstractChart::chartTypeHistogram)
                m_pChartMenuInfo->setOutlierLimit(m_pAbstractChart->xCursorValue() /* * ScalingPower(m_pAbstractChart->referenceTestX()->res_scal)*/);
            else if (abstractChart()->type() == GexAbstractChart::chartTypeTrend)
                m_pChartMenuInfo->setOutlierLimit(m_pAbstractChart->yCursorValue() /* * ScalingPower(m_pAbstractChart->referenceTestX()->res_scal)*/);
            else if (abstractChart()->type() == GexAbstractChart::chartTypeProbabilityPlot)
                m_pChartMenuInfo->setOutlierLimit(m_pAbstractChart->xCursorValue()); // 7062
            m_pChartMenuInfo->m_dFactor = ScalingPower(m_pAbstractChart->referenceTestX()->res_scal);
            m_pChartMenuInfo->m_dScaleFactor = m_pAbstractChart->referenceTestX()->res_scal;

            ///////////////////////////////////////////////////////////////////////////////
            //  Create 'Filter...' sub-menu
            ///////////////////////////////////////////////////////////////////////////////
            filter_submenu = m_pChartMenuInfo->menu()->addMenu(icRemove,"Remove/Filter data...") ;

            // Remove Data >= given value
            strText.sprintf("Remove data GREATER than %g %s",
                            m_pChartMenuInfo->outlierLimit(), strUnitsX.toLatin1().constData());
            filter_submenu->addAction(icRemove,strText, this, SLOT(onRemoveDatapointHigher()));

            // Remove Data <= given value
            strText.sprintf("Remove data LOWER than %g %s", m_pChartMenuInfo->outlierLimit(), strUnitsX.toLatin1().constData());
            filter_submenu->addAction(icRemove, strText, this, SLOT(onRemoveDatapointLower()));

            // Remove Data Within a given range
            filter_submenu->addAction(icRemove,"Remove data Within a Range...", this, SLOT(onRemoveDatapointRange()));

            // Sub-Menu separator
            filter_submenu->addSeparator();

            // N*Sigma filter
            filter_submenu->addAction(icOutlier,"Data cleaning: +/-N*Sigma...", this, SLOT(onRemoveNSigma()));

            // IQR filter
            filter_submenu->addAction(icOutlier,"Data cleaning: IQR (LL=Q1-N*IQR, HL=Q3+N*IQR", this, SLOT(onRemoveIQR()));

            if(pGexMainWindow && pGexMainWindow->getUndoStack() && pGexMainWindow->getUndoStack()->count())
            {
                filter_submenu->addSeparator();

                QAction *poTempAction = pGexMainWindow->getUndoStack()->createUndoAction(this);
                poTempAction->setIcon(QIcon(":/gex/icons/undo_icon.png"));
                filter_submenu->addAction(poTempAction);

                poTempAction = pGexMainWindow->getUndoStack()->createRedoAction(this);
                poTempAction->setIcon(QIcon(":/gex/icons/redo_icon.png"));
                filter_submenu->addAction(poTempAction);

                filter_submenu->addAction(QIcon(":/gex/icons/clean_undo_stack.png"),
                                          "Undo all outlier removal", this, SLOT(UndoAll()));
            }

            ///////////////////////////////////////////////////////////////////////////////
            // WAFERMAP drill-down menu....if wafermap exists (and only if one layer)!
            ///////////////////////////////////////////////////////////////////////////////
            if(m_pAbstractChart->chartOverlays()->chartsList().count() == 1)
            {
                pChart = m_pAbstractChart->chartOverlays()->chartsList().at(0);

                if(pChart != NULL)
                {
                    if ((pChart->iGroupX < 0) || (pChart->iGroupX >= gexReport->getGroupsList().size()))
                        ptGroup = NULL;
                    else
                        ptGroup = gexReport->getGroupsList().at(pChart->iGroupX);
                }

                if(ptGroup && ptGroup->cStackedWaferMapData.bWaferMapExists)
                {
                    wafermap_submenu = m_pChartMenuInfo->menu()->addMenu(icWafermap,"Wafermap drill-down...") ;

                    // Show dies for Data >= given value
                    strText.sprintf("Show dies where data GREATER than %g %s",
                                    m_pChartMenuInfo->outlierLimit(),
                                    strUnitsX.toLatin1().constData());
                    wafermap_submenu->addAction(icWafermap, strText, this, SLOT(onWafermapDatapointHigher()));

                    // Show dies for Data <= given value
                    strText.sprintf("Show dies where data LOWER than %g %s",
                                    m_pChartMenuInfo->outlierLimit(), strUnitsX.toLatin1().constData());
                    wafermap_submenu->addAction(icWafermap, strText, this, SLOT(onWafermapDatapointLower()));

                    // Show dies for Data Within a given range
                    wafermap_submenu->addAction(icWafermap,"Show dies where data Within a Range...",
                                                this, SLOT(onWafermapDatapointRange()));

                    // In trend chart mode, also display the option to highlight the current die pointed.
                    if(abstractChart()->type() == GexAbstractChart::chartTypeTrend)
                    {
                        GexTrendChart * pTrendChart = dynamic_cast<GexTrendChart *> (m_pAbstractChart);

                        if (pTrendChart && pTrendChart->isValidDie())
                        {
                            QString strString = "Ctrl+Click: Show part on wafermap at (X,Y): "
                                    + QString::number(pTrendChart->xDie()) + "," + QString::number(pTrendChart->yDie());
                            wafermap_submenu->addAction(icWafermap, strString, this, SLOT(onWafermapDie()));
                        }
                    }
                }
            }
        }

        // no split in compare mode !
        if (bFullMenu && (abstractChart()->type() != GexAbstractChart::chartTypeScatter) && (gexReport->getGroupsList().count()==1))
        {
            QString an("Split dataset at ");
            if (abstractChart()->type() == GexAbstractChart::chartTypeHistogram || abstractChart()->type() == GexAbstractChart::chartTypeProbabilityPlot)
                an += QString("%1").arg(m_pAbstractChart->xCursorValue());
            else
                an += QString("%1").arg(m_pAbstractChart->yCursorValue());
            m_pChartMenuInfo->menu()->addAction(an,	this,	SLOT(onSplitDataset()));
        }

        // Rebuild report
        if(bFullMenu)
        {
            m_pChartMenuInfo->menu()->addSeparator();
            reportbuild_submenu = m_pChartMenuInfo->menu()->addMenu(icReport,"Create report...") ;
            reportbuild_submenu->addAction(icReportWeb,"Web report", this, SLOT(onRebuildReport_Web()));
        #ifndef unix
            reportbuild_submenu->addAction(icReportWord,"Word document", this, SLOT(onRebuildReport_Word()));
            reportbuild_submenu->addAction(icReportPPT,"PowerPoint slides", this, SLOT(onRebuildReport_PPT()));
        #endif
            reportbuild_submenu->addAction(icReportPDF,"PDF document", this, SLOT(onRebuildReport_PDF()));
            reportbuild_submenu->addAction(icReportExcel,"Spreadsheet CSV", this, SLOT(onRebuildReport_Excel()));

            // Clipboard Copy, Export image...
            m_pChartMenuInfo->menu()->addSeparator();
        }

        m_pChartMenuInfo->menu()->addAction(icCopy,"Copy chart to clipboard",	this,	SLOT(onCopyChartToClipboard()));
        m_pChartMenuInfo->menu()->addAction(icSave,"Save chart to disk",		this,	SLOT(onSaveChartToDisk()));

        connect(m_pChartMenuInfo->menu(), SIGNAL(hovered(QAction*)), this, SLOT(onActionHovered(QAction*)));

//        m_pChartMenuInfo->menu()->setMouseTracking(true);
        m_pChartMenuInfo->menu()->exec(QCursor::pos());
//        m_pChartMenuInfo->menu()->setMouseTracking(false);

        // FIXME: exec return only on action or escape so if several right clic are done: several exec are triggered
        // and at return they all try to delete m_pChartMenuInfo
        // would be better to work with popup hide/show mechannism
        if (m_pChartMenuInfo)
        {
            delete m_pChartMenuInfo;
            m_pChartMenuInfo = NULL;
        }
    }
}

void    GexSingleChartWidget::keyReleaseEvent ( QKeyEvent * e ) {
    if(e->key() == Qt::Key_Control &&
       m_eMouseMode == mouseZoneSelect &&
       abstractChart()->type() == GexAbstractChart::chartTypeScatter &&
       m_eMouseStatus == GexSingleChartWidget::released ){
        if(m_pRubberBand)
            delete m_pRubberBand;
        m_pRubberBand = 0;
        //Selection finished
        //apply the Remove
        RemoveZoneFromScatter();
        return ;
    }
    QWidget::keyReleaseEvent(e);
}


void GexSingleChartWidget::keyPressEvent ( QKeyEvent * e ){

    if(pGexMainWindow && pGexMainWindow->getUndoStack()){
        if(e->matches(QKeySequence::Undo)){
            // Message::information(0, "Undo/redo","Undo Called");
            if(pGexMainWindow->getUndoStack()->canUndo()){
                pGexMainWindow->getUndoStack()->undo();
            }
        }else if(e->matches(QKeySequence::Redo)){
            // Message::information(0, "Undo/redo","Redo Called");
            if(pGexMainWindow->getUndoStack()->canRedo()){
                pGexMainWindow->getUndoStack()->redo();
            }
        }
    }
#ifdef QT_DEBUG
    if( (e->modifiers()&(Qt::ControlModifier|Qt::AltModifier)) && e->key() == Qt::Key_V){
        // Message::information(0, "Undo/redo","View undo/redo stack");
        if(pGexMainWindow && pGexMainWindow->getUndoStack()){
            QDialog oDialog;
            QGridLayout *poGridLayout = new QGridLayout(&oDialog);
            QUndoView *poUndoView = new QUndoView (&oDialog);
            poUndoView->setStack(pGexMainWindow->getUndoStack());
            poGridLayout->addWidget(poUndoView, 0, 0, 1, 1);
            oDialog.exec();
        }else
            GS::Gex::Message::information("Undo/redo",
                                          "No undo/redo stack found");
    }
#endif

    QWidget::keyPressEvent(e);
}


void GexSingleChartWidget::UndoAll(){
    if(pGexMainWindow && pGexMainWindow->getUndoStack()){
        pGexMainWindow->getUndoStack()->setIndex(0);
        pGexMainWindow->getUndoStack()->clear();
        pGexMainWindow->getUndoStack()->setClean();
        resetViewPortManager(true);
    }
}
