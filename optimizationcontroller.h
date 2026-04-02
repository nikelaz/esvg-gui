#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QVariantList>
#include <QColor>
#include "esvg_rs.h"

class OptimizationController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool         optimizing        READ isOptimizing     NOTIFY optimizingChanged)
    Q_PROPERTY(bool         fileLoaded        READ isFileLoaded     NOTIFY fileLoadedChanged)
    Q_PROPERTY(QString      svgPath           READ svgPath          NOTIFY svgPathChanged)
    Q_PROPERTY(QByteArray   originalSvgBytes  READ originalSvgBytes NOTIFY originalSvgBytesChanged)
    Q_PROPERTY(QByteArray   optimizedSvgBytes READ optimizedSvgBytes NOTIFY optimizedSvgBytesChanged)
    Q_PROPERTY(QString      originalSvgText   READ originalSvgText  NOTIFY originalSvgTextChanged)
    Q_PROPERTY(QString      optimizedSvgText  READ optimizedSvgText NOTIFY optimizedSvgTextChanged)
    Q_PROPERTY(QString      origSize  READ origSize  NOTIFY statsChanged)
    Q_PROPERTY(QString      optSize   READ optSize   NOTIFY statsChanged)
    Q_PROPERTY(QString      origGzip  READ origGzip  NOTIFY statsChanged)
    Q_PROPERTY(QString      optGzip   READ optGzip   NOTIFY statsChanged)
    Q_PROPERTY(QVariantList pluginStates READ pluginStates NOTIFY pluginStatesChanged)
    Q_PROPERTY(int          precision  READ precision WRITE setPrecision NOTIFY precisionChanged)

public:
    explicit OptimizationController(QObject *parent = nullptr);

    bool         isOptimizing()     const { return m_optimizing; }
    bool         isFileLoaded()     const { return !m_originalBytes.isEmpty(); }
    QString      svgPath()          const { return m_svgPath; }
    QByteArray   originalSvgBytes() const { return m_originalBytes; }
    QByteArray   optimizedSvgBytes()const { return m_optimizedBytes; }
    QString      originalSvgText()  const { return QString::fromUtf8(m_originalBytes); }
    QString      optimizedSvgText() const { return QString::fromUtf8(m_optimizedBytes); }
    QString      origSize()         const { return m_origSize; }
    QString      optSize()          const { return m_optSize; }
    QString      origGzip()         const { return m_origGzip; }
    QString      optGzip()          const { return m_optGzip; }
    QVariantList pluginStates()     const;
    int          precision()        const { return m_precision; }
    void         setPrecision(int v);

    Q_INVOKABLE void    openFileDialog();
    Q_INVOKABLE void    openFile(const QString &path);
    Q_INVOKABLE void    setPluginEnabled(int index, bool enabled);
    Q_INVOKABLE QString pluginName(int index) const;
    Q_INVOKABLE int     pluginCount() const { return ESVG_PLUGIN_COUNT; }
    Q_INVOKABLE bool    isNumberPrecisionPlugin(int index) const { return index == 4; }
    Q_INVOKABLE void    pickColor(int index, const QColor &current);

signals:
    void optimizingChanged();
    void fileLoadedChanged();
    void svgPathChanged();
    void originalSvgBytesChanged();
    void optimizedSvgBytesChanged();
    void originalSvgTextChanged();
    void optimizedSvgTextChanged();
    void statsChanged();
    void pluginStatesChanged();
    void precisionChanged();
    void colorChanged(int index, QColor color);

private:
    void reoptimize();
    static QString formatSize(qint64 bytes);
    static qint64  gzipSize(const QByteArray &data);

    QByteArray  m_originalBytes;
    QByteArray  m_optimizedBytes;
    QString     m_svgPath;
    bool        m_optimizing = false;
    bool        m_pluginStates[ESVG_PLUGIN_COUNT] = {};
    int         m_precision = 3;
    QString     m_origSize, m_optSize, m_origGzip, m_optGzip;
};
