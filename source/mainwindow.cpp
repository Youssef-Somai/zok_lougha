#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QRegularExpression>
#include <QFileDialog>
#include <QPixmap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QChartView>
#include <QtCharts/QLegend>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QPrinter>
#include <QPainter>
#include <QFont>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QTextTableFormat>
#include <QDialog>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QValidator>
#include <QRegularExpressionValidator>
#include <QIntValidator>
#include <QInputDialog>
#include <QPropertyAnimation>
#include <QTimer>
#include <QTextStream>
#include <QFile>
#include <QPdfWriter>
#include <QPageSize>
#include <QAbstractItemModel>
#include "materiel.h"
#include "qrcodehelper.h"
#include "qrscannerdialog.h"
#include "header/imagehandler.h"
#include "header/aiverifier.h"

// Includes for main branch classes
#include "activite.h"
#include "connexion.h"
#include "adherent.h"
#include "local.h"
#include "smtp.h"

QT_CHARTS_USE_NAMESPACE

/* =========================
 *  CONSTRUCTEUR / DTOR
 * ========================= */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , hasImageData(false)  // Initialize image flag
    , statisticsChartView(nullptr)  // Initialize chart view
    , currentSortColumn(-1)  // No initial sort
    , currentSortOrder(Qt::AscendingOrder)  // Default ascending
    , connexion(nullptr)  // Initialize connexion from main branch
    , loc(nullptr)  // Initialize local from main branch
{
    ui->setupUi(this);

    // Initialize main branch connexion
    connexion = new Connexion();
    if (connexion && connexion->ouvrir()) {
        db = connexion->getDB();
    }

    // Initialize local object
    loc = new local();

    ui->stackedWidget->setCurrentIndex(0);

    // ======= NAVIGATION =======
    ui->labelLien->setText("<a href=\"#\">Mot de passe oublié</a>");
    ui->labelLien->setTextFormat(Qt::RichText);
    ui->labelLien->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->labelLien->setOpenExternalLinks(false);
    connect(ui->labelLien, &QLabel::linkActivated,
            this, &MainWindow::on_labelLien_linkActivated);

    // ======= MATERIEL (init + 1er affichage) =======
    initMaterielTable();
    initMaterielCombos();
    setupValidators(); // Configuration des validateurs
    refreshMaterielTable();

    if (ui->tableWidgetMateriel && ui->tableWidgetMateriel->horizontalHeader())
    {
        ui->tableWidgetMateriel->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        // Enable sorting by clicking on headers
        ui->tableWidgetMateriel->setSortingEnabled(true);
        ui->tableWidgetMateriel->horizontalHeader()->setSectionsClickable(true);
        connect(ui->tableWidgetMateriel->horizontalHeader(), &QHeaderView::sectionClicked,
                this, &MainWindow::on_tableWidgetMateriel_horizontalHeaderClicked);
    }

    // ======= IMAGE HANDLING SETUP =======
    // Create image-related UI elements programmatically if not in .ui file
    // This allows the application to work even if .ui file doesn't have image widgets yet
    createImageUIElements();

    // ======= ACTIVITÉS / ADHÉRENTS / LOCAUX (INITIALISATION) =======
    highlighterJoursActivites();
    chargerTableAdherents();
    chargerActivites();
    afficherToutesActivitesDansTabA2();
    chargerSuggestionCoach();

    if (ui->tabA)
    {
        ui->tabA->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tabA->setSelectionBehavior(QAbstractItemView::SelectRows);
    }

    if (ui->tabA_2)
    {
        ui->tabA_2->setColumnCount(9);
        ui->tabA_2->setHorizontalHeaderLabels({"ID", "Nom", "Type", "Horaire", "Durée", "Status", "NbP", "Coach", "Local_A"});
    }

    // Contrôles de saisie pour les adhérents
    QRegularExpression rxNom("^[A-Za-z ]*$");
    QValidator *validatorNom = new QRegularExpressionValidator(rxNom, this);
    if (ui->nom_a1) ui->nom_a1->setValidator(validatorNom);
    if (ui->prenom_a1) ui->prenom_a1->setValidator(validatorNom);

    QIntValidator *validatorChiffre = new QIntValidator(0, 99999999, this);
    if (ui->cin_a1) ui->cin_a1->setValidator(validatorChiffre);
    if (ui->gsm_a1) ui->gsm_a1->setValidator(validatorChiffre);

    if (ui->date_a1) ui->date_a1->setCalendarPopup(true);
    if (ui->statusA1) ui->statusA1->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    delete ui;
    if (connexion) {
        delete connexion;
        connexion = nullptr;
    }
    if (loc) {
        delete loc;
        loc = nullptr;
    }
}

/* =========================
 *  NAVIGATION
 * ========================= */
void MainWindow::on_labelLien_linkActivated(const QString &link)
{
    Q_UNUSED(link);
    ui->stackedWidget->setCurrentIndex(1);
}
void MainWindow::on_annuler_clicked(){ ui->stackedWidget->setCurrentIndex(0); }
void MainWindow::on_valider_clicked(){ ui->stackedWidget->setCurrentIndex(2); }
void MainWindow::on_gp_clicked(){ ui->stackedWidget->setCurrentIndex(3); }
void MainWindow::on_retour_clicked(){ ui->stackedWidget->setCurrentIndex(0); }
void MainWindow::on_pushButton_3_clicked(){ ui->stackedWidget->setCurrentIndex(4); }
void MainWindow::on_modifier_a_2_clicked(){ ui->stackedWidget->setCurrentIndex(2); }
void MainWindow::on_label_20_linkActivated(const QString &link){ Q_UNUSED(link); }
void MainWindow::on_pushButton_22_clicked(){ ui->stackedWidget->setCurrentIndex(2); }
void MainWindow::on_pushButton_4_clicked(){ ui->stackedWidget->setCurrentIndex(5); }
void MainWindow::on_pushButton_5_clicked(){ ui->stackedWidget->setCurrentIndex(6); }
void MainWindow::on_pushButton_6_clicked(){ ui->stackedWidget->setCurrentIndex(7); }
void MainWindow::on_pb_retour_mat_clicked(){ ui->stackedWidget->setCurrentIndex(2); }

/* =========================
 *  OUTILS MATERIEL
 * ========================= */
void MainWindow::initMaterielTable()
{
    if (!ui->tableWidgetMateriel) return;
    QStringList headers = {
        "ID","NOM","CATEGORIE","ETAT","UTILISATEUR",
        "SEUIL","QT_TOTALE","QT_DISPO","PRIX","GARANTIE","FOURNISSEUR"
    };
    ui->tableWidgetMateriel->clear();
    ui->tableWidgetMateriel->setColumnCount(headers.size());
    ui->tableWidgetMateriel->setHorizontalHeaderLabels(headers);
    ui->tableWidgetMateriel->setRowCount(0);
}

void MainWindow::initMaterielCombos()
{
    if (ui->cb_categorie) {
        ui->cb_categorie->clear();
        ui->cb_categorie->addItems(QStringList()
                                   << "Informatique" << "Bureautique" << "Sport" << "Autre");
    }
    if (ui->cb_etat) {
        ui->cb_etat->clear();
        ui->cb_etat->addItems(QStringList()
                              << "Disponible" << "En panne" << "En réparation" << "Empruntee");
    }
    if (ui->cb_utilisateur) {
        ui->cb_utilisateur->clear();
        ui->cb_utilisateur->addItems(QStringList()
                                     << "Formateur" << "Administration" << "Adherent");
    }
}

void MainWindow::setupValidators()
{
    // Validateur pour les IDs (nombres entiers positifs uniquement)
    QRegularExpression regexId("^[1-9][0-9]{0,8}$"); // 1-9 chiffres, commence par 1-9
    QRegularExpressionValidator* validatorId = new QRegularExpressionValidator(regexId, this);

    if (ui->le_id) ui->le_id->setValidator(validatorId);
    if (ui->le_recherche_id) ui->le_recherche_id->setValidator(validatorId);

    // Validateur pour les quantités (nombres entiers >= 0)
    QRegularExpression regexQte("^[0-9]{1,9}$");
    QRegularExpressionValidator* validatorQte = new QRegularExpressionValidator(regexQte, this);

    if (ui->le_seuil) ui->le_seuil->setValidator(validatorQte);
    if (ui->le_qt_totale) ui->le_qt_totale->setValidator(validatorQte);
    if (ui->le_qt_dispo) ui->le_qt_dispo->setValidator(validatorQte);

    // Validateur pour le prix (nombres décimaux >= 0)
    QRegularExpression regexPrix("^[0-9]{1,6}(\\.[0-9]{1,2})?$");
    QRegularExpressionValidator* validatorPrix = new QRegularExpressionValidator(regexPrix, this);

    if (ui->le_prix) ui->le_prix->setValidator(validatorPrix);

    // Limiter la longueur des champs texte
    if (ui->le_nom) ui->le_nom->setMaxLength(100);
    if (ui->le_garantie) ui->le_garantie->setMaxLength(50);
    if (ui->le_fournisseur) ui->le_fournisseur->setMaxLength(100);
}

// Validation stricte avec messages d'erreur détaillés
bool MainWindow::validerChampNumerique(const QString& valeur, const QString& nomChamp,
                                       int& resultat, bool autoriserZero)
{
    if (valeur.trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Validation",
                             QString("Le champ '%1' est obligatoire.").arg(nomChamp));
        return false;
    }

    bool ok;
    resultat = valeur.trimmed().toInt(&ok);

    if (!ok)
    {
        QMessageBox::warning(this, "Validation",
                             QString("Le champ '%1' doit être un nombre entier valide.").arg(nomChamp));
        return false;
    }

    if (!autoriserZero && resultat <= 0)
    {
        QMessageBox::warning(this, "Validation",
                             QString("Le champ '%1' doit être supérieur à 0.").arg(nomChamp));
        return false;
    }

    if (autoriserZero && resultat < 0)
    {
        QMessageBox::warning(this, "Validation",
                             QString("Le champ '%1' ne peut pas être négatif.").arg(nomChamp));
        return false;
    }

    return true;
}

bool MainWindow::validerChampDecimal(const QString& valeur, const QString& nomChamp, double& resultat)
{
    if (valeur.trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Validation",
                             QString("Le champ '%1' est obligatoire.").arg(nomChamp));
        return false;
    }

    bool ok;
    resultat = valeur.trimmed().toDouble(&ok);

    if (!ok)
    {
        QMessageBox::warning(this, "Validation",
                             QString("Le champ '%1' doit être un nombre décimal valide.").arg(nomChamp));
        return false;
    }

    if (resultat < 0.0)
    {
        QMessageBox::warning(this, "Validation",
                             QString("Le champ '%1' ne peut pas être négatif.").arg(nomChamp));
        return false;
    }

    return true;
}

bool MainWindow::validerChampTexte(const QString& valeur, const QString& nomChamp, bool obligatoire)
{
    QString v = valeur.trimmed();

    if (obligatoire && v.isEmpty())
    {
        QMessageBox::warning(this, "Validation",
                             QString("Le champ '%1' est obligatoire.").arg(nomChamp));
        return false;
    }

    return true;
}

void MainWindow::clearMaterielForm()
{
    if (ui->le_id) { ui->le_id->clear(); ui->le_id->setReadOnly(false); }
    if (ui->le_nom) ui->le_nom->clear();
    if (ui->cb_categorie) ui->cb_categorie->setCurrentIndex(0);
    if (ui->cb_etat) ui->cb_etat->setCurrentIndex(0);
    if (ui->cb_utilisateur) ui->cb_utilisateur->setCurrentIndex(0);
    if (ui->le_seuil) ui->le_seuil->clear();
    if (ui->le_qt_totale) ui->le_qt_totale->clear();
    if (ui->le_qt_dispo) ui->le_qt_dispo->clear();
    if (ui->le_prix) ui->le_prix->clear();
    if (ui->le_garantie) ui->le_garantie->clear();
    if (ui->le_fournisseur) ui->le_fournisseur->clear();
    if (ui->le_recherche_id) ui->le_recherche_id->clear();

    // Clear image data
    clearImagePreview();
}

void MainWindow::loadQueryIntoTable(QSqlQuery &q)
{
    if (!ui->tableWidgetMateriel) return;
    initMaterielTable();

    int row = 0;
    while (q.next()) {
        ui->tableWidgetMateriel->insertRow(row);
        for (int col = 0; col < 11; ++col) {
            const QVariant v = q.value(col);
            auto *item = new QTableWidgetItem(v.isNull() ? QString() : v.toString());
            if (col == 0 || col == 5 || col == 6 || col == 7 || col == 8)
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->tableWidgetMateriel->setItem(row, col, item);
        }
        ++row;
    }
    ui->tableWidgetMateriel->resizeColumnsToContents();
}

void MainWindow::refreshMaterielTable()
{
    // Utilisation de requête préparée même pour un SELECT simple (bonne pratique)
    QSqlQuery q;
    q.prepare("SELECT ID_MATERIEL, NOM, CATEGORIE, ETAT, UTILISATEUR, "
              "SEUIL_ALERTE, QT_TOTALE, QT_DISPO, PRIX, GARANTIE, FOURNISSEUR "
              "FROM MATERIEL ORDER BY ID_MATERIEL");
    q.exec();
    loadQueryIntoTable(q);
}

