#include "materiel.h"
#include "qrcodehelper.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonDocument>

// Initialisation des listes de valeurs valides
const QStringList Materiel::CATEGORIES_VALIDES =
    {"Informatique", "Bureautique", "Sport", "Autre"};
const QStringList Materiel::ETATS_VALIDES =
    {"Disponible", "En panne", "En réparation", "Empruntee"};
const QStringList Materiel::UTILISATEUR_VALIDES =
    {"Formateur", "Administration", "Adherent"};


Materiel::Materiel()
    : m_id(0), m_seuilAlerte(0), m_qtTotale(0), m_qtDispo(0), m_prix(0.0)
{}

Materiel::Materiel(int id,
                   const QString& nom,
                   const QString& categorie,
                   const QString& etat,
                   const QString& utilisateur,
                   int seuilAlerte,
                   int qtTotale,
                   int qtDispo,
                   double prix,
                   const QString& garantie,
                   const QString& fournisseur)
    : m_id(id),
    m_nom(nom.trimmed()),
    m_categorie(categorie.trimmed()),
    m_etat(etat.trimmed()),
    m_utilisateur(utilisateur.trimmed()),
    m_garantie(garantie.trimmed()),
    m_fournisseur(fournisseur.trimmed()),
    m_seuilAlerte(seuilAlerte),
    m_qtTotale(qtTotale),
    m_qtDispo(qtDispo),
    m_prix(prix)
{}

// ==================== SETTERS AVEC VALIDATION ====================
void Materiel::setId(int v)
{
    if (v > 0) m_id = v;
}

void Materiel::setNom(const QString& v)
{
    m_nom = v.trimmed();
}

void Materiel::setCategorie(const QString& v)
{
    m_categorie = v.trimmed();
}

void Materiel::setEtat(const QString& v)
{
    m_etat = v.trimmed();
}

void Materiel::setUtilisateur(const QString& v)
{
    m_utilisateur = v.trimmed();
}

void Materiel::setSeuilAlerte(int v)
{
    if (v >= 0) m_seuilAlerte = v;
}

void Materiel::setQtTotale(int v)
{
    if (v >= 0) m_qtTotale = v;
}

void Materiel::setQtDispo(int v)
{
    if (v >= 0) m_qtDispo = v;
}

void Materiel::setPrix(double v)
{
    if (v >= 0.0) m_prix = v;
}

void Materiel::setGarantie(const QString& v)
{
    m_garantie = v.trimmed();
}

void Materiel::setFournisseur(const QString& v)
{
    m_fournisseur = v.trimmed();
}

void Materiel::setImageData(const QByteArray& v)
{
    m_imageData = v;
    qDebug() << "Image data set - Size:" << v.size() << "bytes";
}

// ==================== VALIDATION ====================

bool Materiel::validerId(int id, QString& erreur)
{
    if (id <= 0)
    {
        erreur = "L'ID doit être un nombre positif supérieur à 0.";
        return false;
    }
    return true;
}

bool Materiel::validerNom(const QString& nom, QString& erreur)
{
    QString n = nom.trimmed();

    if (n.isEmpty())
    {
        erreur = "Le nom du matériel est obligatoire.";
        return false;
    }

    if (n.length() < 2)
    {
        erreur = "Le nom doit contenir au moins 2 caractères.";
        return false;
    }

    if (n.length() > 100)
    {
        erreur = "Le nom ne peut pas dépasser 100 caractères.";
        return false;
    }

    // Vérifier que le nom ne contient que des caractères alphanumériques, espaces et tirets
    QRegularExpression regex("^[a-zA-Z0-9àâäéèêëïîôùûüÿçÀÂÄÉÈÊËÏÎÔÙÛÜŸÇ\\s\\-_]+$");
    if (!regex.match(n).hasMatch())
    {
        erreur = "Le nom contient des caractères non autorisés.";
        return false;
    }

    return true;
}

