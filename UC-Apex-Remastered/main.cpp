#include <thread>
#include "globals.h"
#include "Util.h"
#include "XorStr.hpp"
#include "printa/printa.hpp"
#include "overlay.h"
#include "Modules.h"

// i kinda overcommented this and at some parts i didn't comment at all

int main(int argCount, char** argVector) /* the file name is the first element of
										 the argVector, we need it so we can rename it */
{
	std::string filePath = argVector[0]; // i'll create this variable in case we need it later

	if (!Util::RenameFile(filePath)) /* renaming the file name and console window name
									 and saving it to the filePath variable */
	{
		printa->print<fail>(xorstr_("Failed to rename file!\n"));
		Sleep(5000);
		return -1;
	}

	bool eacRunning = Util::GetPid(xorstr_(L"EasyAntiCheat.exe"));
	
	if (!eacRunning)
	{
		printa->print<loading>(xorstr_("Loading driver..\n"));
		system(xorstr_("kdmapper.exe Driver.sys")); // mapping the driver | there is much better ways for doing this but it's enough

		Sleep(5000); // waiting before clearing the screen so you can see if the driver mapped successfully
		system(xorstr_("cls"));
	}

	if (!Driver.Init())
	{
		printa->print<fail>(xorstr_("Failed to get the hooked function\n"));
		Sleep(5000);
		return -1;
	}

	if (!eacRunning)
		printa->print<input>(xorstr_("Start Apex\n"));

	while (!globals.hWnd) /* make sure the game is running and has a window instance,
						  create a handle to the window and save it to a variable */
	{
		globals.hWnd = FindWindowA(NULL, xorstr_("Apex Legends"));
		Sleep(500);
	}

	while (!globals.pID) // get the process id
	{
		globals.pID = Util::GetPid(xorstr_(L"r5apex.exe"));
		Sleep(500);
	}

	printa->print<loading>(xorstr_("Requesting base from driver\n"));

	while (!globals.moduleBase) // request the module base from driver
	{
		globals.moduleBase = Driver.GetModuleBase(globals.pID);
		Sleep(500);
	}

	// create window
	if (!overlay::InitWindow())
	{
		printa->print<fail>(xorstr_("Failed to create window\n"));
		Sleep(5000);
		return 0;
	}

	if (!overlay::DirectXInit())
	{
		printa->print<fail>(xorstr_("Failed to initialize directx\n"));
		Sleep(5000);
		return 0;
	}

	// start render thread
	std::thread(overlay::Render).detach();
	//std::thread(overlay::UpdateWindowPos).detach();

	globals.entityList = globals.moduleBase + OFFSET_ENTITYLIST;
	globals.nameList = globals.moduleBase + OFFSET_NAMELIST;
	printa->print<ok>(xorstr_("success!\n"));

	while (!GetAsyncKeyState(VK_END)) // run the main logic of the cheat until the user presses end
	{
		Modules::Run();

		Sleep(1);
	}

	overlay::DirectXShutdown();
	
	return 0;
}