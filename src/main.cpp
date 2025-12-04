#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include "connection.h"
#include "aiverifier.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // ========================================================================
    // CONFIGURATION GEMINI 2.0 FLASH (GRATUIT!)
    // ========================================================================
    // Obtenez votre clé gratuite sur: https://aistudio.google.com/app/apikey
    //
    // MÉTHODE 1 (Recommandée): Fichier de configuration
    // Créez un fichier "config.ini" dans le dossier du projet:
    //   [AI]
    //   api_key=AIzaSy...
    //   enabled=true
    //
    // MÉTHODE 2: Variable d'environnement
    // PowerShell:  $env:GEMINI_API_KEY="AIzaSy..."
    // ========================================================================

    QString apiKey;

    // PRIORITÉ 1: Lire depuis config.ini
    // Rechercher config.ini dans plusieurs emplacements
    QStringList searchPaths;
    searchPaths << "config.ini"  // Dossier courant
                << QCoreApplication::applicationDirPath() + "/config.ini"  // Dossier de l'exécutable
                << QDir::currentPath() + "/config.ini"  // Dossier de travail
                << QDir::currentPath() + "/../config.ini"  // Dossier parent
                << QDir::currentPath() + "/../../config.ini"  // Deux niveaux au-dessus
                << QDir::currentPath() + "/../../../config.ini";  // Trois niveaux au-dessus

    QString configPath;
    for (const QString &path : searchPaths) {
        QString absolutePath = QFileInfo(path).absoluteFilePath();
        if (QFile::exists(absolutePath)) {
            configPath = absolutePath;
            qDebug() << "✓ config.ini trouvé à:" << configPath;
            break;
        }
    }

    if (!configPath.isEmpty()) {
        QSettings config(configPath, QSettings::IniFormat);

        // Vérifier si le fichier peut être lu
        if (config.status() != QSettings::NoError) {
            qCritical() << "❌ Impossible de lire config.ini - Erreur de permissions?";
            qCritical() << "   Fichier:" << configPath;
        } else {
            config.beginGroup("AI");

            bool enabled = config.value("enabled", false).toBool();
            apiKey = config.value("api_key").toString().trimmed();

            qDebug() << "✓ Clé Gemini chargée depuis config.ini";
            qDebug() << "   Enabled:" << enabled;
            qDebug() << "   Longueur de la clé:" << apiKey.length() << "caractères";
            qDebug() << "   Commence par 'AIza':" << apiKey.startsWith("AIza");

            config.endGroup();
        }
    } else {
        qWarning() << "⚠ config.ini non trouvé dans les emplacements standards";
        qWarning() << "   Recherché dans:";
        qWarning() << "   - Dossier courant:" << QDir::currentPath();
        qWarning() << "   - Dossier exécutable:" << QCoreApplication::applicationDirPath();
    }

    // PRIORITÉ 2: Lire depuis les variables d'environnement
    if (apiKey.isEmpty()) {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

        if (env.contains("GEMINI_API_KEY")) {
            apiKey = env.value("GEMINI_API_KEY");
            qDebug() << "✓ Clé Gemini trouvée dans les variables d'environnement";
        }
    }

    // Configuration de Gemini 2.0 Flash
    if (!apiKey.isEmpty() && apiKey.startsWith("AIza")) {
        AIVerifier::setApiKey(apiKey);
        qDebug() << "✓ Gemini 2.0 Flash configuré avec succès!";
        qDebug() << "  Modèle: gemini-2.0-flash-exp (GRATUIT)";
    } else {
        qWarning() << "⚠ Clé Gemini non configurée - La vérification AI sera désactivée";
        qWarning() << "";
        qWarning() << "  Créez un fichier config.ini:";
        qWarning() << "    [AI]";
        qWarning() << "    enabled=true";
        qWarning() << "    api_key=AIzaSy...";
        qWarning() << "";
        qWarning() << "  Obtenez une clé GRATUITE (starts with AIza):";
        qWarning() << "    https://aistudio.google.com/app/apikey";
    }

    // ========================================================================

    // Obtenir l'instance unique de Connection (Pattern Singleton)
    Connection* c = Connection::getInstance();

    // Tenter d'établir la connexion
    bool test = c->createConnection();

    if (test)
    {
        // Connexion réussie : afficher la fenêtre principale
        MainWindow w;
        w.show();

        QMessageBox::information(nullptr,
                                 QObject::tr("Connexion à la base de données"),
                                 QObject::tr("Connexion établie avec succès.\n"
                                             "L'application est prête à être utilisée."),
                                 QMessageBox::Ok);

        return a.exec();
    }
    else
    {
        // Connexion échouée : afficher l'erreur et quitter
        QMessageBox::critical(nullptr,
                              QObject::tr("Erreur de connexion"),
                              QObject::tr("Impossible de se connecter à la base de données.\n"
                                          "Vérifiez vos paramètres de connexion ODBC.\n\n"
                                          "L'application va se fermer."),
                              QMessageBox::Ok);
        return -1; // Code d'erreur
    }
}