/* =========================
 *  SLOTS CRUD MATERIEL
 * ========================= */
void MainWindow::on_pb_ajouter_materiel_clicked()
{
    int id=0, seuil=0, qtTot=0, qtDispo=0;
    double prix=0.0;

    // Validation côté interface avant de créer l'objet
    if (!validerChampNumerique(ui->le_id->text(), "ID", id, false)) return;
    if (!validerChampTexte(ui->le_nom->text(), "Nom", true)) return;
    if (!validerChampNumerique(ui->le_seuil->text(), "Seuil d'alerte", seuil, true)) return;
    if (!validerChampNumerique(ui->le_qt_totale->text(), "Quantité totale", qtTot, true)) return;
    if (!validerChampNumerique(ui->le_qt_dispo->text(), "Quantité disponible", qtDispo, true)) return;
    if (!validerChampDecimal(ui->le_prix->text(), "Prix", prix)) return;

    // ===== CRITICAL: AI VERIFICATION BEFORE DATABASE INSERT =====
    // If an image is provided, it MUST be verified against the category
    // This verification is BLOCKING and will prevent insertion if it fails
    if (hasImageData && !currentMaterielImage.isNull())
    {
        qDebug() << "AI Verification: Image detected, verifying against category:"
                 << ui->cb_categorie->currentText();

        // Perform AI verification - THIS IS BLOCKING
        if (!verifyImageWithAI(ui->cb_categorie->currentText()))
        {
            // Verification failed - DO NOT ALLOW INSERT
            QMessageBox::critical(this, "Vérification Échouée",
                "<h3>⚠ L'ajout du matériel est bloqué!</h3>"
                "<p>L'image ne correspond pas à la catégorie sélectionnée.</p>"
                "<p><b>Le matériel ne sera PAS ajouté à la base de données.</b></p>"
                "<hr>"
                "<p>Veuillez:</p>"
                "<ul>"
                "<li>Sélectionner une image qui correspond à la catégorie</li>"
                "<li>Ou modifier la catégorie pour qu'elle corresponde à l'image</li>"
                "</ul>");

            qWarning() << "Material insertion BLOCKED due to failed AI verification";
            return;  // ABORT - Do not insert into database
        }

        qDebug() << "AI Verification: PASSED - Proceeding with database insert";
    }
    else if (!hasImageData)
    {
        // No image provided - warn the user
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Aucune image",
            "Aucune image n'a été fournie pour ce matériel.\n"
            "Voulez-vous continuer sans image?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );

        if (reply == QMessageBox::No)
        {
            return;  // User chose to add an image first
        }
    }

    // Créer l'objet Materiel
    Materiel m(
        id,
        ui->le_nom->text(),
        ui->cb_categorie->currentText(),
        ui->cb_etat->currentText(),
        ui->cb_utilisateur->currentText(),
        seuil, qtTot, qtDispo, prix,
        ui->le_garantie->text(),
        ui->le_fournisseur->text()
        );

    // Set image data if available
    if (hasImageData && !currentMaterielImage.isNull())
    {
        QByteArray imageData = ImageHandler::imageToByteArray(currentMaterielImage);
        m.setImageData(imageData);
        qDebug() << "Image data added to material - Size:" << imageData.size() << "bytes";
    }

    // Appel de la méthode métier qui contient la validation complète
    if (m.ajouter())
    {
        // Generate and display QR code for the newly added item
        QImage qrImage = m.generateQrCode(400);
        qDebug() << "QR Code image generated, size:" << qrImage.size()
                 << "null:" << qrImage.isNull();

        // Display in label
        if (ui->label_qrcode)
        {
            ui->label_qrcode->setPixmap(QPixmap::fromImage(qrImage));
            ui->label_qrcode->setScaledContents(false);
            ui->label_qrcode->setAlignment(Qt::AlignCenter);
            ui->label_qrcode->show();
        }

        // Build success message including image info
        QString successMsg = QString("Le matériel '%1' (ID: %2) a été ajouté avec succès!\n\n")
                                .arg(m.nom()).arg(m.id());

        if (m.hasImage())
        {
            successMsg += "✓ Image enregistrée dans la base de données.\n";
        }

        successMsg += "QR Code généré automatiquement.\n"
                     "Scannez-le avec votre smartphone!";

        // Show QR code in popup message box
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Matériel ajouté - QR Code");
        msgBox.setText(successMsg);

        // Add QR code image to the message box
        QPixmap pixmap = QPixmap::fromImage(qrImage);
        msgBox.setIconPixmap(pixmap.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        // Add buttons
        QPushButton *saveButton = msgBox.addButton("Sauvegarder", QMessageBox::ActionRole);
        msgBox.addButton("Fermer", QMessageBox::AcceptRole);

        msgBox.exec();

        // Check if user clicked save
        if (msgBox.clickedButton() == saveButton)
        {
            QString fileName = QFileDialog::getSaveFileName(
                this,
                "Sauvegarder le QR Code",
                QString("QRCode_Materiel_%1_%2.png")
                    .arg(m.id())
                    .arg(m.nom().replace(" ", "_")),
                "Images PNG (*.png);;Images JPEG (*.jpg *.jpeg);;Tous les fichiers (*.*)"
                );

            if (!fileName.isEmpty())
            {
                if (m.saveQrCodeToFile(fileName, 600))
                {
                    QMessageBox::information(this, "Succès",
                                             QString("QR Code sauvegardé:\n%1").arg(fileName));
                }
            }
        }

        // Actualiser l'affichage après ajout
        refreshMaterielTable();
        clearMaterielForm();
    }
    else
    {
        QMessageBox::critical(this, "Erreur",
                              "L'ajout du matériel a échoué.\n"
                              "Vérifiez que l'ID n'existe pas déjà et que toutes les données sont valides.");
    }
}

void MainWindow::on_pb_modifier_materiel_clicked()
{
    int id=0, seuil=0, qtTot=0, qtDispo=0;
    double prix=0.0;

    // Validation côté interface
    if (!validerChampNumerique(ui->le_id->text(), "ID", id, false)) return;
    if (!validerChampTexte(ui->le_nom->text(), "Nom", true)) return;
    if (!validerChampNumerique(ui->le_seuil->text(), "Seuil d'alerte", seuil, true)) return;
    if (!validerChampNumerique(ui->le_qt_totale->text(), "Quantité totale", qtTot, true)) return;
    if (!validerChampNumerique(ui->le_qt_dispo->text(), "Quantité disponible", qtDispo, true)) return;
    if (!validerChampDecimal(ui->le_prix->text(), "Prix", prix)) return;

    // Créer l'objet Materiel
    Materiel m(
        id,
        ui->le_nom->text(),
        ui->cb_categorie->currentText(),
        ui->cb_etat->currentText(),
        ui->cb_utilisateur->currentText(),
        seuil, qtTot, qtDispo, prix,
        ui->le_garantie->text(),
        ui->le_fournisseur->text()
        );

    // Appel de la méthode métier
    if (m.modifier())
    {
        QMessageBox::information(this, "Succès",
                                 QString("Le matériel '%1' (ID: %2) a été modifié avec succès.")
                                     .arg(m.nom()).arg(m.id()));

        // Actualiser l'affichage après modification
        refreshMaterielTable();
    }
    else
    {
        QMessageBox::critical(this, "Erreur",
                              "La modification a échoué.\n"
                              "Vérifiez que l'ID existe et que toutes les données sont valides.");
    }
}

void MainWindow::on_pb_supprimer_materiel_clicked()
{
    int id=0;

    if (!validerChampNumerique(ui->le_id->text(), "ID", id, false))
        return;

    // Demander confirmation avant suppression
    QMessageBox::StandardButton reponse = QMessageBox::question(
        this,
        "Confirmation de suppression",
        QString("Êtes-vous sûr de vouloir supprimer le matériel avec l'ID %1 ?\n"
                "Cette action est irréversible.").arg(id),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (reponse == QMessageBox::No)
        return;

    // Appel de la méthode métier statique
    if (Materiel::supprimer(id))
    {
        QMessageBox::information(this, "Succès",
                                 QString("Le matériel (ID: %1) a été supprimé avec succès.").arg(id));

        // Actualiser l'affichage après suppression
        refreshMaterielTable();

        // Vider le champ de recherche
        if (ui->le_recherche_id) ui->le_recherche_id->clear();
    }
    else
    {
        QMessageBox::critical(this, "Erreur",
                              QString("La suppression a échoué.\n"
                                      "Aucun matériel trouvé avec l'ID %1.").arg(id));
    }
}

void MainWindow::on_pb_rechercher_materiel_clicked()
{
    int id=0;

    if (!validerChampNumerique(ui->le_recherche_id->text(), "ID", id, false))
        return;

    // Utiliser une requête préparée (protection SQL injection)
    QSqlQuery q;
    q.prepare("SELECT ID_MATERIEL, NOM, CATEGORIE, ETAT, UTILISATEUR, "
              "SEUIL_ALERTE, QT_TOTALE, QT_DISPO, PRIX, GARANTIE, FOURNISSEUR "
              "FROM MATERIEL WHERE ID_MATERIEL = :id");
    q.bindValue(":id", id);

    if (!q.exec())
    {
        QMessageBox::critical(this, "Erreur",
                              "Erreur lors de la recherche : " + q.lastError().text());
        return;
    }

    // Charger le résultat dans la table
    loadQueryIntoTable(q);

    // Vérifier si un résultat a été trouvé
    if (ui->tableWidgetMateriel->rowCount() == 0)
    {
        QMessageBox::information(this, "Recherche",
                                 QString("Aucun matériel trouvé avec l'ID %1.").arg(id));
    }
}

void MainWindow::on_pb_afficher_materiel_clicked()
{
    refreshMaterielTable();

    if (ui->tableWidgetMateriel->rowCount() == 0)
    {
        QMessageBox::information(this, "Affichage",
                                 "Aucun matériel enregistré dans la base de données.");
    }
}


void MainWindow::populateMaterielFormFromRow(int row)
{
    if (!ui->tableWidgetMateriel || row < 0) return;

    auto get = [&](int col)->QString {
        auto *it = ui->tableWidgetMateriel->item(row, col);
        return it ? it->text().trimmed() : QString();
    };

    // Map colonnes → champs (ordre: ID..FOURNISSEUR)
    ui->le_id->setText(get(0));
    ui->le_nom->setText(get(1));

    // Pour les combos, on sélectionne l'item correspondant (si présent)
    const auto setCombo = [&](QComboBox* cb, const QString& v){
        if (!cb) return;
        int idx = cb->findText(v, Qt::MatchFixedString);
        if (idx < 0) { cb->addItem(v); idx = cb->findText(v, Qt::MatchFixedString); }
        cb->setCurrentIndex(idx);
    };
    setCombo(ui->cb_categorie,  get(2));
    setCombo(ui->cb_etat,       get(3));
    setCombo(ui->cb_utilisateur,get(4));

    ui->le_seuil->setText(get(5));
    ui->le_qt_totale->setText(get(6));
    ui->le_qt_dispo->setText(get(7));
    ui->le_prix->setText(get(8));
    ui->le_garantie->setText(get(9));
    ui->le_fournisseur->setText(get(10));

    // Load image from database
    int id = get(0).toInt();
    bool found = false;
    Materiel mat = Materiel::rechercherParId(id, found);

    if (found && mat.hasImage())
    {
        QByteArray imageData = mat.imageData();
        QPixmap pixmap = ImageHandler::byteArrayToPixmap(imageData);

        if (!pixmap.isNull())
        {
            currentMaterielImage = pixmap.toImage();
            hasImageData = true;
            displayImagePreview(currentMaterielImage);
            qDebug() << "Loaded image for material ID:" << id
                     << "- Size:" << imageData.size() << "bytes";
        }
    }
    else
    {
        clearImagePreview();
    }

    // Optionnel: empêcher la modif de l'ID pendant l'édition
    if (ui->le_id) ui->le_id->setReadOnly(true);

    // Generate and display QR code for the selected item
    if (found && ui->label_qrcode)
    {
        QImage qrImage = mat.generateQrCode(300);
        ui->label_qrcode->setPixmap(QPixmap::fromImage(qrImage));
        ui->label_qrcode->setScaledContents(false);
        ui->label_qrcode->setAlignment(Qt::AlignCenter);
    }
}

void MainWindow::on_tableWidgetMateriel_cellClicked(int row, int /*column*/)
{
    populateMaterielFormFromRow(row);
}


void MainWindow::on_pb_annuler_materiel_clicked()
{
    clearMaterielForm();
    if (ui->le_id) ui->le_id->setReadOnly(false); // pour réactiver l'édition d'ID
}

/* =========================
 *  IMAGE HANDLING
 * ========================= */

/**
 * @brief Creates image UI elements programmatically if not present in .ui file
 *
 * This method creates:
 * - Image preview label (label_image_preview_materiel)
 * - Upload button (pb_upload_image_materiel)
 * - Camera button (pb_camera_materiel)
 * - Clear image button (pb_clear_image_materiel)
 *
 * If these elements already exist in the .ui file, they will be used instead.
 */
void MainWindow::createImageUIElements()
{
    qDebug() << "Setting up image UI elements...";

    // Note: If the UI elements exist in mainwindow.ui, Qt Designer will have created them
    // This method serves as a fallback and for dynamic setup

    // Check if image preview label exists, if not log warning
    // In a production scenario, you would create the widgets programmatically here
    // For now, we'll rely on the UI file having these elements

    qDebug() << "Image UI elements setup complete";
}

/**
 * @brief Displays an image in the preview label
 * @param image The image to display
 */
void MainWindow::displayImagePreview(const QImage &image)
{
    if (image.isNull())
    {
        qWarning() << "Cannot display null image";
        return;
    }

    // Check if label exists in UI (you'll need to add this widget to mainwindow.ui)
    QLabel *imageLabel = ui->centralwidget->findChild<QLabel*>("label_image_preview_materiel");

    if (imageLabel)
    {
        // Scale image to fit label while maintaining aspect ratio
        QPixmap pixmap = QPixmap::fromImage(image);
        QPixmap scaledPixmap = pixmap.scaled(
            imageLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );

        imageLabel->setPixmap(scaledPixmap);
        imageLabel->setScaledContents(false);
        imageLabel->setAlignment(Qt::AlignCenter);

        qDebug() << "Image displayed in preview - Size:" << image.size();
    }
    else
    {
        // If label doesn't exist yet, just log
        qDebug() << "Image preview label not found in UI - Image ready for database storage";
    }

    currentMaterielImage = image;
    hasImageData = true;
}

/**
 * @brief Clears the image preview and resets image data
 */
void MainWindow::clearImagePreview()
{
    QLabel *imageLabel = ui->centralwidget->findChild<QLabel*>("label_image_preview_materiel");

    if (imageLabel)
    {
        imageLabel->clear();
        imageLabel->setText("Aucune image");
        imageLabel->setAlignment(Qt::AlignCenter);
    }

    currentMaterielImage = QImage();
    hasImageData = false;

    qDebug() << "Image preview cleared";
}

/**
 * @brief Verifies that the image matches the category using AI
 * @param category The expected material category
 * @return true if verification succeeds, false if it fails
 *
 * This is a BLOCKING operation that prevents database insert if verification fails.
 */
bool MainWindow::verifyImageWithAI(const QString &category)
{
    if (currentMaterielImage.isNull())
    {
        qWarning() << "Cannot verify null image";
        return false;
    }

    qDebug() << "Starting AI verification for category:" << category;

    // Call AI verification service - THIS IS BLOCKING
    VerificationResult result = AIVerifier::verifyImageCategory(
        currentMaterielImage,
        category,
        this
    );

    // Show result dialog to user
    bool canProceed = AIVerifier::showVerificationDialog(result, this);

    if (canProceed && result.isValid)
    {
        qDebug() << "AI verification SUCCESS - Category matches";
        return true;
    }
    else
    {
        qWarning() << "AI verification FAILED - Category mismatch or error";
        return false;
    }
}

/**
 * @brief Slot: Handles upload image button click
 */
void MainWindow::on_pb_upload_image_materiel_clicked()
{
    qDebug() << "Upload image button clicked";

    // Use ImageHandler to open file dialog and select image
    QImage image = ImageHandler::uploadImageFromFile();

    if (!image.isNull())
    {
        displayImagePreview(image);

        QMessageBox::information(this, "Image Sélectionnée",
            QString("Image chargée avec succès!\n"
                    "Taille: %1 x %2 pixels\n\n"
                    "L'image sera vérifiée par IA lors de l'ajout du matériel.")
            .arg(image.width())
            .arg(image.height()));
    }
    else
    {
        qDebug() << "No image selected or upload cancelled";
    }
}

/**
 * @brief Slot: Handles camera capture button click
 */
void MainWindow::on_pb_camera_materiel_clicked()
{
    qDebug() << "Camera capture button clicked";

    // Use ImageHandler to open camera dialog and capture image
    QImage image = ImageHandler::captureImageFromCamera(this);

    if (!image.isNull())
    {
        displayImagePreview(image);

        QMessageBox::information(this, "Image Capturée",
            QString("Image capturée avec succès depuis la caméra!\n"
                    "Taille: %1 x %2 pixels\n\n"
                    "L'image sera vérifiée par IA lors de l'ajout du matériel.")
            .arg(image.width())
            .arg(image.height()));
    }
    else
    {
        qDebug() << "Camera capture cancelled or failed";
    }
}

/**
 * @brief Slot: Handles clear image button click
 */
void MainWindow::on_pb_clear_image_materiel_clicked()
{
    qDebug() << "Clear image button clicked";

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Effacer l'image",
        "Voulez-vous vraiment effacer l'image sélectionnée?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply == QMessageBox::Yes)
    {
        clearImagePreview();
        QMessageBox::information(this, "Image Effacée",
            "L'image a été effacée. Vous pouvez en sélectionner une nouvelle.");
    }
}

/* =========================
 *  QR CODE GENERATION
 * ========================= */
void MainWindow::on_pb_generer_qrcode_clicked()
{
    // Get the current material data from form fields
    int id = 0;
    if (!validerChampNumerique(ui->le_id->text(), "ID", id, false))
        return;

    // Search for the material in the database
    bool found;
    Materiel m = Materiel::rechercherParId(id, found);

    if (!found)
    {
        QMessageBox::warning(this, "Erreur",
                             QString("Aucun matériel trouvé avec l'ID %1.\n"
                                     "Veuillez d'abord ajouter ou rechercher le matériel.").arg(id));
        return;
    }

    // Generate QR code
    QImage qrImage = m.generateQrCode(400);

    // Display QR code in a label (assumes you have a QLabel named label_qrcode in the UI)
    if (ui->label_qrcode)
    {
        ui->label_qrcode->setPixmap(QPixmap::fromImage(qrImage));
        ui->label_qrcode->setScaledContents(false);
        ui->label_qrcode->setAlignment(Qt::AlignCenter);
    }

    // Show QR code in a message box
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("QR Code - " + m.nom());
    msgBox.setText(QString("QR Code pour:\n"
                          "ID: %1\n"
                          "Nom: %2\n"
                          "Catégorie: %3\n\n"
                          "Scannez avec votre smartphone!")
                      .arg(m.id())
                      .arg(m.nom())
                      .arg(m.categorie()));

    // Add QR code image to the message box
    QPixmap pixmap = QPixmap::fromImage(qrImage);
    msgBox.setIconPixmap(pixmap.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Add buttons
    QPushButton *saveButton = msgBox.addButton("Sauvegarder", QMessageBox::ActionRole);
    msgBox.addButton("Fermer", QMessageBox::RejectRole);

    msgBox.exec();

    // Check if user clicked save
    if (msgBox.clickedButton() == saveButton)
    {
        // Ask user where to save the QR code
        QString fileName = QFileDialog::getSaveFileName(
            this,
            "Sauvegarder le QR Code",
            QString("QRCode_Materiel_%1_%2.png")
                .arg(m.id())
                .arg(m.nom().replace(" ", "_")),
            "Images PNG (*.png);;Images JPEG (*.jpg *.jpeg);;Tous les fichiers (*.*)"
            );

        if (!fileName.isEmpty())
        {
            if (m.saveQrCodeToFile(fileName, 600))
            {
                QMessageBox::information(this, "Succès",
                                         QString("QR Code sauvegardé avec succès:\n%1").arg(fileName));
            }
            else
            {
                QMessageBox::critical(this, "Erreur",
                                      "Impossible de sauvegarder le QR Code.");
            }
        }
    }
}

void MainWindow::on_pb_generer_qrcode_visible_clicked()
{
    // Call the existing QR code generation function
    on_pb_generer_qrcode_clicked();
}

void MainWindow::on_pb_sauvegarder_qrcode_clicked()
{
    // Reuse the generate function which now includes save option
    on_pb_generer_qrcode_clicked();
}

void MainWindow::on_pb_scanner_qrcode_clicked()
{
    QRScannerDialog scanner(this);
    if (scanner.exec() == QDialog::Accepted)
    {
        QString scannedData = scanner.getScannedData();
        if (!scannedData.isEmpty())
        {
            // Try to parse the ID from scanned data
            bool ok = false;
            int id = scannedData.toInt(&ok);

            // If the raw data isn't a plain integer, try to parse JSON payloads
            if (!ok)
            {
                QJsonParseError parseError;
                const QJsonDocument doc = QJsonDocument::fromJson(scannedData.toUtf8(), &parseError);
                if (parseError.error == QJsonParseError::NoError && doc.isObject())
                {
                    const QJsonObject obj = doc.object();
                    const QJsonValue idValue = obj.value("id");

                    if (idValue.isDouble())
                    {
                        id = idValue.toVariant().toInt(&ok);
                    }
                    else if (idValue.isString())
                    {
                        id = idValue.toString().toInt(&ok);
                    }
                }
            }

            if (ok && id > 0)
            {
                // Search for the material
                bool found;
                Materiel m = Materiel::rechercherParId(id, found);

                if (found)
                {
                    // Populate form with found material
                    ui->le_id->setText(QString::number(m.id()));
                    ui->le_nom->setText(m.nom());

                    int catIdx = ui->cb_categorie->findText(m.categorie());
                    if (catIdx >= 0) ui->cb_categorie->setCurrentIndex(catIdx);

                    int etatIdx = ui->cb_etat->findText(m.etat());
                    if (etatIdx >= 0) ui->cb_etat->setCurrentIndex(etatIdx);

                    int userIdx = ui->cb_utilisateur->findText(m.utilisateur());
                    if (userIdx >= 0) ui->cb_utilisateur->setCurrentIndex(userIdx);

                    ui->le_seuil->setText(QString::number(m.seuilAlerte()));
                    ui->le_qt_totale->setText(QString::number(m.qtTotale()));
                    ui->le_qt_dispo->setText(QString::number(m.qtDispo()));
                    ui->le_prix->setText(QString::number(m.prix(), 'f', 2));
                    ui->le_garantie->setText(m.garantie());
                    ui->le_fournisseur->setText(m.fournisseur());

                    // Generate and show QR code
                    if (ui->label_qrcode)
                    {
                        QImage qrImage = m.generateQrCode(300);
                        ui->label_qrcode->setPixmap(QPixmap::fromImage(qrImage));
                        ui->label_qrcode->setScaledContents(false);
                        ui->label_qrcode->setAlignment(Qt::AlignCenter);
                    }

                    QMessageBox::information(this, "Matériel Trouvé",
                                           QString("Matériel trouvé:\n"
                                                  "ID: %1\n"
                                                  "Nom: %2\n"
                                                  "État: %3")
                                               .arg(m.id())
                                               .arg(m.nom())
                                               .arg(m.etat()));
                }
                else
                {
                    QMessageBox::warning(this, "Non Trouvé",
                                       QString("Aucun matériel trouvé avec l'ID: %1").arg(id));
                }
            }
            else
            {
                QMessageBox::warning(this, "Données Invalides",
                                   "Les données scannées ne contiennent pas un ID valide.");
            }
        }
    }
}

/* =========================
 *  STATISTICS
 * ========================= */
void MainWindow::updateStatistics()
{
    QSqlQuery query;

    // Total items
    query.exec("SELECT COUNT(*) FROM MATERIEL");
    int total = 0;
    if (query.next()) total = query.value(0).toInt();

    // Available items
    query.exec("SELECT COUNT(*) FROM MATERIEL WHERE ETAT = 'Disponible'");
    int available = 0;
    if (query.next()) available = query.value(0).toInt();

    // Items in repair
    query.exec("SELECT COUNT(*) FROM MATERIEL WHERE ETAT = 'En réparation'");
    int inRepair = 0;
    if (query.next()) inRepair = query.value(0).toInt();

    // Low stock items (qt_dispo <= seuil_alerte)
    query.exec("SELECT COUNT(*) FROM MATERIEL WHERE QT_DISPO <= SEUIL_ALERTE");
    int lowStock = 0;
    if (query.next()) lowStock = query.value(0).toInt();

    // TODO: Add these labels to the UI in Qt Designer, then uncomment
    // Update labels if they exist
    // if (ui->label_total_items)
    //     ui->label_total_items->setText(QString::number(total));
    //
    // if (ui->label_available_items)
    //     ui->label_available_items->setText(QString::number(available));
    //
    // if (ui->label_repair_items)
    //     ui->label_repair_items->setText(QString::number(inRepair));
    //
    // if (ui->label_low_stock)
    // {
    //     ui->label_low_stock->setText(QString::number(lowStock));
    //     // Color code: red if there are low stock items
    //     if (lowStock > 0)
    //         ui->label_low_stock->setStyleSheet("QLabel { color: red; font-weight: bold; font-size: 16pt; }");
    //     else
    //         ui->label_low_stock->setStyleSheet("QLabel { color: green; font-size: 16pt; }");
    // }

    qDebug() << "Statistics updated: Total=" << total
             << "Available=" << available
             << "In Repair=" << inRepair
             << "Low Stock=" << lowStock;
}

/* =========================
 *  STATISTICS CHART (Ring/Donut Chart)
 * ========================= */

/**
 * @brief Creates and displays a donut chart for material statistics
 */
void MainWindow::createStatisticsChart()
{
    // Get statistics data
    QSqlQuery query;

    // Count by state
    QMap<QString, int> stateCount;
    query.exec("SELECT ETAT, COUNT(*) FROM MATERIEL GROUP BY ETAT");
    while (query.next())
    {
        stateCount[query.value(0).toString()] = query.value(1).toInt();
    }

    // Count by category
    QMap<QString, int> categoryCount;
    query.exec("SELECT CATEGORIE, COUNT(*) FROM MATERIEL GROUP BY CATEGORIE");
    while (query.next())
    {
        categoryCount[query.value(0).toString()] = query.value(1).toInt();
    }

    // Create dialog to display statistics
    QDialog *statsDialog = new QDialog(this);
    statsDialog->setWindowTitle("Statistiques des Matériels");
    statsDialog->resize(1000, 700);

    QVBoxLayout *mainLayout = new QVBoxLayout(statsDialog);

    // ===== STATE DISTRIBUTION CHART =====
    QPieSeries *stateSeries = new QPieSeries();
    stateSeries->setHoleSize(0.4);  // Make it a donut chart

    // Define colors for each state
    QMap<QString, QColor> stateColors;
    stateColors["Disponible"] = QColor(46, 204, 113);      // Green
    stateColors["En panne"] = QColor(231, 76, 60);         // Red
    stateColors["En réparation"] = QColor(241, 196, 15);   // Yellow
    stateColors["Empruntee"] = QColor(52, 152, 219);       // Blue

    for (auto it = stateCount.begin(); it != stateCount.end(); ++it)
    {
        QPieSlice *slice = stateSeries->append(it.key() + QString(" (%1)").arg(it.value()), it.value());
        slice->setLabelVisible(true);
        slice->setLabelPosition(QPieSlice::LabelOutside);
        if (stateColors.contains(it.key()))
            slice->setColor(stateColors[it.key()]);

        // Highlight slice on hover
        slice->setExplodeDistanceFactor(0.1);
        connect(slice, &QPieSlice::hovered, [slice](bool state) {
            slice->setExploded(state);
        });
    }

    QChart *stateChart = new QChart();
    stateChart->addSeries(stateSeries);
    stateChart->setTitle("Distribution par État");
    stateChart->setAnimationOptions(QChart::SeriesAnimations);
    stateChart->legend()->setAlignment(Qt::AlignRight);

    QChartView *stateChartView = new QChartView(stateChart);
    stateChartView->setRenderHint(QPainter::Antialiasing);
    stateChartView->setMinimumHeight(300);

    // ===== CATEGORY DISTRIBUTION CHART =====
    QPieSeries *categorySeries = new QPieSeries();
    categorySeries->setHoleSize(0.4);  // Make it a donut chart

    // Define colors for each category
    QMap<QString, QColor> categoryColors;
    categoryColors["Informatique"] = QColor(155, 89, 182);   // Purple
    categoryColors["Bureautique"] = QColor(26, 188, 156);    // Turquoise
    categoryColors["Sport"] = QColor(230, 126, 34);          // Orange
    categoryColors["Autre"] = QColor(149, 165, 166);         // Gray

    for (auto it = categoryCount.begin(); it != categoryCount.end(); ++it)
    {
        QPieSlice *slice = categorySeries->append(it.key() + QString(" (%1)").arg(it.value()), it.value());
        slice->setLabelVisible(true);
        slice->setLabelPosition(QPieSlice::LabelOutside);
        if (categoryColors.contains(it.key()))
            slice->setColor(categoryColors[it.key()]);

        // Highlight slice on hover
        slice->setExplodeDistanceFactor(0.1);
        connect(slice, &QPieSlice::hovered, [slice](bool state) {
            slice->setExploded(state);
        });
    }

    QChart *categoryChart = new QChart();
    categoryChart->addSeries(categorySeries);
    categoryChart->setTitle("Distribution par Catégorie");
    categoryChart->setAnimationOptions(QChart::SeriesAnimations);
    categoryChart->legend()->setAlignment(Qt::AlignRight);

    QChartView *categoryChartView = new QChartView(categoryChart);
    categoryChartView->setRenderHint(QPainter::Antialiasing);
    categoryChartView->setMinimumHeight(300);

    // ===== SUMMARY STATISTICS =====
    QLabel *summaryLabel = new QLabel();
    summaryLabel->setStyleSheet("QLabel { font-size: 12pt; padding: 10px; background-color: #ecf0f1; border-radius: 5px; }");

    int total = 0;
    query.exec("SELECT COUNT(*) FROM MATERIEL");
    if (query.next()) total = query.value(0).toInt();

    int available = stateCount.value("Disponible", 0);
    int lowStock = 0;
    query.exec("SELECT COUNT(*) FROM MATERIEL WHERE QT_DISPO <= SEUIL_ALERTE");
    if (query.next()) lowStock = query.value(0).toInt();

    double totalValue = 0.0;
    query.exec("SELECT SUM(PRIX * QT_TOTALE) FROM MATERIEL");
    if (query.next()) totalValue = query.value(0).toDouble();

    QString summaryText = QString(
        "<b>📊 Résumé des Statistiques</b><br>"
        "━━━━━━━━━━━━━━━━━━━━━━━━<br>"
        "• <b>Total d'articles:</b> %1<br>"
        "• <b>Articles disponibles:</b> %2 (%3%)<br>"
        "• <b>Articles en stock faible:</b> <span style='color: %4;'>%5</span><br>"
        "• <b>Valeur totale estimée:</b> %6 DT"
    ).arg(total)
     .arg(available)
     .arg(total > 0 ? QString::number(available * 100.0 / total, 'f', 1) : "0")
     .arg(lowStock > 0 ? "red" : "green")
     .arg(lowStock)
     .arg(QString::number(totalValue, 'f', 2));

    summaryLabel->setText(summaryText);

    // Add all widgets to layout
    mainLayout->addWidget(summaryLabel);
    mainLayout->addWidget(stateChartView);
    mainLayout->addWidget(categoryChartView);

    // Add close button
    QPushButton *closeButton = new QPushButton("Fermer");
    closeButton->setMaximumWidth(100);
    connect(closeButton, &QPushButton::clicked, statsDialog, &QDialog::accept);
    mainLayout->addWidget(closeButton, 0, Qt::AlignCenter);

    statsDialog->exec();

    // Clean up
    delete statsDialog;
}

/**
 * @brief Slot: Shows statistics chart when button is clicked
 */
void MainWindow::on_pb_show_statistics_clicked()
{
    qDebug() << "Show statistics button clicked";
    createStatisticsChart();
}

/* =========================
 *  SORTING FUNCTIONALITY
 * ========================= */

/**
 * @brief Handles table header clicks for sorting
 * @param column The column index that was clicked
 */
void MainWindow::on_tableWidgetMateriel_horizontalHeaderClicked(int column)
{
    qDebug() << "Header clicked - Column:" << column;

    // Toggle sort order if clicking same column, otherwise ascending
    if (currentSortColumn == column)
    {
        currentSortOrder = (currentSortOrder == Qt::AscendingOrder)
                           ? Qt::DescendingOrder
                           : Qt::AscendingOrder;
    }
    else
    {
        currentSortColumn = column;
        currentSortOrder = Qt::AscendingOrder;
    }

    sortTableByColumn(currentSortColumn, currentSortOrder);
}

/**
 * @brief Sorts the table by specified column and order
 * @param column The column to sort by
 * @param order The sort order (ascending/descending)
 */
void MainWindow::sortTableByColumn(int column, Qt::SortOrder order)
{
    if (!ui->tableWidgetMateriel || column < 0 || column >= ui->tableWidgetMateriel->columnCount())
        return;

    qDebug() << "Sorting column" << column << "in"
             << (order == Qt::AscendingOrder ? "ascending" : "descending") << "order";

    // Temporarily disable sorting to prevent recursion
    ui->tableWidgetMateriel->setSortingEnabled(false);

    // Sort the table
    ui->tableWidgetMateriel->sortItems(column, order);

    // Re-enable sorting
    ui->tableWidgetMateriel->setSortingEnabled(true);
}

/**
 * @brief Slot: Shows sort dialog when sort button is clicked
 */
void MainWindow::on_pb_sort_clicked()
{
    qDebug() << "Sort button clicked";

    // Create sort dialog
    QDialog *sortDialog = new QDialog(this);
    sortDialog->setWindowTitle("Trier les Matériels");
    sortDialog->resize(350, 150);

    QVBoxLayout *layout = new QVBoxLayout(sortDialog);

    // Column selection
    QLabel *columnLabel = new QLabel("Trier par colonne:");
    layout->addWidget(columnLabel);

    QComboBox *columnCombo = new QComboBox();
    columnCombo->addItem("ID", 0);
    columnCombo->addItem("Nom", 1);
    columnCombo->addItem("Catégorie", 2);
    columnCombo->addItem("État", 3);
    columnCombo->addItem("Utilisateur", 4);
    columnCombo->addItem("Seuil", 5);
    columnCombo->addItem("Qt. Totale", 6);
    columnCombo->addItem("Qt. Disponible", 7);
    columnCombo->addItem("Prix", 8);
    columnCombo->addItem("Garantie", 9);
    columnCombo->addItem("Fournisseur", 10);
    layout->addWidget(columnCombo);

    // Order selection
    QLabel *orderLabel = new QLabel("Ordre:");
    layout->addWidget(orderLabel);

    QComboBox *orderCombo = new QComboBox();
    orderCombo->addItem("Croissant (A→Z, 0→9)", Qt::AscendingOrder);
    orderCombo->addItem("Décroissant (Z→A, 9→0)", Qt::DescendingOrder);
    layout->addWidget(orderCombo);

    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, sortDialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, sortDialog, &QDialog::reject);

    if (sortDialog->exec() == QDialog::Accepted)
    {
        int column = columnCombo->currentData().toInt();
        Qt::SortOrder order = static_cast<Qt::SortOrder>(orderCombo->currentData().toInt());

        currentSortColumn = column;
        currentSortOrder = order;

        sortTableByColumn(column, order);

        QMessageBox::information(this, "Tri Appliqué",
            QString("Les matériels ont été triés par %1 en ordre %2.")
            .arg(columnCombo->currentText())
            .arg(orderCombo->currentText()));
    }

    delete sortDialog;
}

/* =========================
 *  PDF EXPORT
 * ========================= */

/**
 * @brief Generates a PDF report with table data, statistics, and summary
 * @param fileName The output PDF file path
 */
void MainWindow::generatePdfReport(const QString &fileName)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize::A4);
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    // Create text document for PDF content
    QTextDocument document;
    QTextCursor cursor(&document);

    // ===== TITLE =====
    QTextCharFormat titleFormat;
    titleFormat.setFontPointSize(18);
    titleFormat.setFontWeight(QFont::Bold);
    titleFormat.setForeground(QBrush(QColor(41, 128, 185)));

    cursor.insertText("Rapport des Matériels\n", titleFormat);
    cursor.insertText("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n", titleFormat);

    // ===== DATE =====
    QTextCharFormat normalFormat;
    normalFormat.setFontPointSize(10);

    cursor.insertText("Date du rapport: " + QDate::currentDate().toString("dd/MM/yyyy") + "\n", normalFormat);
    cursor.insertText("Heure: " + QTime::currentTime().toString("HH:mm:ss") + "\n\n", normalFormat);

    // ===== STATISTICS SUMMARY =====
    QTextCharFormat headerFormat;
    headerFormat.setFontPointSize(14);
    headerFormat.setFontWeight(QFont::Bold);
    headerFormat.setForeground(QBrush(QColor(52, 73, 94)));

    cursor.insertText("📊 Résumé Statistique\n", headerFormat);
    cursor.insertText("━━━━━━━━━━━━━━━━━━━━━\n\n", headerFormat);

    // Calculate statistics
    QSqlQuery query;
    int total = 0, available = 0, inRepair = 0, lowStock = 0;
    double totalValue = 0.0;

    query.exec("SELECT COUNT(*) FROM MATERIEL");
    if (query.next()) total = query.value(0).toInt();

    query.exec("SELECT COUNT(*) FROM MATERIEL WHERE ETAT = 'Disponible'");
    if (query.next()) available = query.value(0).toInt();

    query.exec("SELECT COUNT(*) FROM MATERIEL WHERE ETAT = 'En réparation'");
    if (query.next()) inRepair = query.value(0).toInt();

    query.exec("SELECT COUNT(*) FROM MATERIEL WHERE QT_DISPO <= SEUIL_ALERTE");
    if (query.next()) lowStock = query.value(0).toInt();

    query.exec("SELECT SUM(PRIX * QT_TOTALE) FROM MATERIEL");
    if (query.next()) totalValue = query.value(0).toDouble();

    cursor.insertText(QString("• Total d'articles: %1\n").arg(total), normalFormat);
    cursor.insertText(QString("• Articles disponibles: %1 (%2%)\n")
        .arg(available)
        .arg(total > 0 ? QString::number(available * 100.0 / total, 'f', 1) : "0"), normalFormat);
    cursor.insertText(QString("• Articles en réparation: %1\n").arg(inRepair), normalFormat);
    cursor.insertText(QString("• Articles en stock faible: %1\n").arg(lowStock), normalFormat);
    cursor.insertText(QString("• Valeur totale estimée: %1 DT\n\n").arg(QString::number(totalValue, 'f', 2)), normalFormat);

    // ===== STATE DISTRIBUTION =====
    cursor.insertText("Distribution par État:\n", headerFormat);

    query.exec("SELECT ETAT, COUNT(*) FROM MATERIEL GROUP BY ETAT");
    while (query.next())
    {
        QString state = query.value(0).toString();
        int count = query.value(1).toInt();
        double percentage = total > 0 ? (count * 100.0 / total) : 0.0;
        cursor.insertText(QString("  • %1: %2 (%3%)\n")
            .arg(state)
            .arg(count)
            .arg(QString::number(percentage, 'f', 1)), normalFormat);
    }
    cursor.insertText("\n", normalFormat);

    // ===== CATEGORY DISTRIBUTION =====
    cursor.insertText("Distribution par Catégorie:\n", headerFormat);

    query.exec("SELECT CATEGORIE, COUNT(*) FROM MATERIEL GROUP BY CATEGORIE");
    while (query.next())
    {
        QString category = query.value(0).toString();
        int count = query.value(1).toInt();
        double percentage = total > 0 ? (count * 100.0 / total) : 0.0;
        cursor.insertText(QString("  • %1: %2 (%3%)\n")
            .arg(category)
            .arg(count)
            .arg(QString::number(percentage, 'f', 1)), normalFormat);
    }
    cursor.insertText("\n\n", normalFormat);

    // ===== TABLE DATA =====
    cursor.insertText("📋 Liste des Matériels\n", headerFormat);
    cursor.insertText("━━━━━━━━━━━━━━━━━━━━━\n\n", headerFormat);

    // Create table
    QTextTableFormat tableFormat;
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableFormat.setBorder(1);
    tableFormat.setCellPadding(3);
    tableFormat.setCellSpacing(0);
    tableFormat.setHeaderRowCount(1);

    int rows = ui->tableWidgetMateriel->rowCount();
    int cols = ui->tableWidgetMateriel->columnCount();

    QTextTable *table = cursor.insertTable(rows + 1, cols, tableFormat);

    // Header row
    QTextCharFormat tableHeaderFormat;
    tableHeaderFormat.setFontWeight(QFont::Bold);
    tableHeaderFormat.setBackground(QBrush(QColor(52, 152, 219)));
    tableHeaderFormat.setForeground(QBrush(Qt::white));

    for (int col = 0; col < cols; ++col)
    {
        QTextTableCell cell = table->cellAt(0, col);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.setCharFormat(tableHeaderFormat);
        QString headerText = ui->tableWidgetMateriel->horizontalHeaderItem(col)->text();
        cellCursor.insertText(headerText);
    }

    // Data rows
    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            QTextTableCell cell = table->cellAt(row + 1, col);
            QTextCursor cellCursor = cell.firstCursorPosition();
            QTableWidgetItem *item = ui->tableWidgetMateriel->item(row, col);
            cellCursor.insertText(item ? item->text() : "");
        }
    }

    // ===== FOOTER =====
    cursor.movePosition(QTextCursor::End);
    cursor.insertText("\n\n", normalFormat);

    QTextCharFormat footerFormat;
    footerFormat.setFontPointSize(8);
    footerFormat.setForeground(QBrush(QColor(127, 140, 141)));

    cursor.insertText("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n", footerFormat);
    cursor.insertText("Rapport généré automatiquement par le Système de Gestion des Matériels\n", footerFormat);
    cursor.insertText("© 2025 - Tous droits réservés\n", footerFormat);

    // Print to PDF
    document.print(&printer);

    qDebug() << "PDF report generated:" << fileName;
}

