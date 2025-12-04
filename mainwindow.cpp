#include "mainwindow.h"
#include "connexion.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QTime>
#include <QDebug>
#include"adherent.h"


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

#include <QSqlQuery>
#define file_tx "^[A-Za-z ]+$"
#define file_ex "^[0-9]+$"
#include <QPdfWriter>
#include <QPainter>
#include <QTextDocument>
#include <QFileDialog>

#include <QVBoxLayout>
#include <QPainter>
#include <QMap>
#include <QLabel>
#include <QTextEdit>


#include <QProcess>
#include <QFile>
#include <QMessageBox>
#include <QDir>

#include <QPropertyAnimation>
#include <QTimer>
#include <QPropertyAnimation>
#include <QTimer>
#include <QLabel>
#include <QDebug>







MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connexion = new Connexion();

    if (!connexion->ouvrir()) return;
    db = connexion->getDB();
    highlighterJoursActivites();

    chargerTableAdherents();
    chargerActivites();
    afficherToutesActivitesDansTabA2();
    chargerSuggestionCoach();

    ui->tabA->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tabA->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tabA_2->setColumnCount(9);
    ui->tabA_2->setHorizontalHeaderLabels({"ID", "Nom", "Type", "Horaire", "Durée", "Status", "NbP", "Coach","Local_A"});

    // 🔹 Contrôle pour les noms et prénoms (lettres uniquement)
    QRegularExpression rxNom("^[A-Za-z ]*$");  // lettres et espaces uniquement
    QValidator *validatorNom = new QRegularExpressionValidator(rxNom, this);
    ui->nom_a1->setValidator(validatorNom);
    ui->prenom_a1->setValidator(validatorNom);

    // 🔹 Contrôle pour CIN et GSM (chiffres uniquement, max 8)
    QIntValidator *validatorChiffre = new QIntValidator(0, 99999999, this);
    ui->cin_a1->setValidator(validatorChiffre);
    ui->gsm_a1->setValidator(validatorChiffre);
    ui->cin_a1->setMaxLength(8);
    ui->gsm_a1->setMaxLength(8);




    connect(ui->Bcalendrier, &QPushButton::clicked, this, &MainWindow::afficherCalendrier);
    connect(ui->calendarWidget, &QCalendarWidget::clicked, this, &MainWindow::afficherActivitesDuJour);



    connect (ui->stat_act, &QPushButton::clicked, [this]() {
        ui->tabActiviter->setCurrentIndex(4);
        afficher_stat_act();
    });
    connect(ui->Bexporter, &QPushButton::clicked, this, &MainWindow::on_Bexporter_clicked);

    ui->IDA1->setValidator(new QIntValidator(0, 99999999, this));
    connect(ui->btnArchiveViewer, &QPushButton::clicked, this, &MainWindow::on_ArchiveViewer_clicked);
    connect(ui->EtatLoc, &QComboBox::currentTextChanged, this, &MainWindow::onEtatChanged);
    checkStatesOnStartup();

    QRegularExpression regExp("[A-Za-z]+");
    ui->nomA1->setValidator(new QRegularExpressionValidator(regExp, this));



}

MainWindow::~MainWindow()
{
    delete connexion;
    delete ui;
}


void MainWindow::on_labelLien_linkActivated(const QString &link) { Q_UNUSED(link); ui->stackedWidget->setCurrentIndex(1); }
void MainWindow::on_annuler_clicked() { ui->stackedWidget->setCurrentIndex(0); }
void MainWindow::on_valider_clicked() { ui->stackedWidget->setCurrentIndex(2); }
void MainWindow::on_gp_clicked() { ui->stackedWidget->setCurrentIndex(3); }
void MainWindow::on_retour_clicked() { ui->stackedWidget->setCurrentIndex(2); }
void MainWindow::on_pushButton_3_clicked() { ui->stackedWidget->setCurrentIndex(4); }
void MainWindow::on_modifier_a_2_clicked() { ui->stackedWidget->setCurrentIndex(2); }
void MainWindow::on_label_20_linkActivated(const QString &link) { Q_UNUSED(link); }
void MainWindow::on_B_A_retour_clicked() {ui->stackedWidget->setCurrentIndex(2); }
void MainWindow::on_pushButton_4_clicked() { ui->stackedWidget->setCurrentIndex(5); }
void MainWindow::on_pushButton_5_clicked() { ui->stackedWidget->setCurrentIndex(6); }
void MainWindow::on_pushButton_6_clicked() { ui->stackedWidget->setCurrentIndex(7); }








