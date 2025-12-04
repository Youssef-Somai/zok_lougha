#include "qrscannerdialog.h"
#include "qrcodegen_real.hpp"
#include "qrdecoder.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QPainter>
#include <QTimer>
#include <QDebug>

QRScannerDialog::QRScannerDialog(QWidget *parent)
    : QDialog(parent)
    , camera(nullptr)
    , isScanning(false)
{
    setWindowTitle("Scanner QR Code - Caméra");
    resize(640, 580);
    setupUI();
}

QRScannerDialog::~QRScannerDialog()
{
    if (camera) {
        camera->stop();
        delete camera;
    }
}

void QRScannerDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Status label
    statusLabel = new QLabel("Cliquez sur 'Démarrer' pour activer la caméra", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("QLabel { padding: 10px; background-color: #E3F2FD; "
                               "border: 2px solid #2196F3; border-radius: 5px; "
                               "font-size: 11pt; }");
    mainLayout->addWidget(statusLabel);

    // Video viewfinder
    viewfinder = new QVideoWidget(this);
    viewfinder->setMinimumSize(640, 480);
    viewfinder->setStyleSheet("QVideoWidget { background-color: #000; border: 2px solid #666; }");
    mainLayout->addWidget(viewfinder);

    // Result label
    resultLabel = new QLabel("", this);
    resultLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setWordWrap(true);
    resultLabel->setStyleSheet("QLabel { padding: 10px; background-color: #F5F5F5; "
                              "border: 2px solid #4CAF50; border-radius: 5px; "
                              "font-family: 'Courier New'; font-size: 10pt; }");
    resultLabel->setVisible(false);
    mainLayout->addWidget(resultLabel);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    startButton = new QPushButton("▶ Démarrer Caméra", this);
    startButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; "
                              "padding: 10px 20px; font-size: 11pt; border-radius: 5px; }"
                              "QPushButton:hover { background-color: #45a049; }");
    connect(startButton, &QPushButton::clicked, this, &QRScannerDialog::startCamera);
    buttonLayout->addWidget(startButton);

    stopButton = new QPushButton("■ Arrêter", this);
    stopButton->setEnabled(false);
    stopButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; "
                             "padding: 10px 20px; font-size: 11pt; border-radius: 5px; }"
                             "QPushButton:hover { background-color: #da190b; }"
                             "QPushButton:disabled { background-color: #ccc; }");
    connect(stopButton, &QPushButton::clicked, this, &QRScannerDialog::stopCamera);
    buttonLayout->addWidget(stopButton);

    manualButton = new QPushButton("⌨ Saisie Manuelle", this);
    manualButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; "
                               "padding: 10px 20px; font-size: 11pt; border-radius: 5px; }"
                               "QPushButton:hover { background-color: #0b7dda; }");
    connect(manualButton, &QPushButton::clicked, this, &QRScannerDialog::onManualInput);
    buttonLayout->addWidget(manualButton);

    closeButton = new QPushButton("✖ Fermer", this);
    closeButton->setStyleSheet("QPushButton { background-color: #607D8B; color: white; "
                              "padding: 10px 20px; font-size: 11pt; border-radius: 5px; }"
                              "QPushButton:hover { background-color: #455A64; }");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(closeButton);

    mainLayout->addLayout(buttonLayout);

    // Instructions
    QLabel *instructions = new QLabel(
        "Instructions:\n"
        "1. Cliquez sur 'Démarrer Caméra' pour activer la webcam\n"
        "2. Placez le QR code devant la caméra\n"
        "3. Le QR code sera scanné automatiquement\n"
        "4. Ou utilisez 'Saisie Manuelle' pour entrer l'ID directement",
        this);
    instructions->setStyleSheet("QLabel { color: #666; font-size: 9pt; padding: 10px; }");
    instructions->setWordWrap(true);
    mainLayout->addWidget(instructions);
}

