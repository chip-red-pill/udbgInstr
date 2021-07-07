#pragma once

extern "C"
{
	enum
	{
		UDBG_CMD_CRBUS = 0,
		UDBG_CMD_URAM = 0x10
	};

	void udbgrd(unsigned command, unsigned address, unsigned __int64* out_data);
	void udbgwr(unsigned command, unsigned address, unsigned __int64 data);
}
