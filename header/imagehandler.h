#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QByteArray>
#include <QCamera>
#include <QMediaDevices>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QVideoWidget>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QBuffer>
#include <QDebug>

/**
 * @class ImageHandler
 * @brief Manages image acquisition from camera or file system and conversion for database storage
 *
 * This class provides functionality for:
 * - Uploading images from the file system
 * - Capturing images from laptop camera with live preview
 * - Converting images to QByteArray for BLOB storage
 * - Converting QByteArray back to QPixmap for display
 */
class ImageHandler : public QObject
{
    Q_OBJECT

public:
    explicit ImageHandler(QObject *parent = nullptr);
    ~ImageHandler();

    /**
     * @brief Opens a file dialog to select an image from device
     * @return QImage object containing the selected image, or null image if cancelled
     */
    static QImage uploadImageFromFile();

    /**
     * @brief Opens a camera capture dialog with live preview
     * @param parent Parent widget for the camera dialog
     * @return QImage object containing the captured image, or null image if cancelled
     */
    static QImage captureImageFromCamera(QWidget *parent = nullptr);

    /**
     * @brief Converts QImage to QByteArray for database BLOB storage
     * @param image The image to convert
     * @param format Image format (default: PNG for lossless compression)
     * @param quality Compression quality 0-100 (default: 95)
     * @return QByteArray containing the image data
     */
    static QByteArray imageToByteArray(const QImage &image, const char *format = "PNG", int quality = 95);

    /**
     * @brief Converts QByteArray from database to QPixmap for display
     * @param byteArray The byte array from database
     * @return QPixmap for display in UI
     */
    static QPixmap byteArrayToPixmap(const QByteArray &byteArray);

    /**
     * @brief Validates image size and format
     * @param image The image to validate
     * @param maxSizeMB Maximum size in megabytes (default: 10MB)
     * @return true if valid, false otherwise
     */
    static bool validateImage(const QImage &image, int maxSizeMB = 10);

    /**
     * @brief Resizes image while maintaining aspect ratio
     * @param image The image to resize
     * @param maxWidth Maximum width in pixels
     * @param maxHeight Maximum height in pixels
     * @return Resized QImage
     */
    static QImage resizeImage(const QImage &image, int maxWidth = 800, int maxHeight = 600);

signals:
    void imageSelected(const QImage &image);
    void imageCaptured(const QImage &image);
    void errorOccurred(const QString &error);
};

/**
 * @class CameraCaptureDialog
 * @brief Dialog for capturing images from camera with live preview
 */
class CameraCaptureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraCaptureDialog(QWidget *parent = nullptr);
    ~CameraCaptureDialog();

    QImage getCapturedImage() const { return capturedImage; }
    bool wasImageCaptured() const { return imageCaptured; }

private slots:
    void onCaptureClicked();
    void onCancelClicked();
    void onImageCaptured(int id, const QImage &preview);
    void onCameraError();

private:
    void setupUI();
    void initializeCamera();

    QCamera *camera;
    QImageCapture *imageCapture;
    QMediaCaptureSession *captureSession;
    QVideoWidget *videoWidget;
    QLabel *previewLabel;
    QPushButton *captureButton;
    QPushButton *cancelButton;
    QPushButton *acceptButton;
    QImage capturedImage;
    bool imageCaptured;
};

#endif // IMAGEHANDLER_H
