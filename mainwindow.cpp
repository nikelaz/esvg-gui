#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "sidebarpanel.h"
#include "exportdialog.h"
#include <QFile>
#include <QPlainTextEdit>
#include <QtSvgWidgets/QGraphicsSvgItem>
#include <QtSvg/QSvgRenderer>
#include <QGraphicsRectItem>
#include <QOpenGLWidget>
#include <QSurfaceFormat>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QSlider>
#include <QTimer>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QSplitter>
#include <QCheckBox>
#include <QComboBox>
#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QColorDialog>
#include <QRegularExpressionValidator>
#include <QProgressDialog>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include "esvg_rs.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scene(new QGraphicsScene(this))
{
    ui->setupUi(this);

    buildSidebar();

    QFont monoFont("Courier New", 9);
    monoFont.setStyleHint(QFont::Monospace);
    ui->codeBefore->setFont(monoFont);
    ui->codeAfter->setFont(monoFont);

    ui->svgView->setScene(m_scene);
    QSurfaceFormat format;
    format.setSamples(8);
    auto *glWidget = new QOpenGLWidget;
    glWidget->setFormat(format);
    ui->svgView->setViewport(glWidget);
    ui->svgView->setDragMode(QGraphicsView::NoDrag);
    ui->svgView->setTransformationAnchor(QGraphicsView::NoAnchor);
    ui->svgView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->svgView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->svgView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    m_scene->setSceneRect(-5000, -5000, 10000, 10000);

    // SVG A — primary (clipped to left of slider)
    m_clipContainerA = new QGraphicsRectItem();
    m_clipContainerA->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
    m_clipContainerA->setPen(Qt::NoPen);
    m_clipContainerA->setBrush(Qt::NoBrush);
    m_scene->addItem(m_clipContainerA);

    m_svgItem = nullptr;

    // SVG B — comparison (clipped to right of slider)
    m_clipContainer = new QGraphicsRectItem();
    m_clipContainer->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
    m_clipContainer->setPen(Qt::NoPen);
    m_clipContainer->setBrush(Qt::NoBrush);
    m_scene->addItem(m_clipContainer);

    m_svgItemB = nullptr;

    // Slider divider handle
    m_sliderHandle = new QWidget(ui->svgView);
    m_sliderHandle->setStyleSheet("background: white;");
    m_sliderHandle->setFixedWidth(2);
    m_sliderHandle->setCursor(Qt::SizeHorCursor);
    m_sliderHandle->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_sliderHandle->raise();

    // Zoom overlay
    m_zoomControls = new QWidget(ui->svgView);
    m_zoomControls->setObjectName("zoomControls");
    m_zoomControls->setStyleSheet(
        "QWidget#zoomControls {"
        "  background-color: palette(window);"
        "  border-radius: 6px;"
        "}"
    );

    auto *zoomLayout = new QHBoxLayout(m_zoomControls);
    zoomLayout->setContentsMargins(6, 4, 6, 4);
    zoomLayout->setSpacing(4);

    auto *btnMinus = new QPushButton("-", m_zoomControls);
    btnMinus->setFixedSize(24, 24);

    m_zoomInput = new QLineEdit(m_zoomControls);
    m_zoomInput->setFixedWidth(64);
    m_zoomInput->setAlignment(Qt::AlignCenter);

    auto *btnPlus = new QPushButton("+", m_zoomControls);
    btnPlus->setFixedSize(24, 24);

    zoomLayout->addWidget(btnMinus);
    zoomLayout->addWidget(m_zoomInput);
    zoomLayout->addWidget(btnPlus);
    m_zoomControls->adjustSize();

    connect(btnMinus, &QPushButton::clicked, this, [this]{ applyZoom(1.0 / 1.15); });
    connect(btnPlus,  &QPushButton::clicked, this, [this]{ applyZoom(1.15); });
    connect(m_zoomInput, &QLineEdit::returnPressed, this, [this]{
        QString text = m_zoomInput->text().remove('%').trimmed();
        bool ok;
        double pct = text.toDouble(&ok);
        if (!ok || pct <= 0) return;
        double target = m_fitScale * (pct / 100.0);
        double current = ui->svgView->transform().m11();
        ui->svgView->scale(target / current, target / current);
        updateZoomDisplay();
        updateComparisonClip();
    });

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionExport, &QAction::triggered, this, [this]{
        ExportDialog dlg(m_svgPath, this);
        dlg.exec();
    });

    ui->svgView->viewport()->setMouseTracking(true);
    ui->svgView->viewport()->installEventFilter(this);

    QTimer::singleShot(0, this, [this]() {
        if (m_svgItem) {
            ui->svgView->fitInView(m_svgItem, Qt::KeepAspectRatio);
            m_fitScale = ui->svgView->transform().m11();
            updateZoomDisplay();
        }
        m_sliderViewX = ui->svgView->viewport()->width() / 2;
        updateComparisonClip();
        repositionSliderHandle();
        repositionZoomControls();
        m_zoomControls->raise();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Open SVG"), QString(), tr("SVG Files (*.svg)"));
    if (path.isEmpty())
        return;

    m_svgPath = path;
    ui->actionExport->setEnabled(true);

    m_svgBytes = QByteArray{};
    {
        QFile f(path);
        if (f.open(QIODevice::ReadOnly))
            m_svgBytes = f.readAll();
    }
    ui->codeBefore->setPlainText(QString::fromUtf8(m_svgBytes));

    // Load the original SVG on the left immediately, before async optimization.
    delete m_svgItem;
    m_svgItem = new QGraphicsSvgItem(path, m_clipContainerA);
    m_svgItem->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    ui->svgView->resetTransform();
    ui->svgView->fitInView(m_svgItem, Qt::KeepAspectRatio);
    m_fitScale = ui->svgView->transform().m11();
    updateZoomDisplay();
    updateComparisonClip();

    reoptimize();
}

void MainWindow::reoptimize()
{
    if (m_svgBytes.isEmpty() || m_optimizing)
        return;

    m_optimizing = true;
    for (auto *cb : m_pluginChecks)
        if (cb) cb->setEnabled(false);
    if (m_precisionSlider) m_precisionSlider->setEnabled(false);

    uint64_t flags = 0;
    for (int i = 0; i < ESVG_PLUGIN_COUNT; ++i)
        if (m_pluginChecks[i] && m_pluginChecks[i]->isChecked())
            flags |= (UINT64_C(1) << i);

    auto *progress = new QProgressDialog(tr("Optimizing SVG\u2026"), QString(), 0, 0, this);
    progress->setWindowTitle(tr("Please wait"));
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->setMinimumDuration(0);
    progress->setValue(0);

    auto *watcher = new QFutureWatcher<QByteArray>(this);
    connect(watcher, &QFutureWatcher<QByteArray>::finished, this,
            [this, watcher, progress]() {
        QByteArray optimizedBytes = watcher->result();
        watcher->deleteLater();
        progress->close();
        progress->deleteLater();

        m_optimizing = false;
        for (auto *cb : m_pluginChecks)
            if (cb) cb->setEnabled(true);
        // Restore slider enabled state: enabled only when the Number Precision checkbox is checked
        if (m_precisionSlider && m_pluginChecks[4])
            m_precisionSlider->setEnabled(m_pluginChecks[4]->isChecked());

        ui->codeAfter->setPlainText(QString::fromUtf8(optimizedBytes));

        auto formatSize = [](qint64 b) -> QString {
            if (b < 1024) return QString("%1 B").arg(b);
            if (b < 1024 * 1024) return QString("%1 KB").arg(b / 1024.0, 0, 'f', 1);
            return QString("%1 MB").arg(b / (1024.0 * 1024.0), 0, 'f', 1);
        };
        auto gzipSize = [](const QByteArray &data) -> qint64 {
            return qMax(qint64(0), qint64(qCompress(data, 9).size()) - 4);
        };
        m_labelOrigSize->setText(formatSize(m_svgBytes.size()));
        m_labelOptSize->setText(formatSize(optimizedBytes.size()));
        m_labelOrigGzip->setText(formatSize(gzipSize(m_svgBytes)));
        m_labelOptGzip->setText(formatSize(gzipSize(optimizedBytes)));

        delete m_svgItemB;
        delete m_rendererB;
        m_rendererB = new QSvgRenderer(optimizedBytes, this);
        m_svgItemB = new QGraphicsSvgItem(m_clipContainer);
        m_svgItemB->setSharedRenderer(m_rendererB);
        m_svgItemB->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

        updateComparisonClip();
    });

    QByteArray svgBytes = m_svgBytes;
    int precision = m_precisionSlider ? m_precisionSlider->value() : 3;
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

void MainWindow::applyZoom(double factor)
{
    const QPointF centerViewPos(ui->svgView->viewport()->width() / 2.0,
                                ui->svgView->viewport()->height() / 2.0);
    const QPointF centerScenePos = ui->svgView->mapToScene(centerViewPos.toPoint());
    ui->svgView->scale(factor, factor);
    const QPointF newCenterViewPos = ui->svgView->mapFromScene(centerScenePos);
    const QPointF delta = newCenterViewPos - centerViewPos;
    ui->svgView->horizontalScrollBar()->setValue(
        ui->svgView->horizontalScrollBar()->value() + qRound(delta.x()));
    ui->svgView->verticalScrollBar()->setValue(
        ui->svgView->verticalScrollBar()->value() + qRound(delta.y()));
    updateZoomDisplay();
    updateComparisonClip();
}

void MainWindow::updateZoomDisplay()
{
    if (m_fitScale <= 0 || !m_zoomInput) return;
    int pct = qRound(ui->svgView->transform().m11() / m_fitScale * 100.0);
    m_zoomInput->setText(QString::number(pct) + "%");
}

void MainWindow::repositionZoomControls()
{
    if (!m_zoomControls) return;
    QSize vs = ui->svgView->size();
    QSize cs = m_zoomControls->size();
    m_zoomControls->move(vs.width() - cs.width() - 8, vs.height() - cs.height() - 8);
}

void MainWindow::updateComparisonClip()
{
    if (!m_svgItem || !m_svgItemB || !m_clipContainerA || !m_clipContainer) return;
    double sliderSceneX = ui->svgView->mapToScene(QPoint(m_sliderViewX, 0)).x();

    QRectF bA = m_svgItem->boundingRect();
    QRectF bB = m_svgItemB->boundingRect();
    sliderSceneX = qBound(bA.left(), sliderSceneX, bA.right());

    // Both containers stay at scene (0,0) — only clip rects change.
    // SVG A: show left of slider
    m_clipContainerA->setRect(0, 0, sliderSceneX, bA.height());
    // SVG B: show right of slider (rect origin at sliderSceneX keeps the item stationary)
    m_clipContainer->setRect(sliderSceneX, 0, bB.right() - sliderSceneX, bB.height());
}

void MainWindow::repositionSliderHandle()
{
    if (!m_sliderHandle) return;
    m_sliderHandle->setGeometry(m_sliderViewX - 1, 0, 2, ui->svgView->height());
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != ui->svgView->viewport())
        return QMainWindow::eventFilter(watched, event);

    switch (event->type()) {
    case QEvent::Resize:
        repositionZoomControls();
        repositionSliderHandle();
        updateComparisonClip();
        break;
    case QEvent::Wheel: {
        auto *we = static_cast<QWheelEvent *>(event);
        const double factor = (we->angleDelta().y() > 0) ? 1.15 : (1.0 / 1.15);
        const QPointF cursorViewPos = we->position();
        const QPointF cursorScenePos = ui->svgView->mapToScene(cursorViewPos.toPoint());
        ui->svgView->scale(factor, factor);
        const QPointF newCursorViewPos = ui->svgView->mapFromScene(cursorScenePos);
        const QPointF delta = newCursorViewPos - cursorViewPos;
        ui->svgView->horizontalScrollBar()->setValue(
            ui->svgView->horizontalScrollBar()->value() + qRound(delta.x()));
        ui->svgView->verticalScrollBar()->setValue(
            ui->svgView->verticalScrollBar()->value() + qRound(delta.y()));
        updateZoomDisplay();
        updateComparisonClip();
        return true;
    }
    case QEvent::MouseButtonPress: {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton) {
            if (qAbs(me->pos().x() - m_sliderViewX) < 6) {
                m_draggingSlider = true;
                ui->svgView->setCursor(Qt::SizeHorCursor);
            } else {
                m_lastPanPos = me->pos();
                m_panning = true;
                ui->svgView->setCursor(Qt::ClosedHandCursor);
            }
            return true;
        }
        break;
    }
    case QEvent::MouseMove: {
        auto *me = static_cast<QMouseEvent *>(event);
        if (m_draggingSlider) {
            m_sliderViewX = qBound(0, me->pos().x(), ui->svgView->viewport()->width());
            repositionSliderHandle();
            updateComparisonClip();
            return true;
        }
        if (!m_panning) {
            if (qAbs(me->pos().x() - m_sliderViewX) < 6)
                ui->svgView->viewport()->setCursor(Qt::SizeHorCursor);
            else
                ui->svgView->viewport()->setCursor(Qt::ArrowCursor);
            return true;
        }
        if (m_panning) {
            QPoint delta = me->pos() - m_lastPanPos;
            m_lastPanPos = me->pos();
            ui->svgView->horizontalScrollBar()->setValue(
                ui->svgView->horizontalScrollBar()->value() - delta.x());
            ui->svgView->verticalScrollBar()->setValue(
                ui->svgView->verticalScrollBar()->value() - delta.y());
            updateComparisonClip();
            return true;
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton) {
            m_draggingSlider = false;
            m_panning = false;
            ui->svgView->setCursor(Qt::ArrowCursor);
            return true;
        }
        break;
    }
    default:
        break;
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::buildSidebar()
{
    QSplitter *sp = ui->sidebarSplitter;
    m_panelPlugins  = new SidebarPanel(tr("Plugins"),      sp);
    m_panelOptimize = new SidebarPanel(tr("Optimization"), sp);
    m_panelColors   = new SidebarPanel(tr("Colors"),       sp);
    m_panelExport   = new SidebarPanel(tr("Quick Export"), sp);

    for (auto *p : { m_panelPlugins, m_panelOptimize, m_panelColors, m_panelExport })
        sp->addWidget(p);
    for (int i = 0; i < sp->count(); ++i)
        sp->setCollapsible(i, false);

    buildPluginsPanel();
    buildOptimizePanel();
    buildColorsPanel();
    buildExportPanel();

    connect(ui->actionViewPlugins,      &QAction::toggled, m_panelPlugins,  &QWidget::setVisible);
    connect(ui->actionViewOptimization, &QAction::toggled, m_panelOptimize, &QWidget::setVisible);
    connect(ui->actionViewColors,       &QAction::toggled, m_panelColors,   &QWidget::setVisible);
    connect(ui->actionViewQuickExport,  &QAction::toggled, m_panelExport,   &QWidget::setVisible);
}

void MainWindow::buildPluginsPanel()
{
    struct PluginInfo { const char *name; bool defaultOn; };
    static const PluginInfo kPlugins[ESVG_PLUGIN_COUNT] = {
        { QT_TR_NOOP("Remove Unnecessary Attributes"), true  },
        { QT_TR_NOOP("Shape to Path"),                 true  },
        { QT_TR_NOOP("Optimize Colors"),               true  },
        { QT_TR_NOOP("Collapse Groups"),               false },
        { QT_TR_NOOP("Number Precision"),              false },
        { QT_TR_NOOP("Remove Empty Text"),             false },
        { QT_TR_NOOP("Remove Unnecessary Clip Paths"), false },
        { QT_TR_NOOP("Sort Attributes"),               false },
        { QT_TR_NOOP("Apply Transforms"),              true  },
        { QT_TR_NOOP("CSS to Attributes"),             true  },
        { QT_TR_NOOP("Combine Paths"),                 true  },
        { QT_TR_NOOP("Mangle IDs"),                    false },
        { QT_TR_NOOP("Simplify Paths"),                false },
    };

    auto *l = new QVBoxLayout(m_panelPlugins->contentWidget());
    l->setContentsMargins(10, 8, 10, 8);
    l->setSpacing(4);

    // Index of the Number Precision plugin
    static constexpr int kNumberPrecisionIndex = 4;

    for (int i = 0; i < ESVG_PLUGIN_COUNT; ++i) {
        auto *cb = new QCheckBox(tr(kPlugins[i].name));
        cb->setChecked(kPlugins[i].defaultOn);
        connect(cb, &QCheckBox::stateChanged, this, &MainWindow::reoptimize);
        m_pluginChecks[i] = cb;
        l->addWidget(cb);

        if (i == kNumberPrecisionIndex) {
            // Precision slider row: label on the left, slider on the right
            auto *row = new QWidget;
            auto *hl  = new QHBoxLayout(row);
            hl->setContentsMargins(20, 0, 0, 0); // indent under the checkbox
            hl->setSpacing(6);

            auto *label = new QLabel(tr("Precision:"));
            label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

            m_precisionSlider = new QSlider(Qt::Horizontal);
            m_precisionSlider->setRange(1, 10);
            m_precisionSlider->setValue(3);
            m_precisionSlider->setTickPosition(QSlider::TicksBelow);
            m_precisionSlider->setTickInterval(1);
            m_precisionSlider->setEnabled(cb->isChecked());

            auto *valueLabel = new QLabel(QString::number(m_precisionSlider->value()));
            valueLabel->setFixedWidth(16);
            valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

            // Keep value label in sync with slider
            connect(m_precisionSlider, &QSlider::valueChanged, this,
                    [this, valueLabel](int v) {
                valueLabel->setText(QString::number(v));
                reoptimize();
            });

            // Enable/disable slider with the checkbox
            connect(cb, &QCheckBox::stateChanged, this,
                    [this](int state) {
                m_precisionSlider->setEnabled(state == Qt::Checked);
            });

            hl->addWidget(label);
            hl->addWidget(m_precisionSlider);
            hl->addWidget(valueLabel);
            l->addWidget(row);
        }
    }
    l->addStretch();
}

void MainWindow::buildOptimizePanel()
{
    auto *l = new QVBoxLayout(m_panelOptimize->contentWidget());
    l->setContentsMargins(10, 8, 10, 8);
    l->setSpacing(8);
    auto *grid = new QGridLayout;
    grid->setColumnStretch(1, 1);

    struct Row { const char *heading; QLabel **value; };
    for (auto [heading, value] : { Row{QT_TR_NOOP("Original size:"), &m_labelOrigSize},
                                   Row{QT_TR_NOOP("Optimized size:"), &m_labelOptSize},
                                   Row{QT_TR_NOOP("Original gzip:"), &m_labelOrigGzip},
                                   Row{QT_TR_NOOP("Optimized gzip:"), &m_labelOptGzip} }) {
        int row = grid->rowCount();
        grid->addWidget(new QLabel(tr(heading)), row, 0);
        *value = new QLabel(tr("\u2014"));
        grid->addWidget(*value, row, 1);
    }
    l->addLayout(grid);
    l->addStretch();
}

void MainWindow::buildColorsPanel()
{
    auto *l = new QVBoxLayout(m_panelColors->contentWidget());
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);

    auto *scroll = new QScrollArea;
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);

    auto *inner = new QWidget;
    m_colorsLayout = new QVBoxLayout(inner);
    m_colorsLayout->setContentsMargins(8, 6, 8, 6);
    m_colorsLayout->setSpacing(4);
    m_colorsLayout->addStretch();

    scroll->setWidget(inner);
    l->addWidget(scroll);

    // Placeholder colors until SVG parsing is wired up
    for (const QColor &c : { QColor("#C0392B"), QColor("#2980B9"),
                              QColor("#27AE60"), QColor("#F39C12") })
        addColorEntry(c);
}

