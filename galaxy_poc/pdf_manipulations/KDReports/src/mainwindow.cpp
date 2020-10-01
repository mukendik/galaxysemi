#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "KDReports.h"

#include <QDir>
#include <QPainter>
#include <QStandardItemModel>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // extract resource file and return resulting paths
    std::string background_jpg_path, kdab_small_jpg_path, system_png_path;
    extractResourceFiles
        ( background_jpg_path, kdab_small_jpg_path, system_png_path );

    createReport
        ( background_jpg_path, kdab_small_jpg_path, system_png_path );

    renderReport();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::extractResourceFiles
    (
        std::string &background_jpg_path,
        std::string &kdab_small_jpg_path,
        std::string &system_png_path
    )
{
    // set the input jpg file path
    background_jpg_path =
        QDir::tempPath().append( "/images_background.jpg" ).toStdString();

    // set the input jpg file path
    kdab_small_jpg_path =
        QDir::tempPath().append( "/images_kdab_small.jpg" ).toStdString();

    // set the input png file path
    system_png_path =
        QDir::tempPath().append( "/images_system.png" ).toStdString();

    // extract resources in a temporary directory
    QFile::copy
        ( ":/images/background.jpg", QString::fromStdString( background_jpg_path ) );
    QFile::copy
        (
            ":/images/kdab_small.jpg",
            QString::fromStdString( kdab_small_jpg_path )
        );
    QFile::copy
        (
            ":/images/system.png",
            QString::fromStdString( system_png_path )
        );
}

