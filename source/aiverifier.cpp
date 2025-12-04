#include "aiverifier.h"
#include "imagehandler.h"
#include <QApplication>

// Static member initialization
QString AIVerifier::apiKey = "";  // Configure this with your Gemini API key
QString AIVerifier::serviceUrl = "https://generativelanguage.googleapis.com/v1/models/gemini-2.5-flash:generateContent";  // Gemini 2.5 Flash (latest stable)

AIVerifier::AIVerifier(QObject *parent) : QObject(parent)
{
    qDebug() << "AIVerifier initialized";
}

AIVerifier::~AIVerifier()
{
    qDebug() << "AIVerifier destroyed";
}

void AIVerifier::setApiKey(const QString &key)
{
    apiKey = key;
    qDebug() << "Gemini API key configured";
}

void AIVerifier::setServiceUrl(const QString &url)
{
    serviceUrl = url;
    qDebug() << "AI service URL set to:" << url;
}

QString AIVerifier::getCategoryDescription(const QString &category)
{
    // Translate categories to detailed French descriptions for AI
    if (category == "Informatique") {
        return "matériel informatique (ordinateur, clavier, souris, écran, imprimante, serveur, câble réseau, etc.)";
    } else if (category == "Bureautique") {
        return "matériel de bureautique (stylo, cahier, classeur, agrafeuse, perforatrice, calculatrice, etc.)";
    } else if (category == "Sport") {
        return "matériel de sport (ballon, raquette, tapis de yoga, haltères, filet, équipement sportif, etc.)";
    } else if (category == "Autre") {
        return "autre matériel (meubles, équipement divers, etc.)";
    }
    return category;
}

QString AIVerifier::buildVerificationPrompt(const QString &expectedCategory)
{
    QString categoryDesc = getCategoryDescription(expectedCategory);

    QString prompt = QString(
        "Vous êtes un système de vérification d'images pour une application de gestion de matériel de club d'été.\n\n"
        "Analysez cette image et déterminez si elle correspond à la catégorie: '%1' qui inclut: %2\n\n"
        "Répondez UNIQUEMENT avec un objet JSON au format suivant (sans texte supplémentaire):\n"
        "{\n"
        "  \"matches\": true ou false,\n"
        "  \"detected_category\": \"la catégorie détectée\",\n"
        "  \"confidence\": score entre 0 et 1,\n"
        "  \"explanation\": \"explication détaillée en français\"\n"
        "}\n\n"
        "Soyez strict: répondez false si l'image ne correspond clairement pas à la catégorie attendue."
    ).arg(expectedCategory, categoryDesc);

    return prompt;
}

