#include "exportcontroller.h"
#include <QFileDialog>

ExportController::ExportController(QObject *parent)
    : QObject(parent)
{}

void ExportController::setSvgBytes(const QByteArray &b)
{
    if (m_svgBytes == b) return;
    m_svgBytes = b;
    emit svgBytesChanged();
}

void ExportController::setIncludeViewBox(bool v)
{
    if (m_includeViewBox == v) return;
    m_includeViewBox = v;
    emit includeViewBoxChanged();
}

void ExportController::setCustomSize(bool v)
{
    if (m_customSize == v) return;
    m_customSize = v;
    emit customSizeChanged();
}

void ExportController::setExportWidth(int v)
{
    if (m_exportWidth == v) return;
    m_exportWidth = v;
    emit exportWidthChanged();
}

void ExportController::setExportHeight(int v)
{
    if (m_exportHeight == v) return;
    m_exportHeight = v;
    emit exportHeightChanged();
}

void ExportController::setFormatIndex(int v)
{
    if (m_formatIndex == v) return;
    m_formatIndex = v;
    emit formatIndexChanged();
}

QStringList ExportController::formatNames() const
{
    return { tr("SVG"), tr("PNG"), tr("React Component"),
             tr("CSS URL Encoded Background"), tr("PDF") };
}

void ExportController::doExport()
{
    static const QStringList filters = {
        QObject::tr("SVG Files (*.svg)"),
        QObject::tr("PNG Images (*.png)"),
        QObject::tr("JavaScript Files (*.jsx *.tsx)"),
        QObject::tr("CSS Files (*.css)"),
        QObject::tr("PDF Files (*.pdf)"),
    };

    QString filter = (m_formatIndex >= 0 && m_formatIndex < filters.size())
                     ? filters[m_formatIndex] : filters[0];

    const QString path = QFileDialog::getSaveFileName(nullptr, tr("Export"), QString(), filter);
    if (path.isEmpty()) return;

    // TODO: wire actual export logic per format
    emit exportDone();
}