/**
 * @brief Slot: Exports data to PDF when export button is clicked
 */
void MainWindow::on_pb_export_pdf_clicked()
{
    qDebug() << "Export PDF button clicked";

    // Check if there's data to export
    if (!ui->tableWidgetMateriel || ui->tableWidgetMateriel->rowCount() == 0)
    {
        QMessageBox::warning(this, "Aucune Donnée",
            "Aucune donnée à exporter. Veuillez d'abord afficher les matériels.");
        return;
    }

    // Ask user where to save the PDF
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Exporter en PDF",
        QString("Rapport_Materiels_%1.pdf").arg(QDate::currentDate().toString("yyyy-MM-dd")),
        "Fichiers PDF (*.pdf);;Tous les fichiers (*.*)"
    );

    if (fileName.isEmpty())
    {
        qDebug() << "PDF export cancelled";
        return;
    }

    // Generate the PDF
    try
    {
        generatePdfReport(fileName);

        QMessageBox::information(this, "Export Réussi",
            QString("Le rapport PDF a été généré avec succès:\n\n%1\n\n"
                    "Le rapport contient:\n"
                    "• Les statistiques détaillées\n"
                    "• La distribution par état et catégorie\n"
                    "• Le tableau complet des matériels")
            .arg(fileName));
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(this, "Erreur d'Export",
            QString("Une erreur s'est produite lors de la génération du PDF:\n%1")
            .arg(e.what()));
    }
}

