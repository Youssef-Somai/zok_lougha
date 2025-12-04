#include "connection.h"
#include <QDebug>

// Initialisation du pointeur statique à nullptr
Connection* Connection::instance = nullptr;

// Constructeur privé
Connection::Connection()
{
    // Le constructeur ne fait rien ici
    // La connexion sera créée dans createConnection()
}

// Méthode statique pour obtenir l'instance unique (Singleton)
Connection* Connection::getInstance()
{
    if (instance == nullptr)
    {
        instance = new Connection();
    }
    return instance;
}

// Méthode pour créer la connexion à la base de données
bool Connection::createConnection()
{
    // Vérifier si une connexion existe déjà
    if (QSqlDatabase::contains("qt_sql_default_connection"))
    {
        db = QSqlDatabase::database("qt_sql_default_connection");
        if (db.isOpen())
        {
            qDebug() << "Connexion déjà établie";
            return true;
        }
    }

    // Créer une nouvelle connexion
    db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("projet_qt"); // Nom de la source de données ODBC
    db.setUserName("youssef");       // Utilisateur
    db.setPassword("22137358");      // Mot de passe

    if (db.open())
    {
        qDebug() << "Connexion réussie à la base de données";
        return true;
    }
    else
    {
        qDebug() << "Erreur de connexion:" << db.lastError().text();
        return false;
    }
}
