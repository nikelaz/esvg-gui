#include "optimizationcontroller.h"
#include <QFile>
#include <QFileDialog>
#include <QColorDialog>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include "esvg_rs.h"

static const char * const kPluginNames[ESVG_PLUGIN_COUNT] = {
    QT_TR_NOOP("Remove Unnecessary Attributes"),
    QT_TR_NOOP("Shape to Path"),
    QT_TR_NOOP("Optimize Colors"),
    QT_TR_NOOP("Collapse Groups"),
    QT_TR_NOOP("Number Precision"),
    QT_TR_NOOP("Remove Empty Text"),
    QT_TR_NOOP("Remove Unnecessary Clip Paths"),
    QT_TR_NOOP("Sort Attributes"),
    QT_TR_NOOP("Apply Transforms"),
    QT_TR_NOOP("CSS to Attributes"),
    QT_TR_NOOP("Combine Paths"),
    QT_TR_NOOP("Mangle IDs"),
    QT_TR_NOOP("Simplify Paths"),
};

static const bool kPluginDefaults[ESVG_PLUGIN_COUNT] = {
    true,   // Remove Unnecessary Attributes
    true,   // Shape to Path
    true,   // Optimize Colors
    false,  // Collapse Groups
    false,  // Number Precision
    false,  // Remove Empty Text
    false,  // Remove Unnecessary Clip Paths
    false,  // Sort Attributes
    true,   // Apply Transforms
    true,   // CSS to Attributes
    true,   // Combine Paths
    false,  // Mangle IDs
    false,  // Simplify Paths
};

OptimizationController::OptimizationController(QObject *parent)
    : QObject(parent)
{
    for (int i = 0; i < ESVG_PLUGIN_COUNT; ++i)
        m_pluginStates[i] = kPluginDefaults[i];

    // Load the embedded test SVG on startup
    QFile f(":/test-svg.svg");
    if (f.open(QIODevice::ReadOnly)) {
        m_originalBytes = f.readAll();
        m_svgPath = ":/test-svg.svg";
        emit originalSvgBytesChanged();
        emit originalSvgTextChanged();
        emit fileLoadedChanged();
        emit svgPathChanged();
        reoptimize();
    }
}

void OptimizationController::openFileDialog()
{
    const QString path = QFileDialog::getOpenFileName(
        nullptr, tr("Open SVG"), QString(), tr("SVG Files (*.svg)"));
    if (!path.isEmpty())
        openFile(path);
}

void OptimizationController::openFile(const QString &path)
{
    QByteArray bytes;
    {
        QFile f(path);
        if (f.open(QIODevice::ReadOnly))
            bytes = f.readAll();
    }
    if (bytes.isEmpty())
        return;

    bool wasLoaded = isFileLoaded();
    m_originalBytes = bytes;
    m_svgPath = path;
    m_optimizedBytes.clear();

    emit originalSvgBytesChanged();
    emit originalSvgTextChanged();
    emit optimizedSvgBytesChanged();
    emit optimizedSvgTextChanged();
    emit svgPathChanged();
    if (!wasLoaded)
        emit fileLoadedChanged();

    reoptimize();
}

void OptimizationController::setPluginEnabled(int index, bool enabled)
{
    if (index < 0 || index >= ESVG_PLUGIN_COUNT) return;
    if (m_pluginStates[index] == enabled) return;
    m_pluginStates[index] = enabled;
    emit pluginStatesChanged();
    reoptimize();
}

void OptimizationController::setPrecision(int v)
{
    if (m_precision == v) return;
    m_precision = v;
    emit precisionChanged();
    reoptimize();
}

QString OptimizationController::pluginName(int index) const
{
    if (index < 0 || index >= ESVG_PLUGIN_COUNT) return {};
    return tr(kPluginNames[index]);
}

QVariantList OptimizationController::pluginStates() const
{
    QVariantList list;
    list.reserve(ESVG_PLUGIN_COUNT);
    for (int i = 0; i < ESVG_PLUGIN_COUNT; ++i)
        list.append(m_pluginStates[i]);
    return list;
}

void OptimizationController::pickColor(int index, const QColor &current)
{
    QColor picked = QColorDialog::getColor(current.isValid() ? current : Qt::white, nullptr);
    if (picked.isValid())
        emit colorChanged(index, picked);
}

void OptimizationController::reoptimize()
{
    if (m_originalBytes.isEmpty() || m_optimizing)
        return;

    m_optimizing = true;
    emit optimizingChanged();

    uint64_t flags = 0;
    for (int i = 0; i < ESVG_PLUGIN_COUNT; ++i)
        if (m_pluginStates[i])
            flags |= (UINT64_C(1) << i);

    auto *watcher = new QFutureWatcher<QByteArray>(this);
    connect(watcher, &QFutureWatcher<QByteArray>::finished, this,
            [this, watcher]() {
        QByteArray optimizedBytes = watcher->result();
        watcher->deleteLater();

        m_optimizing = false;
        emit optimizingChanged();

        m_optimizedBytes = optimizedBytes;
        emit optimizedSvgBytesChanged();
        emit optimizedSvgTextChanged();

        m_origSize  = formatSize(m_originalBytes.size());
        m_optSize   = formatSize(optimizedBytes.size());
        m_origGzip  = formatSize(gzipSize(m_originalBytes));
        m_optGzip   = formatSize(gzipSize(optimizedBytes));
        emit statsChanged();
    });

    QByteArray svgBytes = m_originalBytes;
    int precision = m_precision;
    watcher->setFuture(QtConcurrent::run([svgBytes, flags, precision]() -> QByteArray {
        char *result = esvg_optimize_with_flags_ex(
            svgBytes.constData(), static_cast<size_t>(svgBytes.size()),
            flags, static_cast<uint32_t>(precision));
        if (result) {
            QByteArray optimized(result);
            esvg_free(result);
            return optimized;
        }
        return svgBytes;
    }));
}

QString OptimizationController::formatSize(qint64 b)
{
    if (b < 1024) return QString("%1 B").arg(b);
    if (b < 1024 * 1024) return QString("%1 KB").arg(b / 1024.0, 0, 'f', 1);
    return QString("%1 MB").arg(b / (1024.0 * 1024.0), 0, 'f', 1);
}

qint64 OptimizationController::gzipSize(const QByteArray &data)
{
    return qMax(qint64(0), qint64(qCompress(data, 9).size()) - 4);
}
