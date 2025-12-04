#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlQuery>
#include <QTableWidget>
#include <QImage>
#include <QByteArray>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include "imagehandler.h"
#include "aiverifier.h"
#include "local.h"

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
    // === NAVIGATION ===
    void on_labelLien_linkActivated(const QString &link);
    void on_annuler_clicked();
    void on_valider_clicked();
    void on_gp_clicked();
    void on_retour_clicked();
    void on_pushButton_3_clicked();
    void on_modifier_a_2_clicked();
    void on_label_20_linkActivated(const QString &link);
    void on_pushButton_22_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pb_retour_mat_clicked();

    // === CRUD LOCAL ===
    void on_pushButton_29_clicked(); // Ajouter
    void on_pushButton_30_clicked(); // Supprimer (form)
    void on_pushButton_31_clicked(); // Chercher
    void on_pushButton_32_clicked(); // Trier
    void on_pushButton_33_clicked(); // Exporter
    void on_pushButton_34_clicked(); // Modifier
    void on_pushButton_35_clicked(); // Supprimer (selection)
    void on_pushButton_36_clicked(); // Statistiques
    void on_tableWidget_4_cellClicked(int row, int column);

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

    // ===== Local helpers =====
    void initLocalUi();                 // configure combo and headers
    void refreshLocalTable();           // reload tableWidget_4 from DB
    void clearLocalForm();              // clear local entry fields
    void populateLocalFormFromRow(int); // fill form from selection


};


#endif // MAINWINDOW_H
