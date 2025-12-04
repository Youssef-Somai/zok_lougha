#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

class Connection
{
private:
    // Constructeur privé - empêche l'instanciation externe
    Connection();

    // Empêcher la copie et l'affectation
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    // Instance unique (static)
    static Connection* instance;

    QSqlDatabase db;

public:
    // Méthode statique pour obtenir l'instance unique
    static Connection* getInstance();

    // Méthode pour créer la connexion
    bool createConnection();

    // Getter pour la base de données
    QSqlDatabase getDatabase() const { return db; }

    // Méthode pour vérifier si la connexion est active
    bool isOpen() const { return db.isOpen(); }
};

#endif // CONNECTION_H