VerificationResult AIVerifier::callGeminiAPI(const QString &imageData, const QString &expectedCategory)
{
    VerificationResult result;
    result.expectedCategory = expectedCategory;

    qDebug() << "Calling Gemini 2.0 Flash for category verification:";

    // Create network manager
    QNetworkAccessManager manager;

    // Build request URL with API key as query parameter (Gemini style)
    QString urlWithKey = serviceUrl + "?key=" + apiKey;
    QNetworkRequest request{QUrl(urlWithKey)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Build JSON payload for Gemini Vision API
    QJsonObject payload;

    // Add generation config
    QJsonObject generationConfig;
    generationConfig["temperature"] = 0.3;
    generationConfig["maxOutputTokens"] = 500;
    payload["generationConfig"] = generationConfig;

    // Build contents array
    QJsonArray contents;
    QJsonObject content;
    content["role"] = "user";

    QJsonArray parts;

    // Add text prompt
    QJsonObject textPart;
    textPart["text"] = buildVerificationPrompt(expectedCategory);
    parts.append(textPart);

    // Add image (inline data for Gemini)
    QJsonObject imagePart;
    QJsonObject inlineData;
    inlineData["mime_type"] = "image/jpeg";
    inlineData["data"] = imageData;
    imagePart["inline_data"] = inlineData;
    parts.append(imagePart);

    content["parts"] = parts;
    contents.append(content);
    payload["contents"] = contents;

    QJsonDocument doc(payload);
    QByteArray jsonData = doc.toJson();

    qDebug() << "Sending request to Gemini API...";
    qDebug() << "Payload size:" << jsonData.size() << "bytes";

    // Send request with timeout
    QNetworkReply *reply = manager.post(request, jsonData);

    // Setup timeout
    QTimer timer;
    timer.setSingleShot(true);

    // Event loop for synchronous request
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(REQUEST_TIMEOUT_MS);
    loop.exec();

    // Check if timeout occurred
    if (!timer.isActive()) {
        qWarning() << "Gemini API request timeout";
        result.errorMessage = "Timeout de la requête API";
        reply->abort();
        reply->deleteLater();
        return result;
    }

    timer.stop();

    // Check for network errors
    if (reply->error() != QNetworkReply::NoError) {
        // Get HTTP status code
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        qWarning() << "Network error:" << reply->errorString();
        qWarning() << "HTTP Status Code:" << statusCode;

        // Specific handling for rate limit (429)
        if (statusCode == 429) {
            result.errorMessage = "RATE LIMIT DÉPASSÉ!\n\n"
                                 "Vous avez fait trop de requêtes.\n"
                                 "Limites gratuites:\n"
                                 "• 15 requêtes par minute\n"
                                 "• 1,500 requêtes par jour\n\n"
                                 "Solutions:\n"
                                 "1. Attendez 1 minute et réessayez\n"
                                 "2. Attendez demain (00:00 UTC)\n"
                                 "3. Créez une nouvelle clé API";
            qCritical() << "⚠⚠⚠ RATE LIMIT (429) - Too many requests!";
            qCritical() << "⚠ Wait 1 minute or use a new API key";
        } else {
            result.errorMessage = QString("Erreur réseau (HTTP %1): %2")
                                 .arg(statusCode)
                                 .arg(reply->errorString());
        }

        reply->deleteLater();
        return result;
    }

    // Parse response
    QByteArray responseData = reply->readAll();
    reply->deleteLater();

    qDebug() << "Received response from Gemini API - Size:" << responseData.size() << "bytes";

    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);

    if (jsonResponse.isNull()) {
        qWarning() << "Failed to parse Gemini response JSON";
        result.errorMessage = "Réponse Gemini invalide";
        return result;
    }

    // Parse the response
    return parseGeminiResponse(jsonResponse, expectedCategory);
}

VerificationResult AIVerifier::parseGeminiResponse(const QJsonDocument &jsonResponse, const QString &expectedCategory)
{
    VerificationResult result;
    result.expectedCategory = expectedCategory;

    qDebug() << "Parsing Gemini response...";

    QJsonObject root = jsonResponse.object();

    // Check for API errors
    if (root.contains("error")) {
        QJsonObject error = root["error"].toObject();
        QString errorMsg = error["message"].toString();
        qWarning() << "Gemini API error:" << errorMsg;
        result.errorMessage = QString("Erreur API: %1").arg(errorMsg);
        return result;
    }

    // Extract the Gemini response
    if (!root.contains("candidates")) {
        qWarning() << "Invalid Gemini response format - missing 'candidates'";
        result.errorMessage = "Format de réponse invalide";
        return result;
    }

    QJsonArray candidates = root["candidates"].toArray();
    if (candidates.isEmpty()) {
        qWarning() << "Gemini response contains no candidates";
        result.errorMessage = "Réponse vide";
        return result;
    }

    QJsonObject candidate = candidates[0].toObject();
    QJsonObject content = candidate["content"].toObject();
    QJsonArray parts = content["parts"].toArray();

    if (parts.isEmpty()) {
        qWarning() << "Gemini response has no parts";
        result.errorMessage = "Réponse vide";
        return result;
    }

    QString textContent = parts[0].toObject()["text"].toString().trimmed();
    qDebug() << "Gemini response content:" << textContent;

    // Parse the JSON content from Gemini
    QString jsonContent = textContent;
    if (textContent.startsWith("```json")) {
        int startIdx = textContent.indexOf('{');
        int endIdx = textContent.lastIndexOf('}');
        if (startIdx >= 0 && endIdx >= 0) {
            jsonContent = textContent.mid(startIdx, endIdx - startIdx + 1);
        }
    } else if (textContent.startsWith("```")) {
        int startIdx = textContent.indexOf('{');
        int endIdx = textContent.lastIndexOf('}');
        if (startIdx >= 0 && endIdx >= 0) {
            jsonContent = textContent.mid(startIdx, endIdx - startIdx + 1);
        }
    }

    QJsonDocument verificationDoc = QJsonDocument::fromJson(jsonContent.toUtf8());
    if (verificationDoc.isNull()) {
        qWarning() << "Failed to parse verification result JSON from Gemini response";
        result.errorMessage = "Format de vérification invalide";
        return result;
    }

    QJsonObject verification = verificationDoc.object();

    // Extract verification fields
    result.isValid = verification["matches"].toBool();
    result.category = verification["detected_category"].toString();
    result.confidence = verification["confidence"].toDouble();
    result.details = verification["explanation"].toString();

    qDebug() << "Gemini verification result parsed:"
             << "Valid:" << result.isValid
             << "Category:" << result.category
             << "Confidence:" << result.confidence;

    return result;
}

