#include "adherent.h"
#include <QMessageBox>
#include <QSqlError>
#include <QMap>
#include <QDate>
#include <QString>
#include"smtp.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QDebug>
#include <QSqlQuery>
#include"activite.h"
#include <QPdfWriter>
#include <QPainter>
#include <QtCharts>
#include <QChartView>
#include <QPieSeries>
#include <QPieSlice>
#include <QLayout>
#include <QTableWidget>
#include <QTableWidgetItem>


Adherent::Adherent() {
}

Adherent::Adherent(int id, QString nom, QString prenom, QString genre, QString adresse, QString email, QString gsm, int cin, QString date_naissance)
{
    this->id_adherent = id;
    this->nom_adherent = nom;
    this->prenom_adherent = prenom;
    this->genre_adherent = genre;
    this->adresse_adherent = adresse;
    this->email_adherent = email;
    this->gsm_adherent = gsm;
    this->cin_adherent = cin;
    this->date_de_naissance_adherent = date_naissance;
}

bool Adherent::ajouter(QSqlDatabase &db)
{
    if (!db.isOpen())
        return false;

    QSqlQuery query(db);
    query.prepare("INSERT INTO ADHERENT (ID_ADHERENT, NOM_ADHERENT, PRENOM_ADHERENT, GENRE_ADHERENT, "
                  "ADRESSE_ADHERENT, EMAIL_ADHERENT, GSM_ADHERENT, CIN_ADHERENT, DATE_DE_NAISSANCE_ADHERENT) "
                  "VALUES (:id, :nom, :prenom, :genre, :adresse, :email, :gsm, :cin, TO_DATE(:dateN, 'MM/DD/YYYY'))");

    query.bindValue(":id", id_adherent);
    query.bindValue(":nom", nom_adherent);
    query.bindValue(":prenom", prenom_adherent);
    query.bindValue(":genre", genre_adherent);
    query.bindValue(":adresse", adresse_adherent);
    query.bindValue(":email", email_adherent);
    query.bindValue(":gsm", gsm_adherent);
    query.bindValue(":cin", cin_adherent);

    QDate date = QDate::fromString(date_de_naissance_adherent, "dd/MM/yyyy");
    QString formattedDate = date.isValid() ? date.toString("MM/dd/yyyy") : date_de_naissance_adherent;
    query.bindValue(":dateN", formattedDate);

    if (!query.exec()) {
        QMessageBox::critical(nullptr, "Erreur Ajout",
                              "Échec de l'ajout de l'adhérent : " + query.lastError().text());
        return false;
    }


    envoyerEmailConfirmation();

    return true;
}

bool Adherent::envoyerEmailConfirmation()
{
    Smtp *smtp = new Smtp("farahguirat4@gmail.com",
                          "cuis fsec pwed tkid",
                          "smtp.gmail.com",
                          465);

    QString corps = QString(
                        "🌞☀️🏖️ Bonjour %1 ! 🏄‍♂️🌴🍹\n\n"
                        "Bienvenue dans notre Summer Club AQUANOVA ! 😎🏖️🌊\n"
                        "L’équipe WAVEMAKERS est ravie de te compter parmi nous ! 🎉🌺🍦\n\n"
                        "Profite bien de l’été et de toutes nos activités ! 🐠🍹🌞\n"
                        "À très bientôt ! 🏖️🏄‍♂️🌺🎶"
                        ).arg(nom_adherent);

    smtp->sendMail("ton_email@gmail.com",
                   email_adherent,
                   "Confirmation inscription AQUANOVA",
                   corps);

    return true;
}


QMap<QString, QVariant> Adherent::afficher(QSqlDatabase &db, int id)
{
    QMap<QString, QVariant> result;

    if (!db.isOpen())
        return result;

    QSqlQuery query(db);
    query.prepare("SELECT * FROM ADHERENT WHERE ID_ADHERENT = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        result["ID_ADHERENT"] = query.value("ID_ADHERENT");
        result["NOM_ADHERENT"] = query.value("NOM_ADHERENT");
        result["PRENOM_ADHERENT"] = query.value("PRENOM_ADHERENT");
        result["GENRE_ADHERENT"] = query.value("GENRE_ADHERENT");
        result["DATE_DE_NAISSANCE_ADHERENT"] = query.value("DATE_DE_NAISSANCE_ADHERENT");
        result["CIN_ADHERENT"] = query.value("CIN_ADHERENT");
        result["ADRESSE_ADHERENT"] = query.value("ADRESSE_ADHERENT");
        result["GSM_ADHERENT"] = query.value("GSM_ADHERENT");
        result["EMAIL_ADHERENT"] = query.value("EMAIL_ADHERENT");
    }

    return result;
}



