#include "GlobalNetworkAccessManager.h"

namespace GS
{
namespace Gex
{

GlobalNetworkAccessManager * GlobalNetworkAccessManager::m_instance = NULL;

GlobalNetworkAccessManager::GlobalNetworkAccessManager():
    m_network_access_manager( NULL )
{
    m_network_access_manager = new QNetworkAccessManager();
}

GlobalNetworkAccessManager * GlobalNetworkAccessManager::GetInstance()
{
    // // singleton pattern : through static definition inside method
    // // less performant that a static member as the compiler generate a small piece of code to see if instance has
    // // already been created, but much more efficient that a .Net/Java singleton pattern using dynamic allocation.
    // // does not trigger the event loop error as instance is created at first GetInstance() call
    // static GlobalNetworkAccessManager instance;

    // return instance;

    // revert to a dynamic singleton allocation because of glitches on windows platforms
    if( m_instance == NULL )
    {
        m_instance = new GlobalNetworkAccessManager();
    }

    return m_instance;
}

GlobalNetworkAccessManager::~GlobalNetworkAccessManager()
{
    if( m_network_access_manager != NULL )
        delete m_network_access_manager;
}

void GlobalNetworkAccessManager::ResetNetworkConnectionManager()
{
    if( m_network_access_manager != NULL )
        delete m_network_access_manager;

    m_network_access_manager = NULL;

    m_network_access_manager = new QNetworkAccessManager();
}
}
}