VerificationResult AIVerifier::performLocalVerification(const QImage &image, const QString &expectedCategory)
{
    qDebug() << "Performing local verification as fallback";

    VerificationResult result;
    result.expectedCategory = expectedCategory;

    // Simple heuristic-based verification as fallback
    // In production, this could use local ML models or more sophisticated rules

    if (image.isNull() || image.width() < 10 || image.height() < 10) {
        result.isValid = false;
        result.confidence = 0.0;
        result.details = "Image invalide ou trop petite pour être vérifiée.";
        qWarning() << "Invalid image for local verification";
        return result;
    }

    // For now, we'll use a permissive approach as fallback
    // In production, you could add color analysis, edge detection, etc.
    result.isValid = true;  // Allow with warning when AI service unavailable
    result.confidence = 0.5;  // Medium confidence for local verification
    result.category = expectedCategory;
    result.details = QString(
        "Vérification locale effectuée (service AI non disponible).\n"
        "Image acceptée avec une confiance moyenne.\n"
        "Assurez-vous que l'image correspond bien à la catégorie '%1'."
    ).arg(expectedCategory);

    qDebug() << "Local verification completed with medium confidence";

    return result;
}

VerificationResult AIVerifier::verifyImageCategory(
    const QImage &image,
    const QString &expectedCategory,
    QWidget *parentWidget)
{
    qDebug() << "Starting image verification for category:" << expectedCategory;

    VerificationResult result;
    result.expectedCategory = expectedCategory;

    // Validate inputs
    if (image.isNull()) {
        result.isValid = false;
        result.errorMessage = "Image null - impossible de vérifier";
        qWarning() << "Cannot verify null image";
        return result;
    }

    if (expectedCategory.isEmpty()) {
        result.isValid = false;
        result.errorMessage = "Catégorie non spécifiée";
        qWarning() << "Cannot verify without category";
        return result;
    }

    // Show progress message
    QMessageBox *progressBox = nullptr;
    if (parentWidget) {
        progressBox = new QMessageBox(parentWidget);
        progressBox->setWindowTitle("Vérification AI");
        progressBox->setText("Vérification de l'image en cours...\nVeuillez patienter.");
        progressBox->setStandardButtons(QMessageBox::NoButton);
        progressBox->setModal(true);
        progressBox->show();
        QApplication::processEvents();
    }

    try {
        // Convert image to base64 for API
        QByteArray imageData = ImageHandler::imageToByteArray(image, "JPEG", 85);
        QString base64Image = imageData.toBase64();

        qDebug() << "Image converted to base64 - Size:" << base64Image.length() << "chars";

        // Call Gemini API - NO FALLBACK, ONLY GEMINI
        if (!apiKey.isEmpty()) {
            result = callGeminiAPI(base64Image, expectedCategory);

            // If API failed, DO NOT use local verification
            // Let the error propagate and block insertion
            if (!result.errorMessage.isEmpty()) {
                qCritical() << "❌ Gemini API failed - BLOCKING insertion";
                qCritical() << "❌ NO local fallback - AI verification is REQUIRED";
                result.isValid = false;  // Force invalid
                result.confidence = 0.0;  // Zero confidence
                // Keep the error message from Gemini API
            }
        } else {
            qCritical() << "❌ No API key configured - CANNOT verify";
            result.isValid = false;
            result.confidence = 0.0;
            result.errorMessage = "Clé API Gemini non configurée!\n\n"
                                 "La vérification AI est OBLIGATOIRE.\n"
                                 "Configurez votre clé Gemini dans config.ini";
        }

    } catch (const std::exception &e) {
        qCritical() << "Exception during verification:" << e.what();
        result.isValid = false;
        result.errorMessage = QString("Erreur: %1").arg(e.what());
    }

    // Close progress dialog
    if (progressBox) {
        progressBox->close();
        delete progressBox;
    }

    qDebug() << "Verification completed - Result:" << result.isValid;

    return result;
}

