#pragma once

#include <windows.h>
#include "tnetwork.h"

class Tnclip {
private:
	HWND Window;
	TNetwork &Network;

public:
	Tnclip(HWND Window, TNetwork &RefNetwork);
	~Tnclip();

	void Copy(HGLOBAL clipboard_data);
	void Paste();
};
