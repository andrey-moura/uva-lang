#define UVA_IMPLEMENT_APP(x)\
extern HINSTANCE hInstance;\
extern HINSTANCE hPrevInstance;\
extern LPSTR lpCmdLine;\
extern int nCmdShow;\
int WINAPI WinMain(HINSTANCE __hInstance, HINSTANCE __hPrevInstance, LPSTR __lpCmdLine, int __nCmdShow)\
{\
    uva::lang::ui::app * uvaapp = new x();\
    hInstance = __hInstance;\
    hPrevInstance = __hPrevInstance;\
    lpCmdLine = __lpCmdLine;\
    nCmdShow = __nCmdShow;\
    return uvaapp->run(__argc, __argv);\
}