#ifndef HEADER_RESOURCEMANAGER
#define HEADER_RESOURCEMANAGER

#include <OpenHome/Net/Core/DvDevice.h>

namespace OpenHome {
namespace Media {
        
class ResourceManager : public Net::IResourceManager
{
public:
    ResourceManager(const Brx& aNamespace);
    
    virtual void WriteResource(const Brx& aUriTail, TIpAddress aInterface, std::vector<char*>& aLanguageList, Net::IResourceWriter& aResourceWriter);
};
    
} // Media
} // OpenHome

#endif