#include "MacroCursor.h"
#include <functional>
#include <cctype>
#include "Define.h"




using MatchFunc = bool (MacroCursor::*)(std::string linestr, unsigned int& offset);

MacroCursor::MacroCursor()
    :Cursor()
{
}

MacroCursor::MacroCursor(const Cursor& cursor)
    :Cursor(cursor) {

}

Cursor MacroCursor::GetNextCursor() {
    unsigned int offset;
    CXFile file;

    if (!GetNextPosition(file,  offset))
    {
        return Cursor();
    }

    ParseKeys();
    CXSourceLocation newloc = clang_getLocationForOffset(clang_Cursor_getTranslationUnit(cursor_), file, offset);

    modified_cursor_ = clang_getCursor(clang_Cursor_getTranslationUnit(cursor_), newloc);

    std::string spel = modified_cursor_.GetSpelling();
    std::cerr << "find apply unit: " << spel << std::endl;
    return modified_cursor_;
}

std::vector<std::string> MacroCursor::GetKeys()
{
    return keys;
}

void MacroCursor::ParseKeys()
{
    CXSourceRange range = clang_getCursorExtent(cursor_);
    CXSourceLocation start_loc = clang_getRangeStart(range);
    CXSourceLocation end_loc = clang_getRangeEnd(range);

    unsigned int start_line, start_column;
    unsigned start_offset;
    CXFile file;
    clang_getSpellingLocation(start_loc, &file, &start_line, &start_column, &start_offset);
    unsigned int end_line, end_column, end_offset;
    clang_getSpellingLocation(end_loc, NULL, &end_line, &end_column, &end_offset);
    size_t content_size;
    
    const char* data = clang_getFileContents(clang_Cursor_getTranslationUnit(GetCXCursor()), file, &content_size);

    if (start_line != end_line)
    {
        std::cerr << "warning: macro is splitted some lines" << std::endl;
    }

    std::string str(data, content_size);
    std::string macro_str = str.substr(start_offset, end_offset - start_offset);
    size_t start_pos = macro_str.find_first_of('(');

    macro_str = macro_str.substr(start_pos + 1);
    std::string key;
    //split and get keys in macro
    for (size_t i = 0; i < macro_str.size(); i++)
    {
        char c = macro_str.at(i);
        if (c == ' ' )
            continue;
        if (c == ',' || c == ')')
        {
            if (!key.empty()) {
                keys.push_back(key);
                key = "";
            }
        }
        else {
            key += c;
        }
    }
}

bool MacroCursor::GetNextPosition(CXFile& file, unsigned int& offset)
{
    MatchFunc match = nullptr;
    if (GetSpelling() == PROPERTY)
    {
        match = &MacroCursor::PropertyMatch;
    }
    else if (GetSpelling() == CLASS)
    {
        match = &MacroCursor::ClassMatch;
    }
    else if (GetSpelling() == FUNCTION)
    {
        match = &MacroCursor::FunctionMatch;
    }
    else
    {
        return false;
    }

    CXSourceRange range = clang_getCursorExtent(cursor_);
    CXSourceLocation end_loc = clang_getRangeEnd(range);

    clang_getSpellingLocation(end_loc, &file,NULL, NULL, &offset);

    size_t content_size;
    const char* data = clang_getFileContents(clang_Cursor_getTranslationUnit(GetCXCursor()), file, &content_size);

    std::string sub_content = std::string(data, content_size).substr(offset);

    return (this->*match)(sub_content, offset);

}

bool MacroCursor::PropertyMatch(std::string linestr, unsigned int& offset)
{
    size_t pos = linestr.find_first_of(';');
    if (pos == std::string::npos)
        return false;
    for (size_t i = pos; i > 0; i--)
    {
        char c = linestr.at(i);
        if (isdigit(c) || isalpha(c))
        {
            offset += i;
            break;
        }
    }
    return true;
}

bool MacroCursor::ClassMatch(std::string linestr, unsigned int& offset)
{
    size_t pos = linestr.find_first_of('{');
    if (pos == std::string::npos)
        return false;
    for (size_t i = pos; i > 0; i--)
    {
        char c = linestr.at(i);
        if (isdigit(c) || isalpha(c))
        {
            offset += i;
            break;
        }
    }
    return true;

}

bool MacroCursor::FunctionMatch(std::string linestr, unsigned int& offset)
{
    size_t pos = linestr.find_first_of('(');
    if (pos == std::string::npos)
        return false;
    for (size_t i = pos; i > 0; i--)
    {
        char c = linestr.at(i);
        if (isdigit(c) || isalpha(c))
        {
            offset += i;
            break;
        }
    }
    return true;
}

MacrosDelegate::MacrosDelegate()
    :cursors_{}
{
}

void MacrosDelegate::Push(MacroCursor cursor)
{
    if (cursor.GetNextCursor().IsValid()) {
        cursors_.push_back(cursor);
    }
}

bool MacrosDelegate::Modified(Cursor cursor, std::vector<std::string>& keys)
{
    for (auto& cur : cursors_)
    {
        if (cur.modified_cursor_ == cursor)
        {
            //std::string input = cursor.GetSpelling();
            //std::string child = cur.modified_cursor_.GetSpelling();
            keys = cur.GetKeys();
            return true;
        }
    }
    return false;
}
