#include "exportdialog.h"
#include <QtSvgWidgets/QGraphicsSvgItem>
#include <QGraphicsView>
#include <QOpenGLWidget>
#include <QSurfaceFormat>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTimer>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>

ExportDialog::ExportDialog(const QString &svgPath, QWidget *parent)
    : QDialog(parent), m_svgPath(svgPath)
{
    setWindowTitle(tr("Export"));
    resize(900, 600);

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    buildPreview(svgPath);
    root->addWidget(m_view, 1);
    root->addWidget(buildSidebar());
}

void ExportDialog::buildPreview(const QString &svgPath)
{
    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(-5000, -5000, 10000, 10000);

    m_view = new QGraphicsView;
    m_view->setScene(m_scene);

    QSurfaceFormat fmt;
    fmt.setSamples(8);
    auto *gl = new QOpenGLWidget;
    gl->setFormat(fmt);
    m_view->setViewport(gl);

    m_view->setDragMode(QGraphicsView::NoDrag);
    m_view->setTransformationAnchor(QGraphicsView::NoAnchor);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    m_svgItem = new QGraphicsSvgItem(svgPath);
    m_svgItem->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    m_scene->addItem(m_svgItem);

    m_view->viewport()->setMouseTracking(true);
    m_view->viewport()->installEventFilter(this);

    QTimer::singleShot(0, this, [this]{
        m_view->fitInView(m_svgItem, Qt::KeepAspectRatio);
        m_fitScale = m_view->transform().m11();
    });
}

QWidget *ExportDialog::buildSidebar()
{
    auto *w = new QWidget;
    w->setFixedWidth(240);
    auto *l = new QVBoxLayout(w);
    l->setContentsMargins(12, 12, 12, 12);
    l->setSpacing(6);

    l->addWidget(new QLabel(tr("Format")));

    m_radioSvg   = new QRadioButton(tr("SVG"));
    m_radioPng   = new QRadioButton(tr("PNG"));
    m_radioReact = new QRadioButton(tr("React Component"));
    m_radioCss   = new QRadioButton(tr("CSS URL Encoded Background"));
    m_radioPdf   = new QRadioButton(tr("PDF"));
    m_radioSvg->setChecked(true);
    for (auto *r : { m_radioSvg, m_radioPng, m_radioReact, m_radioCss, m_radioPdf })
        l->addWidget(r);

    l->addSpacing(8);

    m_viewBoxCheck = new QCheckBox(tr("Include viewBox"));
    m_viewBoxCheck->setChecked(true);
    l->addWidget(m_viewBoxCheck);

    m_sizeCheck = new QCheckBox(tr("Custom size"));
    l->addWidget(m_sizeCheck);

    m_sizeInputs = new QWidget;
    auto *sizeGrid = new QGridLayout(m_sizeInputs);
    sizeGrid->setContentsMargins(0, 4, 0, 0);
    sizeGrid->setColumnStretch(1, 1);
    m_widthSpin  = new QSpinBox; m_widthSpin->setRange(1, 99999);  m_widthSpin->setSuffix(tr(" px"));
    m_heightSpin = new QSpinBox; m_heightSpin->setRange(1, 99999); m_heightSpin->setSuffix(tr(" px"));
    sizeGrid->addWidget(new QLabel(tr("W:")), 0, 0);
    sizeGrid->addWidget(m_widthSpin,          0, 1);
    sizeGrid->addWidget(new QLabel(tr("H:")), 1, 0);
    sizeGrid->addWidget(m_heightSpin,         1, 1);
    m_sizeInputs->setVisible(false);
    l->addWidget(m_sizeInputs);

    connect(m_sizeCheck, &QCheckBox::toggled, m_sizeInputs, &QWidget::setVisible);

    l->addStretch();

    auto *exportBtn = new QPushButton(tr("Export"));
    exportBtn->setDefault(true);
    l->addWidget(exportBtn);
    connect(exportBtn, &QPushButton::clicked, this, &ExportDialog::onExport);

    return w;
}

void ExportDialog::onExport()
{
    QString filter;
    if      (m_radioSvg->isChecked())   filter = tr("SVG Files (*.svg)");
    else if (m_radioPng->isChecked())   filter = tr("PNG Images (*.png)");
    else if (m_radioReact->isChecked()) filter = tr("JavaScript Files (*.jsx *.tsx)");
    else if (m_radioCss->isChecked())   filter = tr("CSS Files (*.css)");
    else if (m_radioPdf->isChecked())   filter = tr("PDF Files (*.pdf)");

    const QString path = QFileDialog::getSaveFileName(this, tr("Export"), QString(), filter);
    if (path.isEmpty()) return;

    // TODO: wire actual export logic per format
    accept();
}

bool ExportDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_view->viewport())
        return QDialog::eventFilter(watched, event);

    switch (event->type()) {
    case QEvent::Wheel: {
        auto *we = static_cast<QWheelEvent *>(event);
        const double factor = (we->angleDelta().y() > 0) ? 1.15 : (1.0 / 1.15);
        const QPointF cursorViewPos  = we->position();
        const QPointF cursorScenePos = m_view->mapToScene(cursorViewPos.toPoint());
        m_view->scale(factor, factor);
        const QPointF delta = m_view->mapFromScene(cursorScenePos) - cursorViewPos;
        m_view->horizontalScrollBar()->setValue(m_view->horizontalScrollBar()->value() + qRound(delta.x()));
        m_view->verticalScrollBar()->setValue(  m_view->verticalScrollBar()->value()   + qRound(delta.y()));
        return true;
    }
    case QEvent::MouseButtonPress: {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton) {
            m_lastPanPos = me->pos();
            m_panning = true;
            m_view->setCursor(Qt::ClosedHandCursor);
            return true;
        }
        break;
    }
    case QEvent::MouseMove: {
        auto *me = static_cast<QMouseEvent *>(event);
        if (m_panning) {
            const QPoint delta = me->pos() - m_lastPanPos;
            m_lastPanPos = me->pos();
            m_view->horizontalScrollBar()->setValue(m_view->horizontalScrollBar()->value() - delta.x());
            m_view->verticalScrollBar()->setValue(  m_view->verticalScrollBar()->value()   - delta.y());
            return true;
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton) {
            m_panning = false;
            m_view->setCursor(Qt::ArrowCursor);
            return true;
        }
        break;
    }
    default: break;
    }
    return QDialog::eventFilter(watched, event);
}
