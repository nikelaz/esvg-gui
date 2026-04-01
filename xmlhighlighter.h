#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class XmlHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit XmlHighlighter(QTextDocument *parent);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct Rule { QRegularExpression pattern; QTextCharFormat format; };
    QList<Rule>        m_rules;
    QTextCharFormat    m_commentFormat;
    QRegularExpression m_commentStart;
    QRegularExpression m_commentEnd;
};
