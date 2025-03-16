#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <regex>
#include <algorithm>

#define MAX(a, b) (a > b) ? a : b

struct PDFInfo
{
    bool found = false;
    std::string file_name;
    std::string file_path;
    int page_num = 0;
    std::string mode;

    QPushButton *button = nullptr;

    PDFInfo() {}
    PDFInfo(std::string filename) : file_name(filename){}

    void parseSumatraSettings(const std::string &settings_path);
    
    // For linux, each pdf file has a corresponding small xml file so regex shouldnt slow things that much and its easier
    int parseOkularSettings(const std::string& settings_path);

};


struct PDFCat
{
    std::string category;
    std::vector<PDFInfo> PDFFiles;
    QWidget *container;
    QVBoxLayout *layout;
    QPushButton *addButton;

    PDFCat() {}
    PDFCat(const std::string name) : category(name){}
};