bool Adherent::supprimer(QSqlDatabase &db)
{
    QSqlQuery query(db);

    query.prepare("DELETE FROM PARTICIPER WHERE ID_AD = :id");
    query.bindValue(":id", id_adherent);

    if (!query.exec()) {
        qDebug() << "Erreur SQL lors de la suppression des participations :" << query.lastError();
        return false;
    }

    query.prepare("DELETE FROM ADHERENT WHERE ID_ADHERENT = :id");
    query.bindValue(":id", id_adherent);

    if (!query.exec()) {
        qDebug() << "Erreur SQL lors de la suppression de l'adhérent :" << query.lastError();
        return false;
    }

    // Vérifie si l'adhérent a été supprimé
    return query.numRowsAffected() > 0;
}



bool Adherent::modifier(QSqlDatabase &db)
{
    if (!db.isOpen()) return false;

    QSqlQuery query(db);

    // Préparer la requête
    query.prepare("UPDATE ADHERENT SET "
                  "NOM_ADHERENT = :nom, "
                  "PRENOM_ADHERENT = :prenom, "
                  "GENRE_ADHERENT = :genre, "
                  "ADRESSE_ADHERENT = :adresse, "
                  "EMAIL_ADHERENT = :email, "
                  "GSM_ADHERENT = :gsm, "
                  "CIN_ADHERENT = :cin, "
                  "DATE_DE_NAISSANCE_ADHERENT = TO_DATE(:dateN,'MM/DD/YYYY') "
                  "WHERE ID_ADHERENT = :id");

    query.bindValue(":id", id_adherent); // ID remains the same
    query.bindValue(":nom", nom_adherent);
    query.bindValue(":prenom", prenom_adherent);
    query.bindValue(":genre", genre_adherent);
    query.bindValue(":adresse", adresse_adherent);
    query.bindValue(":email", email_adherent);
    query.bindValue(":gsm", gsm_adherent);
    query.bindValue(":cin", cin_adherent);
    query.bindValue(":dateN", date_de_naissance_adherent); // already in MM/DD/YYYY format

    if (!query.exec()) {
        QMessageBox::critical(nullptr, "Erreur Modification", query.lastError().text());
        return false;
    }

    return true;
}

bool Adherent::envoyerSMSviaTwilio(const QString &numero, const QString &message, QString &erreur)
{
    // --- Identifiants Twilio ---
    QString accountSID = "AC0c148fefcaf56f615f6761e70ebc06e1";
    QString authToken = "a6bb9d5e723861f05409d7864e901ee3";
    QString fromNumber = "+12602975478";

    // --- Préparation URL ---
    QUrl url("https://api.twilio.com/2010-04-01/Accounts/" + accountSID + "/Messages.json");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    // --- Authentification ---
    QByteArray credentials = (accountSID + ":" + authToken).toUtf8().toBase64();
    request.setRawHeader("Authorization", "Basic " + credentials);

    // --- Données à envoyer ---
    QUrlQuery postData;
    postData.addQueryItem("To", numero);
    postData.addQueryItem("From", fromNumber);
    postData.addQueryItem("Body", message);

    // --- Requête HTTP synchronisée ---
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request, postData.toString(QUrl::FullyEncoded).toUtf8());

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // --- Gestion d’erreurs ---
    if (reply->error() != QNetworkReply::NoError) {
        erreur = reply->errorString();
        reply->deleteLater();
        return false;
    }

    reply->deleteLater();
    return true;
}



void Adherent::notifierAnnulation(QSqlDatabase &db, int idActivite, const QString &nomActivite)
{
    QSqlQuery q(db);
    q.prepare("SELECT ADHERENT.NOM_ADHERENT, ADHERENT.GSM_ADHERENT "
              "FROM ADHERENT "
              "JOIN PARTICIPER ON ADHERENT.ID_ADHERENT = PARTICIPER.ID_AD "
              "WHERE PARTICIPER.ID_AC = :idAct");
    q.bindValue(":idAct", idActivite);

    if(!q.exec()) {
        qDebug() << "Erreur SQL:" << q.lastError().text();
        return;
    }

    while(q.next()) {
        QString nom = q.value("NOM_ADHERENT").toString();
        QString numero = q.value("GSM_ADHERENT").toString();

        // --- Format correct +216 ---
        if(!numero.startsWith("+216")) {
            if(numero.startsWith("0")) numero.remove(0,1);
            numero = "+216" + numero;
        }

        // --- Message à envoyer ---
        QString message = QString("Bonjour %1, l'activité '%2' est annulée.")
                              .arg(nom)
                              .arg(nomActivite);

        // --- Appel Twilio ---
        QString erreur;
        envoyerSMSviaTwilio(numero, message, erreur);

        if(!erreur.isEmpty())
            qDebug() << "Erreur SMS pour" << nom << "(" << numero << ") :" << erreur;
        else
            qDebug() << "SMS envoyé à" << nom << "(" << numero << ")";
    }
}



