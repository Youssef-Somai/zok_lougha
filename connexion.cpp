#include "connexion.h"

Connexion::Connexion()
{
    db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("projet_qt");
    db.setUserName("youssef");
    db.setPassword("22137358");
}
bool Connexion::ouvrir()
{
    if(!db.open())
    {
        QMessageBox::critical(nullptr, "Erreur", db.lastError().text());
        return false;
    }
    else
    {
        QMessageBox::information(nullptr, "Succès", "Connexion établie !");
        return true;
    }
}
void Connexion::fermer()
{
    if(db.isOpen()) db.close();
}
