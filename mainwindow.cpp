#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QtSvgWidgets/QGraphicsSvgItem>
#include <QGraphicsRectItem>
#include <QOpenGLWidget>
#include <QSurfaceFormat>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTimer>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scene(new QGraphicsScene(this))
{
    ui->setupUi(this);

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

    m_svgItem = new QGraphicsSvgItem(":/test-svg.svg", m_clipContainerA);
    m_svgItem->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    // SVG B — comparison (clipped to right of slider)
    m_clipContainer = new QGraphicsRectItem();
    m_clipContainer->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
    m_clipContainer->setPen(Qt::NoPen);
    m_clipContainer->setBrush(Qt::NoBrush);
    m_scene->addItem(m_clipContainer);

    m_svgItemB = new QGraphicsSvgItem(":/test-svg.svg", m_clipContainer);
    m_svgItemB->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    // Slider divider handle
    m_sliderHandle = new QWidget(ui->svgView);
    m_sliderHandle->setStyleSheet("background: white;");
    m_sliderHandle->setFixedWidth(2);
    m_sliderHandle->setCursor(Qt::SizeHorCursor);
    m_sliderHandle->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_sliderHandle->raise();

    // Zoom overlay
    m_zoomControls = new QWidget(ui->svgView);

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

    ui->svgView->viewport()->setMouseTracking(true);
    ui->svgView->viewport()->installEventFilter(this);

    QTimer::singleShot(0, this, [this]() {
        ui->svgView->fitInView(m_svgItem, Qt::KeepAspectRatio);
        m_fitScale = ui->svgView->transform().m11();
        updateZoomDisplay();
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

    delete m_svgItem;

    m_svgItem = new QGraphicsSvgItem(path, m_clipContainerA);
    m_svgItem->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    ui->svgView->resetTransform();
    ui->svgView->fitInView(m_svgItem, Qt::KeepAspectRatio);
    m_fitScale = ui->svgView->transform().m11();
    updateZoomDisplay();
    updateComparisonClip();
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
