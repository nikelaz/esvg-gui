#pragma once
#include <QDialog>
#include <QGraphicsScene>
#include <QPoint>

class QGraphicsView;
class QGraphicsSvgItem;
class QRadioButton;
class QCheckBox;
class QSpinBox;
class QWidget;

class ExportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ExportDialog(const QString &svgPath, QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void     buildPreview(const QString &svgPath);
    QWidget *buildSidebar();
    void     onExport();

    QGraphicsView    *m_view    = nullptr;
    QGraphicsScene   *m_scene   = nullptr;
    QGraphicsSvgItem *m_svgItem = nullptr;
    double            m_fitScale = 1.0;
    QPoint            m_lastPanPos;
    bool              m_panning = false;

    QRadioButton *m_radioSvg   = nullptr;
    QRadioButton *m_radioPng   = nullptr;
    QRadioButton *m_radioReact = nullptr;
    QRadioButton *m_radioCss   = nullptr;
    QRadioButton *m_radioPdf   = nullptr;

    QCheckBox *m_viewBoxCheck = nullptr;
    QCheckBox *m_sizeCheck    = nullptr;
    QWidget   *m_sizeInputs   = nullptr;
    QSpinBox  *m_widthSpin    = nullptr;
    QSpinBox  *m_heightSpin   = nullptr;

    QString m_svgPath;
};
