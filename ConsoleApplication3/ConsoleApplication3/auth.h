#ifdef DLL_API
#else
#define DLL_API __declspec(dllexport)
#endif
 DLL_API int Authentication(const char *UserName, const char *Password, const char *DeviceName);