#pragma once

VOID
EFIAPI
activate_udebug_insts(
	);

VOID
EFIAPI
udebug_read(
	IN unsigned int command,
	IN unsigned int address,
	OUT unsigned __int64* data);

VOID
EFIAPI
udebug_write(
	IN unsigned int command,
	IN unsigned int address,
	IN unsigned __int64 data);
