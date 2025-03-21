#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch_amalgamated.hpp"
#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm> // For std::max

#define MAX(a, b) (a>b)?a:b

void trim(const std::string& str, std::string::const_iterator& start, std::string::const_iterator& end) 
{
    auto non_ws_start = str.find_first_not_of(" \t\n\r\f\v");
    auto non_ws_end = str.find_last_not_of(" \t\n\r\f\v");

    if (non_ws_start == std::string::npos || non_ws_end == std::string::npos) {
        start = str.end();
        end = str.end();
    } else {
        start = str.begin() + non_ws_start;
        end = str.begin() + non_ws_end + 1; 
    }
}

std::string trim(const std::string& str) 
{
    auto start = str.find_first_not_of(" \t\n\r\f\v");
    auto end = str.find_last_not_of(" \t\n\r\f\v");

    if (start == std::string::npos || end == std::string::npos) {
        return "";
    }

    return str.substr(start, end - start + 1);
}

struct PDFInfo
{
    std::string file_name;
    std::string file_path;
    int page_num = 0;
    std::string mode;
    bool found = false;

    PDFInfo(std::string filename) : file_name(filename)
    {

    }

    void parseSumatraSettings(const std::string &settings_path) 
    {
        std::ifstream file(settings_path, std::ios::binary);
    
        if (!file) {
            std::cerr << "Couldn't read file !\n";
            return;
        }
        
        // read with 8k chunks
        constexpr size_t buf_size = 8192;
        char buffer[buf_size];
    
        file.rdbuf()->pubsetbuf(buffer, buf_size);
    
        constexpr char file_states[] = "FileStates";
        constexpr size_t file_states_len = 10;
    
        std::string line;
        line.reserve(512);
    
        // Find FileStates
        bool found_filestates = false;
        while (std::getline(file, line))
        {
            if ((line.size() >= file_states_len))
            {
                if (memcmp(&line[0], file_states, file_states_len) == 0) 
                {
                    found_filestates = true;
                    break;
                }
            }
        }
    
        // Shouldnt happen !
        if (!found_filestates)
        {
            std::cerr << "Couldn't find FileStates in sumatra settings file !\n";
            return;
        }
    
        // start parsing
        constexpr char filePath[] = "FilePath";
        constexpr size_t filePath_len = 8; 
    
        constexpr char page_no[] = "PageNo";
        constexpr size_t page_no_len = 6;
        
        int bracket_level = 1;
        bool insidePdfEntry = false;
    
        while (std::getline(file, line))
        {
            std::string::const_iterator start, end;
            // skip whitespace
            trim(line, start, end);
    
            // Check if line is empty after trimming
            if (start == end)
                continue; // skip empty lines
    
            if (*start == '[' || *(end-1) == '[') 
            {
                bracket_level++;
                if (bracket_level == 2)  // we are inside a file state
                {
                    insidePdfEntry = true;
                }
                continue;
            }
            else if (*start == ']')
            {
                bracket_level--;
                if (bracket_level == 1 && insidePdfEntry)
                {
                    if (found) {
                        return;
                    }
                    insidePdfEntry = false;
                }
                continue;
            }
    
            // start parsing entries
            if (insidePdfEntry)
            {
                size_t remaining = std::distance(start, end);
    
                // Check filename first for a match
                if (remaining >= filePath_len &&  
                    (memcmp(&(*start), filePath, filePath_len) == 0)) 
                {
                    auto equals_pos = std::find(start, end, '=');
    
                    if (equals_pos != end)
                    {
                        // Get the full path
                        std::string fullPath(equals_pos + 1, end);
    
                        // Trim the path
                        std::string::const_iterator path_start, path_end;
                        trim(fullPath, path_start, path_end);
    
                        // Convert to std::string for easier operations
                        std::string trimmedPath(path_start, path_end);
    
                        size_t lastSlash1 = trimmedPath.rfind('\\');
                        size_t lastSlash2 = trimmedPath.rfind('/' );
                        size_t lastSlash;
    
                        if (lastSlash1 == std::string::npos)
                        {
                            lastSlash = lastSlash2;
                        } 
                        else if (lastSlash2 == std::string::npos)
                        {
                            lastSlash = lastSlash1;
                        } else {
                            lastSlash = std::max(lastSlash1, lastSlash2);
                        }
    
                        if (lastSlash != std::string::npos)
                        {
                            lastSlash++;
                            if (trimmedPath.compare(lastSlash, trimmedPath.length() - lastSlash,
                                                    file_name, 0, file_name.length()) == 0) 
                            {
                                // found a match
                                found = true;
                            } 
                        }
                    }
                }
    
                // Continue parsing if found only
                if(found)
                {
                    if (remaining >= page_no_len &&
                        memcmp(&(*start), page_no, page_no_len) == 0) 
                    {
                        auto equals_pos = std::find(start, end, '=');
    
                        if (equals_pos != end)
                        {
                            std::string pageNoStr(equals_pos + 1, end);
    
                            // Trim the page number
                            std::string::const_iterator pageNo_start, pageNo_end;
                            trim(pageNoStr, pageNo_start, pageNo_end);
    
                            page_num = std::stoi(std::string(pageNo_start, pageNo_end));
                            return;
                        }
                    } 
                }
            }
        }
    }
};

