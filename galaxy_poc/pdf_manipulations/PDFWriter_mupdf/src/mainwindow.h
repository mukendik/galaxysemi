#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "result_array.h"

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class PDFWriter;

enum class render_methods
{
    single_page,
    multi_page
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

    void extractResourceFiles
        (
            std::string &input_jpg_path,
            std::string &input_font_path
        );

    void createDocument
        (
            const std::string &output_pdf_path,
            const std::string &input_jpg_path,
            const std::string &input_font_path
        );

    void renderDocument
        (
            const std::string &input_pdf_path,
            const std::string &output_png_path
        );

    result_array create_big_array
        (
            std::size_t column_count = { 1 },
            std::size_t row_count = { 1 }
        );

    void render_array
        (
            PDFWriter &writer,
            const result_array &array,
            render_methods rendering_method
        );

    void render_array_on_single_page
        ( PDFWriter &writer, const result_array &array );

    void render_array_on_multi_page
        ( PDFWriter &writer, const result_array &array );

    void createPageWithBigArray( PDFWriter &writer );

    void showRenderedInWidget( const std::string &input_png_path );

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
