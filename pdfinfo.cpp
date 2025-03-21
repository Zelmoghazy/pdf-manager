#include "pdfinfo.h"

// Dumbest way I could think of right now, should improve later if required
void PDFInfo::parseSumatraSettings(const std::string &settings_path) 
{
    std::ifstream file(settings_path, std::ios::binary);
    
    if (!file) {
        std::cerr << "Couldn't read file !\n";
        return;
    }
        
    // read in 8k chunks
    constexpr size_t buf_size = 8192;
    char buffer[buf_size];
    
    file.rdbuf()->pubsetbuf(buffer, buf_size);
    
    constexpr char FileStates[] = "FileStates";
    constexpr size_t FileStates_len = 10;
    
    std::string line;
    line.reserve(512);
    
    // Find FileStates
    bool found_filestates = false;
    while (std::getline(file, line))
    {
        if ((line.size() >= FileStates_len))
        {
            if (memcmp(&line[0], FileStates, FileStates_len) == 0) 
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
    constexpr char FilePath[] = "FilePath";
    constexpr size_t FilePath_len = 8; 
    
    constexpr char PageNo[] = "PageNo";
    constexpr size_t PageNo_len = 6;
        
    int  bracket_level = 1;
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
            // the remaining line after trimming white space
            size_t remaining = std::distance(start, end);
    
            // Check filename first for a match
            if (remaining >= FilePath_len &&  
                (memcmp(&(*start), FilePath, FilePath_len) == 0)) 
            {
                auto equals_pos = std::find(start, end, '=');
    
                if (equals_pos != end)
                {
                    // Get the full path
                    std::string fullPath(equals_pos + 1, end);
    
                    // Trim the path
                    std::string::const_iterator path_start, path_end;
                    trim(fullPath, path_start, path_end);
    
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
                    }
                    else 
                    {
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
                if (remaining >= PageNo_len &&
                    memcmp(&(*start), PageNo, PageNo_len) == 0) 
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

// WIP For linux, each pdf file has a corresponding small xml file
int PDFInfo::parseOkularSettings(const std::string& settings_path)
{
    std::regex viewportRegex(R"(<current\s+viewport="(\d+);)");
    std::smatch match;
    std::string xmlContent;
    if (std::regex_search(xmlContent, match, viewportRegex) && match.size() > 1) 
    {
        return std::stoi(match[1].str());
    }
    return -1;
}