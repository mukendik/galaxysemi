#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui
{
class MainWindow;
}

namespace KDReports
{
class Header;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    void extractResourceFiles
        (
            std::string &background_jpg_path,
            std::string &kdab_small_jpg_path,
            std::string &system_png_path
        );

    void fillEvenPagesHeader( KDReports::Header& evenPagesHeader );

    void createReport
    (
        const std::string &background_jpg_path,
        const std::string &kdab_small_jpg_path,
        const std::string &system_png_path
    );

    void renderReport();

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
