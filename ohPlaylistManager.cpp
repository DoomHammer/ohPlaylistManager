#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Maths.h>
#include <OpenHome/Private/OptionParser.h>
#include <OpenHome/Net/Core/DvDevice.h>
#include <OpenHome/Net/Core/OhNet.h>

#include <stdio.h>

#include "PlaylistManager.h"
#include "Icon.h"

using namespace OpenHome;
using namespace OpenHome::Net;
using namespace OpenHome::TestFramework;
using namespace OpenHome::Media;

#ifdef _WIN32

#pragma warning(disable:4355) // use of 'this' in ctor lists safe in this case

#define CDECL __cdecl

#include <conio.h>

int mygetch()
{
    return (_getch());
}

void ComputerName(Bwx& aName)
{
	TUint bytes = aName.MaxBytes();
	
	if(!GetComputerName((LPSTR)aName.Ptr(), (LPDWORD)&bytes))
	{
		THROW(PlaylistManagerError);
	}
	
	aName.SetBytes(bytes);
}

#else

#define CDECL

#include <termios.h>
#include <unistd.h>

int mygetch()
{
	struct termios oldt, newt;
	int ch;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}

#include <sys/utsname.h>

void ComputerName(Bwx& aName)
{
	struct utsname name;
	if(uname(&name) < 0)
	{
		return;
	}
	
	// strip off the ".local" from the end
	Brn computer(name.nodename);
	Brn local(".local");
	if(computer.Bytes() > local.Bytes())
	{
		Brn end = computer.Split(computer.Bytes() - local.Bytes());
		if(Ascii::CaseInsensitiveEquals(end, local))
		{
			computer.Set(computer.Ptr(), computer.Bytes() - local.Bytes());
		}
	}
	
	if(computer.Bytes() > aName.MaxBytes())
	{
		THROW(PlaylistManagerError);
	}
	
	aName.Replace(computer);
}

#endif


static void RandomiseUdn(Bwh& aUdn, TIpAddress aAdapter)
{
    aUdn.Grow(aUdn.Bytes() + 1 + Ascii::kMaxUintStringBytes + 1);
    aUdn.Append('-');
    Bws<Ascii::kMaxUintStringBytes> buf;
    (void)Ascii::AppendDec(buf, Random(aAdapter));
    aUdn.Append(buf);
    aUdn.PtrZ();
}

int CDECL main(int aArgc, char* aArgv[])
{
    OptionParser parser;
    
	Bws<PlaylistManager::kMaxNameBytes - 20> computerName;
	ComputerName(computerName);
	
	Bws<PlaylistManager::kMaxNameBytes> defaultName("ohPlaylistManager (");
	defaultName.Append(computerName);
	defaultName.Append(Brn(")"));
	
    OptionString optionName("-n", "--name", defaultName, "[name] name of the playlist manager");
    parser.AddOption(&optionName);
	
	OptionUint optionAdapter("-a", "--adapter", 0, "[adapter] index of network adapter to use");
    parser.AddOption(&optionAdapter);

    if (!parser.Parse(aArgc, aArgv)) {
        return (1);
    }

    InitialisationParams* initParams = InitialisationParams::Create();

	UpnpLibrary::Initialise(initParams);

    std::vector<NetworkAdapter*>* subnetList = UpnpLibrary::CreateSubnetList();
    TIpAddress subnet = (*subnetList)[optionAdapter.Value()]->Subnet();
    TIpAddress adapter = (*subnetList)[optionAdapter.Value()]->Address();
    UpnpLibrary::DestroySubnetList(subnetList);

    printf("Using subnet %d.%d.%d.%d\n", subnet&0xff, (subnet>>8)&0xff, (subnet>>16)&0xff, (subnet>>24)&0xff);
	printf("Using adapter %d.%d.%d.%d\n", adapter&0xff, (adapter>>8)&0xff, (adapter>>16)&0xff, (adapter>>24)&0xff);
    
    Brhz name(optionName.Value());

	UpnpLibrary::StartCombined(subnet);

	Bwh udn("device");
    RandomiseUdn(udn, adapter);

    DvDeviceStandard* device = new DvDeviceStandard(udn);
    
    device->SetAttribute("Upnp.Domain", "av.openhome.org");
    device->SetAttribute("Upnp.Type", "PlaylistManager");
    device->SetAttribute("Upnp.Version", "1");
    device->SetAttribute("Upnp.FriendlyName", name.CString());
    device->SetAttribute("Upnp.Manufacturer", "Openhome");
    device->SetAttribute("Upnp.ManufacturerUrl", "http://www.openhome.org");
    device->SetAttribute("Upnp.ModelDescription", "Openhome PlaylistManager");
    device->SetAttribute("Upnp.ModelName", "Openhome PlaylistManager");
    device->SetAttribute("Upnp.ModelNumber", "1");
    device->SetAttribute("Upnp.ModelUrl", "http://www.openhome.org");
    device->SetAttribute("Upnp.SerialNumber", "");
    device->SetAttribute("Upnp.Upc", "");
	
	Brn icon(icon_png, icon_png_len);

	// create managers
	
	PlaylistManager* playlistManager = new PlaylistManager(*device, adapter, name, icon, Brx::Empty());
	ProviderPlaylistManager iProvider(*device, *playlistManager, PlaylistManager::kMaxPlaylists, PlaylistData::kMaxTracks);
	playlistManager->SetListener(iProvider);
    
    device->SetEnabled();

	printf("q = quit\n");
	
    for (;;) {
    	int key = mygetch();
    	
    	if (key == 'q') {
    		break;
		}
	}	

	delete playlistManager;
    delete device;
	
	UpnpLibrary::Close();
    
	printf("\n");
	
    return (0);
}
