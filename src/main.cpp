#include "Application.h"
#include "Editor/EditorApp.h"

#define EDITOR_APP

int main(int, char**)
{
#ifdef EDITOR_APP
	Application* app = new Editor::EditorApp();
#else
	Application* app = new Application();
#endif
	app->run();
	delete app;
}
