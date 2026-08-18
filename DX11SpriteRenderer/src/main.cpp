#include "core/App.h"

int main()
{
	App app;
	if (!app.Initialize())
	{
		return -1;
	}

	app.Run();
	return 0;
}
