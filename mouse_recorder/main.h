#pragma once
#pragma comment(lib, "Shcore.lib")

#include <Windows.h>
#include <shellscalingapi.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include "config.h"
#include "mouse_macro.h"

// CTRL + F8 => STAR RECORD and STOP RECORD
#define HOT_KEY_MSG_START_STOP_RECORD WM_USER + 0x0001
// CTRL + F9 => PLAY and STOP
#define HOT_KEY_MSG_PLAY_STOP_MACRO WM_USER + 0x0002
// CTRL + F10 => PLAY and STOP inifinite
#define HOT_KEY_MSG_PLAY_STOP_MACRO_INFINITE WM_USER + 0x0003
// CTRL + F11 (F12) => Save (load) current inputs to file
#define HOT_KEY_MSG_SAVE_TO_FILE WM_USER + 0x0004
#define HOT_KEY_MSG_LOAD_TO_FILE WM_USER + 0x0005

// Global hook pointer
static HHOOK p_hook = nullptr;

// Exit app handler
BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType);

// Handle window hook
LRESULT CALLBACK LowLevelMouseProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam);

// set global shortcuts
static bool register_hot_Keys()
{
	auto done = RegisterHotKey(nullptr, HOT_KEY_MSG_START_STOP_RECORD, MOD_CONTROL, VK_F8);
	if (done == FALSE)
	{
		std::cout << "Could not install shortcut to record macros." << std::endl;
		return false;
	}

	done = RegisterHotKey(nullptr, HOT_KEY_MSG_PLAY_STOP_MACRO, MOD_CONTROL, VK_F9);
	if (done == FALSE)
	{
		UnregisterHotKey(nullptr, HOT_KEY_MSG_START_STOP_RECORD);
		std::cout << "Could not install shortcut to play macros." << std::endl;
		return false;
	}

	done = RegisterHotKey(nullptr, HOT_KEY_MSG_PLAY_STOP_MACRO_INFINITE, MOD_CONTROL, VK_F10);
	if (done == FALSE)
	{
		UnregisterHotKey(nullptr, HOT_KEY_MSG_START_STOP_RECORD);
		UnregisterHotKey(nullptr, HOT_KEY_MSG_PLAY_STOP_MACRO);
		std::cout << "Could not install shortcut to play macros (infinite)." << std::endl;
		return false;
	}

	done = RegisterHotKey(nullptr, HOT_KEY_MSG_SAVE_TO_FILE, MOD_CONTROL, VK_F11);
	if (done == FALSE)
	{
		UnregisterHotKey(nullptr, HOT_KEY_MSG_START_STOP_RECORD);
		UnregisterHotKey(nullptr, HOT_KEY_MSG_PLAY_STOP_MACRO);
		UnregisterHotKey(nullptr, HOT_KEY_MSG_PLAY_STOP_MACRO_INFINITE);
		std::cout << "Could not install shortcut to save to file." << std::endl;
		return false;
	}

	done = RegisterHotKey(nullptr, HOT_KEY_MSG_LOAD_TO_FILE, MOD_CONTROL, VK_F12);
	if (done == FALSE)
	{
		UnregisterHotKey(nullptr, HOT_KEY_MSG_START_STOP_RECORD);
		UnregisterHotKey(nullptr, HOT_KEY_MSG_PLAY_STOP_MACRO);
		UnregisterHotKey(nullptr, HOT_KEY_MSG_PLAY_STOP_MACRO_INFINITE);
		UnregisterHotKey(nullptr, HOT_KEY_MSG_SAVE_TO_FILE);
		std::cout << "Could not install shortcut to load file." << std::endl;
		return false;
	}

	return true;

}

// unregister global shortcuts
static void unregister_hot_keys()
{
	UnregisterHotKey(nullptr, HOT_KEY_MSG_START_STOP_RECORD);
	UnregisterHotKey(nullptr, HOT_KEY_MSG_PLAY_STOP_MACRO);
	UnregisterHotKey(nullptr, HOT_KEY_MSG_PLAY_STOP_MACRO_INFINITE);
	UnregisterHotKey(nullptr, HOT_KEY_MSG_SAVE_TO_FILE);
	UnregisterHotKey(nullptr, HOT_KEY_MSG_LOAD_TO_FILE);
}

// handle shortcut event
void handle_hot_keys(MSG msg)
{
	if (msg.message != WM_HOTKEY) return;
	if (is_handling_file) return;

	if (msg.wParam == HOT_KEY_MSG_START_STOP_RECORD)
	{
		if (play_current_macro) return;
		start_record = !start_record;
		if (start_record)
		{
			reset_inputs();
			reset_input_id();
		}

		std::cout << "Recording:\t" << std::boolalpha << start_record << std::endl;
	}
	else if (msg.wParam == HOT_KEY_MSG_PLAY_STOP_MACRO)
	{
		if (start_record) return;
		play_current_macro = !play_current_macro;

		// execute from different thread
		std::thread play_thread(play_macro);
		play_thread.detach();
	}
	else if (msg.wParam == HOT_KEY_MSG_PLAY_STOP_MACRO_INFINITE)
	{
		if (start_record) return;
		if (play_current_macro) return;
		play_current_macro_infinite = !play_current_macro_infinite;
		std::cout << "Infinite:\t" << std::boolalpha << play_current_macro_infinite << std::endl;
	}
	else if (msg.wParam == HOT_KEY_MSG_SAVE_TO_FILE)
	{
		if (start_record) return;
		if (play_current_macro) return;
		save_current_steps_to_file();
	}
	else if (msg.wParam == HOT_KEY_MSG_LOAD_TO_FILE)
	{
		if (start_record) return;
		if (play_current_macro) return;
		load_current_step_from_file();
	}
}

// show message
static void show_initial_message()
{
	std::cout << "Shortcuts" << std::endl;
	std::cout << "1. RECORD (start and stop):\t" << "CTRL + F8" << std::endl;
	std::cout << "2. PLAY (start and stop):\t" << "CTRL + F9" << std::endl;
	std::cout << "3. SET PLAY INFINITE:\t\t" << "CTRL + F10" << std::endl;
	std::cout << "4. file -> save(overwrite):\t" << "CTRL + F11" << std::endl;
	std::cout << "5. file -> load:\t\t" << "CTRL + F12" << std::endl;
}