#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>
#include <QQuickStyle>
#include "optimizationcontroller.h"
#include "exportcontroller.h"
#include "svgcompareitem.h"

int main(int argc, char *argv[])
{
    QSurfaceFormat fmt;
    fmt.setSamples(8);
    QSurfaceFormat::setDefaultFormat(fmt);

    // QApplication (not QGuiApplication) so native QFileDialog/QColorDialog work
    QApplication app(argc, argv);

    QQuickStyle::setStyle("Fusion");

    qmlRegisterType<SvgCompareItem>("EsvgGui", 1, 0, "SvgCompareItem");

    OptimizationController controller;
    ExportController exportCtrl;

    // Keep exportCtrl bytes in sync with the latest optimized SVG
    QObject::connect(&controller, &OptimizationController::optimizedSvgBytesChanged,
                     [&]{ exportCtrl.setSvgBytes(controller.optimizedSvgBytes()); });

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("controller", &controller);
    engine.rootContext()->setContextProperty("exportCtrl", &exportCtrl);

    const QUrl url(u"qrc:/EsvgGui/qml/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
