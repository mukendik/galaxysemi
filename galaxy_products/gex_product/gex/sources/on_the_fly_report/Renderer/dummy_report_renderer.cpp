#include "dummy_report_renderer.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDialog>
#include <QLabel>
#include <QDateTime>
#include <QBoxLayout>

void GS::Gex::DummyReportRenderer::BeginRendering() {}

void GS::Gex::DummyReportRenderer::EndRendering() {}

void GS::Gex::DummyReportRenderer::NewPage( const QJsonObject & ) {}

void GS::Gex::DummyReportRenderer::NewSection( const QJsonObject & ) {}

void GS::Gex::DummyReportRenderer::AddText( const QJsonObject & ) {}

void GS::Gex::DummyReportRenderer::AddImage( const QJsonObject & ) {}

std::string GS::Gex::DummyReportRenderer::GetReportFileExtension() const
{
    return "txt";
}

void GS::Gex::DummyReportRenderer::SaveAs( const std::string &file_path )
{
    QFile file( QString::fromStdString( file_path ) );

    file.open( QFile::WriteOnly );

    QTextStream stream(&file);
    stream << "What a wonderfull report saved in text file!!!" << '\n'
           << "Created the : " << QDateTime::currentDateTime().toString()
           << '\n'
           << "Quantix logo :" << '\n';

    QFile image( ":/gex/icons/charac.png" );

    image.open( QFile::ReadOnly );

    stream << image.readAll().toBase64();

    image.close();

    file.close();
}

void GS::Gex::DummyReportRenderer::RenderReportFileIn
    ( const std::string &file_path, QDialog *device )
{
    // set the title of the device
    device->setWindowTitle( "Report preview" );
    device->setModal( true );

    device->setLayout( new QVBoxLayout() );

    // static elements rendering, 3 lines
    QLabel *label_title = new QLabel();
    QLabel *label_date = new QLabel();
    QLabel *label_logo = new QLabel();

    QFile report_file( QString::fromStdString( file_path ) );
    report_file.open( QFile::ReadOnly );

    QTextStream stream( &report_file );

    label_title->setText( stream.readLine() );
    label_date->setText( stream.readLine() );

    stream.readLine();

    QByteArray data( stream.readAll().toAscii() );

    QImage image;
    image.loadFromData( QByteArray::fromBase64( data ), "PNG" );

    label_logo->setPixmap( QPixmap::fromImage( image ) );

    device->layout()->addWidget( label_title );
    device->layout()->addWidget( label_date );
    device->layout()->addWidget( label_logo );
}
