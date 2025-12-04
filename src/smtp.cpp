#include "smtp.h"
#include <QTextStream>
#include <QTimer>
#include <QDebug>

Smtp::Smtp(const QString &user, const QString &pass, const QString &host, int port, QObject *parent)
    : QObject(parent), username(user), password(pass), hostName(host), portNumber(port)
{
    socket = new QSslSocket(this);
    connect(socket, &QSslSocket::readyRead, this, &Smtp::onReadyRead);
    connect(socket, &QSslSocket::connected, this, &Smtp::onConnected);

    socket->connectToHostEncrypted(hostName, portNumber);
}

void Smtp::sendMail(const QString &from, const QString &to, const QString &subject, const QString &body)
{
    commands.clear();
    commands << "EHLO localhost"
             << "AUTH LOGIN"
             << QString(username.toUtf8().toBase64())
             << QString(password.toUtf8().toBase64())
             << QString("MAIL FROM:<%1>").arg(from)
             << QString("RCPT TO:<%1>").arg(to)
             << "DATA"
             << QString("Subject: %1\r\nFrom: %2\r\nTo: %3\r\n\r\n%4\r\n.").arg(subject, from, to, body)
             << "QUIT";

    if(socket->isEncrypted())
        executeCommand();
}

QString Smtp::currentCommand()
{
    return commands.isEmpty() ? "" : commands.takeFirst();
}

void Smtp::executeCommand()
{
    if(commands.isEmpty())
        return;

    QString cmd = currentCommand();
    if(!cmd.isEmpty())
    {
        QTextStream ts(socket);
        ts << cmd << "\r\n";
        ts.flush();
    }
}

void Smtp::onConnected()
{
    if(socket->isEncrypted())
        executeCommand();
}

void Smtp::onEncrypted()
{
    executeCommand();
}

void Smtp::onReadyRead()
{
    QString response = socket->readAll();
    qDebug() << response;

    // Vérifier si le mail a été envoyé avec succès
    if(response.contains("250 OK") && commands.isEmpty())
    {
        emit finished(true, "Email envoyé avec succès !");
    }
    else if(response.contains("535")) // erreur authentification
    {
        emit finished(false, "Erreur SMTP : Authentification échouée !");
    }

    if(!commands.isEmpty())
        QTimer::singleShot(200, this, &Smtp::executeCommand);
}
