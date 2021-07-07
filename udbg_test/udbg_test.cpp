#include <stdio.h>
#include <excpt.h>
#include <windows.h>
#include <intrin.h>
#include <cassert>
#include <ctime>

#include "udbgrw.h"

bool is_atom_core()
{
	const __int8 atom_models[] = { 0x9c, 0x96, 0x8a, 0x7a, 0x5f, 0x5c, 0x4c, 0x5d,
								   0x5a, 0x4d, 0x4a, 0x37, 0x36, 0x35, 0x27, 0x26, 0x1c };

	int cpuinfo[4];
	__cpuid(cpuinfo, 1);

	unsigned __int8 family = (cpuinfo[0] & 0xf00) >> 8;
	if (family != 0x06)
		return false;

	unsigned __int8 model = ((cpuinfo[0] & 0xf0000) >> 12) | ((cpuinfo[0] & 0xf0) >> 4);
	for (int i = 0; i < sizeof atom_models / sizeof atom_models[0]; i++)
		if (model == atom_models[i])
			return true;

	return false;
}

unsigned int get_rdtsc_multiplier_uram_addr_bigcore()
{
	return 0x7f2;
}

unsigned int get_rdtsc_multiplier_uram_addr_atom()
{
	return 0x087;
}

unsigned int get_rdtsc_multiplier_uram_addr()
{
	if (is_atom_core())
		return get_rdtsc_multiplier_uram_addr_atom();
	else
		return get_rdtsc_multiplier_uram_addr_bigcore();
}

unsigned int get_last_branch_0_from_ip_crbus_addr_bigcore()
{
	return 0x4f0;
}

unsigned int get_last_branch_0_from_ip_crbus_addr_atom()
{
	return 0x040;
}

unsigned int get_last_branch_0_from_ip_crbus_addr()
{
	if (is_atom_core())
		return get_last_branch_0_from_ip_crbus_addr_atom();
	else
		return get_last_branch_0_from_ip_crbus_addr_bigcore();
}

int test_udbgrw_simple()
{
	const unsigned crbus_addr = get_last_branch_0_from_ip_crbus_addr();
	bool udbgwr_active = false;
	__try
	{
		unsigned __int64 crbus_data;
		udbgwr(UDBG_CMD_CRBUS, crbus_addr, 0xdeadbeefdeadbeef);
		udbgwr_active = true;
		udbgrd(UDBG_CMD_CRBUS, crbus_addr, &crbus_data);

		wprintf(L"[ALERT] udbgrd/udbgwr instructions are active!!!\n");
		return -1;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (udbgwr_active)
		{
			wprintf(L"[ALERT] udbgwr instruction is active!!!\n");
			return -1;
		}
		int except_code = GetExceptionCode();
		if (except_code != EXCEPTION_ILLEGAL_INSTRUCTION)
		{
			wprintf(L"[ALERT] Instructions raised unexpected exception: code = 0x%08x\n", except_code);
			return -1;
		}
		wprintf(L"[OK] Instructions aren't activated\n");
	}

	return 0;
}

const int CACHE_LINE_SIZE = 0x40;
const int SC_LINE_SIZE = CACHE_LINE_SIZE * 8;
__declspec(align(CACHE_LINE_SIZE))
	unsigned char g_sidechan_buf[SC_LINE_SIZE * 0x100];
__declspec(align(CACHE_LINE_SIZE))
	unsigned char g_rob_lock_buf[CACHE_LINE_SIZE];
const unsigned g_sc_buf_line_cnt = sizeof g_sidechan_buf / SC_LINE_SIZE;
unsigned g_cached_mem_access_treshold_ts;

void measure_cached_access()
{
	const int iter_count = 0x10000;
	__declspec(align(CACHE_LINE_SIZE))
		unsigned char buffer[CACHE_LINE_SIZE] = {};
	unsigned __int64 avg_cached_mem_access_ts = 0;
	register unsigned int proc_id;

	volatile unsigned char val = buffer[0];
	for (int iter = 0; iter < iter_count; ++iter)
	{
		unsigned __int64 ld_start_ts = __rdtscp(&proc_id);
		_mm_lfence();
		volatile unsigned char val = buffer[0];
		unsigned __int64 ld_end_ts = __rdtscp(&proc_id);
		avg_cached_mem_access_ts = (avg_cached_mem_access_ts + (ld_end_ts - ld_start_ts)) / 2;
	}

	g_cached_mem_access_treshold_ts = avg_cached_mem_access_ts + avg_cached_mem_access_ts / 10;
}

inline unsigned get_sc_buf_idx(unsigned line_idx)
{
	return SC_LINE_SIZE * line_idx;
}

inline bool measure_sc_buf_line_access(unsigned line_idx)
{
	register unsigned int proc_id;
	unsigned __int64 ld_start_ts = __rdtscp(&proc_id);
	_mm_lfence();
	volatile unsigned char val = g_sidechan_buf[get_sc_buf_idx(line_idx)];
	unsigned __int64 ld_end_ts = __rdtscp(&proc_id);

	assert(g_cached_mem_access_treshold_ts > 0);
	if ((ld_end_ts - ld_start_ts) <= g_cached_mem_access_treshold_ts)
		return true;
	return false;
}

int get_cached_sc_buf_line_idx()
{
	const int idx_mask = g_sc_buf_line_cnt - 1;
	for (int idx = 0; idx < g_sc_buf_line_cnt; ++idx)
	{
		int line_idx = ((idx * 167) + 13) & idx_mask;
		if (measure_sc_buf_line_access(line_idx))
			return line_idx;
	}
	return -1;
}

