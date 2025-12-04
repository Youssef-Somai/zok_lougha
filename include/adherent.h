#ifndef ADHERENT_H
#define ADHERENT_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QtCharts>
#include <QNetworkAccessManager>





class Adherent
{
private:
    int id_adherent;
    QString nom_adherent;
    QString prenom_adherent;
    QString genre_adherent;
    QString adresse_adherent;
    QString email_adherent;
    QString gsm_adherent;
    int cin_adherent;
    QString date_de_naissance_adherent;
    QNetworkAccessManager* manager;

public:
    Adherent();
    Adherent(int id, QString nom, QString prenom, QString genre, QString adresse,
             QString email,  QString  gsm, int cin, QString dateNaissance);

    //Setters
    void setId(int id) { id_adherent = id; }
    void setNom(const QString &nom) { nom_adherent = nom; }
    void setPrenom(const QString &prenom) { prenom_adherent = prenom; }
    void setGenre(const QString &genre) { genre_adherent = genre; }
    void setAdresse(const QString &adresse) { adresse_adherent = adresse; }
    void setEmail(const QString &email) { email_adherent = email; }
    void setGsm(const QString &gsm) { gsm_adherent =gsm; }
    void setCin(int cin) { cin_adherent = cin; }
    void setDateNaissance(const QString &date) { date_de_naissance_adherent = date; }

    //  Getters
    int getId() const { return id_adherent; }
    QString getNom() const { return nom_adherent; }
    QString getPrenom() const { return prenom_adherent; }
    QString getGenre() const { return genre_adherent; }
    QString getAdresse() const { return adresse_adherent; }
    QString getEmail() const { return email_adherent; }
    QString getGsm() const { return gsm_adherent; }
    int getCin() const { return cin_adherent; }
    QString getDateNaissance() const { return date_de_naissance_adherent; }

    // Méthodes SQL
    bool ajouter(QSqlDatabase &db);
    bool supprimer(QSqlDatabase &db);
    bool modifier(QSqlDatabase &db);
    void genererStatistiquesAdherents();
    bool envoyerEmailConfirmation();
    QMap<QString, QVariant> afficher(QSqlDatabase &db, int id);
    bool envoyerSMSviaTwilio(const QString &numero, const QString &message, QString &erreur);
    void notifierAnnulation(QSqlDatabase &db, int idActivite, const QString &nomActivite);
    void exporterPDF(QSqlDatabase &db, const QString &fileName);
    void genererStatistiques(QSqlDatabase &db, QLayout *layout);
    void trier(QSqlDatabase &db, QTableWidget *table, const QString &critere);
    void chercher(QSqlDatabase &db, QTableWidget *table, const QString &id);
    bool inscrireAdherent(int idAdherent, int idActivite);

};


#endif // ADHERENT_H
