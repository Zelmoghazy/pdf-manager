#pragma once

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QContextMenuEvent>
#include <QAction>
#include <QClipBoard>
#include <QLabel>
#include <QMenu>
#include <QString>

class PDFButton : public QPushButton 
{
    Q_OBJECT
public:
    PDFButton(const QString& text, QWidget* parent);
    void setText(const QString& text);
    void setTimestamp(qint64 time);
    QString getText();
    qint64 getTimestamp();
signals:
    void editRequested(PDFButton* button);
    void deleteRequested(PDFButton* button);
private slots:
    void checkAndAdjustText();
protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override; 
private:
    void setupButton();
    void adjustText();
    bool isFullyVisible() const;

    QString m_text;
    QLabel* m_label;
    qint64  m_timestamp;
};