TEST_CASE("PDFInfo test1", "[pdf_info]") {
    PDFInfo info("08-LED Matrix.pdf");
    info.parseSumatraSettings("../SumatraPDF-settings.txt");
    REQUIRE(info.found == true);
    REQUIRE(info.page_num == 12);
}

TEST_CASE("PDFInfo test2", "[pdf_info]") {
    PDFInfo info("Spring 23 midterm_annotated.pdf");
    info.parseSumatraSettings("../SumatraPDF-settings.txt");
    REQUIRE(info.found == true);
    REQUIRE(info.page_num == 1);
}

TEST_CASE("PDFInfo test3", "[pdf_info]") {
    PDFInfo info("pdfcoffee.com_comp-verilog-wkb-60-pdf-free.pdf");
    info.parseSumatraSettings("../SumatraPDF-settings.txt");
    REQUIRE(info.found == true);
    REQUIRE(info.page_num == 322);
}

TEST_CASE("PDFInfo test4", "[pdf_info]") {
    PDFInfo info("Lecture 3_2.pdf");
    info.parseSumatraSettings("../SumatraPDF-settings.txt");
    REQUIRE(info.found == true);
    REQUIRE(info.page_num == 116);
}

TEST_CASE("PDFInfo test5", "[pdf_info]") {
    PDFInfo info("Sheet(1)-1901148.pdf");
    info.parseSumatraSettings("../SumatraPDF-settings.txt");
    REQUIRE(info.found == true);
    REQUIRE(info.page_num == 5);
}

TEST_CASE("PDFInfo test6", "[pdf_info]") {
    PDFInfo info("mastering embedded system from scratch_SecondEdition.pdf");
    info.parseSumatraSettings("../SumatraPDF-settings.txt");
    REQUIRE(info.found == true);
    REQUIRE(info.page_num == 579);
}

TEST_CASE("PDFInfo test7", "[pdf_info]") {
    PDFInfo info("mastering embedded system from scratch.pdf");
    info.parseSumatraSettings("../SumatraPDF-settings.txt");
    REQUIRE(info.found == true);
    REQUIRE(info.page_num == 1247);
}

TEST_CASE("PDFInfo benchmark", "[benchmark]") {
    // Benchmark multiple iterations to test caching effects
    BENCHMARK("Parse settings - repeated 10 times") {
        bool result = false;
        for (int i = 0; i < 10; i++) {
            PDFInfo info("mastering embedded system from scratch.pdf");
            info.parseSumatraSettings("../SumatraPDF-settings.txt");
            result |= info.found;
        }
        return result;
    };
}