void QRScannerDialog::startCamera()
{
    // Get available cameras
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();

    if (cameras.isEmpty()) {
        QMessageBox::warning(this, "Erreur",
                           "Aucune caméra détectée!\n"
                           "Veuillez utiliser la saisie manuelle.");
        return;
    }

    // Use the default camera
    camera = new QCamera(cameras.first(), this);

    // Create media capture session
    QMediaCaptureSession *captureSession = new QMediaCaptureSession(this);
    captureSession->setCamera(camera);
    captureSession->setVideoOutput(viewfinder);

    // Get the video sink from the viewfinder (so we can process frames AND display video)
    QVideoSink *videoSink = viewfinder->videoSink();

    // Connect to process frames
    connect(videoSink, &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame &frame) {
        if (isScanning) {
            processFrame(frame);
        }
    });

    // Start camera
    camera->start();

    if (camera->isActive()) {
        statusLabel->setText("✓ Caméra active - Détection automatique activée!");
        statusLabel->setStyleSheet("QLabel { padding: 10px; background-color: #C8E6C9; "
                                  "border: 2px solid #4CAF50; border-radius: 5px; "
                                  "font-size: 11pt; }");
        startButton->setEnabled(false);
        stopButton->setEnabled(true);
        isScanning = true;

        qDebug() << "✓ Camera started with automatic QR detection!";
    } else {
        QMessageBox::critical(this, "Erreur",
                            "Impossible de démarrer la caméra!");
    }
}

void QRScannerDialog::stopCamera()
{
    if (camera) {
        camera->stop();
        delete camera;
        camera = nullptr;
    }

    statusLabel->setText("Caméra arrêtée");
    statusLabel->setStyleSheet("QLabel { padding: 10px; background-color: #E3F2FD; "
                              "border: 2px solid #2196F3; border-radius: 5px; "
                              "font-size: 11pt; }");
    startButton->setEnabled(true);
    stopButton->setEnabled(false);
    isScanning = false;
}

void QRScannerDialog::processFrame(const QVideoFrame &frame)
{
    if (!isScanning) return;

    // Convert frame to QImage
    QVideoFrame clonedFrame(frame);
    if (!clonedFrame.map(QVideoFrame::ReadOnly)) {
        return;
    }

    QImage image = clonedFrame.toImage();
    clonedFrame.unmap();

    if (image.isNull()) return;

    // Try to decode QR code
    QString decoded = QRDecoder::decode(image);

    if (!decoded.isEmpty()) {
        qDebug() << "🎉 QR Code detected and decoded:" << decoded;

        // Stop scanning
        isScanning = false;

        // Update status
        statusLabel->setText("✓ QR Code détecté!");
        statusLabel->setStyleSheet("QLabel { padding: 10px; background-color: #C8E6C9; "
                                  "border: 2px solid #4CAF50; border-radius: 5px; "
                                  "font-size: 11pt; font-weight: bold; }");

        // Show result
        resultLabel->setText(QString("✓ Données détectées: %1").arg(decoded.left(50)));
        resultLabel->setVisible(true);

        // Set the scanned data
        scannedData = decoded;

        // Stop camera and close dialog after a short delay
        QTimer::singleShot(500, this, [this]() {
            stopCamera();
            QTimer::singleShot(300, this, &QDialog::accept);
        });
    }
}

QString QRScannerDialog::decodeQRCode(const QImage &image)
{
    // Placeholder for QR code decoding
    // Would use ZXing-cpp or similar library
    Q_UNUSED(image);
    return QString();
}

void QRScannerDialog::onManualInput()
{
    bool ok;
    int id = QInputDialog::getInt(this,
                                   "Saisie Manuelle",
                                   "Entrez l'ID du matériel:",
                                   1, 1, 999999, 1, &ok);

    if (ok) {
        scannedData = QString::number(id);
        resultLabel->setText(QString("✓ ID saisi: %1").arg(id));
        resultLabel->setVisible(true);

        QMessageBox::information(this, "ID Saisi",
                                QString("ID du matériel: %1\n\n"
                                       "Cliquez sur 'Fermer' pour rechercher ce matériel.")
                                    .arg(id));
        accept(); // Close dialog with success
    }
}
