#pragma once

#include <Windows.h>
#include <shellscalingapi.h>

struct program_config
{
	/// <summary>
	/// Windows Font Scale factor
	/// </summary>
	DEVICE_SCALE_FACTOR scale_factor = DEVICE_SCALE_FACTOR::DEVICE_SCALE_FACTOR_INVALID;
	float calculated_scale_factor_x = 0.f;
	float calculated_scale_factor_y = 0.f;

	/// <summary>
	/// Window DPI factor
	/// </summary>
	float win_dpi = 0.f;

	int monitor_width = 0;
	int monitor_height = 0;

	/// <summary>
	/// This should be used for coordenates scale
	/// </summary>
	bool is_dpi_aware = false;

	/// <summary>
	/// Enable more logs
	/// </summary>
	bool is_debug = false;
};

// program configuration
static program_config current_config;

static void set_dpi_aware(bool debug)
{
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	BOOL set = SetProcessDPIAware();
	if (debug)
	{
		std::cout << "set_dpi_aware ->\t\tset\t\t\t" << std::boolalpha << set << std::endl;
	}
	current_config.is_dpi_aware = set == TRUE;
}

// read scale factors
static void get_monitor_scale_factor(bool debug)
{
	POINT dummy_point = { 0,0 };
	HMONITOR p_monitor = MonitorFromPoint(dummy_point, MONITOR_DEFAULTTONEAREST);

	MONITORINFOEX monitor_info;
	monitor_info.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(p_monitor, &monitor_info);

	current_config.monitor_width = (monitor_info.rcMonitor.right - monitor_info.rcMonitor.left);
	current_config.monitor_height = (monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top);

	DEVMODE dm;
	dm.dmSize = sizeof(dm);
	dm.dmDriverExtra = 0;
	EnumDisplaySettings(monitor_info.szDevice, ENUM_CURRENT_SETTINGS, &dm);
	int cxPhysical = dm.dmPelsWidth;
	int cyPhysical = dm.dmPelsHeight;

	// Calculate the scaling factor.
	double horzScale = ((double)cxPhysical / (double)current_config.monitor_width);
	double vertScale = ((double)cyPhysical / (double)current_config.monitor_height);

	UINT dpiX, dpiY;
	HRESULT temp2 = GetDpiForMonitor(p_monitor, MDT_DEFAULT, &dpiX, &dpiY);
	current_config.calculated_scale_factor_x = static_cast<float>(dpiX / 96.0f);
	current_config.calculated_scale_factor_y = static_cast<float>(dpiY / 96.0f);

	current_config.win_dpi = static_cast<float>(GetDpiForWindow(GetDesktopWindow()));
	HRESULT res = GetScaleFactorForMonitor(p_monitor, &current_config.scale_factor);

	//float scale_2 = current_config.win_dpi / 96.f;
	//std::cout << "get_monitor_scale_factor ->\tscale_2\t\t" << scale_2 << std::endl;

	int x = GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, (UINT)current_config.win_dpi);
	int y = GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, (UINT)current_config.win_dpi);
	if (debug)
	{
		std::cout << "get_monitor_scale_factor ->\tscale_factor\t\t" << current_config.scale_factor << std::endl;
		std::cout << "get_monitor_scale_factor ->\twin_dpi\t\t\t" << current_config.win_dpi << std::endl;
		std::cout << "get_monitor_scale_factor ->\tSM_CXVIRTUALSCREEN\t" << x << std::endl;
		std::cout << "get_monitor_scale_factor ->\tSM_CYVIRTUALSCREEN\t" << y << std::endl;
	}

	// (144 / 96 == 1.5)
	/*
	*
	*/
}

static void read_configuration()
{
	set_dpi_aware(current_config.is_debug);
	get_monitor_scale_factor(current_config.is_debug);
}