bool Materiel::validerCategorie(const QString& categorie, QString& erreur)
{
    QString cat = categorie.trimmed();

    if (cat.isEmpty())
    {
        erreur = "La catégorie est obligatoire.";
        return false;
    }

    if (!CATEGORIES_VALIDES.contains(cat))
    {
        erreur = QString("Catégorie invalide. Valeurs acceptées: %1")
                     .arg(CATEGORIES_VALIDES.join(", "));
        return false;
    }

    return true;
}

bool Materiel::validerEtat(const QString& etat, QString& erreur)
{
    QString e = etat.trimmed();

    if (e.isEmpty())
    {
        erreur = "L'état est obligatoire.";
        return false;
    }

    if (!ETATS_VALIDES.contains(e))
    {
        erreur = QString("État invalide. Valeurs acceptées: %1")
                     .arg(ETATS_VALIDES.join(", "));
        return false;
    }

    return true;
}

bool Materiel::validerUtilisateur(const QString& utilisateur, QString& erreur)
{
    QString u = utilisateur.trimmed();

    if (u.isEmpty())
    {
        erreur = "L'utilisateur est obligatoire.";
        return false;
    }

    if (u.length() > 50)
    {
        erreur = "Le nom d'utilisateur ne peut pas dépasser 50 caractères.";
        return false;
    }

    return true;
}

bool Materiel::validerQuantites(int qtTotale, int qtDispo, QString& erreur)
{
    if (qtTotale < 0)
    {
        erreur = "La quantité totale ne peut pas être négative.";
        return false;
    }

    if (qtDispo < 0)
    {
        erreur = "La quantité disponible ne peut pas être négative.";
        return false;
    }

    if (qtDispo > qtTotale)
    {
        erreur = "La quantité disponible ne peut pas dépasser la quantité totale.";
        return false;
    }

    return true;
}

bool Materiel::validerPrix(double prix, QString& erreur)
{
    if (prix < 0.0)
    {
        erreur = "Le prix ne peut pas être négatif.";
        return false;
    }

    if (prix > 999999.99)
    {
        erreur = "Le prix ne peut pas dépasser 999999.99.";
        return false;
    }

    return true;
}

bool Materiel::valider(QString& messageErreur) const
{
    QString erreur;

    if (!validerId(m_id, erreur))
    {
        messageErreur = "ID: " + erreur;
        return false;
    }

    if (!validerNom(m_nom, erreur))
    {
        messageErreur = "Nom: " + erreur;
        return false;
    }

    if (!validerCategorie(m_categorie, erreur))
    {
        messageErreur = "Catégorie: " + erreur;
        return false;
    }

    if (!validerEtat(m_etat, erreur))
    {
        messageErreur = "État: " + erreur;
        return false;
    }

    if (!validerUtilisateur(m_utilisateur, erreur))
    {
        messageErreur = "Utilisateur: " + erreur;
        return false;
    }

    if (!validerQuantites(m_qtTotale, m_qtDispo, erreur))
    {
        messageErreur = "Quantités: " + erreur;
        return false;
    }

    if (!validerPrix(m_prix, erreur))
    {
        messageErreur = "Prix: " + erreur;
        return false;
    }

    if (m_seuilAlerte < 0)
    {
        messageErreur = "Le seuil d'alerte ne peut pas être négatif.";
        return false;
    }

    return true;
}

// ==================== CRUD ====================

bool Materiel::idExiste(int id)
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM MATERIEL WHERE ID_MATERIEL = :id");
    q.bindValue(":id", id);

    if (!q.exec())
    {
        qWarning() << "Erreur vérification ID:" << q.lastError().text();
        return false;
    }

    if (q.next())
    {
        return q.value(0).toInt() > 0;
    }

    return false;
}

