# **Disclaimer**

**All information is provided for educational purposes only. Follow these instructions at your own risk. Neither the authors nor their employer are responsible for any direct or consequential damage or loss arising from any person or organization acting or failing to act on the basis of information contained in this page.**

# Description

[**udbg_test**](https://github.com/chip-red-pill/udbgInstr/udbg_test) - special tool that, when run on a processor, checks whether the udbgrd and udbgwr instructions can be executed on it and, if not, checks whether the processor allows speculative writing and reading of microarchitectural data when calling the instructions. This tool executes the udbgwr instruction to write a specific URAM address (Time Stamp Counter (TSC) multiplier used by the rdtsc x86 instruction), which is known for Big Cores as well as for Atom Goldmont, and then attempts to read the written data using architectural mechanisms available in User Mode. The tool also tries to speculatively read the TSC multiplier in URAM with the udbgrd instruction by using CPU cache as a mechanism to retrieve the read data. On top of that, we were the first to publicly provide a list of all (as far as we can tell) possible ways to activate the Red Unlock mode for CPU debugging and to demonstrate that some of them are rather dangerous (for example, software-based unlocking via Intel CSME and PUNIT firmware and processor-specific OTP configuration).
```
udbg_test.exe
[INFO] Big Core is detected
[INFO] Cached read treshold ts: 0x1e
[OK] Instructions aren't activated
[OK] There were not found problems with udbgwr speculative execution
[OK] There was not found a problem with udbgrd speculative execution
```

[**udebug**](https://github.com/chip-red-pill/udbgInstr/udebug) -  EFI application that activates the udbgrd and udbgwr instructions (Red Unlock only).

[**undocumented_x86_insts_for_uarch_control.pdf**](https://github.com/chip-red-pill/udbgInstr/paper/raw/master/undocumented_x86_insts_for_uarch_control.pdf) - our paper "Undocumented x86 instructions to control the CPU at the microarchitecture level in modern Intel processors".


[IPC Scripts](https://github.com/chip-red-pill/crbus_scripts)

[RED Unlock PoC](https://github.com/chip-red-pill/IntelTXE-PoC)

# Research Team

Mark Ermolov ([@\_markel___][1])

Maxim Goryachy ([@h0t_max][2])

Dmitry Sklyarov ([@_Dmit][3])


[1]: https://twitter.com/_markel___
[2]: https://twitter.com/h0t_max
[3]: https://twitter.com/_Dmit
