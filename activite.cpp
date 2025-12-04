#include "activite.h"
#include <QMessageBox>
#include <QSqlError>
#include <QDebug>

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QTime>
#include <QDebug>


#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QSqlQuery>
#include <QSqlError>


#include <QPrinter>
#include <QTextDocument>
#include <QFileDialog>
#include <QPrintDialog>
#include <QTextCursor>
#include <QTextTable>
#include <QCompleter>
#include <QStringListModel>

Activite::Activite() {}

Activite::Activite(int id, QString nom, QString type, QString duree, QDateTime horaire,
                   int coach, int nbp, int local_A, QString status)
{
    this->id = id;
    this->nom = nom;
    this->type = type;
    this->duree = duree;
    this->horaire = horaire;
    this->coach = coach;
    this->local_A = local_A;
    this->nbp = nbp;
    this->status = status;
}

bool Activite::ajouter(QSqlDatabase &db)
{


    if (!db.isOpen()) return false;

    QSqlQuery query(db);

    QSqlQuery checkCoach(db);
    checkCoach.prepare("SELECT COUNT(*) FROM EMPLOYE WHERE ID_EMPLOYE = :id");
    checkCoach.bindValue(":id", coach);
    if (!checkCoach.exec() || !checkCoach.next() || checkCoach.value(0).toInt() == 0) {
        QMessageBox::warning(nullptr, "Erreur", "Le coach n'existe pas dans la table EMPLOYE !");
        return false;
    }

    QSqlQuery checkLocal(db);
    checkLocal.prepare("SELECT COUNT(*) FROM LOCAL WHERE ID_LOCAL = :id");
    checkLocal.bindValue(":id", local_A);
    if (!checkLocal.exec() || !checkLocal.next() || checkLocal.value(0).toInt() == 0) {
        QMessageBox::warning(nullptr, "Erreur", "Le local n'existe pas dans la table LOCAL !");
        return false;
    }

    query.prepare("INSERT INTO ACTIVITE(ID_ACTIVITE, NOM, TYPE, DUREE, HORAIRE, COACH, LOCAL_A, NBP, STATUS) "
                  "VALUES(:id, :nom, :type, :duree, TO_DATE(:horaire,'YYYY-MM-DD'), :coach, :local_A, :nbp, :status)");

    query.bindValue(":id", id);
    query.bindValue(":nom", nom);
    query.bindValue(":type", type);
    query.bindValue(":duree", duree);
    query.bindValue(":horaire", horaire.toString("yyyy-MM-dd"));
    query.bindValue(":coach", coach);
    query.bindValue(":local_A", local_A);
    query.bindValue(":nbp", nbp);
    query.bindValue(":status", status);

    if (!query.exec()) {
        QMessageBox::critical(nullptr, "Erreur Ajout", query.lastError().text());
        return false;
    }

    return true;
}

