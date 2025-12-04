#ifndef QRDECODER_H
#define QRDECODER_H

#include <QImage>
#include <QString>

class QRDecoder
{
public:
    // Basic QR decoder - returns decoded text or empty string
    static QString decode(const QImage &image);

    // Check if image likely contains a QR code
    static bool hasQRCode(const QImage &image);

private:
    // Helper functions for basic pattern detection
    static bool detectFinderPatterns(const QImage &image);
    static QString extractData(const QImage &image);
};

#endif // QRDECODER_H
