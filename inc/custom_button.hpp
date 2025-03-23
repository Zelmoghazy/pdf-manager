#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QString>

class WordWrapButton : public QPushButton 
{
public:
    WordWrapButton(const QString& text, QWidget* parent);
    void setText(const QString& text);
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    void setupButton();
    void adjustText();

    QString m_text;
    QLabel* m_label;
};
