<img width="64" height="64" alt="RunMMEasDate" src="https://github.com/user-attachments/assets/37384720-0614-4430-ab1d-23595ca8236f" />

# RunMMEasDate

A customized version of [JackKuo-tw's RunAsDate](https://github.com/JackKuo-tw/RunAsDate) designed to be a forwarder for Mobiclip Multicore Encoder.

Dependencies
--
Mobiclip Multicore Encoder (Duh)

Usage
--
`MobiclipMulticoreEncoderForwarder [PARAM]...` (Just like MME)

Installation
--
1. Install Mobiclip Multicore Encoder
   - Optional: Uncheck the shortcut components before clicking 'Install' and manually create some shortcuts to this forwarder
3. Install the [Microsoft Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist)
4. Run `MobiclipMulticoreEncoderForwarder.exe`

Compilation
--
1. Download and install [CMake](https://cmake.org/), [Git](https://git-scm.com/downloads/win), and [Microsoft Visual Studio](https://visualstudio.microsoft.com/) with the `Desktop development with C++` workload
2. Clone the entire repository `git clone --recurse-submodules https://github.com/FoofooTheGuy/RunMMEasDate.git`
3. Run `cd RunMMEasDate`
4. Run `build.bat`

But How?
--
When you run this program, it will locate MME via its environment variable, `MOBICLIP_MULTICORE_ENCODER_PATH`. It then creates the process by giving it the path to MME and the parameters given to the forwarder. Next, it injects a custom function to load a custom DLL that calls the `InitDate` function. Finally, `InitDate` uses [minhook](https://github.com/TsudaKageyu/minhook/tree/565968b28583221751cc2810e09ea621745fc3a3) to override a few of the [Windows API Time Functions](https://learn.microsoft.com/en-us/windows/win32/sysinfo/time-functions). Since the fake time is set to midnight 1/1/2010, every license file will work no matter what year it really is.

Credits
--
- [Nir Sofer](https://www.nirsoft.net): Inspiration and example
- [JackKuo-tw](https://github.com/JackKuo-tw): Source material
- [ZeroSkill1](https://github.com/ZeroSkill1): Reverse engineering expertise
- The icon used is a combination of the MME icon and [a rewind icon from Wikimedia](https://commons.wikimedia.org/wiki/File:Eo_circle_grey_white_rewind.svg)
