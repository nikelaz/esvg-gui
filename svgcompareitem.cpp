#include "svgcompareitem.h"
#include <QtSvg/QSvgRenderer>
#include <QPainter>
#include <QRectF>
#include <cmath>

SvgCompareItem::SvgCompareItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
    setAntialiasing(true);
}

SvgCompareItem::~SvgCompareItem() = default;

void SvgCompareItem::setSvgA(const QByteArray &data)
{
    if (m_svgA == data) return;
    m_svgA = data;
    delete m_rendererA;
    m_rendererA = nullptr;
    if (!data.isEmpty()) {
        m_rendererA = new QSvgRenderer(data, this);
    }
    // Reset zoom/pan when A (the primary SVG) changes
    m_zoom = 1.0;
    m_panOffset = {};
    emit svgAChanged();
    emit zoomChanged();
    emit panOffsetChanged();
    update();
}

void SvgCompareItem::setSvgB(const QByteArray &data)
{
    if (m_svgB == data) return;
    m_svgB = data;
    delete m_rendererB;
    m_rendererB = nullptr;
    if (!data.isEmpty()) {
        m_rendererB = new QSvgRenderer(data, this);
    }
    emit svgBChanged();
    update();
}

void SvgCompareItem::setSplitRatio(qreal r)
{
    r = qBound(0.0, r, 1.0);
    if (qFuzzyCompare(m_splitRatio, r)) return;
    m_splitRatio = r;
    emit splitRatioChanged();
    update();
}

void SvgCompareItem::setZoom(qreal z)
{
    z = qMax(0.01, z);
    if (qFuzzyCompare(m_zoom, z)) return;
    m_zoom = z;
    emit zoomChanged();
    update();
}

void SvgCompareItem::setPanOffset(const QPointF &p)
{
    if (m_panOffset == p) return;
    m_panOffset = p;
    emit panOffsetChanged();
    update();
}

QString SvgCompareItem::zoomText() const
{
    return QString::number(qRound(m_zoom * 100)) + "%";
}

qreal SvgCompareItem::fitScale() const
{
    if (!m_rendererA || !m_rendererA->isValid()) return 1.0;
    QSizeF svgSize = m_rendererA->defaultSize();
    if (svgSize.isEmpty()) return 1.0;
    qreal w = this->width();
    qreal h = this->height();
    if (w <= 0 || h <= 0) return 1.0;
    return qMin(w / svgSize.width(), h / svgSize.height());
}

QRectF SvgCompareItem::targetRect() const
{
    if (!m_rendererA || !m_rendererA->isValid()) return {};
    QSizeF svgSize = m_rendererA->defaultSize();
    if (svgSize.isEmpty()) return {};

    qreal scale = fitScale() * m_zoom;
    qreal sw = svgSize.width()  * scale;
    qreal sh = svgSize.height() * scale;

    // Center in item, then apply pan
    qreal x = (this->width()  - sw) / 2.0 + m_panOffset.x();
    qreal y = (this->height() - sh) / 2.0 + m_panOffset.y();

    return QRectF(x, y, sw, sh);
}

void SvgCompareItem::paint(QPainter *painter)
{
    painter->fillRect(boundingRect(), Qt::black);

    if (!m_rendererA && !m_rendererB) return;

    QRectF tr = targetRect();
    if (tr.isEmpty()) return;

    qreal splitX = width() * m_splitRatio;

    // Draw A (original) on the left half
    if (m_rendererA && m_rendererA->isValid()) {
        painter->save();
        painter->setClipRect(QRectF(0, 0, splitX, height()));
        m_rendererA->render(painter, tr);
        painter->restore();
    }

    // Draw B (optimized) on the right half
    if (m_rendererB && m_rendererB->isValid()) {
        painter->save();
        painter->setClipRect(QRectF(splitX, 0, width() - splitX, height()));
        m_rendererB->render(painter, tr);
        painter->restore();
    }

    // Divider line
    painter->save();
    painter->setPen(QPen(Qt::white, 2));
    painter->drawLine(QPointF(splitX, 0), QPointF(splitX, height()));
    painter->restore();
}

void SvgCompareItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    update();
}

void SvgCompareItem::zoomIn()
{
    setZoom(m_zoom * 1.15);
}

void SvgCompareItem::zoomOut()
{
    setZoom(m_zoom / 1.15);
}

void SvgCompareItem::resetZoom()
{
    m_zoom = 1.0;
    m_panOffset = {};
    emit zoomChanged();
    emit panOffsetChanged();
    update();
}

void SvgCompareItem::setZoomPercent(int pct)
{
    if (pct <= 0) return;
    setZoom(pct / 100.0);
}

void SvgCompareItem::zoomAroundPoint(qreal x, qreal y, qreal factor)
{
    // Compute the scene-space point under the cursor before zoom
    QRectF before = targetRect();
    if (before.isEmpty()) {
        setZoom(m_zoom * factor);
        return;
    }

    qreal newZoom = qMax(0.01, m_zoom * factor);
    qreal scaleChange = newZoom / m_zoom;

    // The target rect will scale around its center; we need to compensate pan
    // so the point (x,y) stays fixed.
    // targetRect center = (cx, cy)
    // After scale: point moves by (point - center) * (scaleChange - 1)
    qreal cx = before.center().x();
    qreal cy = before.center().y();
    qreal dx = (x - cx) * (scaleChange - 1.0);
    qreal dy = (y - cy) * (scaleChange - 1.0);

    m_zoom = newZoom;
    m_panOffset -= QPointF(dx, dy);
    emit zoomChanged();
    emit panOffsetChanged();
    update();
}
