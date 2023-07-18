#ifndef ST_H
#define ST_H

#include <QObject>
#include <QSocketNotifier>
#include <QString>

#include <sys/ioctl.h>

#include "st-utils.h"

class SimpleTerminal : public QObject
{
    Q_OBJECT
public:
    Term term;
    Selection selection;

    explicit SimpleTerminal(QObject *parent = nullptr);
    ~SimpleTerminal() override;

    void termNew(int col, int row);
    void ttyNew();
    void execsh();
    void ttyClose();
    void termResize(int col, int row);
    void ttyResize(int tw, int th);
    int termWrite(const char *buf, int size, int show_ctrl);
    void ttyWrite(const char *s, size_t n, int may_echo);
    void ttyWriteRaw(const char *s, size_t n);
    void kScrollUp(int n);
    void kScrollDown(int n);
    void termDumpSelection();
    char *getSelection();
    void selectionStart(int col, int row, int snap);
    void selectionExtEnd(int col, int row, int type, int done);
    void selectionSnap(int *x, int *y, int direction);
    int selected(int x, int y);
    void selectionClear();
    void selectionScroll(int orig, int n);

public slots:
    size_t ttyRead();

signals:
    void sendError(QString);
    void sendClosed();
    void sendUpdateView(Term *state);

private:
    size_t utf8_decode(const char *c, Rune *u, size_t clen);
    Rune utf8_decodeByte(char c, size_t *i);
    size_t utf8_validate(Rune *u, size_t i);
    size_t utf8_encode(Rune u, char *c);
    char utf8_encodeByte(Rune u, size_t i);
    void termPutChar(Rune u);
    void termControlCode(uchar ascii);
    void termPutTab(int n);
    void termMoveTo(int x, int y);
    void termStrSequence(uchar c);
    void strReset();
    void termClearRegion(int x1, int y1, int x2, int y2);
    void selectionNormalize();
    void strHandle();
    void strParse();
    void strDump();
    void termSetChar(Rune u, const Glyph *attr, int x, int y);
    void csiReset();
    void csiParse();
    void csiHandle();
    void csiDump();
    void termSetDirt(int top, int bot);
    void termDump();
    void termDumpLine(int n);
    int termLineLen(int y);
    void termNewLine(int first_col);
    void termInsertBlank(int n);
    void termInsertBlankLine(int n);
    void termScrollUp(int orig, int n, int copyHistory);
    void termScrollDown(int orig, int n, int copyHistory);
    void termSetScroll(int t, int b);
    void termCursor(int mode);
    void termMoveATo(int x, int y);
    void termDeleteChar(int n);
    void termDefineTranslation(char ascii);
    void termDecTest(char c);
    void termDefineUtf8(char ascii);
    void termDeleteLine(int n);
    int escHandle(uchar ascii);
    void termReset();
    void termFullDirt();
    void oscColorResponse(int index, int num);
    void redraw();
    void draw();
    char * base64Dec(const char *src);
    char base64DecGetChar(const char **src);
    void osc4_color_response(int num);
    void termSetAttr(const int *attr, int l);
    int32_t termDefineColor(const int *attr, int *npar, int l);
    void xSetMode(int set, unsigned int flags);
    void termSetMode(int priv, int set, const int *args, int narg);
    void termSwapScreen();
    void bell();

    // TODO
    void xsetsel(char *str);
    void xclipcopy(void);
    int xsetcolorname(int x, const char *name);
    int xgetcolor(int x, unsigned char *r, unsigned char *g, unsigned char *b);
    void xloadcols(void);
    int xsetcursor(int cursor);
    void tprinter(char *s, size_t len); // TODO

private:
    TermWindow m_win;
    winsize m_winSize;

    int m_master, m_slave;
    pid_t m_pid;

    char m_readBuf[BUFSIZ];
    int m_readBufPos = 0;
    int m_readBufSize = 0;

    QSocketNotifier *readNotifier;
    CSIEscape m_CSIEscapeSeq;
    STREscape m_STREscapeSeq;

    /* Default colors (color name index)
     * foreground, background, cursor, reverse cursor */
    unsigned int default_cs = 256;
    unsigned int default_rcs = 257;
    unsigned int default_fg = 258;
    unsigned int default_bg = 259;
    unsigned int default_cursor = 160; // no longer used

    const int tabSpaces = 8;

    /* allow certain non-interactive (insecure) window operations such as:
       setting the clipboard text */
    int allowWindowOps = 0;

    const char *vtiden = "\033[?6c";

    struct Utf8 {
        constexpr static uchar byte[UTF_SIZ + 1] = { 0x80, 0, 0xC0, 0xE0, 0xF0 };
        constexpr static uchar mask[UTF_SIZ + 1] = { 0xC0, 0x80, 0xE0, 0xF0, 0xF8 };
        constexpr static Rune min[UTF_SIZ + 1] = { 0, 0, 0x80, 0x800, 0x10000 };
        constexpr static Rune max[UTF_SIZ + 1] = { 0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF };
    };

};

#endif // ST_H