void Adherent::exporterPDF(QSqlDatabase &db, const QString &fileName)
{
    QString file = fileName;
    if (!file.endsWith(".pdf", Qt::CaseInsensitive))
        file += ".pdf";

    // 🔹 Requête SQL SIMPLIFIÉE (4 colonnes seulement)
    QSqlQuery q(db);
    q.prepare("SELECT NOM_ADHERENT, PRENOM_ADHERENT, GSM_ADHERENT, EMAIL_ADHERENT "
              "FROM ADHERENT ORDER BY NOM_ADHERENT");

    if (!q.exec()) {
        QMessageBox::critical(nullptr, "Erreur SQL",
                              "Impossible de récupérer la liste des adhérents :\n" + q.lastError().text());
        return;
    }

    if (!q.next()) {
        QMessageBox::information(nullptr, "Rapport PDF", "Aucun adhérent trouvé.");
        return;
    }
    q.first();

    QPdfWriter pdf(file);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setPageMargins(QMarginsF(15, 15, 15, 15));

    QPainter painter(&pdf);
    if (!painter.isActive()) {
        QMessageBox::critical(nullptr, "Erreur PDF", "Impossible de créer le fichier PDF.");
        return;
    }

    QFont font = painter.font();
    font.setPointSize(11);
    painter.setFont(font);
    QFontMetrics fm(font);

    int pageWidth = pdf.width();
    int pageHeight = pdf.height();
    int y = 0;

    // 🔹 4 colonnes uniquement
    QStringList headers = {"Nom", "Prénom", "GSM", "Email"};
    int cols = headers.size();
    int colWidth = pageWidth / cols;
    int headerHeight = fm.height() + 15;
    int lineHeight = fm.height() + 10;

    //  Affichage de l’en-tête
    QFont bold = font;
    bold.setBold(true);
    painter.setFont(bold);

    for (int col = 0; col < cols; col++) {
        QRect rect(col * colWidth, y, colWidth, headerHeight);
        painter.drawRect(rect);
        painter.drawText(rect.adjusted(2, 2, -2, -2),
                         Qt::AlignCenter | Qt::TextWordWrap,
                         headers[col]);
    }

    painter.setFont(font);
    y += headerHeight;

    // Remplissage du tableau
    do {
        if (y + lineHeight > pageHeight) {
            pdf.newPage();
            y = 0;

            painter.setFont(bold);
            for (int col = 0; col < cols; col++) {
                QRect rect(col * colWidth, y, colWidth, headerHeight);
                painter.drawRect(rect);
                painter.drawText(rect.adjusted(2, 2, -2, -2),
                                 Qt::AlignCenter | Qt::TextWordWrap,
                                 headers[col]);
            }
            painter.setFont(font);
            y += headerHeight;
        }

        QString values[4] = {
            q.value("NOM_ADHERENT").toString(),
            q.value("PRENOM_ADHERENT").toString(),
            q.value("GSM_ADHERENT").toString(),
            q.value("EMAIL_ADHERENT").toString()
        };

        for (int col = 0; col < cols; col++) {
            QRect rect(col * colWidth, y, colWidth, lineHeight);
            painter.drawRect(rect);
            painter.drawText(rect.adjusted(2, 2, -2, -2),
                             Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap,
                             values[col]);
        }

        y += lineHeight;

    } while (q.next());

    painter.end();
    QMessageBox::information(nullptr, "PDF", "PDF généré avec succès !");
}


void Adherent::genererStatistiques(QSqlDatabase &db, QLayout *layout)
{
    QSqlQuery query(db);
    if (!query.exec("SELECT DATE_DE_NAISSANCE_ADHERENT FROM ADHERENT")) {
        QMessageBox::warning(nullptr, "Erreur", "Impossible de récupérer les adhérents : " + query.lastError().text());
        return;
    }

    int moins20 = 0, entre20_40 = 0, entre41_60 = 0, plus60 = 0;
    QDate aujourdHui = QDate::currentDate();

    while(query.next()) {
        QDate dateNaissance = query.value(0).toDate();
        if (!dateNaissance.isValid()) continue; // ignore les dates invalides

        int age = dateNaissance.daysTo(aujourdHui) / 365; // ton calcul existant

        if(age < 20) moins20++;
        else if(age <= 40) entre20_40++;
        else if(age <= 60) entre41_60++;
        else plus60++;
    }

    QPieSeries *series = new QPieSeries();
    if(moins20 > 0) series->append("<20 ans", moins20);
    if(entre20_40 > 0) series->append("20-40 ans", entre20_40);
    if(entre41_60 > 0) series->append("41-60 ans", entre41_60);
    if(plus60 > 0) series->append(">60 ans", plus60);

    for(QPieSlice *slice : series->slices())
        slice->setLabel(QString("%1 (%2)").arg(slice->label()).arg(slice->value()));

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition des adhérents par âge");
    chart->legend()->setAlignment(Qt::AlignRight);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Supprimer les anciens widgets du layout
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    layout->addWidget(chartView);
}



