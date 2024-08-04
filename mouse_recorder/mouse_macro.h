#pragma once

#include "date_time_utils.h"

// Represents a mouse input
struct mouse_input
{
	int id; //step ip
	int x; // x coordenate
	int y; // y coordenate
	int wait_to_click_ms; // time to sleep
};

enum scale_input_type
{
	NONE,
	SCALE_FACTOR,
	DPI
};

// flag to start record clicks
static bool start_record = false;
// flag to play current steps recorded
static bool play_current_macro = false;
// flat to play current steps infinite
static bool play_current_macro_infinite = false;
// to calculate time between steps
static std::chrono::system_clock::time_point start_time;
// current step id (its a counter)
static int input_counter = 1;
// current recorded steps
static std::vector<mouse_input*> inputs;
// steps file
static const char* file_name = "steps.txt";
// handling file
static bool is_handling_file = false;

// scale mouse input coordenates
static int get_input_by_scale(int value, bool is_x)
{
	if (current_config.scale_factor <= 100) return value;
	if (current_config.is_dpi_aware) return value; //== NONE
	//float factor = static_cast<float>((current_config.scale_factor - DEVICE_SCALE_FACTOR::SCALE_100_PERCENT) / 100.f);

	//not sure
	float factor = static_cast<float>(is_x ? current_config.calculated_scale_factor_x : current_config.calculated_scale_factor_y - 1.0f);
	if (factor == 1.f || factor == 0.0f) return value;
	float to_scale = value * factor;
	auto final_value = static_cast<int>(ceil(value - to_scale));

	std::cout << "get_input_by_scale-> value: " << value
		<< ", scale_factor: " << current_config.scale_factor
		<< ", factor: " << factor
		<< ", to_scale: " << to_scale
		<< ", final_value: " << final_value
		<< std::endl;

	return final_value;

}

static int scale_input(int value, scale_input_type scale_type, bool is_x)
{
	if (current_config.win_dpi == 0.f) return value;

	switch (scale_type)
	{
	case NONE:
		return static_cast<int>(value);
	case SCALE_FACTOR:
		return get_input_by_scale(value, is_x);
	case DPI:
		return static_cast<int>(ceil(value * current_config.win_dpi / 96.f));
	}

	return static_cast<int>(value);
}

static int get_input_id()
{
	return input_counter++;
}

static void reset_input_id()
{
	input_counter = 1;
}

// clean up to record again
static void reset_inputs()
{
	for (mouse_input* input : inputs)
	{
		delete input;
	}
	inputs.clear();
	reset_input_id();
}

static void add_input(LONG x, LONG y)
{
	auto end_time = std::chrono::system_clock::now();
	auto diff = std::chrono::duration<double, std::milli>(end_time - start_time).count();
	start_time = end_time;
	auto in = new mouse_input();
	in->id = get_input_id();
	in->x = scale_input(x, SCALE_FACTOR, true);
	in->y = scale_input(y, SCALE_FACTOR, false);
	in->wait_to_click_ms = in->id == 1 ? 0 : static_cast<int>(diff);

	inputs.emplace_back(in);
	std::cout << "+ add_input -> id: " << in->id << ", x: " << in->x << ", y: " << in->y << ", WaitToClick: " << in->wait_to_click_ms << std::endl;
}

// emulate a left click in current position
static void do_left_click()
{
	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &input, sizeof(INPUT));

	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &input, sizeof(INPUT));
}

// play current steps
static void play_macro()
{
	if (!play_current_macro) return;
	if (inputs.size() == 0)
	{
		play_current_macro = false;
		std::cout << ">! Nothing to play." << std::endl;
		return;
	}

	if (current_config.is_debug)
	{
		std::cout << ">! Steps to execute:" << inputs.size() << std::endl;
	}

	std::cout << "Playing:\t" << std::boolalpha << play_current_macro << std::endl;
	auto start_tick = std::chrono::system_clock::now();
	std::cout << "START:\t" << date_time_utils::get_datetime_now() << std::endl;

	std::uint64_t current_execution = 1;
	do
	{
		for (mouse_input* in : inputs)
		{
			if (play_current_macro == false) break;
			if (current_config.is_debug)
			{
				std::cout << "- Step id: " << in->id << ", x: " << in->x << ", y: " << in->y << ", WaitToClick: " << in->wait_to_click_ms << std::endl;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(in->wait_to_click_ms));
			if (play_current_macro == false) break;
			SetCursorPos(in->x, in->y);
			do_left_click();
		}
		std::cout << "\rExecuted:\t\t" << current_execution;
		current_execution++;

		//dummy sleep
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if (play_current_macro == false) break;
	} while (play_current_macro_infinite);

	play_current_macro = false;
	auto end_tick = std::chrono::system_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end_tick - start_tick);
	auto formatted = date_time_utils::format_duration(diff);
	std::cout << std::endl;
	std::cout << "END:\t" << date_time_utils::get_datetime_now() << "\t" << formatted << std::endl;
	std::cout << "Playing:\t" << std::boolalpha << play_current_macro << std::endl;
}

