#include "sidebarpanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

SidebarPanel::SidebarPanel(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_header = new QFrame(this);
    m_header->setFixedHeight(26);
    m_header->setFrameShape(QFrame::NoFrame);
    QPalette pal = m_header->palette();
    pal.setColor(QPalette::Window, pal.color(QPalette::Button));
    m_header->setAutoFillBackground(true);
    m_header->setPalette(pal);

    auto *hl = new QHBoxLayout(m_header);
    hl->setContentsMargins(8, 0, 8, 0);
    auto *titleLabel = new QLabel(title, m_header);
    QFont f = titleLabel->font();
    f.setWeight(QFont::Medium);
    titleLabel->setFont(f);
    hl->addWidget(titleLabel);

    root->addWidget(m_header);

    m_content = new QWidget(this);
    m_content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    root->addWidget(m_content);
}
