#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch_amalgamated.hpp"
#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm> // For std::max

#define MAX(a, b) (a>b)?a:b

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

    // Dumbest way I could think of right now, should improve later 
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
            if ((line[0] == 'F') && (line.size() >= file_states_len))
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
        constexpr char file_path[] = "FilePath";
        constexpr size_t file_path_len = 8; 

        constexpr char page_no[] = "PageNo";
        constexpr size_t page_no_len = 6;
        
        int bracket_level = 1;
        bool insidePdfEntry = false;

        while (std::getline(file, line))
        {
            // skip whitespace
            size_t pos = 0;
            while (pos < line.size() && std::isspace((unsigned char)line[pos]))
                ++pos;

            size_t end = line.size();
            while (end > pos && std::isspace((unsigned char)line[end - 1]))
                --end;

            // Check if line is empty after trimming
            if (pos >= end)
                continue; // skip empty lines

            if (line[pos] == '[' || line[end-1] == '[') 
            {
                bracket_level++;
                if (bracket_level == 2)  // we are inside a file state
                {
                    insidePdfEntry = true;
                }
                continue;
            }
            else if (line[pos] == ']')
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
                // Check filename first for a match
                if ((pos <= (line.size() - file_path_len)) &&  
                    (memcmp(&line[pos], file_path, file_path_len) == 0)) 
                {
                    size_t path_start = line.find('=', pos);

                    if (path_start != std::string::npos)
                    {
                        // Get the full path
                        std::string fullPath = line.substr(path_start+1);

                        // Trim whitespace
                        size_t startPos = 0;
                        while (startPos < fullPath.length() && std::isspace((unsigned char)(fullPath[startPos]))) {
                            startPos++;
                        }
                        
                        size_t endPos = fullPath.length();
                        while (endPos > startPos && std::isspace((unsigned char)(fullPath[endPos - 1]))) {
                            endPos--;
                        }

                        size_t lastSlash1 = fullPath.rfind('\\', endPos - 1);
                        size_t lastSlash2 = fullPath.rfind('/' , endPos - 1);
                        size_t lastSlash;

                        if (lastSlash1 == std::string::npos)
                        {
                            lastSlash = lastSlash2;
                        } 
                        else if (lastSlash2 == std::string::npos)
                        {
                            lastSlash = lastSlash1;
                        } else {
                            lastSlash = MAX(lastSlash1, lastSlash2);
                        }

                        if (lastSlash != std::string::npos)
                        {
                            lastSlash++;
                            if (fullPath.compare(lastSlash, endPos - lastSlash,
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
                    if ((pos <= line.size() - page_no_len) && 
                        memcmp(&line[pos], page_no, page_no_len) == 0) 
                    {
                        size_t path_start = line.find('=', pos);

                        if (path_start != std::string::npos)
                        {
                            std::string PageNo = line.substr(path_start+1);

                            // Trim whitespace
                            size_t startPos = 0;
                            while (startPos < PageNo.length() && 
                                   std::isspace((unsigned char)(PageNo[startPos]))) 
                            {
                                startPos++;
                            }
                            
                            size_t endPos = PageNo.length();
                            while (endPos > startPos &&
                                   std::isspace((unsigned char)(PageNo[endPos - 1]))) 
                            {
                                endPos--;
                            }

                            page_num = std::stoi(PageNo.substr(startPos, endPos - startPos));
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
