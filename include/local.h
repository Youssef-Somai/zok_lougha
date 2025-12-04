#ifndef LOCAL_H
#define LOCAL_H
#include <QString>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QDebug>
#include <QObject>
#include <QSqlError>
class local
{
private:
    int ID_LOCAL;
    QString NOM_L;
    QString TYPE_LOCAL;
    QString ETAT_LOCAL;

public:
    local();
    local(int ID_LOCAL,QString NOM_L,QString TYPE_LOCAL,QString ETAT_LOCAL);

    void setIdLocal(int ID_LOCAL);
    void setNomLocal(QString NOM_L);
    void setTypeLocal(QString TYPE_LOCAL);
    void setEtatLocal(QString ETAT_LOCAL);

    int getIdLocal() const;
    QString getNomLocal() const;
    QString getTypeLocal() const;
    QString getEtatLocal() const;

    QSqlQueryModel* GETALL();
    bool DELETEE(int);
    bool ADD();
    bool localExists(int ID_LOCAL);
    bool UPDATE();
    QSqlQueryModel* searchById(int id_local);
    bool ARCHIVE_LOCAL(int id);
    QString getEtatLocal(int id);
    QString getNomLocal(int id);
    QSqlQueryModel* trie(QString croissance, QString critere);

};

#endif // LOCAL_H
