#ifndef ACTIVITE_H
#define ACTIVITE_H

#include <QString>
#include <QDateTime>
#include <QtSql>
#include <QMap>
#include <QSqlQueryModel>


#include <QTableWidget>

class Activite
{
private:
    int id;
    QString nom;
    QString type;
    QString duree;
    QDateTime horaire;
    int coach;
    int nbp;
    QString status;
    int local_A ;

public:
    Activite();
    Activite(int id, QString nom, QString type, QString duree, QDateTime horaire, int coach, int nbp,int local_A ,QString status);

    void setId(int i) { id = i; }
    void setNom(const QString &n) { nom = n; }
    void setType(const QString &t) { type = t; }
    void setDuree(const QString &d) { duree = d; }
    void setHoraire(const QDateTime &h) { horaire = h; }
    void setCoach(int c) { coach = c; }
    void setLocal_A(int l) {local_A = l ;}
    void setNbp(int n) { nbp = n; }
    void setStatus(const QString &s) { status = s; }

    int getId() const { return id; }
    QString getNom() const { return nom; }
    QString getType() const { return type; }
    QString getDuree() const { return duree; }
    QDateTime getHoraire() const { return horaire; }
    int  getCoach() const { return coach; }
    int getlocal_A () const {return local_A;}
    int getNbp() const { return nbp; }
    QString getStatus() const { return status; }

    bool ajouter(QSqlDatabase &db);
    bool supprimer(QSqlDatabase &db);
    bool modifier(QSqlDatabase &db);
    QSqlQueryModel* recommandationCoach(QSqlDatabase &db);
    void chargerSuggestionCoach(QComboBox *combo, QSqlDatabase db);
    QVector<QStringList> getToutesActivites(QSqlDatabase db);
    QVector<QStringList> getListeActivites(QSqlDatabase db);
    Activite getActiviteById(int id, QSqlDatabase db, bool &found);
    static QList<Activite> getActivitesDuJour(const QDate &date, QSqlDatabase db);
    static QList<QDate> getDatesAvecActivites(QSqlDatabase db);
    static QSqlQuery trierActivites(QString critere, QSqlDatabase db);
    static QSqlQuery chercherActiviteParID(int id, QSqlDatabase db);
    static QMap<QString,int> statistiquesParType(QSqlDatabase db);
    static bool exporterVersPDF(QTableWidget *table, const QString &fileName);
    static QSqlQueryModel* getRegroupementAdherents();









};

#endif // ACTIVITE_H