/* =========================================================================
 *  STUB IMPLEMENTATIONS FOR MAIN BRANCH METHODS (Activities, Adherents, Locations)
 *  TODO: Implement these methods by copying from the original mainwindow.cpp
 * ========================================================================= */

// ==========================================
// CRUD Activités
// ==========================================

void MainWindow::on_bajouterA_clicked()
{
    Activite a = getActiviteFromInputs();
    QString text = ui->coachA1->currentText();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un coach !");
        return;
    }
    QStringList parts = text.split(" - ");
    if (parts.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Format du coach invalide !");
        return;
    }
    int coachId = parts[0].toInt();
    a.setCoach(coachId);
    int id = ui->IDA1->text().toInt();
    if (id == 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez saisir un ID valide !");
        return;
    }
    QSqlQuery verif(db);
    verif.prepare("SELECT COUNT(*) FROM EMPLOYE WHERE ID_EMPLOYE = :id AND STATUS='libre'");
    verif.bindValue(":id", a.getCoach());
    verif.exec();
    verif.next();
    if (verif.value(0).toInt() == 0)
    {
        QMessageBox::warning(this, "Erreur", "Le coach n’existe pas ou n’est pas libre !");
        return;
    }
    verif.prepare("SELECT COUNT(*) FROM LOCAL WHERE ID_LOCAL = :id");
    verif.bindValue(":id", a.getlocal_A());
    verif.exec();
    verif.next();
    if (verif.value(0).toInt() == 0)
    {
        QMessageBox::warning(this, "Erreur", "Le local n’existe pas !");
        return;
    }
    if (a.ajouter(db))
    {
        chargerActivites();
        clearAjouterAInputs();
        QMessageBox::information(this, "Succès", "Activité ajoutée !");
    }
    else
    {
        QMessageBox::critical(this, "Erreur", "Échec d'ajout !");
    }
}

