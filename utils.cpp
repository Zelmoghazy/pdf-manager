#include "utils.hpp"

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

static QWidget *embedIntoHBoxLayout(QWidget *w, int margin)
{
    auto result = new QWidget;
    auto layout = new QHBoxLayout(result);
    layout->setContentsMargins(margin, margin, margin, margin);
    layout->addWidget(w);
    return result;
}

static QWidget *embedIntoVBoxLayout(QWidget *w, int margin) 
{
    auto result = new QWidget;
    auto layout = new QVBoxLayout(result);
    layout->setContentsMargins(margin, margin, margin, margin);
    layout->addWidget(w);
    return result;
}
