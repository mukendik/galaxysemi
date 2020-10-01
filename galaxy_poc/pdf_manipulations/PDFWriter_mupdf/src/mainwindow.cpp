#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "PDFWriter/PDFWriter.h"
#include "PDFWriter/PDFPage.h"
#include "PDFWriter/PDFFormXObject.h"
#include "PDFWriter/PageContentContext.h"
#include "PDFWriter/PDFUsedFont.h"
#include "result_array.h"

#include <QDir>
#include<QLabel>

// mupdf is a 'C' library
#ifdef __cplusplus
extern "C" {
#endif

#include "mupdf/fitz.h"

#ifdef __cplusplus
}
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // a directory containing some stuff
    auto output_path = QDir::homePath().toStdString();

    // input/output pdf file path
    auto pdf_path = output_path;
    pdf_path.append( "/output.pdf" );

    // extract resource file and return resulting paths
    std::string input_jpg_path, input_font_path;
    extractResourceFiles( input_jpg_path, input_font_path );

    createDocument( pdf_path, input_jpg_path, input_font_path );

    // render the resulting pdf document with mupdf
    // output png file path
    std::string output_png_path{ output_path };
    output_png_path.append( "/output.png" );

    renderDocument( pdf_path, output_png_path );

    showRenderedInWidget( output_png_path );
}

void MainWindow::extractResourceFiles
    ( std::string &input_jpg_path, std::string &input_font_path )
{
    // set the input jpg file path
    input_jpg_path =
        QDir::tempPath().append( "/images_ski_slope.jpg" ).toStdString();

    // set the input font file path
    input_font_path =
        QDir::tempPath().append( "/fonts_Arial.ttf" ).toStdString();

    // extract resources in a temporary directory
    QFile::copy
        ( ":/images/ski_slope.jpg", QString::fromStdString( input_jpg_path ) );
    QFile::copy
        ( ":/fonts/Arial.ttf", QString::fromStdString( input_font_path ) );
}

void MainWindow::createDocument
    (
        const std::string &output_pdf_path,
        const std::string &input_jpg_path,
        const std::string &input_font_path
    )
{
    // main object repsonsible to change the state of many thing in PDF file
    // manipulations
    PDFWriter writer;

    // start a page here
    // indicates where to save the document, the path has to be valid
    writer.StartPDF( output_pdf_path, ePDFVersion13 );

    // a page inside the document
    PDFPage page;

    // create a box to place a media stuff within (A4 portrait dimension)
    page.SetMediaBox( { 0, 0, 595, 842 } );

    // start a context for a page to modify it
    auto context =
        writer.StartPageContentContext( &page );

    // adding an image in the page
    // the real object from the jpg file. Delete asap, as it's been create by a
    // 'Create' prefixed method
    std::unique_ptr< PDFFormXObject > image_object
        { writer.CreateFormXObjectFromJPGFile( input_jpg_path ) };

    // start graphic change block with q and Q operators
    context->q();

    // setting up the matrix to place the image
    context->cm
        (
            // 2x2 matrix describing a 2-dimensional transformation, scaling
            0.4, 0, 0, 0.4,
            // offset from the orgin of the graphic, origin is bottom left
            57.5, 241
        );

    // adding the image in the resource dictionnary of the page, and place
    // accordingly to the context that was set up
    context->Do
        (
            // get the resource dictionnary of the page andd add the previsously
            // created image within it
            page.GetResourcesDictionary().AddXObjectMapping
                ( image_object->GetObjectID() )
        );

    context->Q();

    // preparation to add text in the page
    // get a font, do not delete it as it's been got from a 'Get' prefixed
    // method
    // Warning about permissions on font locations
    auto used_font =
        writer.GetFontForFile( input_font_path );

    // modifying state of the page context to place some text
    context->BT();

    // set the color of the text object using the RGB color specs
    context->rg( 0, 0.25, 0.5 );

    // using the previously got font, size of 20
    context->Tf( used_font, 20 );

    // set up the text placement using something somewhat similar to the cm
    // PDF operator :
    context->Tm
        (
            // 2D transformation matrix, identity here
            1, 0, 0, 1,
            // offset from origin to place the text, origin is bottom left
            90, 210
        );

    // set the text :
    context->Tj
        ( "A really nice ski slope!" );

    context->ET();

    // end a context of a page
    writer.EndPageContentContext( context );

    writer.WritePage( &page );

    // create a page with a big array in it
    createPageWithBigArray( writer );

    writer.EndPDF();
}