void MainWindow::chargerSuggestionCoach()
{
    Activite a;
    a.chargerSuggestionCoach(ui->coachA1, db);
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



void MainWindow::on_bannulerA_clicked() { clearAjouterAInputs(); }


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


void MainWindow::on_annuler_modif_A_clicked() { ui->tabActiviter->setCurrentIndex(0); }




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

void MainWindow::on_ok_aff_A_clicked()
{
    ui->tabActiviter->setCurrentIndex(0);
    setAfficherAEditable(true);
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

void MainWindow::afficherCalendrier()
{
    ui->tabActiviter->setCurrentIndex(3);
}

void MainWindow::on_B_A_retour_2_clicked()
{
    ui->tabActiviter->setCurrentIndex(0); // 0 = ajouterA
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
    series->setHoleSize(0.3); // donut style اختياري
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
        int idAdherent = a.getId(); // récupère l'ID de l'adhérent ajouté
        int idActivite = ui->cbxActivite->text().toInt(); // récupère l'ID saisi dans le LineEdit

        // Vérifier que l'activité existe
        QSqlQuery check(db);
        check.prepare("SELECT COUNT(*) FROM ACTIVITE WHERE ID_ACTIVITE = :id");
        check.bindValue(":id", idActivite);
        if (check.exec() && check.next() && check.value(0).toInt() > 0) {
            a.inscrireAdherent(idAdherent, idActivite); // inscription

            // 🔹 Envoi automatique du SMS si l'activité est annulée
            QSqlQuery q(db);
            q.prepare("SELECT STATUS, NOM FROM ACTIVITE WHERE ID_ACTIVITE = :id");
            q.bindValue(":id", idActivite);
            if(q.exec() && q.next()) {
                QString statut = q.value("STATUS").toString();
                QString nomActivite = q.value("NOM").toString();
                if(statut == "Annulée") {
                    a.notifierAnnulation(db, idActivite, nomActivite); // envoie le SMS
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



void MainWindow::on_modifier_a_3_clicked()  // Bouton "Modifier"
{
    int id = ui->modifier_a1->text().toInt();  // ID utilisé pour trouver la ligne à modifier

    if (id == 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez saisir un ID valide pour la modification !");
        return;
    }

    // Vérification des champs obligatoires
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

    // Vérification du CIN
    if (ui->cin_a1->text().length() != 8) {
        QMessageBox::warning(this, "Erreur de saisie", "Le CIN doit contenir exactement 8 chiffres.");
        return;
    }

    // Vérification du GSM
    if (ui->gsm_a1->text().length() != 8) {
        QMessageBox::warning(this, "Erreur de saisie", "Le numéro GSM doit contenir 8 chiffres.");
        return;
    }

    // Vérification de l’email
    QString email = ui->mail_a1->text();
    QRegularExpression regexEmail("^[\\w\\.]+@[\\w\\.]+\\.[a-z]{2,4}$");
    if (!regexEmail.match(email).hasMatch()) {
        QMessageBox::warning(this, "Email invalide", "Veuillez entrer une adresse email valide !");
        return;
    }

    // Confirmation
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  "Êtes-vous sûr de vouloir modifier cet adhérent ?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No)
        return;

    // Création d’un objet Adherent sans modifier l’ID
    Adherent a;
    a.setId(id);  // ID inchangé
    a.setNom(ui->nom_a1->text());
    a.setPrenom(ui->prenom_a1->text());
    a.setGenre(ui->genre_a1->isChecked() ? "Homme" : "Femme");
    a.setDateNaissance(ui->date_a1->date().toString("MM/dd/yyyy"));
    a.setCin(ui->cin_a1->text().toInt());
    a.setAdresse(ui->adresse_a1->text());
    a.setGsm(ui->gsm_a1->text());
    a.setEmail(email);

    // 🔹 Appel de la méthode modifier
    bool success = a.modifier(db);
    if (success) {
        QMessageBox::information(this, "Succès", "L'adhérent a été modifié avec succès !");
        chargerTableAdherents();
        clearAdherentInputs();
        ui->modifier_a1->clear();
        ui->id_a1->setEnabled(true);  // réactive le champ ID si on veut refaire une recherche
    } else {
        QString err = db.lastError().text();
        qDebug() << "Erreur SQL:" << err;
        QMessageBox::critical(this, "Erreur", "Échec de la modification !\n" + err);
    }
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

void MainWindow::on_afficher_adherent_clicked()
{
    // Recharge la table des adhérents depuis la base
    chargerTableAdherents();

    QMessageBox::information(this, "Affichage", "Table des adhérents mise à jour !");
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



void MainWindow::on_B_A_retour_6_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}





void MainWindow::on_B_A_retour_5_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}
//local
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

void MainWindow::on_AjouterLoc_clicked()
{
    QMessageBox msgBox;

    if (controlSaisie())
    {
        bool ok;
        int ID_LOCAL = ui->IdLoc->text().toInt(&ok);

        if (ok)
        {
            if (!loc.localExists(ID_LOCAL))
            {
                loc.setIdLocal( ID_LOCAL);
                loc.setNomLocal(ui-> NomLoc->text());
                loc.setEtatLocal(ui-> EtatLoc->currentText());
                loc.setTypeLocal(ui-> TypeLoc->currentText());
                bool test = loc.ADD();

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
                              QObject::tr("Please fill all fields.\n !!! There are EMPTY or INCORRECT fields !!!\n"
                                          "Click cancel to EXIT."), QMessageBox::Cancel);
    }

    ui->tableLoc->setModel(loc.GETALL());

    msgBox.exec();
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

    if (!loc.localExists(ID_LOCAL)) {
        msgBox.setText("Le local avec l'ID " + QString::number(ID_LOCAL) + " n'existe pas.\nImpossible de modifier.");
        msgBox.exec();
        return;
    }


    QString ancienEtat = loc.getEtatLocal(ID_LOCAL);


    QString nouveauNom = ui->NomLoc->text();
    QString nouveauType = ui->TypeLoc->currentText();
    QString nouvelEtat = ui->EtatLoc->currentText().toUpper();


    loc.setIdLocal(ID_LOCAL);
    if (!nouveauNom.isEmpty()) loc.setNomLocal(nouveauNom);
    if (!nouvelEtat.isEmpty()) loc.setEtatLocal(nouvelEtat);
    if (!nouveauType.isEmpty()) loc.setTypeLocal(nouveauType);

    bool test = loc.UPDATE();

    if (test)
    {
        msgBox.setText("Modification réussie.");

        if (!nouvelEtat.isEmpty() && ancienEtat.toUpper() != nouvelEtat) {
            QString nomLocal = loc.getNomLocal(ID_LOCAL);
            notificationMessage = "Le local '" + nomLocal + "' (ID: " + QString::number(ID_LOCAL) + ") est maintenant ";

            if (nouvelEtat == "DISPONIBLE") {
                notificationMessage += "**DISPONIBLE**.";
                bgColor = "#4caf50"; // vert
            } else if (nouvelEtat == "EN MAINTENANCE") {
                notificationMessage += "**EN MAINTENANCE**.";
                bgColor = "#ff9800"; // orange
            } else if (nouvelEtat == "Occupée") {
                notificationMessage += "**Occupée**.";
                bgColor = "#f44336"; // rouge
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


    ui->tableLoc->setModel(loc.GETALL());
}


void MainWindow::on_SuppLoc_clicked()
{
    QString idText = ui->Idsupp->text().trimmed();
    bool ok;
    int ID_LOCAL = idText.toInt(&ok);

    if (!ok) {
        QMessageBox::warning(this, "Erreur", "Veuillez entrer un ID valide.");
        return;
    }

    if (!loc.localExists(ID_LOCAL)) {
        QMessageBox::warning(this, "Local non trouvé",
                             "Local avec l'ID " + QString::number(ID_LOCAL) + " n'existe pas.\nImpossible de supprimer.");
        ui->Idsupp->clear();
        return;
    }

    // Confirmation
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation de Suppression",
                                  "Êtes-vous sûr de vouloir supprimer le local ID: " + idText + " ? Les données seront archivées.",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    // --- Archivage ---
    bool archive_ok = loc.ARCHIVE_LOCAL(ID_LOCAL);
    if (!archive_ok) {
        QMessageBox::critical(this, "Échec d'Archivage", "L'archivage a échoué. Suppression annulée.");
        return;
    }

    // --- Suppression réelle dans la base ---
    QSqlDatabase db = QSqlDatabase::database(); // récupère la connexion par défaut
    if (!db.transaction()) {
        QMessageBox::critical(this, "Erreur SQL", "Impossible de démarrer la transaction : " + db.lastError().text());
        return;
    }

    bool delete_ok = loc.DELETEE(ID_LOCAL);
    if (delete_ok) {
        if (!db.commit()) {  // commit pour persister la suppression
            QMessageBox::critical(this, "Erreur SQL", "La suppression n'a pas pu être commitée : " + db.lastError().text());
            return;
        }

        QMessageBox::information(this, "Succès", "Suppression réussie. Local archivé.");
        ui->tableLoc->setModel(loc.GETALL());
    } else {
        db.rollback(); // annule la transaction si delete échoue
        QMessageBox::critical(this, "Échec", "La suppression a échoué malgré l'archivage.");
    }

    ui->Idsupp->clear();
}


void MainWindow::on_tableLoc_activated(const QModelIndex &index)
{
    ui->tableLoc->setModel(loc.GETALL());
}
//RECHERCHER
void MainWindow::on_recherche_clicked()
{
    int id = ui->rr->text().toInt();

    if (id <= 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez entrer un ID valide !");
        return;
    }

    ui->tableLoc->setModel(loc.searchById(id));
}
///exporter
void MainWindow::on_ex_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Exporter PDF", "", "PDF (*.pdf)");
    if(filePath.isEmpty()) return;

    QPdfWriter pdf(filePath);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setResolution(300);

    QPainter painter(&pdf);
    painter.setFont(QFont("Arial", 12));

    // Récupération du modèle de la table
    QAbstractItemModel *model = ui->tableLoc->model();
    if (!model) {
        QMessageBox::warning(this, "Erreur", "Aucun modèle trouvé !");
        return;
    }

    int rowCount = model->rowCount();
    int colCount = model->columnCount();

    // Coordonnées PDF
    int y = 400;
    int cellHeight = 150;

    // Titre PDF
    painter.setFont(QFont("Arial", 18, QFont::Bold));
    painter.drawText(100, 100, "Liste des Locaux");
    painter.setFont(QFont("Arial", 12));

    // Affichage des en-têtes
    int x = 150;
    for(int col = 0; col < colCount; col++) {
        painter.drawText(x, y, model->headerData(col, Qt::Horizontal).toString());
        x += 400; // espace entre colonnes
    }

    y += cellHeight;

    // Affichage des données du tableau
    for(int row = 0; row < rowCount; row++) {

        // Gestion des pages
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
    QMessageBox::information(this, "Succès", "PDF généré !");
}
// TRIER
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

    // Titres des colonnes
    model->setHeaderData(0, Qt::Horizontal, "ID_LOCAL");
    model->setHeaderData(1, Qt::Horizontal, "NOM_L");
    model->setHeaderData(2, Qt::Horizontal, "ETAT_LOCAL");
    model->setHeaderData(3, Qt::Horizontal, "TYPE_LOCAL");

    ui->tableLoc->setModel(model);
}
/// statistique
// Dans mainwindow.cpp

void MainWindow::on_stat_clicked()
{
    // Appelle la fonction qui va récupérer les données et afficher le graphique
    afficherStatistiques();
}


//////////////
class PieChartWidget : public QWidget
{
public:
    QMap<QString, int> data;
    int total = 0;

    PieChartWidget(QMap<QString, int> chartData, QWidget *parent = nullptr) : QWidget(parent)
    {
        data = chartData;
        for (int value : data) {
            total += value;
        }
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        if (total == 0) return;

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int size = qMin(width(), height()) / 2 - 20;
        QRect rect(width() / 2 - size, height() / 2 - size, 2 * size, 2 * size);

        // Palette de couleurs simple
        QList<QColor> colors = {Qt::blue, Qt::red, Qt::green, Qt::yellow, Qt::cyan};
        qreal startAngle = 0;
        int colorIndex = 0;

        // Dessin du diagramme circulaire
        for (auto it = data.constBegin(); it != data.constEnd(); ++it)
        {
            qreal angle = (qreal)it.value() / total * 360.0;

            painter.setBrush(colors[colorIndex % colors.size()]);
            painter.drawPie(rect, (int)(startAngle * 16), (int)(angle * 16));

            startAngle += angle;
            colorIndex++;
        }

        // Légende (simple)
        colorIndex = 0;
        int yOffset = 20;
        for (auto it = data.constBegin(); it != data.constEnd(); ++it)
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
};


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
        // Nettoyer la clé avant de la stocker
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

    PieChartWidget *chartWidget = new PieChartWidget(statsData);
    chartWidget->setMinimumSize(450, 350);


    QVBoxLayout *layout = new QVBoxLayout(statWindow);
    QLabel *title = new QLabel("Répartition des locaux par État (Diagramme Circulaire)");
    title->setStyleSheet("font-weight: bold; font-size: 16px; margin-bottom: 10px;");

    layout->addWidget(title, 0, Qt::AlignCenter);
    layout->addWidget(chartWidget);

    statWindow->show();
}



//notification
void MainWindow::showNotification(const QString &message)
{
    QMessageBox::information(this, "Notification", message);
}
void MainWindow::onEtatChanged(const QString &newEtat)
{
    QString message = "L'état du local a été modifié : " + newEtat;
    showNotification(message);
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

    // Position en bas de la fenêtre
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
// Dans mainwindow.cpp

void MainWindow::on_ArchiveViewer_clicked()
{
    QFile file("archive_locaux.txt");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Erreur d'Archive",
                              "Impossible d'ouvrir le fichier d'archive 'archive_locaux.txt'.");
        return;
    }

    // --- Configuration de la fenêtre d'Archive ---
    QWidget *archiveWindow = new QWidget(this, Qt::Window);
    archiveWindow->setWindowTitle("Historique des Locaux Supprimés");
    archiveWindow->setMinimumSize(700, 400);

    QVBoxLayout *layout = new QVBoxLayout(archiveWindow);

    QLabel *title = new QLabel("Liste des locaux archivés. Double-cliquez pour voir les détails complets :");
    title->setStyleSheet("font-weight: bold; font-size: 14px; margin-bottom: 10px;");
    layout->addWidget(title, 0, Qt::AlignCenter);

    // 🚨 QListWidget : Composant interactif pour afficher la liste
    QListWidget *listWidget = new QListWidget(archiveWindow);
    layout->addWidget(listWidget);

    QTextStream in(&file);

    // --- Remplissage de la Liste ---
    while (!in.atEnd())
    {
        QString fullLine = in.readLine().trimmed();
        if (fullLine.isEmpty()) continue;

        // Création du texte résumé pour la liste (seulement Date, ID et Nom)
        QString summaryLine = fullLine;
        // Coupe la chaîne avant | ETAT: pour garder la date, l'ID et le Nom
        int endOfSummary = fullLine.indexOf("| ETAT:");
        if (endOfSummary != -1) {
            summaryLine = fullLine.left(endOfSummary);
        }

        QListWidgetItem *item = new QListWidgetItem(summaryLine, listWidget);

        // 🚨 Stockage de la ligne complète dans la propriété 'UserRole' de l'élément de liste.
        // C'est ce contenu qui sera affiché en détail plus tard.
        item->setData(Qt::UserRole, fullLine);
    }
    file.close();

    // 🚨 Connexion du signal double-clic (ou activation) au slot d'affichage des détails
    // Nous utilisons un 'lambda' pour garantir la connexion même si le QListWidget est créé dynamiquement
    connect(listWidget, &QListWidget::itemActivated, this, &MainWindow::on_archiveItem_selected);

    archiveWindow->show();
}
// Dans mainwindow.cpp

// Dans mainwindow.cpp

// DANS mainwindow.cpp

void MainWindow::on_archiveItem_selected(QListWidgetItem *item)
{
    QString fullLine = item->data(Qt::UserRole).toString();

    if (fullLine.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Données d'archive introuvables pour cet élément.");
        return;
    }

    // -------- Parsing sécurisé --------
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

    // -------- Confirmation --------
    QString details = QString(
                          "Voulez-vous réajouter ce local ?\n\n"
                          "ID : %1\nNom : %2\nÉtat : %3\nType : %4")
                          .arg(id_str, NOM_LOCAL, ETAT_LOCAL, TYPE_LOCAL);

    if (QMessageBox::question(this, "Restaurer", details) != QMessageBox::Yes)
        return;

    // -------- Gérer conflit d'ID --------
    bool ok = true;
    int original = ID_LOCAL;

    while (loc.localExists(ID_LOCAL))
    {
        int newID = QInputDialog::getInt(this, "ID déjà utilisé",
                                         QString("L'ID %1 est déjà utilisé.\nEntrez un ID unique :").arg(ID_LOCAL),
                                         ID_LOCAL, 1, 999999, 1, &ok);
        if (!ok) return;
        ID_LOCAL = newID;
    }

    // -------- Préparer objet --------
    loc.setIdLocal(ID_LOCAL);
    loc.setNomLocal(NOM_LOCAL);
    loc.setEtatLocal(ETAT_LOCAL);
    loc.setTypeLocal(TYPE_LOCAL);

    // -------- Réinsertion dans la base --------
    if (loc.ADD())
    {
        QMessageBox::information(this, "Succès", "Local restauré !");
        displayInternalNotification("Local restauré !");

        // -------- Rafraîchir la table --------
        ui->tableLoc->setModel(loc.GETALL());

        // -------- Supprimer la ligne du QListWidget --------
        QListWidget *list = item->listWidget();
        int row = list->row(item);
        delete list->takeItem(row);  // retire la ligne affichée

    }
    else
    {
        QMessageBox::critical(this, "Erreur", "Échec du réajout.");
    }
}

void MainWindow::on_btnArchiveViewer_clicked()
{

}
//notif
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

    // Mettre à jour le fichier pour le prochain démarrage
    saveAllStatesToFile();
}


//trie
void MainWindow::on_ASC_clicked()
{
    QString currentCriteria = ui->tt->currentText();
    qDebug() << "Critère de tri : " << currentCriteria;

    if(currentCriteria != "Choisir") {
        QSqlQueryModel* modeleTriee = loc.trie("ASC", currentCriteria);
        qDebug() << "Nombre de lignes dans le modèle trié : " << modeleTriee->rowCount();
        ui->tableLoc->setModel(modeleTriee);
    } else {
        QSqlQueryModel* modeleNonTrie = loc.GETALL();
        qDebug() << "Nombre de lignes dans le modèle non trié : " << modeleNonTrie->rowCount();
        ui->tableLoc->setModel(modeleNonTrie);
    }
}

void MainWindow::on_DESC_clicked()
{
    QString currentCriteria = ui->tt->currentText();
    qDebug() << "Critère de tri : " << currentCriteria;

    if(currentCriteria != "Choisir") {
        QSqlQueryModel* modeleTriee = loc.trie("DESC", currentCriteria);
        qDebug() << "Nombre de lignes dans le modèle trié : " << modeleTriee->rowCount();
        ui->tableLoc->setModel(modeleTriee);
    } else {
        QSqlQueryModel* modeleNonTrie = loc.GETALL();
        qDebug() << "Nombre de lignes dans le modèle non trié : " << modeleNonTrie->rowCount();
        ui->tableLoc->setModel(modeleNonTrie);
    }
}

void MainWindow::on_bt_retour_accueil_clicked()
{
     ui->stackedWidget->setCurrentIndex(2);
}

