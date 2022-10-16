/**
 * @file LuxUtils.h
 * @author Tianen Lu (tianenlu@stu.xidian.edu.cn)
 * @brief 
 * @version 0.1
 * @date 2022-10-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once



namespace LuxUtils
{
    /// \brief Change the lower leters to upper letters, 
    /// ignore the characters that are not letters.
    /// \param str 
    void
    LuxToUpper(char *str);


    /// @brief Delete the left \c char
    /// @param str The original string
    /// @param chr The \c char to be delete
    void
    LuxDeleteLeftChar(char *str, const char chr);

    /// @brief Use str2 to replace str1, if str1 exsists in str
    /// @param str
    /// @param str1
    /// @param str2
    /// @param bloop
    void
    LuxUpdateStr(char *str, const char *str1, const char *str2, bool bloop);
} // namespace LuxUtils



