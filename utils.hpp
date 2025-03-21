#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <string>

std::string trim(const std::string& str);
void trim(const std::string& str, std::string::const_iterator& start, std::string::const_iterator& end);

static QWidget *embedIntoHBoxLayout(QWidget *w, int margin = 5);
static QWidget *embedIntoVBoxLayout(QWidget *w, int margin = 5);
