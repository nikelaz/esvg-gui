#pragma once
#include <QWidget>
#include <QFrame>
#include <QLabel>

class SidebarPanel : public QWidget
{
    Q_OBJECT
public:
    explicit SidebarPanel(const QString &title, QWidget *parent = nullptr);
    QWidget *contentWidget() const { return m_content; }

private:
    QFrame  *m_header  = nullptr;
    QWidget *m_content = nullptr;
};