int udbgwr_test_speculative()
{
	const int persist_err_test_max_iter_count = 0x10000;
	const int temp_err_test_max_iter_count = 0x100000;
	unsigned int uram_addr = get_rdtsc_multiplier_uram_addr();

	// Test for persisten errors in speculative uops execution
	for (int iter = 0; iter < persist_err_test_max_iter_count; ++iter)
	{
		unsigned __int64 start_ts = __rdtsc();
		__try
		{
			// Test for udbgrd also due to the possible taken path to write
			if (rand() % 2 == 0)
				udbgwr(UDBG_CMD_URAM, uram_addr, 0);
			else
				udbgrd(UDBG_CMD_URAM, uram_addr, 0);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			if (__rdtsc() < start_ts)
			{
				wprintf(L"[ALERT] udbgwr persistent speculative execution problem is discovered!!!\n");
				return -1;
			}
		}
	}

	// Test for temporal errors in speculative uops execution
	int sc_buf_stats[2] = {};
	int sc_buf_line_idxes[2] = {0x37, 0xc9};
	for (int iter = 0; iter < temp_err_test_max_iter_count; ++iter)
	{
		assert(g_sc_buf_line_cnt >= 2);
		for (int line_idx = 0; line_idx < 2; ++line_idx)
			_mm_clflush(&g_sidechan_buf[get_sc_buf_idx(line_idx)]);
		_mm_clflush(&g_rob_lock_buf[0]);

		register int cpu_info[4];
		__cpuid(cpu_info, 0);
		
		register unsigned int proc_id;
		__int64 start_ts = __rdtscp(&proc_id);
		_mm_lfence();
		volatile unsigned char val = g_rob_lock_buf[0];
		__try
		{
			if (rand() % 2 == 0)
				udbgwr(UDBG_CMD_URAM, uram_addr, 0);
			else
				udbgrd(UDBG_CMD_URAM, uram_addr, 0);

			int new_tsc_less = __rdtsc() < start_ts;
			int sc_buf_line_idx = sc_buf_line_idxes[new_tsc_less];
			volatile unsigned char val2 = g_sidechan_buf[get_sc_buf_idx(sc_buf_line_idx)];
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			int sc_line_idx = get_cached_sc_buf_line_idx();
			if (sc_line_idx == sc_buf_line_idxes[0] || sc_line_idx == sc_buf_line_idxes[1])
				sc_buf_stats[sc_line_idx == sc_buf_line_idxes[1]]++;
		}
	}

	int res = 0;
	const int hit_count_treshold = temp_err_test_max_iter_count / 100;
	if (sc_buf_stats[1] >= hit_count_treshold)
	{
		wprintf(L"[ALERT] udbgwr temporal speculative execution problem is discovered!!!\n");
		res = -1;
	}
	else
	{
		wprintf(L"[OK] There were not found problems with udbgwr speculative execution\n");
		res = 0;
	}

	if (sc_buf_stats[0] >= hit_count_treshold)
	{
		wprintf(L"[INFO] Speculative execution behind udbgwr is detected\n");
	}
	return res;
}

int udbgrd_test_speculative()
{
	const int max_iter_count = 0x10000;
	unsigned int uram_addr = get_rdtsc_multiplier_uram_addr();

	int sc_buf_stats[0x100] = {};
	const int sc_buf_stats_len = sizeof sc_buf_stats / sizeof sc_buf_stats[0];
	for (int iter = 0; iter < max_iter_count; ++iter)
	{
		for (int line_idx = 0; line_idx < g_sc_buf_line_cnt; ++line_idx)
			_mm_clflush(&g_sidechan_buf[get_sc_buf_idx(line_idx)]);
		_mm_clflush(&g_rob_lock_buf[0]);

		_mm_lfence();

		volatile unsigned char val = g_rob_lock_buf[0];
		__try
		{
			unsigned __int64 tsc_multiplier = 0;
			udbgrd(UDBG_CMD_URAM, uram_addr, &tsc_multiplier);
			_mm_lfence();
			volatile unsigned char val2 = g_sidechan_buf[get_sc_buf_idx(tsc_multiplier & 0xff)];
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			int sc_line_idx = get_cached_sc_buf_line_idx();
			if (sc_line_idx != -1)
			{
				assert(sc_line_idx < sc_buf_stats_len);
				sc_buf_stats[sc_line_idx]++;
			}
		}
	}

	const int hit_count_treshold = max_iter_count / 100;
	int tsc_multiplier = -1;
	for (int idx = 0; idx < sc_buf_stats_len; ++idx)
	{
		if (sc_buf_stats[idx] >= hit_count_treshold)
		{
			tsc_multiplier = idx;
			break;
		}
	}

	int res = 0;
	if (tsc_multiplier != -1)
	{
		wprintf(L"[ALERT] udbgrd speculative execution problem is discovered!!!\n"
			" Read TSC Multiplier value: 0x%02x: Hit Count: 0x%02x\n", tsc_multiplier, sc_buf_stats[tsc_multiplier]);
		res = -1;

		for (int idx = 0; idx < sc_buf_stats_len; ++idx)
		{
			wprintf(L"%02x-%02x: ", idx, sc_buf_stats[idx]);
		}
		wprintf(L"\n");
	}
	else
	{
		wprintf(L"[OK] There was not found a problem with udbgrd speculative execution\n");
	}
	return res;
}

int main()
{
	const wchar_t* str_core[] = { L"Big", L"Atom" };
	wprintf(L"[INFO] %s Core is detected\n", str_core[is_atom_core()]);

	srand(time(NULL));
	measure_cached_access();
	wprintf(L"[INFO] Cached read treshold ts: 0x%x\n", g_cached_mem_access_treshold_ts);

	int res = 0;
	res = test_udbgrw_simple();
	if (res != 0)
		return res;

	res = udbgwr_test_speculative();
	res |= udbgrd_test_speculative();
	return res;
}
