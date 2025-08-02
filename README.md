# RunMMEasDate
A customized version of [JackKuo-tw's RunAsDate](https://github.com/JackKuo-tw/RunAsDate) designed to be a forwarder for Mobiclip Multicore Encoder.

Dependencies:
--
Mobiclip Multicore Encoder

Usage:
--
`RunMMEasDate [PARAM]...` (Just like MME)

But How?
--
When you run this program, it will locate MME via its environment variable, `MOBICLIP_MULTICORE_ENCODER_PATH`. It then creates the process by giving it the path to MME and the parameters given to the forwarder. Next, it injects a custom function to load a custom DLL that calls the `InitDate` function. Finally, `InitDate` uses [minhook](https://github.com/TsudaKageyu/minhook/tree/565968b28583221751cc2810e09ea621745fc3a3) to override a few of the [Windows API Time Functions](https://learn.microsoft.com/en-us/windows/win32/sysinfo/time-functions). Since the fake timestamp is set to 1/1/2010, every license file will work no matter what time it really is.

Credits
--
- [Nir Sofer](https://www.nirsoft.net): Inspiration and example
- [JackKuo-tw](https://github.com/JackKuo-tw): Source material
- [ZeroSkill1](https://github.com/ZeroSkill1): Reverse engineering expertise
