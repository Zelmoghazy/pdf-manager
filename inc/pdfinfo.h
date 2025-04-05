#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QString>
#include <QStackedWidget>
#include <QPixmap>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <regex>
#include <algorithm>

#include "custom_button.hpp"
#include "flowlayout.h"
#include "utils.hpp"
#include "custom_layout.hpp"

struct PDFInfo
{
    std::string file_name;          // The represented name in the UI, the user can edit it to whatever
    std::string file_path;          // What actually distinguishes the file

    bool found = false;             // whether it was found while parsing
    int page_num = 0;               // last opened page

    int total_page_num = 0;
    qint64 last_opened_time = 0;       // sort using it

    QPixmap thumbnail;


    std::string mode;               // to be used to add options like open in full screen or presentation mode or whatever

    WordWrapButton *button = nullptr;
    WordWrapButton *flowButton = nullptr;

    PDFInfo() {}
    PDFInfo(const std::string filename) : file_name(filename){}

    void parseSumatraSettings(const std::string &settings_path);
    // For linux, each pdf file has a corresponding small xml file so regex shouldnt slow things that much and its easier
    int parseOkularSettings(const std::string& settings_path);
};

struct PDFCat
{
    std::string category;

    // each category has a number of pdfs 
    std::vector<PDFInfo> PDFFiles;

    QWidget                        *container     = nullptr;
    QStackedWidget                 *stackedWidget = nullptr;

    SortedVBoxLayout*           layout        = nullptr;
    SortedFlowLayout*           flowLayout    = nullptr; 

    // to add new categories
    QPushButton     *addButton      = nullptr;
    QPushButton     *flowAddButton  = nullptr;

    PDFCat() {}
    PDFCat(const std::string cat_name) : category(cat_name){}
};
