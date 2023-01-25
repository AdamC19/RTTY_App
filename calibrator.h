#ifndef CALIBRATOR_H
#define CALIBRATOR_H

#include <QThread>
#include <QObject>

class Calibrator : public QThread
{
    Q_OBJECT
    void run() override;
public:
    explicit Calibrator(QObject *parent = nullptr);
signals:

};

#endif // CALIBRATOR_H
