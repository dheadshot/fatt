#include <Windows.h>
#include <stdio.h>
#include <WinBase.h> //Lib=kernel32
/*
 * SYSTEMTIME: https://msdn.microsoft.com/en-us/library/ms724950.aspx
 * FileTimeToSystemTime: https://msdn.microsoft.com/en-us/library/ms724280.aspx
 * WIN32_FILE_ATTRIBUTE_DATA: https://msdn.microsoft.com/en-us/library/aa365739.aspx
 * GetFileAttributesEx: https://msdn.microsoft.com/en-us/library/aa364946.aspx
 * File Attribute Constants: https://msdn.microsoft.com/en-us/library/gg258117.aspx
 * GetTimeZoneInformation: https://msdn.microsoft.com/en-us/library/windows/desktop/ms724421(v=vs.85).aspx
 * TIME_ZONE_INFORMATION: https://msdn.microsoft.com/en-us/library/windows/desktop/ms725481(v=vs.85).aspx
*/

#define PROGVERSION "1.00.0002"

#ifndef FILE_ATTRIBUTE_INTEGRITY_STREAM
#define FILE_ATTRIBUTE_INTEGRITY_STREAM 0x8000
#endif

#ifndef FILE_ATTRIBUTE_NO_SCRUB_DATA
#define FILE_ATTRIBUTE_NO_SCRUB_DATA 0x20000
#endif

#ifndef FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
#define FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS 0x400000
#endif

#ifndef FILE_ATTRIBUTE_RECALL_ON_OPEN
#define FILE_ATTRIBUTE_RECALL_ON_OPEN 0x40000
#endif

#ifndef FILE_ATTRIBUTE_VIRTUAL
#define FILE_ATTRIBUTE_VIRTUAL 0x10000
#endif

WIN32_FILE_ATTRIBUTE_DATA fad;
SYSTEMTIME ct, lat, lmt;
TIME_ZONE_INFORMATION tz;
LONG tzbias = 0, mtzbias, htzbias;

int daysinmonth(WORD year, WORD month)
{
    if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12) return 31;
    if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
    if (month == 2)
    {
        if (year % 4 != 0) return 28;
        if (year % 100 != 0) return 29;
        if (year % 400 != 0) return 28;
        return 29;
    }
    return 0;
}

long cleversgn(long n, long low, long high)
{
    if (n<low) return (0-((n-1)/(high+1-low)));
    if (n>high) return ((n+1)/(high+1-low));
    return 0;
}

int converttimes(SYSTEMTIME *atime, LONG biashours, LONG biasmins)
{
    long ati = 0, atdim = 0;
    ati = ((long) (atime->wMinute)) + ((long) biasmins);
    if (ati<0) atime->wMinute = ati+60;
    else if (ati>59) atime->wMinute = ati - 60;
    else atime->wMinute = ati;
    ati = cleversgn(ati,0,59);
    ati += ((long) (atime->wHour)) + ((long) biashours);
    if (ati<0) atime->wHour = ati+24;
    else if (ati>23) atime->wHour = ati - 24;
    else atime->wHour = ati;
    ati = cleversgn(ati,0,23);
    ati += ((long) (atime->wDay));
    atdim = ((long) (daysinmonth(atime->wYear, atime->wMonth)));
    if (atdim == 0) return 0;
    if (ati<1) atime->wDay = ati+atdim;
    else if (ati>atdim) atime->wDay = ati - atdim;
    else atime->wDay = ati;
    ati = cleversgn(ati,1,atdim);
    ati += ((long) (atime->wMonth));
    if (ati<1) atime->wMonth = ati+12;
    else if (ati>12) atime->wMonth = ati - 12;
    else atime->wMonth = ati;
    ati = cleversgn(ati,1,12);
    ati += ((long) (atime->wYear));
    if (ati<1601) atime->wYear = 1601;
    else if (ati>30827) atime->wYear = 30827;
    else atime->wYear = ati;
    return 1;
}

int fileisarchive(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_ARCHIVE) != 0) return 1;
    return 0;
}

int fileiscompressed(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_COMPRESSED) > 0) return 1;
    return 0;
}

int fileisdevice(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_DEVICE) > 0) return 1;
    return 0;
}

int fileisdir(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_DIRECTORY) > 0) return 1;
    return 0;
}

int fileisencrypted(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_ENCRYPTED) > 0) return 1;
    return 0;
}

int fileishidden(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_HIDDEN) > 0) return 1;
    return 0;
}

int fileisistream(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_INTEGRITY_STREAM) > 0) return 1;
    return 0;
}

int fileisnormal(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_NORMAL) > 0) return 1;
    return 0;
}

int fileisnotindexed(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) > 0) return 1;
    return 0;
}

int fileisnoscrub(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_NO_SCRUB_DATA) > 0) return 1;
    return 0;
}

int fileisoffline(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_OFFLINE) > 0) return 1;
    return 0;
}

int fileisreadonly(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_READONLY) > 0) return 1;
    return 0;
}

int fileisrecalldataaccess(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS) > 0) return 1;
    return 0;
}

int fileisrecallopen(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_RECALL_ON_OPEN) > 0) return 1;
    return 0;
}

int fileisreparsepoint(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_REPARSE_POINT) > 0) return 1;
    return 0;
}

