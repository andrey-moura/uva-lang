// winmain.cpp - Windows Entry Point
#ifdef _WIN32  // Check if Windows
    #include <windows.h>

    extern int uva_main(int argc, char *argv[]);

    int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
        return uva_main(0, nullptr);
    }
#endif