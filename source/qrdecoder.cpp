#include "qrdecoder.h"
#include <QDebug>
#include <cstring>

extern "C" {
#include "quirc.h"
}

QString QRDecoder::decode(const QImage &image)
{
    if (image.isNull()) {
        return QString();
    }

    // Convert image to grayscale if needed
    QImage grayImage = image.convertToFormat(QImage::Format_Grayscale8);

    if (grayImage.isNull()) {
        qDebug() << "Failed to convert image to grayscale";
        return QString();
    }

    // Create quirc decoder
    struct quirc *qr = quirc_new();
    if (!qr) {
        qDebug() << "Failed to create quirc decoder";
        return QString();
    }

    // Resize decoder to match image dimensions
    if (quirc_resize(qr, grayImage.width(), grayImage.height()) < 0) {
        qDebug() << "Failed to resize quirc decoder";
        quirc_destroy(qr);
        return QString();
    }

    // Get buffer and copy image data
    int w, h;
    uint8_t *buffer = quirc_begin(qr, &w, &h);

    // Copy grayscale image data to quirc buffer
    for (int y = 0; y < h; y++) {
        const uint8_t *scanLine = grayImage.constScanLine(y);
        memcpy(buffer + y * w, scanLine, w);
    }

    // Process the image
    quirc_end(qr);

    // Check if any QR codes were found
    int count = quirc_count(qr);
    QString result;

    if (count > 0) {
        // Extract and decode the first QR code
        struct quirc_code code;
        struct quirc_data data;

        quirc_extract(qr, 0, &code);
        quirc_decode_error_t err = quirc_decode(&code, &data);

        if (err == QUIRC_SUCCESS) {
            result = QString::fromUtf8(reinterpret_cast<const char*>(data.payload), data.payload_len);
            qDebug() << "✓ QR Code decoded successfully:" << result.left(50) << "...";
        } else {
            qDebug() << "QR decode error:" << quirc_strerror(err);
        }
    }

    quirc_destroy(qr);
    return result;
}

bool QRDecoder::hasQRCode(const QImage &image)
{
    QString decoded = decode(image);
    return !decoded.isEmpty();
}

bool QRDecoder::detectFinderPatterns(const QImage &image)
{
    return hasQRCode(image);
}

QString QRDecoder::extractData(const QImage &image)
{
    return decode(image);
}
