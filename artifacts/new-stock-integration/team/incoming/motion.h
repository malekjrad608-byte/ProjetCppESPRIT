#pragma once
#include <QObject>
#include <QColor>
class QWidget;
class QLabel;
class QProgressBar;

// One preference controls every decorative animation, including the logo GIF.
class Motion : public QObject {
    Q_OBJECT
public:
    static Motion &instance();
    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled, bool persist = false);
    static void reveal(QWidget *widget, const QColor &background = Qt::white, int duration = 190);
    static void countTo(QLabel *label, int value, bool fromZero = false);
    static void progressTo(QProgressBar *bar, int value, bool fromZero = false);
signals:
    void changed(bool enabled);
private:
    explicit Motion(QObject *parent);
    bool m_enabled;
};
