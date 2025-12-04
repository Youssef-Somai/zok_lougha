#ifndef AIVERIFIER_H
#define AIVERIFIER_H

#include <QObject>
#include <QImage>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>
#include <QBuffer>

/**
 * @struct VerificationResult
 * @brief Contains the result of AI image verification
 */
struct VerificationResult {
    bool isValid;               // True if image matches category
    QString category;           // Detected category
    QString expectedCategory;   // Expected category from user input
    double confidence;          // Confidence score (0.0 - 1.0)
    QString details;            // Detailed explanation from AI
    QString errorMessage;       // Error message if verification failed

    VerificationResult()
        : isValid(false), confidence(0.0) {}
};

/**
 * @class AIVerifier
 * @brief Handles AI-based verification of material images against their categories
 *
 * This class provides functionality for:
 * - Verifying that an image matches the selected material category using AI
 * - Blocking database insertion if verification fails
 * - Providing detailed feedback about the verification result
 * - Using OpenAI Vision API or custom AI service for image analysis
 */
class AIVerifier : public QObject
{
    Q_OBJECT

public:
    explicit AIVerifier(QObject *parent = nullptr);
    ~AIVerifier();

    /**
     * @brief Verifies that the image matches the expected material category
     * @param image The material image to verify
     * @param expectedCategory The category selected by user (Informatique, Bureautique, Sport, Autre)
     * @param parentWidget Parent widget for displaying messages
     * @return VerificationResult containing validation status and details
     *
     * This method is BLOCKING and must complete before allowing database insert.
     * It uses AI to analyze the image and determine if it matches the category.
     */
    static VerificationResult verifyImageCategory(
        const QImage &image,
        const QString &expectedCategory,
        QWidget *parentWidget = nullptr
        );

    /**
     * @brief Sets the Gemini API key
     * @param apiKey Your Gemini API key from https://aistudio.google.com/app/apikey
     */
    static void setApiKey(const QString &apiKey);

    /**
     * @brief Sets the AI service endpoint URL (optional - auto-set based on provider)
     * @param url The endpoint URL for the AI service
     */
    static void setServiceUrl(const QString &url);

    /**
     * @brief Shows a detailed verification result dialog to the user
     * @param result The verification result to display
     * @param parentWidget Parent widget for the dialog
     * @return true if user wants to proceed despite warning, false to cancel
     */
    static bool showVerificationDialog(const VerificationResult &result, QWidget *parentWidget = nullptr);

signals:
    void verificationStarted(const QString &category);
    void verificationCompleted(const VerificationResult &result);
    void verificationError(const QString &error);

private:
    /**
     * @brief Calls Google Gemini Vision API to verify the image
     * @param imageData The image data as base64 encoded string
     * @param expectedCategory The expected category
     * @return VerificationResult from Gemini AI analysis
     */
    static VerificationResult callGeminiAPI(const QString &imageData, const QString &expectedCategory);

    /**
     * @brief Converts category name to French for AI prompt
     * @param category The category name
     * @return French translation for better AI understanding
     */
    static QString getCategoryDescription(const QString &category);

    /**
     * @brief Builds the AI prompt for image verification
     * @param expectedCategory The expected category
     * @return Formatted prompt for AI service
     */
    static QString buildVerificationPrompt(const QString &expectedCategory);

    /**
     * @brief Parses Gemini JSON response
     */
    static VerificationResult parseGeminiResponse(const QJsonDocument &jsonResponse, const QString &expectedCategory);

    /**
     * @brief Performs local rule-based verification as fallback
     * @param image The image to verify
     * @param expectedCategory The expected category
     * @return VerificationResult from local analysis
     */
    static VerificationResult performLocalVerification(const QImage &image, const QString &expectedCategory);

    static QString apiKey;
    static QString serviceUrl;
    static const int REQUEST_TIMEOUT_MS = 30000; // 30 seconds timeout
};

#endif // AIVERIFIER_H
