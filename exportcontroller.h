#pragma once

#include <QObject>
#include <QByteArray>
#include <QStringList>

class ExportController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QByteArray svgBytes      READ svgBytes      WRITE setSvgBytes      NOTIFY svgBytesChanged)
    Q_PROPERTY(bool       includeViewBox READ includeViewBox WRITE setIncludeViewBox NOTIFY includeViewBoxChanged)
    Q_PROPERTY(bool       customSize    READ customSize    WRITE setCustomSize    NOTIFY customSizeChanged)
    Q_PROPERTY(int        exportWidth   READ exportWidth   WRITE setExportWidth   NOTIFY exportWidthChanged)
    Q_PROPERTY(int        exportHeight  READ exportHeight  WRITE setExportHeight  NOTIFY exportHeightChanged)
    Q_PROPERTY(int        formatIndex   READ formatIndex   WRITE setFormatIndex   NOTIFY formatIndexChanged)

public:
    explicit ExportController(QObject *parent = nullptr);

    QByteArray svgBytes()      const { return m_svgBytes; }
    bool       includeViewBox()const { return m_includeViewBox; }
    bool       customSize()    const { return m_customSize; }
    int        exportWidth()   const { return m_exportWidth; }
    int        exportHeight()  const { return m_exportHeight; }
    int        formatIndex()   const { return m_formatIndex; }

    void setSvgBytes(const QByteArray &b);
    void setIncludeViewBox(bool v);
    void setCustomSize(bool v);
    void setExportWidth(int v);
    void setExportHeight(int v);
    void setFormatIndex(int v);

    Q_INVOKABLE QStringList formatNames() const;
    Q_INVOKABLE void        doExport();

signals:
    void svgBytesChanged();
    void includeViewBoxChanged();
    void customSizeChanged();
    void exportWidthChanged();
    void exportHeightChanged();
    void formatIndexChanged();
    void exportDone();

private:
    QByteArray m_svgBytes;
    bool       m_includeViewBox = true;
    bool       m_customSize     = false;
    int        m_exportWidth    = 512;
    int        m_exportHeight   = 512;
    int        m_formatIndex    = 0;  // 0=SVG, 1=PNG, 2=React, 3=CSS, 4=PDF
};
