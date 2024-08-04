#include "main.h"

// debug
static bool install_hook = true;

static bool debug_messages = false;

int main()
{
	if (SetConsoleCtrlHandler(HandlerRoutine, TRUE) == 0)
		return ~0;

	current_config.is_debug = debug_messages;
	read_configuration();
	load_current_step_from_file();
	show_initial_message();

	if (install_hook)
	{
		p_hook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, nullptr, 0);
		if (p_hook == nullptr)
			return ~0;

		if (!register_hot_Keys()) return 0;
	}

	MSG msg = { 0 };
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		{
			handle_hot_keys(msg);
		}
	}

	if (install_hook)
	{
		if (p_hook != nullptr)
			UnhookWindowsHookEx(p_hook);

		unregister_hot_keys();		
	}

	reset_inputs();

	return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK LowLevelMouseProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		auto m_struct = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
		//std::cout << "x: " << m_struct->pt.x << " y: " << m_struct->pt.y << std::endl;
		if (wParam == WM_LBUTTONUP && start_record)
		{
			add_input(m_struct->pt.x, m_struct->pt.y);
			//std::cout << "Left click" << std::endl;
		}
	}
	return CallNextHookEx(p_hook, nCode, wParam, lParam);
}

BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_CLOSE_EVENT || dwCtrlType == CTRL_SHUTDOWN_EVENT)
	{
		if (UnhookWindowsHookEx(p_hook) == TRUE)
			p_hook = nullptr;

		play_current_macro = false;
		start_record = false;
		reset_inputs();
		reset_input_id();
		unregister_hot_keys();
	}
	return TRUE;
}