/** \file
 * \brief Windows System Information
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h> 

/* This module should depend only on IUP core headers 
   and Windows system headers. */

#include <windows.h>

#include "iup_str.h"
#include "iup_drvinfo.h"
#include "iup_varg.h"


#ifdef _MSC_VER
/* warning C4996: 'GetVersionExW': was declared deprecated */
#pragma warning( disable : 4996 )
#endif

char* iupdrvLocaleInfo(void)
{
  CPINFOEXA info;
  GetCPInfoExA(CP_ACP, 0, &info);
  return iupStrReturnStr(info.CodePageName);
}

/* TODO: Since Windows 8.1/Visual Studio 2013 GetVersionEx is deprecated. 
         We can replace it using GetProductInfo. But for now leave it. */

char *iupdrvGetSystemName(void)
{
  OSVERSIONINFOA osvi;
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
  GetVersionExA(&osvi);

  if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    if (osvi.dwMajorVersion <= 4)
      return "WinNT";

    if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
      return "Win2K";

    if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion > 0)
      return "WinXP";

    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
      return "Vista";

    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
      return "Win7";

    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2)
      return "Win8";

    /* IMPORTANT: starting here will work only if the Manifest has been changed 
       to include Windows 8+ support. Otherwise GetVersionEx will report 6.2 */

    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3)
      return "Win81";

    if (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0)
      return "Win10";
  }

  return "Windows";
}

char *iupdrvGetSystemVersion(void)
{
  char *str = iupStrGetMemory(256);
  OSVERSIONINFOEXA osvi;
  SYSTEM_INFO si;

  ZeroMemory(&si, sizeof(SYSTEM_INFO));
  GetSystemInfo(&si);

  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
  GetVersionExA((OSVERSIONINFOA*)&osvi);

  sprintf(str, "%d.%d.%d", (int)osvi.dwMajorVersion, (int)osvi.dwMinorVersion, (int)osvi.dwBuildNumber);

  /* Display service pack (if any). */
  if (osvi.szCSDVersion[0] != 0)
  {
    strcat(str, " ");
    strcat(str, osvi.szCSDVersion);
  }

  if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
    strcat(str, " (IA64)");
  else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
    strcat(str, " (x64)");
  else
    strcat(str, " (x86)");

  return str;
}

int iupdrvSetCurrentDirectory(const char* path)
{
  return SetCurrentDirectoryA(path);
}

char* iupdrvGetCurrentDirectory(void)
{
  char* cur_dir = NULL;

  int len = GetCurrentDirectoryA(0, NULL);
  if (len == 0) return NULL;

  cur_dir = iupStrGetMemory(len + 2);
  GetCurrentDirectoryA(len + 1, cur_dir);
  cur_dir[len] = '\\';
  cur_dir[len + 1] = 0;

  return cur_dir;
}

void IupLogV(const char* type, const char* format, va_list arglist)
{
  HANDLE EventSource;
  WORD wtype = 0;

  int size;
  char* value = iupStrGetLargeMem(&size);
  vsnprintf(value, size, format, arglist);

  if (iupStrEqualNoCase(type, "DEBUG"))
  {
    OutputDebugStringA(value);
    return;
  }
  else if (iupStrEqualNoCase(type, "ERROR"))
    wtype = EVENTLOG_ERROR_TYPE;
  else if (iupStrEqualNoCase(type, "WARNING"))
    wtype = EVENTLOG_WARNING_TYPE;
  else if (iupStrEqualNoCase(type, "INFO"))
    wtype = EVENTLOG_INFORMATION_TYPE;

  EventSource = RegisterEventSourceA(NULL, "Application");
  if (EventSource)
  {
    ReportEventA(EventSource, wtype, 0, 0, NULL, 1, 0, &value, NULL);
    DeregisterEventSource(EventSource);
  }
}

void IupLog(const char* type, const char* format, ...)
{
  va_list arglist;
  va_start(arglist, format);
  IupLogV(type, format, arglist);
  va_end(arglist);
}
