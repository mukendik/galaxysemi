#include <QFileInfo>

#include "json_server_file_descriptorIO.h"
#include "xml_server_file_descriptorIO.h"
#include "server_file_descriptorIO_factory.h"

using namespace GS::LPPlugin;

QScopedPointer<ServerFileDescriptorIOFactory>  ServerFileDescriptorIOFactory::mInstance(0);

ServerFileDescriptorIOFactory* ServerFileDescriptorIOFactory::GetInstance()
{
    if(mInstance.data() == 0)
        mInstance.reset( new ServerFileDescriptorIOFactory()) ;
    return mInstance.data();
}

ServerFileDescriptorIOFactory::ServerFileDescriptorIOFactory()
{
    mServerFileDescriptorIOs.push_back( new XMLServerFileDescriptorIO());
    mServerFileDescriptorIOs.push_back( new JSONServerFileDescriptorIO());
}

ServerFileDescriptorIOFactory::~ServerFileDescriptorIOFactory()
{
    foreach(ServerFileDescriptorIO* lPtr, mServerFileDescriptorIOs) {
        delete lPtr;
    }
    mServerFileDescriptorIOs.clear();
}


ServerFileDescriptorIO* ServerFileDescriptorIOFactory::GetServerDescriptor(const QString& fileName)
{
    QFileInfo lFileInfo(fileName);

    QString lExtension = lFileInfo.suffix().toLower();

    int lIndexServerFileDescriptor = 0;
    if(!lExtension.compare("xml")) {
        lIndexServerFileDescriptor = 0;
    }
    else if(!lExtension.compare("json")) {
        lIndexServerFileDescriptor = 1;
    }
    else {
        return 0;
    }

    return mServerFileDescriptorIOs[lIndexServerFileDescriptor];
}
