#ifndef QRSCANNERDIALOG_H
#define QRSCANNERDIALOG_H

#include <QDialog>
#include <QCamera>
#include <QVideoWidget>
#include <QVideoFrame>
#include <QLabel>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class QRScannerDialog; }
QT_END_NAMESPACE

class QRScannerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QRScannerDialog(QWidget *parent = nullptr);
    ~QRScannerDialog();

    QString getScannedData() const { return scannedData; }

private slots:
    void startCamera();
    void stopCamera();
    void processFrame(const QVideoFrame &frame);
    void onManualInput();

private:
    QCamera *camera;
    QVideoWidget *viewfinder;
    QLabel *statusLabel;
    QLabel *resultLabel;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *manualButton;
    QPushButton *closeButton;

    QString scannedData;
    bool isScanning;

    void setupUI();
    QString decodeQRCode(const QImage &image);
};

#endif // QRSCANNERDIALOG_H
