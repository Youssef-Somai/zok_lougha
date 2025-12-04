#include "qrcodehelper.h"
#include "qrcodegen_real.hpp"
#include <QPainter>
#include <QDebug>
#include <vector>
#include <string>

using namespace qrcodegen;

QImage QrCodeHelper::generateQrCode(const QString& text, int size, int border)
{
    if (text.isEmpty()) {
        return QImage(size, size, QImage::Format_RGB32);
    }

    try {
        // Convert QString to std::string
        std::string textStr = text.toStdString();

        // Generate QR code using the real library
        QrCode qr = QrCode::encodeText(textStr.c_str(), QrCode::Ecc::MEDIUM);

        int qrSize = qr.getSize();
        int totalSize = qrSize + border * 2;

        // Create image
        QImage image(totalSize, totalSize, QImage::Format_RGB32);
        image.fill(Qt::white);

        // Draw QR code modules
        for (int y = 0; y < qrSize; y++) {
            for (int x = 0; x < qrSize; x++) {
                if (qr.getModule(x, y)) {
                    image.setPixel(x + border, y + border, qRgb(0, 0, 0));
                }
            }
        }

        // Scale to desired size
        QImage scaled = image.scaled(size, size, Qt::KeepAspectRatio, Qt::FastTransformation);

        qDebug() << "Real QR Code generated successfully!"
                 << "QR version:" << qr.getVersion()
                 << "Size:" << qrSize << "x" << qrSize
                 << "Data length:" << text.length();

        return scaled;

    } catch (const std::exception& e) {
        qDebug() << "QR Code generation error:" << e.what();
        // Return error image
        QImage errorImage(size, size, QImage::Format_RGB32);
        errorImage.fill(Qt::lightGray);

        QPainter painter(&errorImage);
        painter.setPen(Qt::red);
        painter.drawText(errorImage.rect(), Qt::AlignCenter, "QR Error");

        return errorImage;
    }
}

QPixmap QrCodeHelper::generateQrCodePixmap(const QString& text, int size, int border)
{
    return QPixmap::fromImage(generateQrCode(text, size, border));
}

bool QrCodeHelper::saveQrCodeToFile(const QString& text, const QString& filePath,
                                    int size, int border)
{
    QImage qrImage = generateQrCode(text, size, border);
    bool saved = qrImage.save(filePath);
    qDebug() << "QR Code save to" << filePath << ":" << (saved ? "Success" : "Failed");
    return saved;
}
