#include <windows.h>
#include <uva-ui/app.hpp>

uva::lang::ui::app* uvaapp = nullptr;

HINSTANCE hInstance = nullptr;
HINSTANCE hPrevInstance = nullptr;
LPSTR lpCmdLine = nullptr;
int nCmdShow = 0;

uva::lang::ui::app::app(std::string_view __name, std::string_view vendor)
{
}

uva::lang::ui::app::~app()
{
    set_theme(nullptr);
}

int uva::lang::ui::app::run(int _argc, char** _argv)
{
    on_init(_argc, _argv);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}