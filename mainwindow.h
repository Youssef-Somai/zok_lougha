#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QCalendarWidget>
#include <QTextCharFormat>
#include "activite.h"
#include "connexion.h"
#include "adherent.h"
#include <QListWidget>
#include <QListWidgetItem>
#include "local.h"
#include <QMainWindow>
#include <QInputDialog>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // navigation
    void on_labelLien_linkActivated(const QString &link);
    void on_annuler_clicked();
    void on_valider_clicked();
    void on_gp_clicked();
    void on_retour_clicked();
    void on_pushButton_3_clicked();
    void on_modifier_a_2_clicked();
    void on_label_20_linkActivated(const QString &link);
    void on_B_A_retour_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();

    // CRUD
    void on_bajouterA_clicked();
    void on_bannulerA_clicked();
    void on_b_A_supprimer_clicked();
    void on_b_A_modifier_clicked();
    void on_ok_modif_A_clicked();
    void on_annuler_modif_A_clicked();
    void on_b_A_afficher_clicked();
    void on_ok_aff_A_clicked();

    // calendrier
    void afficherCalendrier();
    void afficherActivitesDuJour(const QDate &date);

    void afficherToutesActivitesDansTabA2();
    void on_B_A_retour_2_clicked();

    // tri
    void on_Btrier_clicked();


    //chercher
    void on_Bchercher_clicked();



    void afficher_stat_act();

    void on_B_retour_stat_act_clicked();

    void on_Bexporter_clicked();
    void on_afficher_regroupement_clicked();


    void on_B_retour_stat_act_2_clicked();
    void chargerSuggestionCoach();
    void highlighterJoursActivites();

    void on_valider_a_clicked();
    void on_annuler_a_clicked();
    void on_supprimer_a_clicked();
    void on_modifier_a_clicked();
    void on_afficher_adherent_clicked();
    void on_modifier_a_3_clicked();

    void on_PDF_a_clicked();

    void on_statistics_clicked();

    void on_filtrer_a_clicked();

    void on_look_clicked();

    void on_B_A_retour_6_clicked();

    void on_B_A_retour_5_clicked();
    //local
    void on_AjouterLoc_clicked();
    bool controlSaisie();

    void on_tableLoc_activated(const QModelIndex &index);

    void on_ModifLoc_clicked();
    void on_SuppLoc_clicked();

    void on_recherche_clicked();
    void on_ex_clicked();
    void on_trier_clicked();
    void on_stat_clicked();

    void on_ArchiveViewer_clicked();
    void on_archiveItem_selected(QListWidgetItem *item);
    void on_btnArchiveViewer_clicked();


    void onEtatChanged(const QString &newEtat);

    void on_ASC_clicked();

    void on_DESC_clicked();

    void on_bt_retour_accueil_clicked();

private:
    Ui::MainWindow *ui;
    Connexion *connexion;
    QSqlDatabase db;
    void chargerActivites();
    Activite getActiviteFromInputs();
    void clearAjouterAInputs();
    void remplirModifierA(const Activite &a);
    void setAfficherAEditable(bool editable);
    void remplirAfficherA(const Activite &a);
    void colorerJoursAvecActivites();
    Adherent getAdherentFromInputs();
    void clearAdherentInputs();
    void chargerTableAdherents();
    local loc;

    void afficherStatistiques();

    void displayInternalNotification(const QString &message,
                                     const QString &bgColor = "#4caf50");

    void showNotification(const QString &message);
    void saveAllStatesToFile();
    QMap<int, QString> readLastStatesFromFile();
    void checkStatesOnStartup();

};
#endif // MAINWINDOW_H
