#ifndef MATERIEL_H
#define MATERIEL_H

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QPixmap>
#include <QImage>

class Materiel
{
public:
    Materiel();
    Materiel(int id,
             const QString& nom,
             const QString& categorie,
             const QString& etat,
             const QString& utilisateur,
             int seuilAlerte,
             int qtTotale,
             int qtDispo,
             double prix,
             const QString& garantie,
             const QString& fournisseur);

    // Getters
    int id() const { return m_id; }
    QString nom() const { return m_nom; }
    QString categorie() const { return m_categorie; }
    QString etat() const { return m_etat; }
    QString utilisateur() const { return m_utilisateur; }
    int seuilAlerte() const { return m_seuilAlerte; }
    int qtTotale() const { return m_qtTotale; }
    int qtDispo() const { return m_qtDispo; }
    double prix() const { return m_prix; }
    QString garantie() const { return m_garantie; }
    QString fournisseur() const { return m_fournisseur; }
    QByteArray imageData() const { return m_imageData; }
    bool hasImage() const { return !m_imageData.isEmpty(); }

    // Setters avec validation
    void setId(int v);
    void setNom(const QString& v);
    void setCategorie(const QString& v);
    void setEtat(const QString& v);
    void setUtilisateur(const QString& v);
    void setSeuilAlerte(int v);
    void setQtTotale(int v);
    void setQtDispo(int v);
    void setPrix(double v);
    void setGarantie(const QString& v);
    void setFournisseur(const QString& v);
    void setImageData(const QByteArray& v);

    // CRUD avec requêtes préparées
    bool ajouter();
    static bool supprimer(int id);
    bool modifier();

    // Recherche
    static Materiel rechercherParId(int id, bool& found);
    static QList<Materiel> afficherTous();

    // Validation complète de l'objet
    bool valider(QString& messageErreur) const;

    // Validation des champs individuels (méthodes statiques)
    static bool validerNom(const QString& nom, QString& erreur);
    static bool validerCategorie(const QString& categorie, QString& erreur);
    static bool validerEtat(const QString& etat, QString& erreur);
    static bool validerUtilisateur(const QString& utilisateur, QString& erreur);
    static bool validerQuantites(int qtTotale, int qtDispo, QString& erreur);
    static bool validerPrix(double prix, QString& erreur);
    static bool validerId(int id, QString& erreur);

    // Vérifier si un ID existe déjà
    static bool idExiste(int id);

    // QR Code generation
    QString toQrCodeData() const;           // Generate QR code data string
    QImage generateQrCode(int size = 200) const;  // Generate QR code as QImage
    bool saveQrCodeToFile(const QString& filePath, int size = 400) const;

private:
    int m_id;
    QString m_nom, m_categorie, m_etat, m_utilisateur, m_garantie, m_fournisseur;
    int m_seuilAlerte, m_qtTotale, m_qtDispo;
    double m_prix;
    QByteArray m_imageData;  // Image stored as BLOB

    // Listes des valeurs valides pour les énumérations
    static const QStringList CATEGORIES_VALIDES;
    static const QStringList ETATS_VALIDES;
    static const QStringList UTLISATEUR_VALIDES;
};

#endif // MATERIEL_H
