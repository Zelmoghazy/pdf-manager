#include "custom_button.hpp"

WordWrapButton::WordWrapButton(const QString& text, QWidget* parent = nullptr)
    : QPushButton(parent), m_text(text) 
{
    setupButton();
}

void WordWrapButton::setText(const QString& text){
    m_text = text;
    m_label->setText(text);
    adjustText();
}
    
void WordWrapButton::resizeEvent(QResizeEvent* event) 
{
    QPushButton::resizeEvent(event);
    adjustText();
}

void WordWrapButton::setupButton() 
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

    #if 0
    this->setStyleSheet(
        "QPushButton {"
        "   background-color: #1e1e1e;" // Default background color
        "   border: 1px solid transparent;" // Transparent border
        "   border-radius: 10px;" // Rounded corners
        "   padding: 5px;" // Padding inside the button
        "   color: white;" // Text color
        "}"
        "QPushButton:hover {"
        "   background-color: #8c8c8c;" // Background color on hover
        "   border: 1px solid transparent;" // Transparent border
        "}"
        "QPushButton:pressed {"
        "   background-color: #C0E0FF;" // Background color when pressed
        "   border: 1px solid transparent;" // Transparent border
        "}"
    );
    #endif
}
    
void WordWrapButton::adjustText() 
{
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
