#include "local.h"
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
local::local() {

}
local::local(int ID_LOCAL,QString NOM_L,QString TYPE_LOCAL,QString ETAT_LOCAL) {
    this->ID_LOCAL = ID_LOCAL;
    this->NOM_L = NOM_L;
    this->TYPE_LOCAL = TYPE_LOCAL;
    this->ETAT_LOCAL = ETAT_LOCAL;
}


bool local::ADD()
{
    QSqlQuery query;
    query.prepare("INSERT INTO local (ID_LOCAL, NOM_L, TYPE_LOCAL, ETAT_LOCAL) "
                  "VALUES (:ID_LOCAL, :NOM_L, :TYPE_LOCAL, :ETAT_LOCAL)");


        query.bindValue(":ID_LOCAL", QVariant(ID_LOCAL));


    query.bindValue(":NOM_L", NOM_L);
    query.bindValue(":TYPE_LOCAL", TYPE_LOCAL);
    query.bindValue(":ETAT_LOCAL", ETAT_LOCAL);

    if (query.exec())
    {
        return true;
    }
    else
    {

        qDebug() << "Error adding local: " << query.lastError().text();
        return false;
    }
}

bool local::UPDATE()
{
    QSqlQuery query;
    query.prepare("UPDATE local SET NOM_L=:NOM_L, TYPE_LOCAL=:TYPE_LOCAL, ETAT_LOCAL=:ETAT_LOCAL WHERE ID_LOCAL=:ID_LOCAL");
    query.bindValue(":ID_LOCAL", ID_LOCAL);
    query.bindValue(":NOM_L", NOM_L);
    query.bindValue(":TYPE_LOCAL", TYPE_LOCAL);
    query.bindValue(":ETAT_LOCAL", ETAT_LOCAL);
    return query.exec();
}

bool local::DELETEE(int ID_LOCAL)
{
    QSqlQuery query;
    query.prepare("DELETE FROM local WHERE ID_LOCAL = :ID_LOCAL");
    query.bindValue(":ID_LOCAL", ID_LOCAL);
    return query.exec();
}

QSqlQueryModel* local::GETALL()
{
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT ID_LOCAL, NOM_L, TYPE_LOCAL, ETAT_LOCAL FROM local");

    if (model->lastError().isValid()) {
        qDebug() << "Error in SQL Query: " << model->lastError().text();
        return nullptr;
    }

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID_LOCAL"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("NOM_L"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("TYPE_LOCAL"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("ETAT_LOCAL"));


    return model;
}

bool local::localExists(int ID_LOCAL)
{
    bool exists = false;
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT ID_LOCAL FROM local WHERE ID_LOCAL = :ID_LOCAL");
    checkQuery.bindValue(":ID_LOCAL", ID_LOCAL);

    if (checkQuery.exec())
    {
        if (checkQuery.next())
        {
            exists = true;
        }
    }
    else
    {
        qDebug() << "local not found: " << checkQuery.lastError();
    }

    return exists;
}


void local::setIdLocal(int ID_LOCAL)
{
    this->ID_LOCAL = ID_LOCAL;
}

void local::setNomLocal(QString NOM_L)
{
    this->NOM_L = NOM_L;
}

void local::setTypeLocal(QString TYPE_LOCAL)
{
    this->TYPE_LOCAL = TYPE_LOCAL;
}

void local::setEtatLocal(QString ETAT_LOCAL)
{
    this->ETAT_LOCAL = ETAT_LOCAL;
}


int local::getIdLocal() const
{
    return ID_LOCAL;
}

QString local::getNomLocal() const
{
    return NOM_L;
}

QString local::getTypeLocal() const
{
    return TYPE_LOCAL;
}

QString local::getEtatLocal() const
{
    return ETAT_LOCAL;
}
QSqlQueryModel* local::searchById(int id_local)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    query.prepare("SELECT * FROM LOCAL WHERE ID_LOCAL = :id");
    query.bindValue(":id", id_local);

    query.exec();
    model->setQuery(query);

    return model;
}
/// archivage
bool local::ARCHIVE_LOCAL(int id)
{

    QSqlQuery query;
    query.prepare("SELECT ID_LOCAL, NOM_L, ETAT_LOCAL, TYPE_LOCAL FROM LOCAL WHERE ID_LOCAL = :id");
    query.bindValue(":id", QString::number(id)); // Liaison robuste en chaîne


    if (!query.exec() || !query.next())
    {
        qDebug() << "Erreur: Local ID" << id << "non trouvé pour l'archivage.";
        return false;
    }

    // Récupération de toutes les données
    QString id_loc = query.value(0).toString();
    QString nom_loc = query.value(1).toString();
    QString etat_loc = query.value(2).toString();
    QString type_loc = query.value(3).toString();

    QFile file("archive_locaux.txt");

    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&file);


        QString archive_line = QString(
                                   "[%1] SUPPRESSION ID: %2 | NOM: %3 | ETAT: %4 | TYPE: %5"
                                   ).arg(
                                       QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss"),
                                       id_loc,
                                       nom_loc,
                                       etat_loc,
                                       type_loc
                                       );

        out << archive_line << "\n";
        file.close();
        return true;
    }
    else
    {
        qDebug() << "Erreur: Impossible d'ouvrir le fichier d'archive.";
        return false;
    }
}

QString local::getEtatLocal(int id)
{
    QSqlQuery query;
    query.prepare("SELECT ETAT_LOCAL FROM LOCAL WHERE ID_LOCAL = :id");
    query.bindValue(":id", QString::number(id)); // Liaison robuste

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return QString(); // Retourne vide si échec de la lecture
}

QString local::getNomLocal(int id)
{
    QSqlQuery query;
    query.prepare("SELECT NOM_LOCAL FROM LOCAL WHERE ID_LOCAL = :id");
    query.bindValue(":id", QString::number(id));

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return QString();
}
QSqlQueryModel* local::trie(QString croissance, QString critere)
{
    // Crée un nouveau modèle QSqlQueryModel
    QSqlQueryModel* model = new QSqlQueryModel();

    qDebug() << "Critère de tri : " << critere.trimmed();
    qDebug() << "Croissance : " << croissance.trimmed();

    // Sélectionne la requête SQL en fonction du critère et de l'ordre de tri
    if (critere.trimmed().compare("NOM_L", Qt::CaseInsensitive) == 0 && croissance.trimmed().compare("ASC", Qt::CaseInsensitive) == 0)
        model->setQuery("SELECT * FROM local ORDER BY NOM_L ASC");
    else if (critere.trimmed().compare("Nom_L", Qt::CaseInsensitive) == 0 && croissance.trimmed().compare("DESC", Qt::CaseInsensitive) == 0)
        model->setQuery("SELECT * FROM local ORDER BY NOM_L DESC");
    else if (critere.trimmed().compare("ID_LOCAL", Qt::CaseInsensitive) == 0 && croissance.trimmed().compare("ASC", Qt::CaseInsensitive) == 0)
        model->setQuery("SELECT * FROM local ORDER BY id_local ASC");
    else if (critere.trimmed().compare("ID_LOCAL", Qt::CaseInsensitive) == 0 && croissance.trimmed().compare("DESC", Qt::CaseInsensitive) == 0)
        model->setQuery("SELECT * FROM local ORDER BY id_local DESC");
    else
        qDebug() << "Aucune condition remplie pour définir la requête.";

    qDebug() << "Requête SQL : " << model->query().lastQuery();

    // Retourne le modèle contenant les résultats de la requête de tri
    return model;
}