int fileissparse(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_SPARSE_FILE) > 0) return 1;
    return 0;
}

int fileissystem(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_SYSTEM) > 0) return 1;
    return 0;
}

int fileistemp(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_TEMPORARY) > 0) return 1;
    return 0;
}

int fileisvirtual(DWORD fatt)
{
    if ((fatt & FILE_ATTRIBUTE_VIRTUAL) > 0) return 1;
    return 0;
}

int main(int argc, char *argv[])
{
        int tzdst;
        if (argc != 2)
        {
                printf("DHSC FAtt %s\nLists file attributes.\n",PROGVERSION);
                printf("Usage:\n  %s <File>\n", argv[0]);
                return 0;
        }
        tzdst = GetTimeZoneInformation(&tz);
        if (tzdst == TIME_ZONE_ID_UNKNOWN || tzdst == TIME_ZONE_ID_STANDARD)
        {
            tzbias = 0-(tz.Bias+tz.StandardBias);
        }
        else if (tzdst == TIME_ZONE_ID_DAYLIGHT)
        {
            tzbias = 0-(tz.Bias+tz.DaylightBias);
        }
        else
        {
            printf("Error getting timezone data!\n");
            return 2;
        }
        htzbias = tzbias/60;
        mtzbias = tzbias % 60;
        if (GetFileAttributesEx(argv[1], GetFileExInfoStandard, &fad))
        {
                if (FileTimeToSystemTime(&(fad.ftCreationTime),&ct) == FALSE)
                {
                        printf("Error Converting Time!\n");
                        return 2;
                }
                if (converttimes(&ct,htzbias,mtzbias) == 0)
                {
                    printf("Error Adjusting Time!\n");
                    return 2;
                }
                if (FileTimeToSystemTime(&(fad.ftLastAccessTime),&lat) == FALSE)
                {
                        printf("Error Converting Time!\n");
                        return 2;
                }
                if (converttimes(&lat,htzbias,mtzbias) == 0)
                {
                    printf("Error Adjusting Time!\n");
                    return 2;
                }
                if (FileTimeToSystemTime(&(fad.ftLastWriteTime),&lmt) == FALSE)
                {
                        printf("Error Converting Time!\n");
                        return 2;
                }
                if (converttimes(&lmt,htzbias,mtzbias) == 0)
                {
                    printf("Error Adjusting Time!\n");
                    return 2;
                }
                
                printf("FA: %lu\nCreated Time: %u-%u-%u %u:%u:%u.%u\nAccessed Time: %u-%u-%u %u:%u:%u.%u\nModified Time: %u-%u-%u %u:%u:%u.%u\nSize: %llu\n",
                                fad.dwFileAttributes,
                                ct.wYear,ct.wMonth,ct.wDay,ct.wHour,ct.wMinute,ct.wSecond,ct.wMilliseconds,
                                lat.wYear,lat.wMonth,lat.wDay,lat.wHour,lat.wMinute,lat.wSecond,lat.wMilliseconds,
                                lmt.wYear,lmt.wMonth,lmt.wDay,lmt.wHour,lmt.wMinute,lmt.wSecond,lmt.wMilliseconds,
                                ((unsigned long long) ((fad.nFileSizeHigh<<32)+fad.nFileSizeLow)));
                if (fileisarchive(fad.dwFileAttributes) == 1) printf("File is Archived\n");
                if (fileiscompressed(fad.dwFileAttributes)) printf("File is Compressed\n");
                if (fileisdevice(fad.dwFileAttributes)) printf("File is a Device\n");
                if (fileisdir(fad.dwFileAttributes)) printf("File is a Directory\n");
                if (fileisencrypted(fad.dwFileAttributes)) printf("File is Encrypted\n");
                if (fileishidden(fad.dwFileAttributes)) printf("File is Hidden\n");
                if (fileisistream(fad.dwFileAttributes)) printf("File has Integrity\n");
                if (fileisnormal(fad.dwFileAttributes)) printf("File is Normal\n");
                if (fileisnotindexed(fad.dwFileAttributes)) printf("File is Not Indexed\n");
                if (fileisnoscrub(fad.dwFileAttributes)) printf("File does not provide Inheritance\n");
                if (fileisoffline(fad.dwFileAttributes)) printf("File is Offline\n");
                if (fileisreadonly(fad.dwFileAttributes)) printf("File is Read Only\n");
                if (fileisrecalldataaccess(fad.dwFileAttributes)) printf("File is Remote and will be fetched upon accessing it\n");
                if (fileisrecallopen(fad.dwFileAttributes)) printf("File is Remote and will be recalled upon opening it\n");
                if (fileisreparsepoint(fad.dwFileAttributes)) printf("File is Symbolic Link\n");
                if (fileissparse(fad.dwFileAttributes)) printf("File is Sparse\n");
                if (fileissystem(fad.dwFileAttributes)) printf("File is a System file\n");
                if (fileistemp(fad.dwFileAttributes)) printf("File is Temporary\n");
                if (fileisvirtual(fad.dwFileAttributes)) printf("File is Virtual\n");
                return 0;
        }
        else
        {
                printf("There was an error retrieving file attributes!\n");
                return 1;
        }
}