void MainWindow::createReport
    (
        const std::string &background_jpg_path,
        const std::string &kdab_small_jpg_path,
        const std::string &system_png_path
    )
{
    KDReports::Report report;

    report.setPageSize( QPrinter::A0 );

    report.setDocumentName( "ReferenceReport" );
    report.setWatermarkPixmap
        ( QPixmap{ QString::fromStdString( background_jpg_path ) } );
    report.setWatermarkText( QString{} );

    decltype( auto ) header = report.header( KDReports::FirstPage );

    QPixmap kdab_pixmap{ QString::fromStdString( kdab_small_jpg_path ) };
    QPainter painter{ &kdab_pixmap };

    painter.begin( centralWidget() );
    painter.drawRect( 0, 0, kdab_pixmap.width() - 1, kdab_pixmap.height() - 1 );

    KDReports::ImageElement imageElement{ kdab_pixmap };
    imageElement.setWidth( 500 ); // mm
    header.addElement( imageElement );
    header.addElement
        (
            KDReports::TextElement
            {
                "This header should be on the first page. The kdab logo "
                "should be 50mm wide and 24mm high."
            }
        );

    header.addElement( KDReports::TextElement{ "This should say 1/2: " } );
    header.addVariable( KDReports::PageNumber );
    header.addInlineElement( KDReports::TextElement{ "/" } );
    header.addVariable( KDReports::PageCount );

    header.addInlineElement( KDReports::TextElement( "\t\tDate: " ) );
    header.addVariable( KDReports::DefaultLocaleLongDate );

    decltype( auto ) evenPagesHeader = report.header( KDReports::EvenPages );

    //
    fillEvenPagesHeader( evenPagesHeader );

    decltype( auto ) footer = report.footer();
    footer.addElement
        (
            KDReports::TextElement{ "This should be right-aligned" },
            Qt::AlignRight
        );

    KDReports::TextElement titleElement;
    titleElement <<
        "This text is blue, bold, centered and has a point size of 14";
    titleElement.setBold( true );
    titleElement.setPointSize( 14 );
    titleElement.setTextColor( Qt::blue );
    report.addElement( titleElement, Qt::AlignHCenter );

    report.addVerticalSpacing( 15 );
    report.addElement
        (
            KDReports::TextElement
            { "There should be 15mm above this paragraph" }
        );

    report.addElement
        (
            KDReports::TextElement
            { "There should be an image as watermark behind this text" }
        );
    report.addElement( KDReports::HLineElement() );
    report.addElement
        (
            KDReports::TextElement
            { "There should be a horizontal line above this paragraph" }
        );

    {
        QList<QTextOption::Tab> tabs;
        tabs.append( report.middleAlignedTab() );
        tabs.append( report.rightAlignedTab() );
        report.setTabPositions( tabs );
    }

    report.addElement
        (
            KDReports::TextElement
            { "Left-aligned text\tMiddle\tRight-aligned text" }
        );

    report.setParagraphMargins( 30, 10, 50, 10 );
    report.addElement
        (
            KDReports::TextElement
            {
                "This paragraph has margins on all sides; especially visible "
                "when its text is long enough so that it wraps onto multiple "
                "lines."
            }
        );
    report.setParagraphMargins( 0, 0, 0, 0 );

    report.addPageBreak();

    // Define tab positions for decimal-point alignment
    {
        QList<QTextOption::Tab> tabs;
        QTextOption::Tab tab;
        tab.position = 50; // in mm
        tab.type = QTextOption::DelimiterTab;
        tab.delimiter = QLatin1Char( '.' );
        tabs.append( tab );
        report.setTabPositions( tabs );
    }

    // table
    KDReports::TableElement tableElement;
    tableElement.setHeaderRowCount( 2 );
    tableElement.setPadding( 3 );
    tableElement.setWidth( 1000.0, KDReports::Percent );
    QColor headerColor( "#DADADA" );
    // Merged header in row 0
    KDReports::Cell& topHeader = tableElement.cell( 0, 0 );
    topHeader.setColumnSpan( 2 );
    topHeader.setBackground( headerColor );
    topHeader.addElement
        (
            KDReports::TextElement
            { "This header should be gray and span over two columns" },
            Qt::AlignHCenter
        );

    // Normal header in row 1
    decltype( auto ) headerCell1 = tableElement.cell( 1, 0 );
    headerCell1.setBackground( headerColor );
    QPixmap systemPixmap{ QString::fromStdString( system_png_path ) };
    headerCell1.addElement( KDReports::ImageElement{ systemPixmap } );
    headerCell1.addInlineElement
        ( KDReports::TextElement{ " Header with computer icon" } );

    decltype( auto ) headerCell2 = tableElement.cell( 1, 1 );
    headerCell2.setBackground( headerColor );
    KDReports::TextElement expected( "Italic with dark gray background" );
    expected.setItalic( true );

    // note that this background only applies to this element
    expected.setBackground( QColor{ "#999999" } );
    headerCell2.addElement( expected );
    headerCell2.addInlineElement( KDReports::TextElement{ " Normal text" } );

    // Data in rows 2 and 3
    tableElement.cell( 2, 0 ).addElement
        ( KDReports::TextElement{ "Price:\t250.2" } );
    tableElement.cell( 2, 1 ).addElement
        ( KDReports::TextElement{ "Price:\t1088.5" } );
    tableElement.cell( 3, 0 ).addElement
        ( KDReports::TextElement{ "Reduced price:\t68.52" } );
    tableElement.cell( 3, 1 ).addElement
        ( KDReports::TextElement{ "Reduced price:\t88.584" } );

    report.addElement( tableElement );

    report.addVerticalSpacing( 5 );

    // Auto table test

    QStandardItemModel model;
    auto firstHeaderItem =
        new QStandardItem
        {
            QObject::tr
            ( "<html>This auto-table uses <b>QStandardItemModel</b>" )
        };
    firstHeaderItem->setTextAlignment( Qt::AlignCenter );
    model.setHorizontalHeaderItem( 0, firstHeaderItem );

    auto secondHeaderItem =
        new QStandardItem{ QObject::tr( "Icon on the left in this header" ) };
    secondHeaderItem->setData
        (
            qVariantFromValue
                ( QPixmap{ QString::fromStdString( system_png_path ) } ),
            Qt::DecorationRole
        );

    model.setHorizontalHeaderItem( 1, secondHeaderItem );
    auto thirdHeaderItem =
        new QStandardItem{ QObject::tr( "Small pixmap on the right" ) };

    // size determined by setIconSize below
    thirdHeaderItem->setIcon
        ( QPixmap{ QString::fromStdString( system_png_path ) } );

    thirdHeaderItem->setData
        (
            Qt::AlignRight,
            KDReports::AutoTableElement::DecorationAlignmentRole
        );

    model.setHorizontalHeaderItem( 2, thirdHeaderItem );

    auto firstCellItem =
        new QStandardItem{ QObject::tr( "<html>This is <b>bold</b> text" ) };
    model.setItem( 0, 0, firstCellItem );

    auto secondCellItem =
        new QStandardItem{ QObject::tr( "Icon on the left in this cell" ) };
    secondCellItem->setData
        (
            qVariantFromValue
                ( QPixmap{ QString::fromStdString( system_png_path ) } ),
            Qt::DecorationRole
        );
    model.setItem( 0, 1, secondCellItem );

    auto thirdCellItem =
        new QStandardItem{ QObject::tr( "Small pixmap on the right" ) };

    // size determined by setIconSize below
    thirdCellItem->setIcon
        ( QPixmap{ QString::fromStdString( system_png_path ) } );

    thirdCellItem->setData
        (
            Qt::AlignRight,
            KDReports::AutoTableElement::DecorationAlignmentRole
        );

    model.setItem( 0, 2, thirdCellItem );

    auto italicItem =
        new QStandardItem( QObject::tr( "10 pt italic blue text" ) );

    italicItem->setFont( QFont{ "Arial", 10, -1, true /*italic*/ } );
    italicItem->setForeground( Qt::blue );
    model.setItem( 1, 0, italicItem );

    auto yellowItem =
        new QStandardItem{ QObject::tr( "Yellow background" ) };
    yellowItem->setBackground( Qt::yellow );
    model.setItem( 1, 1, yellowItem );

    auto alignedItem = new QStandardItem{ QObject::tr( "Right-aligned" ) };
    alignedItem->setTextAlignment( Qt::AlignRight );
    model.setItem( 1, 2, alignedItem );

    KDReports::AutoTableElement autoTable( &model );
    autoTable.setIconSize( { 16, 16 } );
    autoTable.setHeaderBackground( {} );
    autoTable.setVerticalHeaderVisible( false );
    report.addElement( autoTable );

    report.addElement( KDReports::TextElement{} );

    // Hyperlink test
    report.addElement
        (
            KDReports::HtmlElement
            {
                "<a href=\"http://www.kdab.com\">click here to open "
                "http://www.kdab.com</a>"
            }
        );

    auto output_directory = QDir::homePath();

    auto pdf_path = output_directory;
    pdf_path.append( "/output.pdf" );

    auto htm_path = output_directory;
    htm_path.append( "/output.htm" );

    auto png_path = output_directory;
    png_path.append( "/output.png" );

    report.exportToFile( pdf_path );

    report.exportToHtml( htm_path );

    report.exportToImage
        (
            { 1024, 768 },
            png_path,
            "png"
        );
}

