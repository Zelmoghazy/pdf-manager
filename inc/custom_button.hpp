#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QString>

class WordWrapButton : public QPushButton 
{
    Q_OBJECT
public:
    WordWrapButton(const QString& text, QWidget* parent);
    void setText(const QString& text);
    void setTimestamp(qint64 time);
    QString getText();
    qint64 getTimestamp();
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    void setupButton();
    void adjustText();

    QString m_text;
    QLabel* m_label;
    qint64  m_timestamp;
};