result_array MainWindow::create_big_array
    (std::size_t column_count, std::size_t row_count)
{
    return result_array{ column_count, row_count };
}

void MainWindow::createPageWithBigArray( PDFWriter &writer )
{
    // first, create some big array with some text inside it
    auto big_array = create_big_array( 50, 50 );

    render_array( writer, big_array, render_methods::single_page );
    render_array( writer, big_array, render_methods::multi_page );
}

void MainWindow::render_array
    (
        PDFWriter &writer,
        const result_array &array,
        render_methods rendering_method
    )
{
    switch( rendering_method )
    {
    case render_methods::single_page :
        render_array_on_single_page( writer, array );
        break;

    case render_methods::multi_page:
        render_array_on_multi_page( writer, array );
        break;
    }
}

void MainWindow::render_array_on_single_page
    ( PDFWriter &writer, const result_array &array )
{
    // on a single page, the page dimension have to be calculated from the
    // content of the array

    // get the size of the content of the array according to PDF font used

    // create a page with previsouly calculated dimensions

    // render the array
}

void MainWindow::render_array_on_multi_page
    ( PDFWriter &writer, const result_array &array )
{
    // on a multi page array, page size is determinized and a page number has to
    // be calculated according to the array content

    // get the size of the content of the array according to PDF font used
}

void MainWindow::renderDocument
    ( const std::string & input_pdf_path , const std::string &output_png_path )
{
    // Create a context to hold the exception stack and various caches.
    fz_context *context = fz_new_context( NULL, NULL, FZ_STORE_UNLIMITED );

    // Register document handlers for the default file types we support.
    fz_register_document_handlers( context );

    // Open the PDF, XPS or CBZ document.
    fz_document *document = fz_open_document( context, input_pdf_path.c_str() );

    // Retrieve the number of pages
    int page_count = fz_count_pages( context, document );
    ( void )page_count;

    // Load the first page. Page numbering starts from zero.
    fz_page *page = fz_load_page( context, document, 0 );

    // Calculate a transform to use when rendering. This transform
    // contains the scale and rotation. Convert zoom percentage to a
    // scaling factor. Without scaling the resolution is 72 dpi.
    fz_matrix transform;
    fz_rotate( &transform, 0.0 );
    fz_pre_scale( &transform, 0.5, 0.5 );

    // Take the page bounds and transform them by the same matrix that
    // we will use to render the page.
    fz_rect page_bounds;
    fz_bound_page( context, page, &page_bounds );
    fz_transform_rect( &page_bounds, &transform );

    // Create a blank pixmap to hold the result of rendering. The
    // pixmap bounds used here are the same as the transformed page
    // bounds, so it will contain the entire page. The page coordinate
    // space has the origin at the top left corner and the x axis
    // extends to the right and the y axis extends down.
    fz_irect bounding_box;
    fz_round_rect( &bounding_box, &page_bounds );

    fz_pixmap *pixmap =
        fz_new_pixmap_with_bbox
        ( context, fz_device_rgb( context ), &bounding_box );

    fz_clear_pixmap_with_value( context, pixmap, 0xFFFFFFFF );

    // A page consists of a series of objects (text, line art, images,
    // gradients). These objects are passed to a device when the
    // interpreter runs the page. There are several devices, used for
    // different purposes:
    //
    //	draw device -- renders objects to a target pixmap.
    //
    //	text device -- extracts the text in reading order with styling
    //	information. This text can be used to provide text search.
    //
    //	list device -- records the graphic objects in a list that can
    //	be played back through another device. This is useful if you
    //	need to run the same page through multiple devices, without
    //	the overhead of parsing the page each time.

    // Create a draw device with the pixmap as its target.
    // Run the page with the transform.
    fz_device *draw_device = fz_new_draw_device( context, pixmap );
    fz_run_page( context, page, draw_device, &transform, NULL );
    fz_drop_device( context, draw_device );

    // Save the pixmap to a file.
    fz_save_pixmap_as_png( context, pixmap, output_png_path.c_str(), 1 );

    // Clean up.
    fz_drop_pixmap( context, pixmap );
    fz_drop_page( context, page );
    fz_drop_document( context, document );
    fz_drop_context( context );
}

void MainWindow::showRenderedInWidget( const std::string &input_png_path )
{
    auto image_label = new QLabel{};
    setCentralWidget( image_label );

    QPixmap image{ QString::fromStdString( input_png_path ) };

    image_label->setPixmap( image );
    adjustSize();
}

MainWindow::~MainWindow()
{
    delete ui;
}
