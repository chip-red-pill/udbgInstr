#pragma once

extern "C"
{
	enum
	{
		UDBG_CMD_CRBUS = 0,
		UDBG_CMD_URAM = 0x10
	};

	unsigned __int64 udbgrd(unsigned command, unsigned address);
	void udbgwr(unsigned command, unsigned address, unsigned __int64 data);
}
