#include "imagehandler.h"
#include <QCameraDevice>
#include <QDir>

// ImageHandler Implementation

ImageHandler::ImageHandler(QObject *parent) : QObject(parent)
{
    qDebug() << "ImageHandler initialized";
}

ImageHandler::~ImageHandler()
{
    qDebug() << "ImageHandler destroyed";
}

QImage ImageHandler::uploadImageFromFile()
{
    qDebug() << "Opening file dialog for image selection";

    // Open file dialog to select an image
    QString fileName = QFileDialog::getOpenFileName(
        nullptr,
        "Sélectionner une image du matériel",
        QDir::homePath(),
        "Images (*.png *.jpg *.jpeg *.bmp *.gif);;Tous les fichiers (*.*)"
        );

    if (fileName.isEmpty()) {
        qDebug() << "Image selection cancelled by user";
        return QImage();
    }

    qDebug() << "Selected file:" << fileName;

    // Load the image
    QImage image(fileName);

    if (image.isNull()) {
        QMessageBox::critical(nullptr, "Erreur",
                              "Impossible de charger l'image sélectionnée.\nVeuillez vérifier le format du fichier.");
        qWarning() << "Failed to load image from" << fileName;
        return QImage();
    }

    qDebug() << "Image loaded successfully - Size:" << image.size()
             << "Format:" << image.format();

    // Validate image
    if (!validateImage(image)) {
        QMessageBox::warning(nullptr, "Image trop volumineuse",
                             "L'image sélectionnée est trop grande. Veuillez choisir une image plus petite (max 10 MB).");
        return QImage();
    }

    // Resize if necessary to optimize storage
    if (image.width() > 800 || image.height() > 600) {
        qDebug() << "Resizing image for optimal storage";
        image = resizeImage(image);
    }

    return image;
}

QImage ImageHandler::captureImageFromCamera(QWidget *parent)
{
    qDebug() << "Opening camera capture dialog";

    CameraCaptureDialog dialog(parent);

    if (dialog.exec() == QDialog::Accepted && dialog.wasImageCaptured()) {
        qDebug() << "Image captured successfully from camera";
        QImage image = dialog.getCapturedImage();

        // Resize if necessary
        if (image.width() > 800 || image.height() > 600) {
            qDebug() << "Resizing captured image for optimal storage";
            image = resizeImage(image);
        }

        return image;
    }

    qDebug() << "Camera capture cancelled";
    return QImage();
}

QByteArray ImageHandler::imageToByteArray(const QImage &image, const char *format, int quality)
{
    if (image.isNull()) {
        qWarning() << "Cannot convert null image to byte array";
        return QByteArray();
    }

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);

    bool success = image.save(&buffer, format, quality);

    if (!success) {
        qWarning() << "Failed to convert image to byte array";
        return QByteArray();
    }

    qDebug() << "Image converted to byte array - Size:" << byteArray.size() << "bytes";
    return byteArray;
}

QPixmap ImageHandler::byteArrayToPixmap(const QByteArray &byteArray)
{
    if (byteArray.isEmpty()) {
        qWarning() << "Cannot convert empty byte array to pixmap";
        return QPixmap();
    }

    QPixmap pixmap;
    bool success = pixmap.loadFromData(byteArray);

    if (!success) {
        qWarning() << "Failed to convert byte array to pixmap";
        return QPixmap();
    }

    qDebug() << "Byte array converted to pixmap - Size:" << pixmap.size();
    return pixmap;
}

bool ImageHandler::validateImage(const QImage &image, int maxSizeMB)
{
    if (image.isNull()) {
        qWarning() << "Cannot validate null image";
        return false;
    }

    // Calculate approximate size in bytes
    qint64 sizeInBytes = image.sizeInBytes();
    qint64 maxSizeInBytes = static_cast<qint64>(maxSizeMB) * 1024 * 1024;

    qDebug() << "Image validation - Size:" << sizeInBytes << "bytes, Max:" << maxSizeInBytes << "bytes";

    if (sizeInBytes > maxSizeInBytes) {
        qWarning() << "Image size exceeds maximum allowed size";
        return false;
    }

    // Check dimensions
    if (image.width() < 1 || image.height() < 1) {
        qWarning() << "Invalid image dimensions";
        return false;
    }

    return true;
}

QImage ImageHandler::resizeImage(const QImage &image, int maxWidth, int maxHeight)
{
    if (image.isNull()) {
        qWarning() << "Cannot resize null image";
        return image;
    }

    qDebug() << "Resizing image from" << image.size() << "to max" << maxWidth << "x" << maxHeight;

    return image.scaled(maxWidth, maxHeight,
                        Qt::KeepAspectRatio,
                        Qt::SmoothTransformation);
}

// CameraCaptureDialog Implementation