bool Activite::supprimer(QSqlDatabase &db)
{
    if (!db.isOpen()) return false;

    QSqlQuery query(db);

    // Supprimer d'abord les participations liées à l'activité
    query.prepare("DELETE FROM PARTICIPER WHERE ID_AC = :id");
    query.bindValue(":id", id);
    query.exec(); // On ne bloque pas la suppression si rien n'est trouvé

    // Puis supprimer l'activité
    query.prepare("DELETE FROM ACTIVITE WHERE ID_ACTIVITE = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur SQL suppression:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}


bool Activite::modifier(QSqlDatabase &db)
{
    if (!db.isOpen()) return false;

    QSqlQuery query(db);

    QSqlQuery checkCoach(db);
    checkCoach.prepare("SELECT COUNT(*) FROM EMPLOYE WHERE ID_EMPLOYE = :id");
    checkCoach.bindValue(":id", coach);
    checkCoach.exec();
    checkCoach.next();
    if (checkCoach.value(0).toInt() == 0) {
        QMessageBox::warning(nullptr, "Erreur", "Le coach n'existe pas !");
        return false;
    }

    QSqlQuery checkLocal(db);
    checkLocal.prepare("SELECT COUNT(*) FROM LOCAL WHERE ID_LOCAL = :id");
    checkLocal.bindValue(":id", local_A);
    checkLocal.exec();
    checkLocal.next();
    if (checkLocal.value(0).toInt() == 0) {
        QMessageBox::warning(nullptr, "Erreur", "Le local n'existe pas !");
        return false;
    }

    query.prepare("UPDATE ACTIVITE SET NOM=:nom, TYPE=:type, DUREE=:duree, "
                  "HORAIRE=TO_DATE(:horaire,'YYYY-MM-DD'), COACH=:coach, LOCAL_A=:local_A, "
                  "NBP=:nbp, STATUS=:status WHERE ID_ACTIVITE=:id");

    query.bindValue(":id", id);
    query.bindValue(":nom", nom);
    query.bindValue(":type", type);
    query.bindValue(":duree", duree);
    query.bindValue(":horaire", horaire.toString("yyyy-MM-dd"));
    query.bindValue(":coach", coach);
    query.bindValue(":local_A", local_A);
    query.bindValue(":nbp", nbp);
    query.bindValue(":status", status);

    if (!query.exec()) {
        QMessageBox::critical(nullptr, "Erreur Modification", query.lastError().text());
        return false;
    }

    return true;
}
QSqlQueryModel* Activite::recommandationCoach(QSqlDatabase &db)
{
    if (!db.isOpen()) return nullptr;

    QSqlQueryModel *model = new QSqlQueryModel();

    QSqlQuery query(db);
    query.prepare("SELECT ID_EMPLOYE FROM EMPLOYE WHERE STATUS = 'libre'");

    if(!query.exec()) {
        qDebug() << "Erreur SQL recommandationCoach:" << query.lastError().text();
        return nullptr;
    }

    model->setQuery(query);

    return model;
}
void Activite::chargerSuggestionCoach(QComboBox *combo, QSqlDatabase db)
{
    combo->clear(); // يمسح القائمة

    QSqlQuery query(db);
    query.prepare("SELECT ID_EMPLOYE FROM EMPLOYE WHERE STATUS='libre' and POSTE='Coach'");

    if(query.exec()) {
        while(query.next()) {
            int id = query.value(0).toInt();
            combo->addItem(QString::number(id));
        }
    } else {
        QMessageBox::warning(nullptr, "Erreur",
                             "Impossible de charger les coaches : "
                                 + query.lastError().text());
    }
}
QVector<QStringList> Activite::getToutesActivites(QSqlDatabase db)
{
    QVector<QStringList> data;

    QSqlQuery query(db);
    if(query.exec("SELECT * FROM ACTIVITE ORDER BY ID_ACTIVITE")) {

        while(query.next()) {
            QStringList row;

            row << query.value("ID_ACTIVITE").toString()
                << query.value("NOM").toString()
                << query.value("TYPE").toString()
                << query.value("HORAIRE").toDateTime().toLocalTime().toString("yyyy-MM-dd ")
                << query.value("DUREE").toString()
                << query.value("STATUS").toString()
                << query.value("NBP").toString()
                << query.value("COACH").toString()
                << query.value("LOCAL_A").toString();

            data.append(row);
        }
    }

    return data;
}
QVector<QStringList> Activite::getListeActivites(QSqlDatabase db)
{
    QVector<QStringList> data;

    QSqlQuery query(db);

    if (query.exec("SELECT * FROM ACTIVITE ORDER BY ID_ACTIVITE")) {

        while (query.next()) {
            QStringList row;
            row << query.value("ID_ACTIVITE").toString()
                << query.value("NOM").toString()
                << query.value("TYPE").toString()
                << query.value("HORAIRE").toDateTime().toString("yyyy-MM-dd")
                << query.value("DUREE").toString()
                << query.value("STATUS").toString()
                << query.value("NBP").toString()
                << query.value("COACH").toString()
                << query.value("LOCAL_A").toString();

            data.append(row);
        }
    }

    return data;
}
Activite Activite::getActiviteById(int id, QSqlDatabase db, bool &found)
{
    found = false;

    QSqlQuery query(db);
    query.prepare("SELECT * FROM ACTIVITE WHERE ID_ACTIVITE = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        found = true;

        return Activite(
            query.value("ID_ACTIVITE").toInt(),
            query.value("NOM").toString(),
            query.value("TYPE").toString(),
            query.value("DUREE").toString(),
            query.value("HORAIRE").toDateTime(),
            query.value("COACH").toInt(),
            query.value("NBP").toInt(),
            query.value("LOCAL_A").toInt(),
            query.value("STATUS").toString()
            );
    }

    return Activite(); // objet vide
}
QList<Activite> Activite::getActivitesDuJour(const QDate &date, QSqlDatabase db)
{
    QList<Activite> liste;

    QSqlQuery query(db);
    query.prepare("SELECT * FROM ACTIVITE WHERE TRUNC(HORAIRE)=TO_DATE(:date,'YYYY-MM-DD') ORDER BY HORAIRE");
    query.bindValue(":date", date.toString("yyyy-MM-dd"));

    if (query.exec()) {
        while (query.next()) {

            Activite a(
                query.value("ID_ACTIVITE").toInt(),
                query.value("NOM").toString(),
                query.value("TYPE").toString(),
                query.value("DUREE").toString(),
                query.value("HORAIRE").toDateTime(),
                query.value("COACH").toInt(),
                query.value("NBP").toInt(),
                query.value("LOCAL_A").toInt(),
                query.value("STATUS").toString()
                );

            liste.append(a);
        }
    }

    return liste;
}
QList<QDate> Activite::getDatesAvecActivites(QSqlDatabase db)
{
    QList<QDate> dates;

    QSqlQuery query(db);
    if (query.exec("SELECT DISTINCT TRUNC(HORAIRE) FROM ACTIVITE")) {

        while (query.next()) {
            QDate d = query.value(0).toDate();
            dates.append(d);
        }
    }

    return dates;
}
QSqlQuery Activite::trierActivites(QString critere, QSqlDatabase db)
{
    QSqlQuery query(db);

    if (critere == "Type") {
        query.exec(
            "SELECT * FROM ACTIVITE ORDER BY "
            "CASE TYPE "
            "WHEN 'Educatif' THEN 1 "
            "WHEN 'Culturel' THEN 2 "
            "WHEN 'Sportif' THEN 3 "
            "END"
            );
    }
    else if (critere == "Nom") {
        query.exec("SELECT * FROM ACTIVITE ORDER BY NOM ASC");
    }

    return query;
}
QSqlQuery Activite::chercherActiviteParID(int id, QSqlDatabase db)
{
    QSqlQuery query(db);
    query.prepare("SELECT * FROM ACTIVITE WHERE ID_ACTIVITE = :id");
    query.bindValue(":id", id);
    query.exec();
    return query;
}
QMap<QString,int> Activite::statistiquesParType(QSqlDatabase db)
{
    QMap<QString,int> stats;
    QSqlQuery query(db);
    query.prepare("SELECT TYPE, COUNT(*) FROM ACTIVITE GROUP BY TYPE");

    if(query.exec()) {
        while(query.next()) {
            QString type = query.value(0).toString();
            int count = query.value(1).toInt();
            stats[type] = count;
        }
    }
    return stats;
}
bool Activite::exporterVersPDF(QTableWidget *table, const QString &fileName)
{
    if(table->rowCount() == 0) return false;

    QTextDocument doc;
    QTextCursor cursor(&doc);

    cursor.insertHtml("<h2 align='center'>Liste des Activités</h2><br>");

    int rows = table->rowCount();
    int cols = table->columnCount();

    QTextTableFormat tableFormat;
    tableFormat.setBorder(1);
    tableFormat.setCellPadding(3);
    tableFormat.setCellSpacing(0);

    QTextTable *textTable = cursor.insertTable(rows+1, cols, tableFormat);

    // entêtes
    for(int col=0; col<cols; col++) {
        textTable->cellAt(0, col).firstCursorPosition().insertText(table->horizontalHeaderItem(col)->text());
    }

    // contenu
    for(int row=0; row<rows; row++) {
        for(int col=0; col<cols; col++) {
            QTableWidgetItem *item = table->item(row, col);
            QString text = item ? item->text() : "";
            textTable->cellAt(row+1, col).firstCursorPosition().insertText(text);
        }
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);

    doc.print(&printer);

    return true;
}
//
QSqlQueryModel* Activite::getRegroupementAdherents()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(
        "SELECT A.NOM AS ACTIVITE, "
        "LISTAGG(AD.NOM_ADHERENT || ' ' || AD.PRENOM_ADHERENT, ', ') "
        "WITHIN GROUP (ORDER BY AD.NOM_ADHERENT) AS ADHERENTS "
        "FROM ACTIVITE A "
        "JOIN PARTICIPER P ON A.ID_ACTIVITE = P.ID_AC "
        "JOIN ADHERENT AD ON AD.ID_ADHERENT = P.ID_AD "
        "GROUP BY A.NOM"
        );

    if (model->lastError().isValid()) {
        delete model;
        return nullptr;
    }

    return model;
}



