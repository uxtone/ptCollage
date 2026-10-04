
# Roadmap
The overall goal of this project is to make an authentic version of the pxtone apps natively usable on all operating systems. After this is a reality, a few of Pixel's desired changes may follow (translated):
>"In fact, there are many things that should be done, but are not.
> - Insufficient copy/paste functionality (not even file-to-file copy...)
> - Multiple simultaneous startups (easy to implement, but more functions are needed at once)
> - Color coding of tracks
> - “Tone preset” function[*]
> - Volume changes need to be smooth ( noise will occur if not careful )
>
> Current(?) I used to manage lists and priorities like this back in the day... 🙃."
>*<small>A tone preset can be understood as a group of instruments that you can load at the same time</small>
## Cross-platform
 Separating pxtone from Windows requires several steps, considering:
- it can only be developed only inside Microsoft Visual Studio
- it requires using one of Microsoft's C runtimes, MSVCRT or UCRT
- it's heavily dependent on the Windows API (Win32) for core facilities and common controls
- it requires the DirectX SDK for XAudio2 and DirectDraw, naturally non-portable libraries

Cross-compatibility is the top priority until the apps are fully functional on all desktop OSes, being able to make project files for LE & BE systems. These are the changes required.
- [x] Step 1: add CMake to replace .sln and .vcxproj, allowing building from more systems
  - [?] still buildable from Visual Studio on Windows
- [x] Step 2: port code to be friendly with winelib & glibc, allowing development on more systems
- [ ] Step 3: replace audio backend to allow full build & run support on all platforms
  - MSYS2 does not work with XAudio2, though Wine does; this will allow functional audio on all systems
- [ ] Step 4: integrate with https://github.com/ewancg/pxtone for bug fixes
  - Certain old game consoles and PCs have big-endian processors and require these fixes
  - [ ] Add an option to save big or little endian .ptcop or .pttunes regardless of the system endianness
- [ ] Step 5: replace win32 windowing with something SDL3-based for full cross-platform runtime independence

|the code works...| w/o MS VS | w/o MS libc | w/o Win32 | w/o XAudio2 | w/o DirectX 9 | on MSYS2 | on LE&BE 
|--|--|--|--|--|--|--|--|
| From starting | :x: | :x: | :x: | :x: | :x: | :x: | :x: |
| After step 1 | :white_check_mark: | :x: | :x: | :x: | :x: | :white_check_mark:* | :x: |
| **After step 2** (now) | :white_check_mark: | :white_check_mark: | :x: | :x: |:x: | :white_check_mark:* | :x: |
| After step 3 | :white_check_mark: | :white_check_mark: | :x: | :white_check_mark: | :x: | :white_check_mark: | :x: | 
| After step 4 | :white_check_mark: | :white_check_mark: | :x: | :white_check_mark: | :x: | :white_check_mark: | :white_check_mark: | 
| After step 5 | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | 

*<small>With non-functional audio</small>