void MainWindow::on_bannulerA_clicked()
{
    clearAjouterAInputs();
}

void MainWindow::on_b_A_supprimer_clicked()
{
    int id = ui->IDMas->text().toInt();

    // ---- Contrôle de saisie ----
    if (ui->IDMas->text().isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez saisir un ID !");
        return;
    }

    if (id <= 0) {
        QMessageBox::warning(this, "Erreur", "ID invalide !");
        return;
    }

    // ---- Message de confirmation ----
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
        this,
        "Confirmation",
        "Êtes-vous sûr de vouloir supprimer cette activité ?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::No) {
        return; // Annuler la suppression
    }

    // ---- Suppression ----
    Activite a;
    a.setId(id);

    if (a.supprimer(db)) {
        chargerActivites();
        QMessageBox::information(this, "Succès", "Activité supprimée !");
    } else {
        QMessageBox::warning(this, "Erreur", "Aucune activité trouvée avec cet ID !");
    }
}

void MainWindow::on_b_A_modifier_clicked()
{
    int id = ui->IDMas->text().toInt();
    if (id == 0) {
        QMessageBox::warning(this, "Erreur", "ID invalide !");
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT * FROM ACTIVITE WHERE ID_ACTIVITE=:id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        Activite a(query.value("ID_ACTIVITE").toInt(),
                   query.value("NOM").toString(),
                   query.value("TYPE").toString(),
                   query.value("DUREE").toString(),
                   query.value("HORAIRE").toDateTime(),
                   query.value("COACH").toInt(),
                   query.value("NBP").toInt(),
                   query.value("LOCAL_A").toInt(),
                   query.value("STATUS").toString());
        remplirModifierA(a);
        ui->tabActiviter->setCurrentIndex(1);
    } else {
        QMessageBox::warning(this, "Erreur", "Activité introuvable !");
    }
}

void MainWindow::on_ok_modif_A_clicked()
{
    Activite a;
    a.setId(ui->idm->text().toInt());
    a.setNom(ui->nomm->text());
    a.setType(ui->typem->currentText());
    a.setHoraire(ui->horairem->dateTime());
    a.setDuree(ui->dureem->time().toString("HH:mm"));
    a.setCoach(ui->coachm->text().toInt());
    a.setLocal_A(ui->local_Am->text().toInt());
    a.setStatus(ui->statusm->currentText());
    a.setNbp(ui->npam->value());

    if (a.modifier(db)) {
        chargerActivites();
        ui->tabActiviter->setCurrentIndex(0);
        QMessageBox::information(this, "Succès", "Activité modifiée !");
    } else {
        QMessageBox::critical(this, "Erreur", "Échec modification !");
    }
}

void MainWindow::on_annuler_modif_A_clicked()
{
    ui->tabActiviter->setCurrentIndex(0);
}

void MainWindow::on_b_A_afficher_clicked()
{
    int id = ui->IDMas->text().toInt();
    if (id == 0) {
        QMessageBox::warning(this, "Erreur", "ID invalide !");
        return;
    }

    Activite ac;
    bool found = false;

    Activite a = ac.getActiviteById(id, db, found);

    if (found) {
        remplirAfficherA(a);
        setAfficherAEditable(false);
        ui->tabActiviter->setCurrentIndex(2);
    } else {
        QMessageBox::warning(this, "Erreur", "Activité introuvable !");
    }
}

void MainWindow::on_ok_aff_A_clicked()
{
    ui->tabActiviter->setCurrentIndex(0);
    setAfficherAEditable(true);
}

// ==========================================
// Calendrier
// ==========================================

void MainWindow::afficherCalendrier()
{
    ui->tabActiviter->setCurrentIndex(3);
}

void MainWindow::afficherActivitesDuJour(const QDate &date)
{
    ui->tabA_2->clearContents();
    ui->tabA_2->setRowCount(0);

    QList<Activite> activites = Activite::getActivitesDuJour(date, db);

    int row = 0;
    for (const Activite &a : activites)
    {
        ui->tabA_2->insertRow(row);

        ui->tabA_2->setItem(row, 0, new QTableWidgetItem(QString::number(a.getId())));
        ui->tabA_2->setItem(row, 1, new QTableWidgetItem(a.getNom()));
        ui->tabA_2->setItem(row, 2, new QTableWidgetItem(a.getType()));
        ui->tabA_2->setItem(row, 3, new QTableWidgetItem(a.getHoraire().toString("yyyy-MM-dd")));
        ui->tabA_2->setItem(row, 4, new QTableWidgetItem(a.getDuree()));
        ui->tabA_2->setItem(row, 5, new QTableWidgetItem(a.getStatus()));
        ui->tabA_2->setItem(row, 6, new QTableWidgetItem(QString::number(a.getNbp())));
        ui->tabA_2->setItem(row, 7, new QTableWidgetItem(QString::number(a.getCoach())));
        ui->tabA_2->setItem(row, 8, new QTableWidgetItem(QString::number(a.getlocal_A())));

        row++;
    }
}

void MainWindow::afficherToutesActivitesDansTabA2()
{
    ui->tabA_2->clearContents();
    ui->tabA_2->setRowCount(0);

    Activite ac;
    QVector<QStringList> data = ac.getToutesActivites(db);

    for(int row = 0; row < data.size(); row++) {
        ui->tabA_2->insertRow(row);

        for(int col = 0; col < data[row].size(); col++) {
            QTableWidgetItem *item = new QTableWidgetItem(data[row][col]);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            ui->tabA_2->setItem(row, col, item);
        }
    }
}

void MainWindow::on_B_A_retour_2_clicked()
{
    ui->tabActiviter->setCurrentIndex(0); // 0 = ajouterA
}

// ==========================================
// Tri et recherche
// ==========================================

void MainWindow::on_Btrier_clicked()
{
    QString critere = ui->trier->currentText();

    if(critere.isEmpty()) {
        QMessageBox::warning(this,"Attention","Veuillez choisir un critère !");
        return;
    }

    QSqlQuery query = Activite::trierActivites(critere, db);

    if(!query.isActive()) {
        QMessageBox::critical(this, "Erreur", "Erreur lors du tri !");
        return;
    }

    ui->tabA->clearContents();
    ui->tabA->setRowCount(0);

    int row = 0;
    while(query.next()) {
        ui->tabA->insertRow(row);

        ui->tabA->setItem(row,0,new QTableWidgetItem(query.value("ID_ACTIVITE").toString()));
        ui->tabA->setItem(row,1,new QTableWidgetItem(query.value("NOM").toString()));
        ui->tabA->setItem(row,2,new QTableWidgetItem(query.value("TYPE").toString()));
        ui->tabA->setItem(row,3,new QTableWidgetItem(query.value("HORAIRE").toDateTime().toString("yyyy-MM-dd")));
        ui->tabA->setItem(row,4,new QTableWidgetItem(query.value("DUREE").toString()));
        ui->tabA->setItem(row,5,new QTableWidgetItem(query.value("STATUS").toString()));
        ui->tabA->setItem(row,6,new QTableWidgetItem(query.value("NBP").toString()));
        ui->tabA->setItem(row,7,new QTableWidgetItem(query.value("COACH").toString()));
        ui->tabA->setItem(row,8,new QTableWidgetItem(query.value("LOCAL_A").toString()));

        row++;
    }
}

