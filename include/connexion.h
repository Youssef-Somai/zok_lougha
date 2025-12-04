#ifndef CONNEXION_H
#define CONNEXION_H
#include <QtSql>
#include <QMessageBox>

class Connexion
{
private:
    QSqlDatabase db;
public: Connexion();
    bool ouvrir();
    void fermer();
    QSqlDatabase getDB() const { return db; } };
#endif // CONNEXION_H ;
