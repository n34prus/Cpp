#pragma once

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"

class UI
{
public:
	enum class Mode
	{
		None,
		Console,
		ImGui
	};

	UI(Mode mode) : mode{ mode } {}

    template <typename... Args>
    void print(Args&&... args)
    {
        if (mode == Mode::Console) {
            // cout
            (std::cout << ... << std::forward<Args>(args));
        }
        else {
            // ImGui buffer
            std::ostringstream oss;
            (oss << ... << std::forward<Args>(args));
            log += oss.str();
        }
    }

	HANDLE InitConsole(short width, short height)
	{
		if (hwnd) return hwnd;

		AllocConsole(); // call FreeConsole() to disable

		hwnd = CreateConsoleScreenBuffer(
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr,
			CONSOLE_TEXTMODE_BUFFER,
			nullptr
		);
		SetConsoleActiveScreenBuffer(hwnd);

		FILE* f;
		freopen_s(&f, "CONOUT$", "w", stdout);
		freopen_s(&f, "CONOUT$", "w", stderr);
		freopen_s(&f, "CONIN$", "r", stdin);

		constexpr int fontSize = 10;

		CONSOLE_FONT_INFOEX fontInfo;
		fontInfo.cbSize = sizeof(fontInfo);
		GetCurrentConsoleFontEx(hwnd, TRUE, &fontInfo);
		fontInfo.dwFontSize.X = fontSize;
		fontInfo.dwFontSize.Y = fontSize;
		SetCurrentConsoleFontEx(hwnd, TRUE, &fontInfo);
		COORD bufferSize = { static_cast<short>(width * 2), static_cast<short>(height * 2) };
		SetConsoleScreenBufferSize(hwnd, bufferSize);

		if (width > 64 && height > 64)
		{
			RECT r;
			GetWindowRect(GetConsoleWindow(), &r);
			MoveWindow(GetConsoleWindow(), r.left, r.top, (fontSize + 1) * (width), (fontSize + 1) * height, TRUE);
		}

		return hwnd;
	}

	HANDLE GetConsole() const { return hwnd; }

	void clear()
	{
		if (mode == Mode::Console) { system("cls"); }
		else { log.clear(); }
	}

    const std::string& getImGuiText() const { return log; }
	const Mode& getMode() const { return mode; }

private:
    std::string log;
    Mode mode = Mode::Console;
	HANDLE hwnd = nullptr;
};