#ifndef QRCODEHELPER_H
#define QRCODEHELPER_H

#include <QImage>
#include <QString>
#include <QPixmap>

class QrCodeHelper
{
public:
    // Generate a QR code image from text
    static QImage generateQrCode(const QString& text, int size = 200, int border = 4);

    // Generate a QR code as QPixmap
    static QPixmap generateQrCodePixmap(const QString& text, int size = 200, int border = 4);

    // Save QR code to file
    static bool saveQrCodeToFile(const QString& text, const QString& filePath,
                                 int size = 400, int border = 4);
};

#endif // QRCODEHELPER_H