bool AIVerifier::showVerificationDialog(const VerificationResult &result, QWidget *parentWidget)
{
    QString title;
    QString message;

    if (result.isValid) {
        title = "✓ Vérification Réussie";

        message = QString(
            "<h3>L'image a été vérifiée avec succès!</h3>"
            "<p><b>Catégorie attendue:</b> %1</p>"
            "<p><b>Catégorie détectée:</b> %2</p>"
            "<p><b>Niveau de confiance:</b> %3%</p>"
            "<p><b>Détails:</b> %4</p>"
        ).arg(
            result.expectedCategory,
            result.category,
            QString::number(result.confidence * 100, 'f', 1),
            result.details
        );

        QMessageBox::information(parentWidget, title, message);
        return true;

    } else {
        title = "✗ Vérification Échouée";

        if (!result.errorMessage.isEmpty()) {
            // Technical error occurred
            message = QString(
                "<h3>Erreur lors de la vérification</h3>"
                "<p><b>Erreur:</b> %1</p>"
                "<p>La vérification AI n'a pas pu être complétée.</p>"
                "<p><b>Voulez-vous continuer quand même?</b></p>"
                "<p style='color: red;'><i>Attention: L'image n'a pas été vérifiée par l'IA.</i></p>"
            ).arg(result.errorMessage);

            QMessageBox msgBox(parentWidget);
            msgBox.setWindowTitle(title);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(message);
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);

            return msgBox.exec() == QMessageBox::Yes;

        } else {
            // Verification failed - image doesn't match category
            message = QString(
                "<h3>L'image ne correspond PAS à la catégorie sélectionnée!</h3>"
                "<p><b>Catégorie attendue:</b> %1</p>"
                "<p><b>Catégorie détectée:</b> %2</p>"
                "<p><b>Niveau de confiance:</b> %3%</p>"
                "<p><b>Explication:</b> %4</p>"
                "<hr>"
                "<p style='color: red;'><b>⚠ ATTENTION: Cette image ne peut pas être ajoutée!</b></p>"
                "<p>Veuillez sélectionner une image qui correspond à la catégorie '%1'.</p>"
            ).arg(
                result.expectedCategory,
                result.category,
                QString::number(result.confidence * 100, 'f', 1),
                result.details
            );

            QMessageBox msgBox(parentWidget);
            msgBox.setWindowTitle(title);
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText(message);
            msgBox.setStandardButtons(QMessageBox::Ok);

            msgBox.exec();

            // Do NOT allow insertion when verification fails
            return false;
        }
    }
}

