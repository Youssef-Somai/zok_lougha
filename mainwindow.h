#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QCalendarWidget>
#include <QTextCharFormat>
#include <QSqlQuery>
#include <QTableWidget>
#include <QImage>
#include <QByteArray>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QListWidget>
#include <QListWidgetItem>
#include <QInputDialog>
#include "activite.h"
#include "connexion.h"
#include "adherent.h"
#include "local.h"
#include "imagehandler.h"
#include "aiverifier.h"
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

    // === MATERIEL NAVIGATION ===
    void on_pushButton_22_clicked();
    void on_pb_retour_mat_clicked();
    void on_tableWidgetMateriel_cellClicked(int row, int column);
    void on_pb_annuler_materiel_clicked();

    // === CRUD MATERIEL ===
    void on_pb_ajouter_materiel_clicked();
    void on_pb_modifier_materiel_clicked();
    void on_pb_supprimer_materiel_clicked();
    void on_pb_rechercher_materiel_clicked();
    void on_pb_afficher_materiel_clicked();

    // === IMAGE HANDLING ===
    void on_pb_upload_image_materiel_clicked();
    void on_pb_camera_materiel_clicked();
    void on_pb_clear_image_materiel_clicked();

    // === QR CODE ===
    void on_pb_generer_qrcode_clicked();
    void on_pb_generer_qrcode_visible_clicked();
    void on_pb_sauvegarder_qrcode_clicked();
    void on_pb_scanner_qrcode_clicked();

    // === STATISTICS ===
    void updateStatistics();
    void on_pb_show_statistics_clicked();

    // === SORTING ===
    void on_pb_sort_clicked();
    void on_tableWidgetMateriel_horizontalHeaderClicked(int column);

    // === PDF EXPORT ===
    void on_pb_export_pdf_clicked();

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

    // ===== Méthodes privées pour Matériel =====

    // Initialisation
    void initMaterielTable();             // Configurer les en-têtes du QTableWidget
    void initMaterielCombos();            // Remplir les ComboBox
    void setupValidators();               // Configurer les validateurs de saisie

    // Gestion des données
    void clearMaterielForm();             // Vider tous les champs du formulaire
    void loadQueryIntoTable(QSqlQuery&);  // Charger un résultat SQL dans la table
    void refreshMaterielTable();          // Actualiser l'affichage de la table

    // Validation côté interface (avant d'appeler les méthodes métier)
    bool validerChampNumerique(const QString& valeur, const QString& nomChamp,
                               int& resultat, bool autoriserZero = false);
    bool validerChampDecimal(const QString& valeur, const QString& nomChamp,
                             double& resultat);
    bool validerChampTexte(const QString& valeur, const QString& nomChamp,
                           bool obligatoire = true);

    void populateMaterielFormFromRow(int row);

    // ===== Image Management =====
    void createImageUIElements();                   // Create image UI elements programmatically
    void displayImagePreview(const QImage &image);  // Display image in preview label
    void clearImagePreview();                       // Clear image preview
    bool verifyImageWithAI(const QString &category); // Verify image matches category

    // ===== Member Variables =====
    QImage currentMaterielImage;  // Stores the current material image
    bool hasImageData;            // Flag to track if image is set

    // ===== Statistics Chart =====
    QChartView *statisticsChartView;  // Chart view for statistics
    void createStatisticsChart();     // Create and display statistics chart
    void updateStatisticsChart();     // Update chart with new data

    // ===== PDF Export =====
    void generatePdfReport(const QString &fileName);  // Generate PDF report

    // ===== Sorting =====
    int currentSortColumn;          // Current sort column
    Qt::SortOrder currentSortOrder; // Current sort order
    void sortTableByColumn(int column, Qt::SortOrder order);

};
#endif // MAINWINDOW_H

