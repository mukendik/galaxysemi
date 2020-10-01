#ifndef SERVERDESCROTORIOFACTORY_H
#define SERVERDESCROTORIOFACTORY_H

#include <vector>
#include <QScopedPointer>
#include <QString>
#include "license_provider_global.h"
namespace GS
{
    namespace LPPlugin
    {

        class ServerFileDescriptorIO;

        class LICENSE_PROVIDERSHARED_EXPORT ServerFileDescriptorIOFactory
        {
        public :
            static ServerFileDescriptorIOFactory* GetInstance();

            ~ServerFileDescriptorIOFactory();

            ServerFileDescriptorIO*  GetServerDescriptor(const QString& fileName);

        private:
            static QScopedPointer<ServerFileDescriptorIOFactory> mInstance;

            std::vector<ServerFileDescriptorIO*> mServerFileDescriptorIOs;


            ServerFileDescriptorIOFactory();


            ServerFileDescriptorIOFactory(const ServerFileDescriptorIOFactory&);
            ServerFileDescriptorIOFactory& operator=(const ServerFileDescriptorIOFactory&) ;

        };

    }
}

#endif

