#include "custom_button.hpp"

PDFButton::PDFButton(const QString& text, QWidget* parent = nullptr)
    : QPushButton(parent), m_text(text) 
{
    setupButton();
}

void PDFButton::setText(const QString& text)
{
    m_text = text;
    m_label->setText(text);
    adjustText();
}

void PDFButton::setTimestamp(qint64 time) 
{
    m_timestamp = time;
}

QString PDFButton::getText() 
{ 
    return m_text; 
}

qint64 PDFButton::getTimestamp() 
{
    return m_timestamp; 
}
    
void PDFButton::resizeEvent(QResizeEvent* event) 
{
    QPushButton::resizeEvent(event);
    adjustText();
}

void PDFButton::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu contextMenu(this);
    contextMenu.setStyleSheet(
        "QMenu {"
        "    background-color: #3B4252;"     
        "    color: #D8DEE9;"                
        "    border: 1px solid #4C566A;"     
        "    padding: 5px;"                  
        "}"
        "QMenu::item {"
        "    padding: 8px 20px;"             
        "    border-radius: 4px;"            
        "}"
        "QMenu::item:selected {"             
        "    background-color: #5E81AC;"     
        "    color: #ECEFF4;"                
        "}"
        "QMenu::separator {"
        "    height: 1px;"                   
        "    background: #4C566A;"           
        "    margin: 4px 10px;"              
        "}"
    );
    
    QAction* copyAction = contextMenu.addAction("Copy Text");
    QAction* editAction = contextMenu.addAction("Edit");
    contextMenu.addSeparator();
    QAction* deleteAction = contextMenu.addAction("Delete");
    
    connect(copyAction, &QAction::triggered, this, [this]() {
        QApplication::clipboard()->setText(m_text);
    });
    
    connect(editAction, &QAction::triggered, this, [this]() {
        emit editRequested(this);
    });
    
    connect(deleteAction, &QAction::triggered, this, [this]() {
        emit deleteRequested(this);
    });
    
    contextMenu.exec(event->globalPos());
}

void PDFButton::setupButton() 
{
    m_label = new QLabel(this);
    m_label->setText(m_text);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setWordWrap(true); 
    m_label->setTextInteractionFlags(Qt::NoTextInteraction);
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_label->setAttribute(Qt::WA_TranslucentBackground);
    m_label->setStyleSheet("background: transparent; color: white;");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_label);
    layout->setSpacing(0);
    layout->setContentsMargins(5, 5, 5, 5); // Add some padding

    setLayout(layout);

    QPushButton::setText("");

    if (isVisible() && isFullyVisible()) 
    {
        adjustText();
    }
    
    installEventFilter(this);

    #if 0
    this->setStyleSheet(
        "QPushButton {"
        "   background-color: #1e1e1e;" 
        "   border: 1px solid transparent;" 
        "   border-radius: 10px;" 
        "   padding: 5px;" 
        "   color: white;" 
        "}"
        "QPushButton:hover {"
        "   background-color: #8c8c8c;" 
        "   border: 1px solid transparent;" 
        "}"
        "QPushButton:pressed {"
        "   background-color: #C0E0FF;"
        "   border: 1px solid transparent;" 
        "}"
    );
    #endif
}

bool PDFButton::isFullyVisible() const
{
    if (visibleRegion().isEmpty()) 
    {
        return false;
    }
    
    QRect visibleRect = visibleRegion().boundingRect();
    return (visibleRect.width() == width() && visibleRect.height() == height());
}

void PDFButton::checkAndAdjustText()
{
    if (isVisible() && isFullyVisible()) 
    {
        adjustText();
    }
}

bool PDFButton::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == this) 
    {
        if (event->type() == QEvent::Paint        || 
            event->type() == QEvent::Show         || 
            event->type() == QEvent::ShowToParent ||
            event->type() == QEvent::ParentChange) 
        {
            checkAndAdjustText();
        }
    }
    return QPushButton::eventFilter(watched, event);
}

void PDFButton::showEvent(QShowEvent* event)
{
    QPushButton::showEvent(event);
    checkAndAdjustText();
}

// Simple word wrapping algorithm
void PDFButton::adjustText() 
{
    if (!isVisible() || !isFullyVisible()) {
        return;
    }

    QFontMetrics fm(m_label->font());
    int availableWidth = width() - 10;

    QStringList words = m_text.split(' ', Qt::SkipEmptyParts);

    QString wrappedText;
    wrappedText.reserve(words.size() * 8);
    
    QString currentLine;

    for (const QString& word : words) 
    {
        QString testLine = currentLine.isEmpty() ? word : currentLine + ' ' + word;

        if (fm.horizontalAdvance(testLine) <= availableWidth) 
        {
            currentLine = testLine;
        } 
        else 
        {
            if (!wrappedText.isEmpty()){
                wrappedText += '\n';
            }
            wrappedText += currentLine;
            currentLine = word;
        }
    }

    if (!currentLine.isEmpty()) 
    {
        if (!wrappedText.isEmpty()) {
            wrappedText += '\n';
        }
        wrappedText += currentLine;
    }

    m_label->setText(wrappedText);
}

