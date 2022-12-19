#pragma once
#include "Cursor.h"
#include <fstream>
#include <string>
#include <iostream>
#include <vector>



class MacroCursor :public Cursor {

    friend class MacrosDelegate;
public:
    MacroCursor();
    MacroCursor(const Cursor& cursor);
    Cursor GetNextCursor();
    std::vector<std::string> GetKeys();
private:
    void ParseKeys();
    bool GetNextPosition(CXFile& file, unsigned int& offset);

    bool PropertyMatch(std::string linestr, unsigned int& offset);
    bool ClassMatch(std::string linestr, unsigned int& offset);
    bool FunctionMatch(std::string linestr, unsigned int& offset);
private:
    Cursor modified_cursor_;
    std::vector<std::string> keys;
};

class MacrosDelegate
{
public:
    MacrosDelegate();
    void Push(MacroCursor cursor);
    bool Modified(Cursor cursor,std::vector<std::string>& keys);
private:
    std::vector<MacroCursor> cursors_;
};
