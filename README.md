# AmongUSTaskBar
AmongUS Total Task Bar Update At Meetings

## What is it?
Well, that try to solve the issue with people checking Task Bar to see who is crumate by
only showing the "TOTAL TASKS COMPLETED" on Meetings.

## How it does it?
It intercepts the calls to PlayerControl.CompleteTask(), queue the requests and call them
 when the MeetingHud.Awake() is called.
There is a thread running in background that will empty the queue while procress the 
requests properly.

## Why have you made this?
So, I watch tons of hours of Among US streamers, especially [5up](https://www.twitch.tv/5uppp) and [hafu](https://twitch.tv/itshafu).
After seeing them claiming that Task Bar was something that ruined some of their games, and hearing the suggestion from 5up about possible solutions, I thought I could create something to help them.

## How I can use it?
I provided a DLL pre-built to be used and to inject I would recommend [this](https://github.com/Arvanaghi/Windows-DLL-Injector), which is quite simple to use.
But of course you can always built it yourself and/or inject it by yourself as well.

**You may need the Visual Studio C++ 2019 Redistributable, so here they are [x86](https://aka.ms/vs/16/release/vc_redist.x86.exe) and [x64](https://aka.ms/vs/16/release/vc_redist.x64.exe).**


## References
- [Good place to understand how you dig into the memory of the game](https://github.com/shlifedev/AmongUsMemory)