CameraCaptureDialog::CameraCaptureDialog(QWidget *parent)
    : QDialog(parent)
    , camera(nullptr)
    , imageCapture(nullptr)
    , captureSession(nullptr)
    , videoWidget(nullptr)
    , previewLabel(nullptr)
    , captureButton(nullptr)
    , cancelButton(nullptr)
    , acceptButton(nullptr)
    , imageCaptured(false)
{
    setWindowTitle("Capture d'image - Caméra");
    setMinimumSize(800, 600);

    setupUI();
    initializeCamera();

    qDebug() << "CameraCaptureDialog initialized";
}

CameraCaptureDialog::~CameraCaptureDialog()
{
    if (camera) {
        camera->stop();
        delete camera;
    }

    if (imageCapture) {
        delete imageCapture;
    }

    if (captureSession) {
        delete captureSession;
    }

    qDebug() << "CameraCaptureDialog destroyed";
}

void CameraCaptureDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Video widget for live camera preview (Qt 6)
    videoWidget = new QVideoWidget(this);
    videoWidget->setMinimumSize(640, 480);
    mainLayout->addWidget(videoWidget);

    // Preview label for captured image
    previewLabel = new QLabel(this);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setMinimumSize(640, 480);
    previewLabel->hide();
    previewLabel->setScaledContents(false);
    mainLayout->addWidget(previewLabel);

    // Buttons layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    captureButton = new QPushButton("Capturer", this);
    captureButton->setStyleSheet("QPushButton { padding: 10px; font-size: 14px; }");
    connect(captureButton, &QPushButton::clicked, this, &CameraCaptureDialog::onCaptureClicked);
    buttonLayout->addWidget(captureButton);

    acceptButton = new QPushButton("Utiliser cette image", this);
    acceptButton->setStyleSheet("QPushButton { padding: 10px; font-size: 14px; background-color: #4CAF50; color: white; }");
    acceptButton->hide();
    connect(acceptButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(acceptButton);

    cancelButton = new QPushButton("Annuler", this);
    cancelButton->setStyleSheet("QPushButton { padding: 10px; font-size: 14px; }");
    connect(cancelButton, &QPushButton::clicked, this, &CameraCaptureDialog::onCancelClicked);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

void CameraCaptureDialog::initializeCamera()
{
    // Get available cameras using Qt 6 QMediaDevices
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();

    if (cameras.isEmpty()) {
        QMessageBox::critical(this, "Erreur",
                              "Aucune caméra détectée sur cet ordinateur.\n"
                              "Veuillez connecter une caméra ou utiliser l'option d'upload d'image.");
        qWarning() << "No cameras available";
        reject();
        return;
    }

    qDebug() << "Available cameras:" << cameras.size();

    // Use the default camera (first available)
    camera = new QCamera(cameras.first(), this);

    // Create capture session (Qt 6 requirement)
    captureSession = new QMediaCaptureSession(this);
    captureSession->setCamera(camera);
    captureSession->setVideoOutput(videoWidget);

    // Setup image capture
    imageCapture = new QImageCapture(this);
    captureSession->setImageCapture(imageCapture);

    // Connect signals (Qt 6 style)
    connect(imageCapture, &QImageCapture::imageCaptured,
            this, &CameraCaptureDialog::onImageCaptured);
    connect(camera, &QCamera::errorOccurred,
            this, &CameraCaptureDialog::onCameraError);

    // Start camera
    camera->start();

    qDebug() << "Camera started successfully";
}

void CameraCaptureDialog::onCaptureClicked()
{
    if (!imageCapture || !camera) {
        qWarning() << "Camera or image capture not initialized";
        return;
    }

    qDebug() << "Capturing image from camera";

    // Capture image
    imageCapture->capture();
}

void CameraCaptureDialog::onCancelClicked()
{
    qDebug() << "Camera capture cancelled by user";
    reject();
}

void CameraCaptureDialog::onImageCaptured(int id, const QImage &preview)
{
    Q_UNUSED(id);

    qDebug() << "Image captured - ID:" << id << "Size:" << preview.size();

    // Store the captured image
    capturedImage = preview;
    imageCaptured = true;

    // Hide video widget and show preview
    videoWidget->hide();
    captureButton->hide();

    // Display captured image
    QPixmap pixmap = QPixmap::fromImage(preview);

    // Scale pixmap to fit label while maintaining aspect ratio
    QPixmap scaledPixmap = pixmap.scaled(previewLabel->size(),
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);

    previewLabel->setPixmap(scaledPixmap);
    previewLabel->show();
    acceptButton->show();

    // Stop camera
    if (camera) {
        camera->stop();
    }

    qDebug() << "Image preview displayed";
}

void CameraCaptureDialog::onCameraError()
{
    if (!camera) return;

    QString errorMsg = QString("Erreur caméra: %1").arg(camera->errorString());
    qWarning() << "Camera error:" << errorMsg;

    QMessageBox::critical(this, "Erreur caméra",
                          QString("Erreur lors de l'utilisation de la caméra:\n%1").arg(errorMsg));
}
