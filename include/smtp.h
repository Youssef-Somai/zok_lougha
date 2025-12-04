#ifndef SMTP_H
#define SMTP_H

#include <QObject>
#include <QSslSocket>

class Smtp : public QObject
{
    Q_OBJECT
public:
    Smtp(const QString &user, const QString &password, const QString &host, int port, QObject *parent = nullptr);
    void sendMail(const QString &from, const QString &to, const QString &subject, const QString &body);

signals:
    void finished(bool success, const QString &message); // signal pour notifier résultat

private slots:
    void onReadyRead();
    void onConnected();
    void onEncrypted();

private:
    QString username;
    QString password;
    QString hostName;
    int portNumber;

    QSslSocket *socket;
    QStringList commands;
    QString currentCommand();

    void executeCommand();
};
#endif // SMTP_H