bool Materiel::ajouter()
{
    // Validation complète avant l'ajout
    QString erreur;
    if (!valider(erreur))
    {
        qWarning() << "Validation échouée:" << erreur;
        return false;
    }

    // Vérifier que l'ID n'existe pas déjà
    if (idExiste(m_id))
    {
        qWarning() << "L'ID" << m_id << "existe déjà dans la base de données.";
        return false;
    }

    // Requête préparée (protection contre SQL injection)
    QSqlQuery q;
    q.prepare(R"(
        INSERT INTO MATERIEL
        (ID_MATERIEL, NOM, CATEGORIE, ETAT, UTILISATEUR,
         SEUIL_ALERTE, QT_TOTALE, QT_DISPO, PRIX, GARANTIE, FOURNISSEUR, IMAGE_DATA)
        VALUES (:id, :nom, :cat, :etat, :user, :seuil, :qtTot, :qtDispo, :prix, :gar, :four, :image)
    )");

    q.bindValue(":id", m_id);
    q.bindValue(":nom", m_nom);
    q.bindValue(":cat", m_categorie);
    q.bindValue(":etat", m_etat);
    q.bindValue(":user", m_utilisateur);
    q.bindValue(":seuil", m_seuilAlerte);
    q.bindValue(":qtTot", m_qtTotale);
    q.bindValue(":qtDispo", m_qtDispo);
    q.bindValue(":prix", m_prix);
    q.bindValue(":gar", m_garantie.isEmpty() ? QVariant() : m_garantie);
    q.bindValue(":four", m_fournisseur.isEmpty() ? QVariant() : m_fournisseur);
    q.bindValue(":image", m_imageData.isEmpty() ? QVariant() : m_imageData);

    if (!q.exec())
    {
        qWarning() << "Materiel::ajouter error:" << q.lastError().text();
        return false;
    }

    qDebug() << "Matériel ajouté avec succès - ID:" << m_id
             << "- Image size:" << m_imageData.size() << "bytes";
    return true;
}

bool Materiel::supprimer(int id)
{
    QString erreur;
    if (!validerId(id, erreur))
    {
        qWarning() << "ID invalide pour suppression:" << erreur;
        return false;
    }

    // Requête préparée
    QSqlQuery q;
    q.prepare("DELETE FROM MATERIEL WHERE ID_MATERIEL = :id");
    q.bindValue(":id", id);

    if (!q.exec())
    {
        qWarning() << "Materiel::supprimer error:" << q.lastError().text();
        return false;
    }

    bool succes = q.numRowsAffected() > 0;
    if (succes)
    {
        qDebug() << "Matériel supprimé avec succès - ID:" << id;
    }
    else
    {
        qWarning() << "Aucun matériel trouvé avec l'ID:" << id;
    }

    return succes;
}

bool Materiel::modifier()
{
    // Validation complète avant la modification
    QString erreur;
    if (!valider(erreur))
    {
        qWarning() << "Validation échouée:" << erreur;
        return false;
    }

    // Vérifier que l'ID existe
    if (!idExiste(m_id))
    {
        qWarning() << "L'ID" << m_id << "n'existe pas dans la base de données.";
        return false;
    }

    // Requête préparée
    QSqlQuery q;
    q.prepare(R"(
        UPDATE MATERIEL SET
            NOM = :nom,
            CATEGORIE = :cat,
            ETAT = :etat,
            UTILISATEUR = :user,
            SEUIL_ALERTE = :seuil,
            QT_TOTALE = :qtTot,
            QT_DISPO = :qtDispo,
            PRIX = :prix,
            GARANTIE = :gar,
            FOURNISSEUR = :four,
            IMAGE_DATA = :image
        WHERE ID_MATERIEL = :id
    )");

    q.bindValue(":nom", m_nom);
    q.bindValue(":cat", m_categorie);
    q.bindValue(":etat", m_etat);
    q.bindValue(":user", m_utilisateur);
    q.bindValue(":seuil", m_seuilAlerte);
    q.bindValue(":qtTot", m_qtTotale);
    q.bindValue(":qtDispo", m_qtDispo);
    q.bindValue(":prix", m_prix);
    q.bindValue(":gar", m_garantie.isEmpty() ? QVariant() : m_garantie);
    q.bindValue(":four", m_fournisseur.isEmpty() ? QVariant() : m_fournisseur);
    q.bindValue(":image", m_imageData.isEmpty() ? QVariant() : m_imageData);
    q.bindValue(":id", m_id);

    if (!q.exec())
    {
        qWarning() << "Materiel::modifier error:" << q.lastError().text();
        return false;
    }

    qDebug() << "Matériel modifié avec succès - ID:" << m_id;
    return q.numRowsAffected() > 0;
}