void MainWindow::addColorEntry(const QColor &color)
{
    auto *row = new QWidget;
    auto *hl  = new QHBoxLayout(row);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(6);

    auto *swatch = new QPushButton;
    swatch->setFixedSize(24, 24);
    swatch->setFlat(true);
    swatch->setCursor(Qt::PointingHandCursor);

    auto *hexEdit = new QLineEdit;
    hexEdit->setFixedWidth(72);
    hexEdit->setValidator(new QRegularExpressionValidator(
        QRegularExpression("^#[0-9A-Fa-f]{0,6}$"), hexEdit));
    hexEdit->setMaxLength(7);

    auto updateSwatch = [swatch](const QColor &c) {
        swatch->setStyleSheet(QString("background-color: %1; border: 1px solid palette(mid);")
                              .arg(c.name()));
    };

    updateSwatch(color);
    hexEdit->setText(color.name().toUpper());

    connect(swatch, &QPushButton::clicked, this, [=]() mutable {
        QColor current(hexEdit->text());
        QColor picked = QColorDialog::getColor(current.isValid() ? current : Qt::white, this);
        if (!picked.isValid()) return;
        updateSwatch(picked);
        hexEdit->setText(picked.name().toUpper());
    });

    connect(hexEdit, &QLineEdit::textEdited, this, [=](const QString &text) {
        if (text.length() == 7) {
            QColor c(text);
            if (c.isValid()) updateSwatch(c);
        }
    });

    hl->addWidget(swatch);
    hl->addWidget(hexEdit);
    hl->addStretch();

    // Insert before the trailing stretch
    m_colorsLayout->insertWidget(m_colorsLayout->count() - 1, row);
}

void MainWindow::buildExportPanel()
{
    auto *l = new QVBoxLayout(m_panelExport->contentWidget());
    l->setContentsMargins(10, 8, 10, 8);
    l->setSpacing(8);
    l->addWidget(new QLabel(tr("Format / Preset:")));
    auto *combo = new QComboBox;
    combo->addItems({ tr("SVG"), tr("PNG"), tr("PDF") });
    l->addWidget(combo);
    l->addWidget(new QPushButton(tr("Export")));
    l->addStretch();
}
