#include <windows.h>
#include <stdio.h>

int main()
{
    FILETIME ft;
    SYSTEMTIME st;

    // Get system time as FILETIME
    GetSystemTimeAsFileTime(&ft);

    // Convert FILETIME to SYSTEMTIME
    FileTimeToSystemTime(&ft, &st);

    // Print in format YYYY/MM/DD HH:MM:SS
    printf("%04d/%02d/%02d %02d:%02d:%02d\n",
           st.wYear, st.wMonth, st.wDay,
           st.wHour, st.wMinute, st.wSecond);

    system("pause");
    return 0;
}
