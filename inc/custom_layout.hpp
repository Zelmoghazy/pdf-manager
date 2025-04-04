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

/*
using CompareFunc = std::function<bool(QWidget*, QWidget*)>;

template <class BaseLayout>
class SortedLayout : public BaseLayout 
{
public:
    
    template <typename... Args>
    SortedLayout(CompareFunc compareFunc, Args&&... args)
        : BaseLayout(std::forward<Args>(args)...),
          m_compareFunc(compareFunc) {}
    
    void setCompareFunc(CompareFunc compareFunc) 
    {
        m_compareFunc = compareFunc;
        SortandUpdate();
    }
    
    void addWidget(QWidget *widget) 
    {
        m_widgets.push_back(widget);
        BaseLayout::addWidget(widget);
        SortandUpdate();
    }

    void SortandUpdate() 
    {
        if (m_compareFunc) {
            sortWidgets();
            updateLayout();
        }
    }
    
    void removeWidget(QWidget *widget) 
    {
        auto it = std::find(m_widgets.begin(), m_widgets.end(), widget);
        if(it != m_widgets.end()) {
            m_widgets.erase(it);
        }
        BaseLayout::removeWidget(widget);
        updateLayout();
    }
    


private:
    std::vector<QWidget*> m_widgets;
    CompareFunc m_compareFunc;
    
    void sortWidgets() 
    {
        if (m_compareFunc) {
            std::sort(m_widgets.begin(), m_widgets.end(), m_compareFunc);
        }
    }
    
    void updateLayout() 
    {
        std::vector<QWidget*> currentWidgets;
        
        for (int i = 0; i < BaseLayout::count(); ++i) {
            QLayoutItem* item = BaseLayout::itemAt(i);
            if (item->widget()) {
                currentWidgets.push_back(item->widget());
            }
        }
        
        for (QWidget* widget : currentWidgets) {
            BaseLayout::removeWidget(widget);
        }
        
        for (QWidget* widget : m_widgets) {
            BaseLayout::addWidget(widget);
        }
    }
};
*/