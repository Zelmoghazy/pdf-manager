#pragma once

#include <QVBoxLayout>
#include <functional>
#include <vector>
#include <algorithm>
#include "FlowLayout.h"

using CompareFunc = std::function<bool(QWidget*, QWidget*)>;

class SortedVBoxLayout : public QVBoxLayout 
{
public:
    // Constructors
    SortedVBoxLayout(CompareFunc compareFunc, QWidget* parent = nullptr);
    
    void setCompareFunc(CompareFunc compareFunc);
    void addWidget(QWidget *widget);
    void SortandUpdate();
    void removeWidget(QWidget *widget);
    
private:
    std::vector<QWidget*> m_widgets;
    CompareFunc m_compareFunc;
    
    void sortWidgets();
    void updateLayout();
};

class SortedFlowLayout : public FlowLayout 
{
public:
    // Constructors
    SortedFlowLayout(CompareFunc compareFunc, QWidget* parent = nullptr, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    
    void setCompareFunc(CompareFunc compareFunc);
    void addWidget(QWidget *widget);
    void SortandUpdate();
    void removeWidget(QWidget *widget);
    
private:
    std::vector<QWidget*> m_widgets;
    CompareFunc m_compareFunc;
    
    void sortWidgets();
    void updateLayout();
};
