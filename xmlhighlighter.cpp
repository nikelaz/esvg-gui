#include "xmlhighlighter.h"

XmlHighlighter::XmlHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    QTextCharFormat tagFormat;
    tagFormat.setForeground(QColor("#569CD6"));
    m_rules.append({ QRegularExpression("</?[\\w:-]+"), tagFormat });

    QTextCharFormat attrFormat;
    attrFormat.setForeground(QColor("#9CDCFE"));
    m_rules.append({ QRegularExpression("\\b[\\w:-]+="), attrFormat });

    QTextCharFormat valueFormat;
    valueFormat.setForeground(QColor("#CE9178"));
    m_rules.append({ QRegularExpression("\"[^\"]*\""), valueFormat });

    QTextCharFormat punctFormat;
    punctFormat.setForeground(QColor("#808080"));
    m_rules.append({ QRegularExpression("[<>/]"), punctFormat });

    QTextCharFormat piFormat;
    piFormat.setForeground(QColor("#808080"));
    m_rules.append({ QRegularExpression("<\\?[^?]*\\?>"), piFormat });

    m_commentFormat.setForeground(QColor("#6A9955"));
    m_commentFormat.setFontItalic(true);
    m_commentStart = QRegularExpression("<!--");
    m_commentEnd   = QRegularExpression("-->");
}

void XmlHighlighter::highlightBlock(const QString &text)
{
    for (const Rule &rule : m_rules) {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            auto match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);
    int start = (previousBlockState() == 1) ? 0 : text.indexOf(m_commentStart);

    while (start >= 0) {
        auto endMatch = m_commentEnd.match(text, start);
        int length;
        if (endMatch.hasMatch()) {
            length = endMatch.capturedStart() - start + endMatch.capturedLength();
            setCurrentBlockState(0);
        } else {
            setCurrentBlockState(1);
            length = text.length() - start;
        }
        setFormat(start, length, m_commentFormat);
        start = text.indexOf(m_commentStart, start + length);
    }
}