void Adherent::trier(QSqlDatabase &db, QTableWidget *table, const QString &critere)
{
    if (!db.isOpen()) {
        QMessageBox::warning(nullptr, "Erreur", "La base de données n'est pas ouverte !");
        return;
    }

    QSqlQuery query(db);

    if (critere == "Nom")
        query.prepare("SELECT * FROM ADHERENT ORDER BY NOM_ADHERENT ASC");
    else if (critere == "ID")
        query.prepare("SELECT * FROM ADHERENT ORDER BY ID_ADHERENT ASC");
    else {
        QMessageBox::warning(nullptr, "Erreur", "Critère de tri inconnu !");
        return;
    }

    if (!query.exec()) {
        QMessageBox::warning(nullptr, "Erreur SQL", "Impossible de trier les adhérents : " + query.lastError().text());
        return;
    }

    // Vider le tableau avant de remplir
    table->setRowCount(0);
    int row = 0;

    while (query.next()) {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(query.value("ID_ADHERENT").toString()));
        table->setItem(row, 1, new QTableWidgetItem(query.value("NOM_ADHERENT").toString()));
        table->setItem(row, 2, new QTableWidgetItem(query.value("PRENOM_ADHERENT").toString()));
        table->setItem(row, 3, new QTableWidgetItem(query.value("DATE_DE_NAISSANCE_ADHERENT").toString()));
        table->setItem(row, 4, new QTableWidgetItem(query.value("GENRE_ADHERENT").toString()));
        table->setItem(row, 5, new QTableWidgetItem(query.value("CIN_ADHERENT").toString()));
        table->setItem(row, 6, new QTableWidgetItem(query.value("ADRESSE_ADHERENT").toString()));
        table->setItem(row, 7, new QTableWidgetItem(query.value("GSM_ADHERENT").toString()));
        table->setItem(row, 8, new QTableWidgetItem(query.value("EMAIL_ADHERENT").toString()));
        row++;
    }
}

void Adherent::chercher(QSqlDatabase &db, QTableWidget *table, const QString &id)
{
    if (!db.isOpen()) {
        QMessageBox::warning(nullptr, "Erreur", "La base de données n'est pas ouverte !");
        return;
    }

    if (id.isEmpty()) {
        QMessageBox::warning(nullptr, "Erreur", "Veuillez saisir un ID.");
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT * FROM ADHERENT WHERE ID_ADHERENT = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        QMessageBox::warning(nullptr, "Erreur SQL", "Erreur lors de la recherche : " + query.lastError().text());
        return;
    }

    // Vider le tableau avant de remplir
    table->setRowCount(0);
    int row = 0;

    while (query.next()) {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(query.value("ID_ADHERENT").toString()));
        table->setItem(row, 1, new QTableWidgetItem(query.value("NOM_ADHERENT").toString()));
        table->setItem(row, 2, new QTableWidgetItem(query.value("PRENOM_ADHERENT").toString()));
        table->setItem(row, 3, new QTableWidgetItem(query.value("DATE_DE_NAISSANCE_ADHERENT").toString()));
        table->setItem(row, 4, new QTableWidgetItem(query.value("GENRE_ADHERENT").toString()));
        table->setItem(row, 5, new QTableWidgetItem(query.value("CIN_ADHERENT").toString()));
        table->setItem(row, 6, new QTableWidgetItem(query.value("ADRESSE_ADHERENT").toString()));
        table->setItem(row, 7, new QTableWidgetItem(query.value("GSM_ADHERENT").toString()));
        table->setItem(row, 8, new QTableWidgetItem(query.value("EMAIL_ADHERENT").toString()));
        row++;
    }

    if (row == 0) {
        QMessageBox::information(nullptr, "Résultat", "Aucun adhérent trouvé avec cet ID.");
    }
}

bool Adherent::inscrireAdherent(int idAdherent, int idActivite) {
    QSqlQuery q;
    q.prepare("INSERT INTO PARTICIPER (ID_AD, ID_AC) VALUES (:idA, :idAct)");
    q.bindValue(":idA", idAdherent);
    q.bindValue(":idAct", idActivite);

    if(!q.exec()) {
        qDebug() << "Erreur inscription:" << q.lastError().text();
        return false;
    }
    return true;
}

