#include <Uefi.h>
#include <Library/PrintLib.h>
#include "udebug_rw.h"

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE* SystemTable
    )
{
    unsigned __int64 crbus_data;

    activate_udebug_insts();
    udebug_write(0, 0x40, 0xf0f1f2f3f4f5f6f7ull);
    udebug_read(0, 0x40, &crbus_data);

    CHAR16 print_buf[0x100];
    UnicodeSPrint(print_buf, sizeof print_buf, L"SAVED CRBUS VAL: %L016x", crbus_data);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, print_buf);
    return EFI_SUCCESS;
}
