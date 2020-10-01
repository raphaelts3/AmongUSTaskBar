// dllmain.cpp : Defines the entry point for the DLL application.
#include "framework.h"

void (*__cdecl Original_PlayerControl_CompleteTask)(void*, int, int);
int inMeeting = 0;
bool shutdown = false;
std::stack<std::tuple<void*, int, int>> tasksCompleted;

/// <summary>
/// This function take the place of the original PlayerControl.CompleteTask()
/// and just queue up the same args to be called later
/// </summary>
/// <param name="player"></param>
/// <param name="taskId"></param>
/// <param name="zero"></param>
/// <returns></returns>
void __cdecl PlayerControl_CompleteTask(void* player, int taskId, int zero)
{
	tasksCompleted.push(std::make_tuple(player, taskId, zero));
}
unsigned int addr_playercontrol_completetask = (unsigned int)&PlayerControl_CompleteTask;

/// <summary>
/// The best way I could think so far to process the queued tasks was with this thread
/// , because it prevent the procress to hanging if many tasks are queue
/// </summary>
/// <param name="lpParam"></param>
/// <returns></returns>
DWORD WINAPI UpdateTasks(LPVOID lpParam)
{
	while (true) {
		if (!inMeeting) {
			if (shutdown) {
				return 0;
			}
			Sleep(75);
			continue;
		}
		while (!tasksCompleted.empty()) {
			auto callArgs = tasksCompleted.top();
			Original_PlayerControl_CompleteTask(std::get<0>(callArgs), std::get<1>(callArgs), std::get<2>(callArgs));
			tasksCompleted.pop();
		}
		inMeeting = 0;
	}
}

void* return_meetinghud_awake = 0;
/// <summary>
/// Switch the variable inMeeting to enables the query to run the queue
/// </summary>
void __declspec(naked) Hook_MeetingHud_Awake()
{
	__asm {
		mov inMeeting, 1
		push esi
		mov eax, dword ptr ds : [eax + 0x5C]
		mov esi, dword ptr ds : [eax]
		jmp return_meetinghud_awake
	}
}
unsigned int addr_hook_meetinghud_awake = (unsigned int)&Hook_MeetingHud_Awake;

void Attached()
{
	DWORD threadId;
	// Start thread that will update tasks when at Meeting
	CreateThread(NULL, 0, UpdateTasks, NULL, 0, &threadId);
	unsigned int hModule = (unsigned int)GetModuleHandleA("gameassembly.dll");
	// Generate the address for hook to return
	return_meetinghud_awake = (void*)(hModule + 0x8D990C);
	DWORD flOldProtect;
	// Get address for original PlayerControl.CompleteTask()
	Original_PlayerControl_CompleteTask = (void (*)(void*, int, int))(hModule + 0x8876C0);

	// Put the jmp to the meeting hook
	addr_hook_meetinghud_awake = (unsigned int)&Hook_MeetingHud_Awake;
	VirtualProtect((void*)(hModule + 0x8D9906), 6, PAGE_EXECUTE_READWRITE, &flOldProtect);
	*(unsigned short*)(hModule + 0x8D9906) = 0x25FF;
	*(unsigned int*)(hModule + 0x8D9908) = (unsigned int)&addr_hook_meetinghud_awake;
	VirtualProtect((void*)(hModule + 0x8D9906), 6, flOldProtect, &flOldProtect);

	// Switch one call to PlayerControl.CompleteTask() to our own
	VirtualProtect((void*)(hModule + 0x888989), 5, PAGE_EXECUTE_READWRITE, &flOldProtect);
	*(unsigned int*)(hModule + 0x88898A) = *(unsigned int*)(hModule + 0x88898A) - 6;
	VirtualProtect((void*)(hModule + 0x888989), 5, flOldProtect, &flOldProtect);

	// Switch second call to PlayerControl.CompleteTask() to our own
	VirtualProtect((void*)(hModule + 0x88AB98), 6, PAGE_EXECUTE_READWRITE, &flOldProtect);
	*(unsigned int*)(hModule + 0x88AB99) = *(unsigned int*)(hModule + 0x88AB99) - 6;
	VirtualProtect((void*)(hModule + 0x88AB98), 6, flOldProtect, &flOldProtect);

	// Put a JMP on place to be used for the two changes above
	VirtualProtect((void*)(hModule + 0x8876C0 - 6), 6, PAGE_EXECUTE_READWRITE, &flOldProtect);
	*(unsigned short*)(hModule + 0x8876C0 - 6) = 0x25FF;
	*(unsigned int*)(hModule + 0x8876C0 - 6 + 2) = (unsigned int)&addr_playercontrol_completetask;
	VirtualProtect((void*)(hModule + 0x8876C0 - 6), 6, flOldProtect, &flOldProtect);
	MessageBoxA(0, "Done :-)", "Done :-)", 0);
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Attached();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		shutdown = true;
		break;
	}
	return TRUE;
}