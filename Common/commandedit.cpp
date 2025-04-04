/*
  Copyright © 2017 Hasan Yavuz Özderya

  This file is part of serialplot.

  serialplot is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  serialplot is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with serialplot.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QKeyEvent>

#include <QtDebug>

#include "commandedit.h"
// #include <QRegExpValidator>
#include <QRegularExpressionValidator>
#include <QRegularExpression>

#if QT_VERSION == QT_VERSION_CHECK(5, 15, 2)
class HexCommandValidator : public QRegExpValidator
#elif QT_VERSION == QT_VERSION_CHECK(6, 5, 3)
class HexCommandValidator : public QRegularExpressionValidator
#endif
{
public:
    explicit HexCommandValidator(QObject* parent = 0);
    QValidator::State validate(QString & input, int & pos) const;
};


#if QT_VERSION == QT_VERSION_CHECK(5, 15, 2)
HexCommandValidator::HexCommandValidator(QObject* parent) :QRegExpValidator(parent)
{
    QRegExp regExp("^(?:(?:[0-9A-F]{2}[ ])+(?:[0-9A-F]{2}))|(?:[0-9A-F]{2})$");
    setRegExp(regExp);
#elif QT_VERSION == QT_VERSION_CHECK(6, 5, 3)

HexCommandValidator::HexCommandValidator(QObject* parent) :QRegularExpressionValidator(parent)
{
    QRegularExpression regExp("^(?:(?:[0-9A-F]{2}[ ])+(?:[0-9A-F]{2}))|(?:[0-9A-F]{2})$");
    setRegularExpression(regExp);
#endif


}

QValidator::State HexCommandValidator::validate(QString & input, int & pos) const
{
    input = input.toUpper();

    // don't let pos to be altered at this stage
    int orgPos = pos;

#if QT_VERSION == QT_VERSION_CHECK(5, 15, 2)
    auto r = QRegExpValidator::validate(input, pos);
#elif QT_VERSION == QT_VERSION_CHECK(6, 5, 3)
    auto r = QRegularExpressionValidator::validate(input, pos);
#endif

    pos = orgPos;

    // try fixing up spaces
    if (r != QValidator::Acceptable)
    {
        input = input.replace(" ", "");
#if QT_VERSION == QT_VERSION_CHECK(5, 15, 2)
        input.replace(QRegExp("([0-9A-F]{2}(?!$))"), "\\1 ");
#elif QT_VERSION == QT_VERSION_CHECK(6, 5, 3)
        input.replace(QRegularExpression("([0-9A-F]{2}(?!$))"), "\\1 ");
#endif
        if (pos == input.size()-1) pos = input.size();

#if QT_VERSION == QT_VERSION_CHECK(5, 15, 2)
        r = QRegExpValidator::validate(input, pos);
#elif QT_VERSION == QT_VERSION_CHECK(6, 5, 3)
        r = QRegularExpressionValidator::validate(input, pos);
#endif

    }

    return r;
}

CommandEdit::CommandEdit(QWidget *parent) :
    QLineEdit(parent)
{
    hexValidator = new HexCommandValidator(this);

#if QT_VERSION == QT_VERSION_CHECK(5, 15, 2)
    asciiValidator = new QRegExpValidator(QRegExp("[\\x0000-\\x007F]+"), this);
#elif QT_VERSION == QT_VERSION_CHECK(6, 5, 3)
    asciiValidator = new QRegularExpressionValidator(QRegularExpression("[\\x00-\\x7F]+"), this);
    // asciiValidator = new QRegularExpressionValidator(QRegularExpression("[\\x0000-\\x007F]+"), this);
#endif


    ascii_mode = true;
    setValidator(asciiValidator);
}

CommandEdit::~CommandEdit()
{
    delete hexValidator;
}

static QString unEscape(QString str);
static QString escape(QString str);

void CommandEdit::setMode(bool ascii)
{
    ascii_mode = ascii;
    if (ascii)
    {
        setValidator(asciiValidator);

        auto hexText = text().remove(" ");
        // try patching HEX string in case of missing nibble so that
        // input doesn't turn into gibberish
        if (hexText.size() % 2 == 1)
        {
            hexText.replace(hexText.size()-1, 1, "3F"); // 0x3F = '?'
            qWarning() << "Broken byte in hex command is replaced. Check your command!";
        }

        setText(escape(QByteArray::fromHex(hexText.toLatin1())));
    }
    else
    {
        setValidator(hexValidator);
        setText(unEscape(text()).toLatin1().toHex());
    }
}

void CommandEdit::keyPressEvent(QKeyEvent * event)
{
    if (ascii_mode)
    {
        QLineEdit::keyPressEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Backspace && !hasSelectedText())
    {
        int cursor = cursorPosition();
        if (cursor != 0 && text()[cursor-1] == ' ')
        {
            setCursorPosition(cursor-1);
        }
    }

    QLineEdit::keyPressEvent(event);
}

QString CommandEdit::unEscapedText()
{
    return unEscape(text());
}

static QString unEscape(QString str)
{
    const QMap<QString, QString> replacements({
            {"\\\\", "\\"},
            {"\\n", "\n"},
            {"\\r", "\r"},
            {"\\t", "\t"}
        });

    QString result;

    int i = 0;
    while (i < str.size())
    {
        bool found = false;

        for (const auto &k : replacements.keys())
        {
            // has enough text left?
            if (str.size() - i < 1) continue;

            // try matching the key at current position
#if QT_VERSION == QT_VERSION_CHECK(5, 15, 2)
            // if (k == str.midRef(i, k.size()))
#elif QT_VERSION == QT_VERSION_CHECK(6, 5, 3)
            // if(k==str.mid(i,k.size()))
             if (k == QStringView(str).mid(i, k.size()))
#endif
            {
                // append replacement
                result += replacements[k];
                i += k.size();
                found = true;
                break; // skip other keys
            }
        }

        if (!found)
        {
            // append unmatched character
            result += str[i];
            i++;
        }
    }

    return result;
}

static QString escape(QString str)
{
    str.replace("\\", "\\\\");
    str.replace("\n", "\\n");
    str.replace("\r", "\\r");
    str.replace("\t", "\\t");
    return str;
}