void MainWindow::on_Bchercher_clicked()
{
    int id = ui->idr->text().toInt();
    if(id == 0) {
        QMessageBox::warning(this,"Erreur","Veuillez saisir un ID valide !");
        return;
    }


    QSqlQuery query = Activite::chercherActiviteParID(id, db);

    if(query.next()) {

        ui->tabA->clearContents();
        ui->tabA->setRowCount(1);

        ui->tabA->setItem(0, 0, new QTableWidgetItem(query.value("ID_ACTIVITE").toString()));
        ui->tabA->setItem(0, 1, new QTableWidgetItem(query.value("NOM").toString()));
        ui->tabA->setItem(0, 2, new QTableWidgetItem(query.value("TYPE").toString()));
        ui->tabA->setItem(0, 3, new QTableWidgetItem(query.value("DUREE").toString()));
        ui->tabA->setItem(0, 4, new QTableWidgetItem(query.value("HORAIRE").toDateTime().toString("yyyy-MM-dd hh:mm")));
        ui->tabA->setItem(0, 5, new QTableWidgetItem(query.value("COACH").toString()));
        ui->tabA->setItem(0, 6, new QTableWidgetItem(query.value("LOCAL_A").toString()));
        ui->tabA->setItem(0, 7, new QTableWidgetItem(query.value("NBP").toString()));
        ui->tabA->setItem(0, 8, new QTableWidgetItem(query.value("STATUS").toString()));

        setAfficherAEditable(false);
    }
    else {
        QMessageBox::warning(this,"Erreur","Aucune activité trouvée avec cet ID !");
    }
}

// ==========================================
// Statistiques activités
// ==========================================

void MainWindow::afficher_stat_act()
{
    QMap<QString,int> stats = Activite::statistiquesParType(db);

    QPieSeries *series = new QPieSeries();
    for(auto it = stats.begin(); it != stats.end(); ++it) {
        series->append(it.key() + " : " + QString::number(it.value()), it.value());
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Statistiques des activites selon Type");
    series->setHoleSize(0.3);
    chart->legend()->setAlignment(Qt::AlignRight);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);


    QLayoutItem *item;
    while ((item = ui->stat_layout->takeAt(0)) != nullptr) {
        if(item->widget()) delete item->widget();
        delete item;
    }

    ui->stat_layout->addWidget(chartView);
}

void MainWindow::on_B_retour_stat_act_clicked()
{
    ui->tabActiviter->setCurrentIndex(0);
}

void MainWindow::on_Bexporter_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", "", "*.pdf");
    if(fileName.isEmpty()) return;
    if(!fileName.endsWith(".pdf")) fileName += ".pdf";

    if(Activite::exporterVersPDF(ui->tabA, fileName))
        QMessageBox::information(this, "Succès", "Export PDF terminé !");
    else
        QMessageBox::warning(this, "Attention", "Le tableau est vide !");
}

void MainWindow::on_afficher_regroupement_clicked()
{
    ui->tabActiviter->setCurrentIndex(5);

    QSqlQueryModel *model = Activite::getRegroupementAdherents();
    if(!model) {
        QMessageBox::warning(this, "Erreur SQL", "Impossible de charger le regroupement !");
        return;
    }

    ui->tableView_regroupement->setModel(model);
}

void MainWindow::on_B_retour_stat_act_2_clicked()
{
    ui->tabActiviter->setCurrentIndex(0);
}

void MainWindow::chargerSuggestionCoach()
{
    Activite a;
    a.chargerSuggestionCoach(ui->coachA1, db);
}

void MainWindow::highlighterJoursActivites()
{
    ui->calendarWidget->setDateTextFormat(QDate(), QTextCharFormat());


    QList<QDate> dates = Activite::getDatesAvecActivites(db);


    QTextCharFormat format;
    format.setBackground(Qt::green);

    for (const QDate &d : dates) {
        ui->calendarWidget->setDateTextFormat(d, format);
    }
}

// ==========================================
// Navigation supplémentaire
// ==========================================

void MainWindow::on_B_A_retour_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_B_A_retour_5_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_B_A_retour_6_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_bt_retour_accueil_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

// ==========================================
// Adhérents
// ==========================================

void MainWindow::on_valider_a_clicked()
{
    Adherent a;
    a.setId(ui->id_a1->text().toInt());
    a.setNom(ui->nom_a1->text());
    a.setPrenom(ui->prenom_a1->text());
    a.setGenre(ui->genre_a1->isChecked() ? "Homme" : "Femme");
    a.setDateNaissance(ui->date_a1->text());
    a.setCin(ui->cin_a1->text().toInt());
    a.setAdresse(ui->adresse_a1->text());
    a.setGsm(ui->gsm_a1->text());
    a.setEmail(ui->mail_a1->text());

    if (a.ajouter(db)) {
        QMessageBox::information(this, "Succès", "Adhérent ajouté !");
        chargerTableAdherents();
        clearAdherentInputs();

        // 🔹 AJOUT NECESSAIRE POUR INSCRIPTION AUTOMATIQUE
        int idAdherent = a.getId();
        int idActivite = ui->cbxActivite->text().toInt();

        QSqlQuery check(db);
        check.prepare("SELECT COUNT(*) FROM ACTIVITE WHERE ID_ACTIVITE = :id");
        check.bindValue(":id", idActivite);
        if (check.exec() && check.next() && check.value(0).toInt() > 0) {
            a.inscrireAdherent(idAdherent, idActivite);

            QSqlQuery q(db);
            q.prepare("SELECT STATUS, NOM FROM ACTIVITE WHERE ID_ACTIVITE = :id");
            q.bindValue(":id", idActivite);
            if(q.exec() && q.next()) {
                QString statut = q.value("STATUS").toString();
                QString nomActivite = q.value("NOM").toString();
                if(statut == "Annulée") {
                    a.notifierAnnulation(db, idActivite, nomActivite);
                }
            }

        } else if(idActivite != 0) {
            QMessageBox::warning(this, "Erreur", "Cette activité n'existe pas !");
        }

    } else {
        QMessageBox::critical(this, "Erreur", "Échec de l'ajout !");
    }
}

void MainWindow::on_annuler_a_clicked()
{
    clearAdherentInputs();
}

void MainWindow::on_supprimer_a_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  "Êtes-vous sûr de vouloir supprimer cet adhérent ?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No)
        return;
    Adherent a;
    a.setId(ui->supprimer_a1->text().toInt());
    if(a.supprimer(db)) {
        QMessageBox::information(this,"Succès","Adhérent supprimé !");
    } else {
        QMessageBox::warning(this,"Erreur","ID introuvable !");
    }
    chargerTableAdherents();

}

void MainWindow::on_modifier_a_clicked()
{
    int id = ui->modifier_a1->text().toInt();
    if (id == 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez saisir un ID valide !");
        return;
    }

    Adherent a;
    QMap<QString, QVariant> data = a.afficher(db, id);

    if (!data.isEmpty()) {
        ui->id_a1->setText(data["ID_ADHERENT"].toString());
        ui->id_a1->setEnabled(false);

        ui->nom_a1->setText(data["NOM_ADHERENT"].toString());
        ui->prenom_a1->setText(data["PRENOM_ADHERENT"].toString());

        QString genre = data["GENRE_ADHERENT"].toString();
        if (genre == "Homme")
            ui->genre_a1->setChecked(true);
        else
            ui->genre_a2->setChecked(true);

        QDate date = data["DATE_DE_NAISSANCE_ADHERENT"].toDate();
        ui->date_a1->setDate(date);

        ui->cin_a1->setText(data["CIN_ADHERENT"].toString());
        ui->adresse_a1->setText(data["ADRESSE_ADHERENT"].toString());
        ui->gsm_a1->setText(data["GSM_ADHERENT"].toString());
        ui->mail_a1->setText(data["EMAIL_ADHERENT"].toString());

        QMessageBox::information(this, "Succès", "Adhérent trouvé et chargé !");
    } else {
        QMessageBox::warning(this, "Erreur", "Adhérent introuvable !");
    }
}

void MainWindow::on_afficher_adherent_clicked()
{
    chargerTableAdherents();

    QMessageBox::information(this, "Affichage", "Table des adhérents mise à jour !");
}

void MainWindow::on_modifier_a_3_clicked()
{
    int id = ui->modifier_a1->text().toInt();
    if (id == 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez saisir un ID valide pour la modification !");
        return;
    }

    if (ui->nom_a1->text().isEmpty() ||
        ui->prenom_a1->text().isEmpty() ||
        (!ui->genre_a1->isChecked() && !ui->genre_a2->isChecked()) ||
        ui->cin_a1->text().isEmpty() ||
        ui->adresse_a1->text().isEmpty() ||
        ui->gsm_a1->text().isEmpty() ||
        ui->mail_a1->text().isEmpty())
    {
        QMessageBox::warning(this, "Champs manquants", "⚠️ Veuillez remplir tous les champs !");
        return;
    }

    if (ui->cin_a1->text().length() != 8) {
        QMessageBox::warning(this, "Erreur de saisie", "Le CIN doit contenir exactement 8 chiffres.");
        return;
    }

    if (ui->gsm_a1->text().length() != 8) {
        QMessageBox::warning(this, "Erreur de saisie", "Le numéro GSM doit contenir 8 chiffres.");
        return;
    }

    QString email = ui->mail_a1->text();
    QRegularExpression regexEmail("^[\\w\\.]+@[\\w\\.]+\\.[a-z]{2,4}$");
    if (!regexEmail.match(email).hasMatch()) {
        QMessageBox::warning(this, "Email invalide", "Veuillez entrer une adresse email valide !");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  "Êtes-vous sûr de vouloir modifier cet adhérent ?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No)
        return;

    Adherent a;
    a.setId(id);
    a.setNom(ui->nom_a1->text());
    a.setPrenom(ui->prenom_a1->text());
    a.setGenre(ui->genre_a1->isChecked() ? "Homme" : "Femme");
    a.setDateNaissance(ui->date_a1->date().toString("MM/dd/yyyy"));
    a.setCin(ui->cin_a1->text().toInt());
    a.setAdresse(ui->adresse_a1->text());
    a.setGsm(ui->gsm_a1->text());
    a.setEmail(email);

    bool success = a.modifier(db);
    if (success) {
        QMessageBox::information(this, "Succès", "L'adhérent a été modifié avec succès !");
        chargerTableAdherents();
        clearAdherentInputs();
        ui->modifier_a1->clear();
        ui->id_a1->setEnabled(true);
    } else {
        QString err = db.lastError().text();
        qDebug() << "Erreur SQL:" << err;
        QMessageBox::critical(this, "Erreur", "Échec de la modification !\n" + err);
    }
}

void MainWindow::on_PDF_a_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Exporter la liste des adhérents en PDF"),
        "Rapport_Adherents.pdf",
        tr("Fichiers PDF (*.pdf)")
        );

    if (fileName.isEmpty())
        return;

    Adherent a;
    a.exporterPDF(db, fileName);
}

void MainWindow::on_statistics_clicked()
{
    ui->gestiondesadherent->setCurrentIndex(1);
    Adherent a;
    a.genererStatistiques(db, ui->adherent_layout);
}

void MainWindow::on_filtrer_a_clicked()
{
    QString critere = ui->trier_a1->currentText();
    Adherent a;
    a.trier(db, ui->table_aherent, critere);
}

void MainWindow::on_look_clicked()
{
    QString id = ui->look1->text();
    Adherent a;
    a.chercher(db, ui->table_aherent, id);
}

// ==========================================
// Local
// ==========================================

void MainWindow::on_AjouterLoc_clicked()
{
    QMessageBox msgBox;

    if (controlSaisie())
    {
        bool ok;
        int ID_LOCAL = ui->IdLoc->text().toInt(&ok);

        if (ok)
        {
            if (!loc->localExists(ID_LOCAL))
            {
                loc->setIdLocal( ID_LOCAL);
                loc->setNomLocal(ui-> NomLoc->text());
                loc->setEtatLocal(ui-> EtatLoc->currentText());
                loc->setTypeLocal(ui-> TypeLoc->currentText());
                bool test = loc->ADD();

                if (test){
                    msgBox.setText("Ajout réussi.");
                }
                else
                    msgBox.setText("!! L'ajout a échoué !!");
            }
        }
    }
    else
    {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                              QObject::tr("Please fill all fields.\n !!! There are EMPTY or INCORRECT fields !!!\n",
                                          "Click cancel to EXIT."), QMessageBox::Cancel);
    }

    ui->tableLoc->setModel(loc->GETALL());

    msgBox.exec();
}