// ==================== RECHERCHE ET AFFICHAGE ====================

Materiel Materiel::rechercherParId(int id, bool& found)
{
    found = false;

    QSqlQuery q;
    q.prepare("SELECT ID_MATERIEL, NOM, CATEGORIE, ETAT, UTILISATEUR, "
              "SEUIL_ALERTE, QT_TOTALE, QT_DISPO, PRIX, GARANTIE, FOURNISSEUR, IMAGE_DATA "
              "FROM MATERIEL WHERE ID_MATERIEL = :id");
    q.bindValue(":id", id);

    if (!q.exec())
    {
        qWarning() << "Erreur recherche:" << q.lastError().text();
        return Materiel();
    }

    if (q.next())
    {
        found = true;
        Materiel mat(
            q.value(0).toInt(),
            q.value(1).toString(),
            q.value(2).toString(),
            q.value(3).toString(),
            q.value(4).toString(),
            q.value(5).toInt(),
            q.value(6).toInt(),
            q.value(7).toInt(),
            q.value(8).toDouble(),
            q.value(9).toString(),
            q.value(10).toString()
            );
        // Set image data separately
        mat.setImageData(q.value(11).toByteArray());
        return mat;
    }

    return Materiel();
}

QList<Materiel> Materiel::afficherTous()
{
    QList<Materiel> liste;

    QSqlQuery q("SELECT ID_MATERIEL, NOM, CATEGORIE, ETAT, UTILISATEUR, "
                "SEUIL_ALERTE, QT_TOTALE, QT_DISPO, PRIX, GARANTIE, FOURNISSEUR, IMAGE_DATA "
                "FROM MATERIEL ORDER BY ID_MATERIEL");

    if (!q.exec())
    {
        qWarning() << "Erreur affichage:" << q.lastError().text();
        return liste;
    }

    while (q.next())
    {
        Materiel mat(
            q.value(0).toInt(),
            q.value(1).toString(),
            q.value(2).toString(),
            q.value(3).toString(),
            q.value(4).toString(),
            q.value(5).toInt(),
            q.value(6).toInt(),
            q.value(7).toInt(),
            q.value(8).toDouble(),
            q.value(9).toString(),
            q.value(10).toString()
            );
        // Set image data separately
        mat.setImageData(q.value(11).toByteArray());
        liste.append(mat);
    }

    return liste;
}

// ==================== QR CODE GENERATION ====================

QString Materiel::toQrCodeData() const
{
    // Create a JSON object with all material information
    QJsonObject json;
    json["id"] = m_id;
    json["nom"] = m_nom;
    json["categorie"] = m_categorie;
    json["etat"] = m_etat;
    json["utilisateur"] = m_utilisateur;
    json["seuilAlerte"] = m_seuilAlerte;
    json["qtTotale"] = m_qtTotale;
    json["qtDispo"] = m_qtDispo;
    json["prix"] = m_prix;
    json["garantie"] = m_garantie;
    json["fournisseur"] = m_fournisseur;

    QJsonDocument doc(json);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QImage Materiel::generateQrCode(int size) const
{
    QString qrData = toQrCodeData();
    return QrCodeHelper::generateQrCode(qrData, size);
}

bool Materiel::saveQrCodeToFile(const QString& filePath, int size) const
{
    QString qrData = toQrCodeData();
    return QrCodeHelper::saveQrCodeToFile(qrData, filePath, size);
}
