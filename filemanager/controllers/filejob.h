#ifndef FILEJOB_H
#define FILEJOB_H

#include <QObject>
#include <QMap>
#include <QElapsedTimer>
#include <QUrl>

#define TRANSFER_RATE 5
#define MSEC_FOR_DISPLAY 1000
#define DATA_BLOCK_SIZE 65536
#define ONE_MB_SIZE 1048576
#define ONE_KB_SIZE 1024

class FileJob : public QObject
{
    Q_OBJECT
public:
    enum Status
    {
        Started,
        Paused,
        Cancelled,
        Run,
        Conflicted,
    };
    void setStatus(Status status);
    explicit FileJob(QObject *parent = 0);
    void setJobId(const QString &id);
    QString getJobId();
    QString checkDuplicateName(const QString &name);
    void setApplyToAll(bool v);
signals:
    void progressPercent(int value);
    void error(QString content);
    void result(QString content);
    void finished();
public slots:
    void doCopy(const QList<QUrl> &files, const QString &destination);
    void doDelete(const QList<QUrl> &files);
    void doMoveToTrash(const QList<QUrl> &files);
    void doCut(const QList<QUrl> &files, const QString &destination);
    void paused();
    void started();
    void cancelled();
private:
    Status m_status;
    QString m_trashLoc;
    QString m_id;
    QMap<QString, QString> m_jobDetail;
    qint64 m_bytesCopied;
    qint64 m_totalSize;
    double m_bytesPerSec;
    float m_factor;
    bool m_isJobAdded = false;
    QString m_srcFileName;
    QString m_tarFileName;
    QString m_srcPath;
    QString m_tarPath;
    QElapsedTimer m_timer;
    qint64 lastMsec;
    qint64 currentMsec;
    bool m_applyToAll = false;

    void jobUpdated();
    void jobAdded();
    void jobRemoved();
    void jobAborted();
    void jobPrepared();
    void jobConflicted();

    bool copyFile(const QString &srcFile, const QString &tarDir);
    bool copyDir(const QString &srcPath, const QString &tarPath);
    bool deleteFile(const QString &file);
    bool deleteDir(const QString &dir);
    bool moveDirToTrash(const QString &dir);
    bool moveFileToTrash(const QString &file);
    void writeTrashInfo(const QString &name, const QString &path, const QString &time);
    QString baseName( QString path );
};

#endif // FILEJOB_H