static void stop_macro()
{
	play_current_macro = false;
}

static bool save_current_steps_to_file()
{
	if (play_current_macro) return false;
	if (inputs.size() == 0)
	{
		std::cout << ">! Nothing to save." << std::endl;
		return false;
	}

	is_handling_file = true;
	std::cout << ">! Saving file:\t" << file_name << std::endl;

	FILE* p_file = nullptr;
	errno_t file_status = fopen_s(&p_file, file_name, "w+");
	if (p_file == nullptr)
	{
		is_handling_file = false;
		std::cout << ">! Saving file:\t" << "FAILED" << std::endl;
		return false;
	}

	for (mouse_input* in : inputs)
	{
		fprintf(p_file, "%d|%d|%d|%d\r\n", in->id, in->x, in->y, in->wait_to_click_ms);
	}

	fclose(p_file);
	std::cout << ">! Saving file:\t" << "SUCCESS" << std::endl;
	is_handling_file = false;
	return true;
}

static void load_current_step_from_file()
{
	reset_inputs();
	is_handling_file = true;
	std::cout << ">! Reading file:\t" << file_name << std::endl;

	FILE* p_file = nullptr;
	errno_t file_status = fopen_s(&p_file, file_name, "r");
	if (p_file == nullptr)
	{
		is_handling_file = false;
		std::cout << ">! Reading file:\t" << "FAILED" << std::endl;
		return;
	}

	fseek(p_file, 0, SEEK_END);
	const auto file_size = static_cast<size_t>(ftell(p_file));
	fseek(p_file, 0, SEEK_SET);

	if (file_size == 0)
	{
		fclose(p_file);
		is_handling_file = false;
		std::cout << ">! Reading file:\t" << "FAILED[EMPTY]" << std::endl;
		return;
	}

	char* p_buffer = reinterpret_cast<char*>(malloc(file_size + 1));
	if (p_buffer == nullptr)
	{
		fclose(p_file);
		is_handling_file = false;
		std::cout << ">! Reading file:\t" << "FAILED[NO_MEMORY]" << std::endl;
		return;
	}

	const auto read = fread(p_buffer, file_size, 1, p_file);
	const size_t len = strlen(p_buffer);
	if (len == 0)
	{
		free(p_buffer);
		fclose(p_file);
		std::cout << ">! Reading file:\t" << "FAILED[EMPTY]" << std::endl;
		return;
	}
	p_buffer[file_size] = '\0';

	char* file_context = nullptr;
	char* line = strtok_s(p_buffer, "\r\n", &file_context);
	while (line != nullptr)
	{
		char* line_context = nullptr;
		char* value = strtok_s(line, "|", &line_context);

		if (value != nullptr)
		{
			mouse_input* in = new mouse_input();
			in->id = std::atoi(value);
			const auto s_x = strtok_s(nullptr, "|", &line_context);
			const auto s_y = strtok_s(nullptr, "|", &line_context);
			const auto wait_to_click = strtok_s(nullptr, "|", &line_context);
			if (s_x == nullptr || s_y == nullptr || wait_to_click == nullptr)
			{
				delete in;
			}
			else
			{
				in->x = std::atoi(s_x);
				in->y = std::atoi(s_y);
				in->wait_to_click_ms = std::atoi(wait_to_click);
				inputs.emplace_back(in);
			}
		}

		line = strtok_s(nullptr, "\r\n", &file_context);
	}

	fclose(p_file);
	free(p_buffer);
	is_handling_file = false;
	std::cout << "Steps loaded from file: " << inputs.size() << std::endl;
	std::cout << ">! Reading file:\t" << "SUCCESS" << std::endl;
	return;
}