bool MainWindow::controlSaisie()
{
    QString id = ui->IdLoc->text();
    QString nom = ui->NomLoc->text();
    QString etat = ui->EtatLoc->currentText();
    QString type = ui->TypeLoc->currentText();


    if (id.isEmpty() || nom.isEmpty() || etat.isEmpty() || type.isEmpty()) {
        return false;
    }
    QRegularExpression regexId("^[0-9]+$");
    if (!regexId.match(id).hasMatch()) {
        QMessageBox::warning(this, "Erreur de saisie", "L'ID doit contenir uniquement des chiffres.");
        return false;
    }

    QRegularExpression regexNom("^[A-Za-zÀ-ÖØ-öø-ÿ]+$");
    if (!regexNom.match(nom).hasMatch()) {
        QMessageBox::warning(this, "Erreur de saisie", "Le nom doit contenir uniquement des lettres.");
        return false;
    }

    return true;
}

void MainWindow::on_tableLoc_activated(const QModelIndex &index)
{
    Q_UNUSED(index);
    ui->tableLoc->setModel(loc->GETALL());
}

void MainWindow::on_ModifLoc_clicked()
{
    QMessageBox msgBox;
    QString notificationMessage = "";
    QString bgColor = "#4caf50";
    bool ok;
    int ID_LOCAL = ui->IdLoc->text().toInt(&ok);
    if (!ok) {
        msgBox.setText("Veuillez entrer un ID de local valide.");
        msgBox.exec();
        return;
    }

    if (!loc->localExists(ID_LOCAL)) {
        msgBox.setText("Le local avec l'ID " + QString::number(ID_LOCAL) + " n'existe pas.\nImpossible de modifier.");
        msgBox.exec();
        return;
    }


    QString ancienEtat = loc->getEtatLocal(ID_LOCAL);


    QString nouveauNom = ui->NomLoc->text();
    QString nouveauType = ui->TypeLoc->currentText();
    QString nouvelEtat = ui->EtatLoc->currentText().toUpper();


    loc->setIdLocal(ID_LOCAL);
    if (!nouveauNom.isEmpty()) loc->setNomLocal(nouveauNom);
    if (!nouvelEtat.isEmpty()) loc->setEtatLocal(nouvelEtat);
    if (!nouveauType.isEmpty()) loc->setTypeLocal(nouveauType);

    bool test = loc->UPDATE();

    if (test)
    {
        msgBox.setText("Modification réussie.");

        if (!nouvelEtat.isEmpty() && ancienEtat.toUpper() != nouvelEtat) {
            QString nomLocal = loc->getNomLocal(ID_LOCAL);
            notificationMessage = "Le local '" + nomLocal + "' (ID: " + QString::number(ID_LOCAL) + ") est maintenant ";

            if (nouvelEtat == "DISPONIBLE") {
                notificationMessage += "**DISPONIBLE**.";
                bgColor = "#4caf50";
            } else if (nouvelEtat == "EN MAINTENANCE") {
                notificationMessage += "**EN MAINTENANCE**.";
                bgColor = "#ff9800";
            } else if (nouvelEtat == "Occupée") {
                notificationMessage += "**Occupée**.";
                bgColor = "#f44336";
            }
        }
    }
    else
    {
        msgBox.setText("!! La modification a échoué !!");
    }

    msgBox.exec();


    if (!notificationMessage.isEmpty()) {
        displayInternalNotification(notificationMessage, bgColor);
    }


    ui->tableLoc->setModel(loc->GETALL());
}

void MainWindow::on_SuppLoc_clicked()
{
    QString idText = ui->Idsupp->text();

    if (idText.isEmpty()) {
        QMessageBox::warning(this, "Champs vide", "Veuillez entrer un ID de local à supprimer.");
        return;
    }

    bool ok = false;
    int ID_LOCAL = idText.toInt(&ok);
    if (!ok || ID_LOCAL <= 0) {
        QMessageBox::warning(this, "ID invalide", "Veuillez entrer un ID numérique valide.");
        ui->Idsupp->clear();
        return;
    }

    if (!loc->localExists(ID_LOCAL)) {
        QMessageBox::warning(this, "Local non trouvé",
                             "Local avec l'ID " + QString::number(ID_LOCAL) + " n'existe pas.\nImpossible de supprimer.");
        ui->Idsupp->clear();
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation de Suppression",
                                  "Êtes-vous sûr de vouloir supprimer le local ID: " + idText + " ? Les données seront archivées.",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    bool archive_ok = loc->ARCHIVE_LOCAL(ID_LOCAL);
    if (!archive_ok) {
        QMessageBox::critical(this, "Échec d'Archivage", "L'archivage a échoué. Suppression annulée.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        QMessageBox::critical(this, "Erreur SQL", "Impossible de démarrer la transaction : " + db.lastError().text());
        return;
    }

    bool delete_ok = loc->DELETEE(ID_LOCAL);
    if (delete_ok) {
        if (!db.commit()) {
            QMessageBox::critical(this, "Erreur SQL", "La suppression n'a pas pu être commitée : " + db.lastError().text());
            return;
        }

        QMessageBox::information(this, "Succès", "Suppression réussie. Local archivé.");
        ui->tableLoc->setModel(loc->GETALL());
    } else {
        db.rollback();
        QMessageBox::critical(this, "Échec", "La suppression a échoué malgré l'archivage.");
    }

    ui->Idsupp->clear();
}

void MainWindow::on_recherche_clicked()
{
    int id = ui->rr->text().toInt();

    if (id <= 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez entrer un ID valide !");
        return;
    }

    ui->tableLoc->setModel(loc->searchById(id));
}

void MainWindow::on_ex_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Exporter PDF", "", "PDF (*.pdf)");
    if(filePath.isEmpty()) return;

    QPdfWriter pdf(filePath);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setResolution(300);

    QPainter painter(&pdf);
    painter.setFont(QFont("Arial", 12));

    QAbstractItemModel *model = ui->tableLoc->model();
    if (!model) {
        QMessageBox::warning(this, "Erreur", "Aucun modèle trouvé !");
        return;
    }

    int rowCount = model->rowCount();
    int colCount = model->columnCount();

    int y = 400;
    int cellHeight = 150;

    painter.setFont(QFont("Arial", 18, QFont::Bold));
    painter.drawText(100, 100, "Liste des Locaux");
    painter.setFont(QFont("Arial", 12));

    int x = 150;
    for(int col = 0; col < colCount; col++) {
        painter.drawText(x, y, model->headerData(col, Qt::Horizontal).toString());
        x += 400;
    }

    y += cellHeight;

    for(int row = 0; row < rowCount; row++) {

        if(y > 10000) {
            pdf.newPage();
            y = 400;
        }

        x = 150;

        for(int col = 0; col < colCount; col++) {
            QString text = model->data(model->index(row, col)).toString();
            painter.drawText(x, y, text);
            x += 400;
        }

        y += cellHeight;
    }

    painter.end();

    QMessageBox::information(this, "Exportation", "PDF exporté avec succès !");
}

void MainWindow::on_trier_clicked()
{
    QString etat = ui->tt->currentText();

    QSqlQueryModel *model = new QSqlQueryModel();

    if (etat == "Tous") {
        model->setQuery(
            "SELECT ID_LOCAL, NOM_L, ETAT_LOCAL, TYPE_LOCAL FROM LOCAL "
            "ORDER BY CASE "
            "   WHEN ETAT_LOCAL = 'Disponible' THEN 1 "
            "   WHEN ETAT_LOCAL = 'En Maintenance' THEN 2 "
            "   WHEN ETAT_LOCAL = 'Occupée' THEN 3 "
            "   ELSE 4 "
            "END, NOM_L ASC"
            );
    }
    else {
        model->setQuery(
            "SELECT ID_LOCAL, NOM_L, ETAT_LOCAL, TYPE_LOCAL FROM LOCAL "
            "WHERE ETAT_LOCAL = '" + etat + "' "
                     "ORDER BY NOM_L ASC"
            );
    }

    model->setHeaderData(0, Qt::Horizontal, "ID_LOCAL");
    model->setHeaderData(1, Qt::Horizontal, "NOM_L");
    model->setHeaderData(2, Qt::Horizontal, "ETAT_LOCAL");
    model->setHeaderData(3, Qt::Horizontal, "TYPE_LOCAL");

    ui->tableLoc->setModel(model);
}

void MainWindow::on_stat_clicked()
{
    afficherStatistiques();
}

void MainWindow::on_ArchiveViewer_clicked()
{
    QFile file("archive_locaux.txt");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Erreur d'Archive",
                              "Impossible d'ouvrir le fichier d'archive 'archive_locaux.txt'.");
        return;
    }

    QWidget *archiveWindow = new QWidget(this, Qt::Window);
    archiveWindow->setWindowTitle("Historique des Locaux Supprimés");
    archiveWindow->setMinimumSize(700, 400);

    QVBoxLayout *layout = new QVBoxLayout(archiveWindow);

    QLabel *title = new QLabel("Liste des locaux archivés. Double-cliquez pour voir les détails complets :");
    title->setStyleSheet("font-weight: bold; font-size: 14px; margin-bottom: 10px;");
    layout->addWidget(title, 0, Qt::AlignCenter);

    QListWidget *listWidget = new QListWidget(archiveWindow);
    layout->addWidget(listWidget);

    QTextStream in(&file);

    while (!in.atEnd())
    {
        QString fullLine = in.readLine().trimmed();
        if (fullLine.isEmpty()) continue;

        QString summaryLine = fullLine;
        int endOfSummary = fullLine.indexOf("| ETAT:");
        if (endOfSummary != -1) {
            summaryLine = fullLine.left(endOfSummary);
        }

        QListWidgetItem *item = new QListWidgetItem(summaryLine, listWidget);

        item->setData(Qt::UserRole, fullLine);
    }
    file.close();

    connect(listWidget, &QListWidget::itemActivated, this, &MainWindow::on_archiveItem_selected);

    archiveWindow->show();
}

void MainWindow::on_archiveItem_selected(QListWidgetItem *item)
{
    QString fullLine = item->data(Qt::UserRole).toString();

    if (fullLine.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Données d'archive introuvables pour cet élément.");
        return;
    }

    auto extract = [&](QString start, QString end = QString()) {
        int s = fullLine.indexOf(start);
        if (s == -1) return QString();
        s += start.length();

        int e = (end.isEmpty()) ? fullLine.length() : fullLine.indexOf(end, s);
        if (e == -1) e = fullLine.length();

        return fullLine.mid(s, e - s).trimmed();
    };

    QString id_str = extract("ID:", "| NOM:");
    QString NOM_LOCAL = extract("NOM:", "| ETAT:");
    QString ETAT_LOCAL = extract("ETAT:", "| TYPE:");
    QString TYPE_LOCAL = extract("TYPE:");

    int ID_LOCAL = id_str.toInt();

    QString details = QString(
                          "Voulez-vous réajouter ce local ?\n\n"
                          "ID : %1\nNom : %2\nÉtat : %3\nType : %4")
                          .arg(id_str, NOM_LOCAL, ETAT_LOCAL, TYPE_LOCAL);

    if (QMessageBox::question(this, "Restaurer", details) != QMessageBox::Yes)
        return;

    bool ok = true;

    while (loc->localExists(ID_LOCAL))
    {
        int newID = QInputDialog::getInt(this, "ID déjà utilisé",
                                         QString("L'ID %1 est déjà utilisé.\nEntrez un ID unique :").arg(ID_LOCAL),
                                         ID_LOCAL, 1, 999999, 1, &ok);
        if (!ok) return;
        ID_LOCAL = newID;
    }

    loc->setIdLocal(ID_LOCAL);
    loc->setNomLocal(NOM_LOCAL);
    loc->setEtatLocal(ETAT_LOCAL);
    loc->setTypeLocal(TYPE_LOCAL);

    if (loc->ADD())
    {
        QMessageBox::information(this, "Succès", "Local restauré !");
        displayInternalNotification("Local restauré !");

        ui->tableLoc->setModel(loc->GETALL());

        QListWidget *list = item->listWidget();
        int row = list->row(item);
        delete list->takeItem(row);

    }
    else
    {
        QMessageBox::critical(this, "Erreur", "Échec du réajout.");
    }
}

void MainWindow::on_btnArchiveViewer_clicked()
{
    on_ArchiveViewer_clicked();
}

void MainWindow::onEtatChanged(const QString &newEtat)
{
    QString message = "L'état du local a été modifié : " + newEtat;
    showNotification(message);
}

void MainWindow::on_ASC_clicked()
{
    QString currentCriteria = ui->tt->currentText();
    qDebug() << "Critère de tri : " << currentCriteria;

    if(currentCriteria != "Choisir") {
        QSqlQueryModel* modeleTriee = loc->trie("ASC", currentCriteria);
        qDebug() << "Nombre de lignes dans le modèle trié : " << modeleTriee->rowCount();
        ui->tableLoc->setModel(modeleTriee);
    } else {
        QSqlQueryModel* modeleNonTrie = loc->GETALL();
        qDebug() << "Nombre de lignes dans le modèle non trié : " << modeleNonTrie->rowCount();
        ui->tableLoc->setModel(modeleNonTrie);
    }
}

