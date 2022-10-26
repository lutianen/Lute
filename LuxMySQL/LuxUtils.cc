/**
 * @file LuxUtils.cc
 * @author Tianen Lu (tianenlu@stu.xidian.edu.cn)
 * @brief 
 * @version 0.1
 * @date 2022-10-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "LuxUtils.h"

#include <cstring>  /// strlen strcpy memset


void
LuxUtils::LuxToUpper(char *str)
{
    if (nullptr == str)
        return;

    size_t strLen = ::strlen(str);

    if(0 == strLen)
        return;

    for (size_t i = 0; i < strLen; ++i) 
        if ((str[i] >= 'a') && (str[i] <= 'z'))
            str[i] = static_cast<char>(str[i] - 32);
}


void
LuxUtils::LuxDeleteLeftChar(char *str, const char chr)
{
    if(nullptr == str)
        return;

    size_t strLen = ::strlen(str);

    if (0 == strLen)
        return;
    
    char strTemp[strLen + 1];
    ::memset(strTemp, 0, sizeof(strTemp));
    ::strcpy(strTemp, str);

    // Find the start index
    int i = 0;
    while (chr == strTemp[i])
        ++i;
    
    // Clear original str and copy valid string
    ::memset(str, 0, strLen + 1);
    ::strcpy(str, strTemp + i);

    return;
}


void
LuxUtils::LuxUpdateStr(char *str, 
                        const char *str1, 
                        const char *str2, 
                        bool bloop)
{
    /// Check validation
    if (nullptr == str)
        return;
        
    size_t strLen = ::strlen(str);
    if (0 == strLen)
        return;
    if ((nullptr == str1) || (nullptr == str2))
        return;
    
    /// Return if bloop == true and str2 in str1.
    /// Or, it will be into endless cycle and OOM
    if ((true == bloop) && (::strstr(str2, str1) != nullptr))
        return;

    // FIXME 尽可能分配更多的空间，但仍有可能出现内存溢出的情况，最好优化成string。
    size_t len = strLen * 10;
    if (len < 1000)
        len = 1000;
    
    char strTemp[len];
    char *strStart=str;
    char *strPos=0;

    while (true)
    {
        strPos = (bloop == true) ? ::strstr(str, str1) : ::strstr(strStart, str1);

        if(nullptr == strPos)
            break;
        
        ::memset(strTemp, 0, sizeof(strTemp));
        ::strncpy(strTemp, str, static_cast<size_t>(strPos - str));
        ::strcat(strTemp, str2);
        ::strcat(strTemp, strPos + ::strlen(str1));
        ::strcpy(str, strTemp);

        strStart = strPos + ::strlen(str2);
    }
}

