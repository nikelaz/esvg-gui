#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QPoint>
#include <QByteArray>
#include <QCheckBox>
#include <QSlider>
#include "sidebarpanel.h"
#include "esvg_rs.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QGraphicsSvgItem;
class QGraphicsRectItem;
class QSvgRenderer;
class QLabel;
class QLineEdit;
class QVBoxLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void openFile();
    void reoptimize();

private:
    void applyZoom(double factor);
    void updateZoomDisplay();
    void repositionZoomControls();
    void updateComparisonClip();
    void repositionSliderHandle();

    // Sidebar
    void buildSidebar();
    void buildPluginsPanel();
    void buildOptimizePanel();
    void buildColorsPanel();
    void buildExportPanel();
    void addColorEntry(const QColor &color);

    Ui::MainWindow *ui;
    QGraphicsScene    *m_scene;
    QGraphicsSvgItem  *m_svgItem        = nullptr;
    QGraphicsSvgItem  *m_svgItemB       = nullptr;
    QSvgRenderer      *m_rendererB      = nullptr;
    QGraphicsRectItem *m_clipContainerA = nullptr;
    QGraphicsRectItem *m_clipContainer  = nullptr;
    QWidget           *m_zoomControls   = nullptr;
    QLineEdit         *m_zoomInput      = nullptr;
    QWidget           *m_sliderHandle   = nullptr;
    double             m_fitScale       = 1.0;
    int                m_sliderViewX    = 0;
    QPoint             m_lastPanPos;
    bool               m_panning        = false;
    bool               m_draggingSlider = false;
    QString            m_svgPath;
    QByteArray         m_svgBytes;
    bool               m_optimizing     = false;
    QCheckBox         *m_pluginChecks[ESVG_PLUGIN_COUNT] = {};
    QSlider           *m_precisionSlider = nullptr;

    SidebarPanel *m_panelPlugins  = nullptr;
    SidebarPanel *m_panelOptimize = nullptr;
    SidebarPanel *m_panelColors   = nullptr;
    SidebarPanel *m_panelExport   = nullptr;
    QVBoxLayout    *m_colorsLayout      = nullptr;
    QLabel *m_labelOrigSize = nullptr;
    QLabel *m_labelOptSize  = nullptr;
    QLabel *m_labelOrigGzip = nullptr;
    QLabel *m_labelOptGzip  = nullptr;
};
#endif // MAINWINDOW_H
