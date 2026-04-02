#pragma once

#include <QQuickPaintedItem>
#include <QByteArray>
#include <QPointF>
#include <QString>

class QSvgRenderer;

class SvgCompareItem : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(QByteArray svgA       READ svgA       WRITE setSvgA       NOTIFY svgAChanged)
    Q_PROPERTY(QByteArray svgB       READ svgB       WRITE setSvgB       NOTIFY svgBChanged)
    Q_PROPERTY(qreal      splitRatio READ splitRatio WRITE setSplitRatio NOTIFY splitRatioChanged)
    Q_PROPERTY(qreal      zoom       READ zoom       WRITE setZoom       NOTIFY zoomChanged)
    Q_PROPERTY(QPointF    panOffset  READ panOffset  WRITE setPanOffset  NOTIFY panOffsetChanged)
    Q_PROPERTY(QString    zoomText   READ zoomText                       NOTIFY zoomChanged)

public:
    explicit SvgCompareItem(QQuickItem *parent = nullptr);
    ~SvgCompareItem() override;

    QByteArray svgA()       const { return m_svgA; }
    QByteArray svgB()       const { return m_svgB; }
    qreal      splitRatio() const { return m_splitRatio; }
    qreal      zoom()       const { return m_zoom; }
    QPointF    panOffset()  const { return m_panOffset; }
    QString    zoomText()   const;

    void setSvgA(const QByteArray &data);
    void setSvgB(const QByteArray &data);
    void setSplitRatio(qreal r);
    void setZoom(qreal z);
    void setPanOffset(const QPointF &p);

    void paint(QPainter *painter) override;

    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();
    Q_INVOKABLE void resetZoom();
    Q_INVOKABLE void setZoomPercent(int pct);
    Q_INVOKABLE void zoomAroundPoint(qreal x, qreal y, qreal factor);

signals:
    void svgAChanged();
    void svgBChanged();
    void splitRatioChanged();
    void zoomChanged();
    void panOffsetChanged();

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    QRectF  targetRect() const;
    qreal   fitScale()   const;

    QSvgRenderer *m_rendererA = nullptr;
    QSvgRenderer *m_rendererB = nullptr;
    QByteArray    m_svgA;
    QByteArray    m_svgB;
    qreal         m_zoom       = 1.0;
    QPointF       m_panOffset;
    qreal         m_splitRatio = 0.5;
};