void MainWindow::fillEvenPagesHeader( KDReports::Header& evenPagesHeader )
{
    evenPagesHeader.addElement
        (
            KDReports::TextElement
            {
                "This header should be on even pages, and contain a 2x2 table "
                "with blue borders"
            }
        );

    KDReports::TableElement table;
    table.setBorder( 1 );
    table.setBorderBrush( Qt::blue );
    table.setHeaderRowCount( 0 );
    table.cell( 0, 0 ).addInlineElement( KDReports::TextElement( "1" ) );
    table.cell( 0, 1 ).addInlineElement( KDReports::TextElement( "2" ) );

    decltype( auto ) cell = table.cell( 1, 0 );
    cell.addElement( KDReports::TextElement{ "This should say 2/2: " } );
    cell.addVariable( KDReports::PageNumber );
    cell.addInlineElement( KDReports::TextElement{ "/" } );
    cell.addVariable( KDReports::PageCount );
    evenPagesHeader.addInlineElement( table );
}

void MainWindow::renderReport()
{
    auto image_label = new QLabel{};
    setCentralWidget( image_label );

    auto output_directory = QDir::homePath();

    auto png_path = output_directory;
    png_path.append( "/output.png" );

    QPixmap image{ png_path };

    image_label->setPixmap( image );
    adjustSize();
}
