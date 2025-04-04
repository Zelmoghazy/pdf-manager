#include "custom_layout.hpp"

SortedVBoxLayout::SortedVBoxLayout(CompareFunc compareFunc, QWidget* parent)
    : QVBoxLayout(parent),
      m_compareFunc(compareFunc) 
{
}

void SortedVBoxLayout::setCompareFunc(CompareFunc compareFunc) 
{
    m_compareFunc = compareFunc;
    SortandUpdate();
}

void SortedVBoxLayout::addWidget(QWidget *widget) 
{
    m_widgets.push_back(widget);
    SortandUpdate();
}

void SortedVBoxLayout::SortandUpdate() 
{
    if (m_compareFunc) {
        sortWidgets();
        updateLayout();
    }
}

void SortedVBoxLayout::removeWidget(QWidget *widget) 
{
    auto it = std::find(m_widgets.begin(), m_widgets.end(), widget);
    if(it != m_widgets.end()) {
        m_widgets.erase(it);
        QVBoxLayout::removeWidget(widget);
    }
}

void SortedVBoxLayout::sortWidgets() 
{
    if (m_compareFunc) {
        std::sort(m_widgets.begin(), m_widgets.end(), m_compareFunc);
    }
}

void SortedVBoxLayout::updateLayout() 
{
    // Clear the layout
    QLayoutItem* item;
    while ((item = takeAt(0))) {
        delete item;
    }
        
    // Add widgets back in sorted order
    for (QWidget* widget : m_widgets) {
        QVBoxLayout::addWidget(widget);
    }
}

SortedFlowLayout::SortedFlowLayout(CompareFunc compareFunc, QWidget* parent, int margin, int hSpacing, int vSpacing)
    : FlowLayout(parent, margin, hSpacing, vSpacing),
      m_compareFunc(compareFunc) 
{
}


void SortedFlowLayout::setCompareFunc(CompareFunc compareFunc) 
{
    m_compareFunc = compareFunc;
    SortandUpdate();
}

void SortedFlowLayout::addWidget(QWidget *widget) 
{
    m_widgets.push_back(widget);
    FlowLayout::addWidget(widget);
    SortandUpdate();
}

void SortedFlowLayout::SortandUpdate() 
{
    if (m_compareFunc) {
        sortWidgets();
        updateLayout();
    }
}

void SortedFlowLayout::removeWidget(QWidget *widget) 
{
    auto it = std::find(m_widgets.begin(), m_widgets.end(), widget);
    if(it != m_widgets.end()) {
        m_widgets.erase(it);
    }
    FlowLayout::removeWidget(widget);
    updateLayout();
}

void SortedFlowLayout::sortWidgets() 
{
    if (m_compareFunc) {
        std::sort(m_widgets.begin(), m_widgets.end(), m_compareFunc);
    }
}

void SortedFlowLayout::updateLayout() 
{
    std::vector<QWidget*> currentWidgets;
    for (int i = 0; i < count(); ++i) {
        QLayoutItem* item = FlowLayout::itemAt(i);
        if (item->widget()) {
            currentWidgets.push_back(item->widget());
        }
    }
        
    // Remove all widgets (carefully without deleting QLayoutItems)
    for (QWidget* widget : currentWidgets) {
        FlowLayout::removeWidget(widget);
    }
        
    // Add widgets back in sorted order
    for (QWidget* widget : m_widgets) {
        FlowLayout::addWidget(widget);
    }
}
