#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QMap>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

class PerformanceTimer : public QObject
{
    Q_OBJECT

public:
    static PerformanceTimer& instance()
    {
        static PerformanceTimer instance;
        return instance;
    }

    void startTimer(const QString& functionName)
    {
        m_timers[functionName].restart();
    }

    qint64 stopTimer(const QString& functionName)
    {
        if (!m_timers.contains(functionName))
            return -1;

        qint64 elapsed = m_timers[functionName].elapsed();
        m_results[functionName] = elapsed;
        return elapsed;
    }

    void saveResults(const QString& filePath = "")
    {
        QString path = filePath;
        if (path.isEmpty()) {
            QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
            path = QString("performance_log_%1.txt").arg(timestamp);
        }

        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            
            out << "Performance Measurements - " << QDateTime::currentDateTime().toString() << "\n";
            out << "===============================================\n\n";
            
            // Sort results by execution time (descending)
            QList<QPair<QString, qint64>> sortedResults;
            QMapIterator<QString, qint64> i(m_results);
            while (i.hasNext()) {
                i.next();
                sortedResults.append(QPair<QString, qint64>(i.key(), i.value()));
            }
            
            std::sort(sortedResults.begin(), sortedResults.end(), 
                [](const QPair<QString, qint64>& a, const QPair<QString, qint64>& b) {
                    return a.second > b.second;
                });
            
            // Print all results sorted
            for (const auto& result : sortedResults) {
                out << result.first << ": " << result.second << " ms\n";
            }
            
            file.close();
            qDebug() << "Performance results saved to:" << path;
        } else {
            qWarning() << "Could not open file for writing:" << path;
        }
    }

private:
    PerformanceTimer() {}
    ~PerformanceTimer() {}
    
    QMap<QString, QElapsedTimer> m_timers;
    QMap<QString, qint64> m_results;
};

#define START_TIMING(func) PerformanceTimer::instance().startTimer(func)
#define STOP_TIMING(func) PerformanceTimer::instance().stopTimer(func)
#define SAVE_TIMING_RESULTS(path) PerformanceTimer::instance().saveResults(path)