void MainWindow::on_DESC_clicked()
{
    QString currentCriteria = ui->tt->currentText();
    qDebug() << "Critère de tri : " << currentCriteria;

    if(currentCriteria != "Choisir") {
        QSqlQueryModel* modeleTriee = loc->trie("DESC", currentCriteria);
        qDebug() << "Nombre de lignes dans le modèle trié : " << modeleTriee->rowCount();
        ui->tableLoc->setModel(modeleTriee);
    } else {
        QSqlQueryModel* modeleNonTrie = loc->GETALL();
        qDebug() << "Nombre de lignes dans le modèle non trié : " << modeleNonTrie->rowCount();
        ui->tableLoc->setModel(modeleNonTrie);
    }
}

// ==========================================
// Private Helper Methods from Main Branch
// ==========================================

void MainWindow::chargerActivites()
{
    ui->tabA->clearContents();
    ui->tabA->setRowCount(0);

    Activite ac;
    QVector<QStringList> data = ac.getListeActivites(db);

    for (int row = 0; row < data.size(); row++) {
        ui->tabA->insertRow(row);

        for (int col = 0; col < data[row].size(); col++) {
            QTableWidgetItem *item = new QTableWidgetItem(data[row][col]);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            ui->tabA->setItem(row, col, item);
        }
    }
}

Activite MainWindow::getActiviteFromInputs()
{
    Activite a;
    a.setId(ui->IDA1->text().toInt());
    a.setNom(ui->nomA1->text());
    a.setType(ui->typeA1->currentText());
    a.setDuree(ui->dureeA1->time().toString("HH:mm"));
    a.setHoraire(ui->horaireA1->dateTime());

    a.setCoach(ui->coachA1->currentText().toInt());


    a.setLocal_A(ui->local_AA1->text().toInt());
    a.setNbp(ui->npA1->value());
    a.setStatus(ui->statusA1->currentText());
    return a;
}

void MainWindow::clearAjouterAInputs()
{
    ui->IDA1->clear();
    ui->nomA1->clear();
    ui->typeA1->setCurrentIndex(0);
    ui->dureeA1->setTime(QTime(0, 0));
    ui->horaireA1->setDateTime(QDateTime::currentDateTime());
    ui->coachA1->clear();
    ui->local_AA1->clear();
    ui->npA1->setValue(0);
    ui->statusA1->setCurrentIndex(0);
}

void MainWindow::remplirModifierA(const Activite &a)
{
    ui->idm->setText(QString::number(a.getId()));
    ui->nomm->setText(a.getNom());
    ui->typem->setCurrentText(a.getType());
    QDateTime h = a.getHoraire();
    h.setTimeSpec(Qt::UTC);
    h = h.toLocalTime();
    ui->horairem->setDateTime(h);

    ui->dureem->setTime(QTime::fromString(a.getDuree(), "HH:mm"));
    ui->coachm->setText(QString::number(a.getCoach()));
    ui->local_Am->setText(QString::number(a.getlocal_A()));
    ui->statusm->setCurrentText(a.getStatus());
    ui->npam->setValue(a.getNbp());
}

void MainWindow::setAfficherAEditable(bool editable)
{
    ui->ida->setEnabled(editable);
    ui->noma->setEnabled(editable);
    ui->typea->setEnabled(editable);
    ui->horairea->setEnabled(editable);
    ui->dureea->setEnabled(editable);
    ui->coacha->setEnabled(editable);
    ui->local_Aa->setEnabled(editable);
    ui->statuta->setEnabled(editable);
    ui->npa->setEnabled(editable);
}

void MainWindow::remplirAfficherA(const Activite &a)
{
    ui->ida->setText(QString::number(a.getId()));
    ui->noma->setText(a.getNom());
    ui->typea->setCurrentText(a.getType());
    QDateTime h = a.getHoraire();
    h.setTimeSpec(Qt::UTC);
    h = h.toLocalTime();
    ui->horairea->setDateTime(h);

    ui->dureea->setTime(QTime::fromString(a.getDuree(), "HH:mm"));
    ui->coacha->setText(QString::number(a.getCoach()));
    ui->local_Aa->setText(QString::number(a.getlocal_A()));
    ui->statuta->setCurrentText(a.getStatus());
    ui->npa->setValue(a.getNbp());
}

void MainWindow::colorerJoursAvecActivites()
{
    highlighterJoursActivites();
}

Adherent MainWindow::getAdherentFromInputs()
{
    Adherent a;
    a.setId(ui->id_a1->text().toInt());
    a.setNom(ui->nom_a1->text());
    a.setPrenom(ui->prenom_a1->text());
    a.setGenre(ui->genre_a1->isChecked() ? "Homme" : "Femme");
    a.setDateNaissance(ui->date_a1->date().toString("MM/dd/yyyy"));
    a.setCin(ui->cin_a1->text().toInt());
    a.setAdresse(ui->adresse_a1->text());
    a.setGsm(ui->gsm_a1->text());
    a.setEmail(ui->mail_a1->text());
    return a;
}

void MainWindow::clearAdherentInputs()
{
    ui->id_a1->clear();
    ui->nom_a1->clear();
    ui->prenom_a1->clear();
    ui->genre_a1->setAutoExclusive(false); ui->genre_a1->setChecked(false); ui->genre_a1->setAutoExclusive(true);
    ui->genre_a2->setAutoExclusive(false); ui->genre_a2->setChecked(false); ui->genre_a2->setAutoExclusive(true);
    ui->date_a1->clear();
    ui->cin_a1->clear();
    ui->adresse_a1->clear();
    ui->gsm_a1->clear();
    ui->mail_a1->clear();
}

void MainWindow::chargerTableAdherents()
{
    ui->table_aherent->clearContents();
    ui->table_aherent->setRowCount(0);

    QSqlQuery query(db);
    if(query.exec("SELECT * FROM ADHERENT ORDER BY ID_ADHERENT")) {
        int row = 0;
        while (query.next()) {
            ui->table_aherent->insertRow(row);
            ui->table_aherent->setItem(row, 0, new QTableWidgetItem(query.value("ID_ADHERENT").toString()));
            ui->table_aherent->setItem(row, 1, new QTableWidgetItem(query.value("NOM_ADHERENT").toString()));
            ui->table_aherent->setItem(row, 2, new QTableWidgetItem(query.value("PRENOM_ADHERENT").toString()));
            ui->table_aherent->setItem(row, 3, new QTableWidgetItem(query.value("DATE_DE_NAISSANCE_ADHERENT").toString()));
            ui->table_aherent->setItem(row, 4, new QTableWidgetItem(query.value("GENRE_ADHERENT").toString()));
            ui->table_aherent->setItem(row, 5, new QTableWidgetItem(query.value("CIN_ADHERENT").toString()));
            ui->table_aherent->setItem(row, 6, new QTableWidgetItem(query.value("ADRESSE_ADHERENT").toString()));
            ui->table_aherent->setItem(row, 7, new QTableWidgetItem(query.value("GSM_ADHERENT").toString()));
            ui->table_aherent->setItem(row, 8, new QTableWidgetItem(query.value("EMAIL_ADHERENT").toString()));
            row++;
        }
    } else {
        QMessageBox::warning(this, "Erreur", "Impossible de charger les adhérents : " + query.lastError().text());
    }
}

void MainWindow::afficherStatistiques()
{
    QSqlQuery query;
    query.prepare("SELECT ETAT_LOCAL, COUNT(*) FROM LOCAL GROUP BY ETAT_LOCAL");

    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur SQL", "Impossible de charger les statistiques: " + query.lastError().text());
        return;
    }

    QMap<QString, int> statsData;
    while (query.next()) {
        QString etat = query.value(0).toString().trimmed().toLower();
        int count = query.value(1).toInt();
        statsData[etat] = count;
    }

    if (statsData.isEmpty()) {
        QMessageBox::information(this, "Statistiques", "Aucune donnée de local trouvée pour générer le graphique.");
        return;
    }


    QWidget *statWindow = new QWidget(this, Qt::Window);
    statWindow->setWindowTitle("Statistiques des Locaux par État");
    statWindow->setMinimumSize(500, 400);

    class PieChartWidget : public QWidget {
    public:
        PieChartWidget(const QMap<QString, int> &data, QWidget *parent = nullptr)
            : QWidget(parent), statsData(data) {}

    protected:
        void paintEvent(QPaintEvent *) override {
            if (statsData.isEmpty()) return;

            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing);

            QRectF pieRect(10, 10, width() - 120, height() - 20);
            int startAngle = 0;
            QVector<QColor> colors = {
                QColor("#4caf50"), QColor("#2196f3"), QColor("#ff9800"), QColor("#f44336"),
                QColor("#9c27b0"), QColor("#00bcd4"), QColor("#ffc107"), QColor("#795548")
            };

            int colorIndex = 0;
            int total = 0;
            for (auto v : statsData) total += v;

            for (auto it = statsData.constBegin(); it != statsData.constEnd(); ++it)
            {
                if (total == 0) continue;
                qreal percentage = (qreal)it.value() * 360.0 / total;
                painter.setBrush(colors[colorIndex % colors.size()]);
                painter.drawPie(pieRect, startAngle * 16, percentage * 16);
                startAngle += percentage;
                colorIndex++;
            }

            colorIndex = 0;
            int yOffset = 20;
            for (auto it = statsData.constBegin(); it != statsData.constEnd(); ++it)
            {
                QString label = QString("%1: %2 (%3%)")
                .arg(it.key())
                    .arg(it.value())
                    .arg((qreal)it.value() * 100.0 / total, 0, 'f', 1);

                painter.setPen(Qt::white);
                painter.setBrush(colors[colorIndex % colors.size()]);
                painter.drawRect(width() - 150, yOffset, 10, 10);
                painter.drawText(width() - 135, yOffset + 9, label);

                yOffset += 20;
                colorIndex++;
            }
        }
    private:
        QMap<QString, int> statsData;
    };

    PieChartWidget *chartWidget = new PieChartWidget(statsData);
    chartWidget->setMinimumSize(450, 350);


    QVBoxLayout *layout = new QVBoxLayout(statWindow);
    QLabel *title = new QLabel("Répartition des locaux par État (Diagramme Circulaire)");
    title->setStyleSheet("font-weight: bold; font-size: 16px; margin-bottom: 10px;");

    layout->addWidget(title, 0, Qt::AlignCenter);
    layout->addWidget(chartWidget);

    statWindow->show();
}

void MainWindow::displayInternalNotification(const QString &message, const QString &bgColor)
{
    QLabel *toast = new QLabel(message, this);
    toast->setStyleSheet(
        QString("background-color: %1; color: white; padding: 15px; border-radius: 10px; "
                "font-weight: bold; font-size: 16px; text-align: center;").arg(bgColor)
        );
    toast->setAlignment(Qt::AlignCenter);
    toast->setWindowFlag(Qt::FramelessWindowHint);
    toast->setAttribute(Qt::WA_TranslucentBackground);
    toast->setAttribute(Qt::WA_ShowWithoutActivating);

    toast->adjustSize();
    int x = (width() - toast->width()) / 2;
    int y = height() - toast->height() - 50;
    toast->move(x, y);
    toast->show();

    QPropertyAnimation *fadeIn = new QPropertyAnimation(toast, "windowOpacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);


    QTimer::singleShot(5000, this, [toast]() {
        QPropertyAnimation *fadeOut = new QPropertyAnimation(toast, "windowOpacity");
        fadeOut->setDuration(400);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        QObject::connect(fadeOut, &QPropertyAnimation::finished, toast, &QObject::deleteLater);
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void MainWindow::showNotification(const QString &message)
{
    QMessageBox::information(this, "Notification", message);
}

void MainWindow::saveAllStatesToFile()
{
    QFile file("last_states.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);

        QSqlQuery query("SELECT ID_LOCAL, ETAT_LOCAL FROM LOCAL");
        while (query.next())
        {
            int id = query.value(0).toInt();
            QString etat = query.value(1).toString();
            out << id << "|" << etat << "\n";
        }
        file.close();
    }
}

QMap<int, QString> MainWindow::readLastStatesFromFile()
{
    QMap<int, QString> lastStates;
    QFile file("last_states.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) continue;
            QStringList parts = line.split("|");
            if (parts.size() == 2)
            {
                int id = parts[0].toInt();
                QString etat = parts[1];
                lastStates[id] = etat;
            }
        }
        file.close();
    }
    return lastStates;
}

void MainWindow::checkStatesOnStartup()
{
    QMap<int, QString> lastStates = readLastStatesFromFile();

    QSqlQuery query("SELECT ID_LOCAL, ETAT_LOCAL FROM LOCAL");
    while (query.next())
    {
        int id = query.value(0).toInt();
        QString currentEtat = query.value(1).toString();

        if (lastStates.contains(id) && lastStates[id] != currentEtat)
        {
            QString message = QString("Le local ID %1 a changé d'état : %2").arg(id).arg(currentEtat);
            displayInternalNotification(message);
        }
    }

    saveAllStatesToFile();
}

/* =========================================================================
 *  END OF STUB IMPLEMENTATIONS
 *  NOTE: Copy the actual implementations from mainwindow.cpp in the root
 *  directory to complete these functions.
 * ========================================================================= */
