#include <OpenHome/Private/Debug.h>
#include <OpenHome/Buffer.h>

#include "ResourceManager.h"

using namespace std;
using namespace OpenHome;
using namespace OpenHome::Media;
using namespace OpenHome::Net;

ResourceManager::ResourceManager(const Brx& aNamespace)
{
}

void ResourceManager::WriteResource(const Brx& aUriTail, TIpAddress aInterface, std::vector<char*>& aLanguageList, IResourceWriter& aResourceWriter)
{
    Log::Print(aUriTail);
}