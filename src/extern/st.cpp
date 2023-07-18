#include "st.h"
#include <pwd.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include <QApplication>
#include <QString>

#if defined(__linux)
#include <pty.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#include <util.h>
#elif defined(__FreeBSD__) || defined(__DragonFly__)
#include <libutil.h>
#endif

SimpleTerminal::SimpleTerminal(QObject *parent)
    : QObject(parent)
{
    m_readBufSize = sizeof(m_readBuf) / sizeof(m_readBuf[0]);

    termNew(80, 80);
    ttyNew();

    readNotifier = new QSocketNotifier(m_master, QSocketNotifier::Read);
    readNotifier->setEnabled(true);

    connect(readNotifier, &QSocketNotifier::activated, this, &SimpleTerminal::ttyRead);

    // Fix for Zorin OS (error: invalid old space)
    // Needed since we only call realloc later
    m_STREscapeSeq.buf = (char *)malloc(STR_BUF_SIZ);
}

SimpleTerminal::~SimpleTerminal()
{
    readNotifier->disconnect();

    for (int i = 0; i <= term.row; i++) {
        free(term.line[i]);
        free(term.alt[i]);
    }

    free(term.dirty);
    free(term.tabs);
    free(m_STREscapeSeq.buf);

    readNotifier->deleteLater();
}

void
SimpleTerminal::termNew(int col, int row)
{
    term = (Term){ .c = { .attr = {
                              .u = default_cursor,
                              .fg = default_fg,
                              .bg = default_bg,
                          } } };
    termResize(col, row);
    termReset();
}

void
SimpleTerminal::ttyClose()
{
    if (m_slave > 1) {
        ::close(m_slave);
        m_slave = -1;
    }

    if (m_master > -1) {
        ::close(m_master);
        m_master = -1;
    }

    emit sendClosed();
}

void
SimpleTerminal::ttyNew()
{
    m_master = -1;
    m_slave = -1;

    /* seems to work fine on linux, openbsd and freebsd */
    if (::openpty(&m_master, &m_slave, NULL, NULL, NULL) < 0) {
        emit sendError("Could not open new file descriptor.");
        return;
    }

    dup(STDOUT_FILENO);
    dup2(m_slave, STDOUT_FILENO);

    switch (m_pid = fork()) {
    case -1:
        ttyClose();
        emit sendError("Could not fork process.");
        return;
        break;
    case 0:
        ::close(m_master);
        ::setsid(); /* create a new process group */
        ::dup2(m_slave, 0);
        ::dup2(m_slave, 1);
        ::dup2(m_slave, 2);
        if (::ioctl(m_slave, TIOCSCTTY, NULL) < 0) {
            ttyClose();
            emit sendError("Error setting the controlling terminal.");
            return;
        }
        if (m_slave > 2)
            ::close(m_slave);
#ifdef __OpenBSD__
        if (::pledge("stdio getpw proc exec", NULL) == -1) {
            closePty();
            emit s_error("Error on pledge.");
            return;
        }
#endif
        execsh();
        break;
    default:
#ifdef __OpenBSD__
        if (::pledge("stdio rpath tty proc", NULL) == -1) {
            closePty();
            emit s_error("Error on pledge.");
            return;
        }
#endif
        ::close(m_slave);
        break;
    }
}

void
SimpleTerminal::execsh()
{
    const struct passwd *pw;

    errno = 0;
    if ((pw = ::getpwuid(getuid())) == NULL) {
        if (errno) {
            ttyClose();
            emit sendError("Error on getpwuid.");
            return;
        } else {
            ttyClose();
            emit sendError("Could not retrieve user identity.");
            return;
        }
    }

    QByteArray shell = qgetenv("SHELL");
    if (shell.size() == 0) {
        shell = (pw->pw_shell[0]) ? pw->pw_shell : "/bin/sh";
    }

    ::unsetenv("COLUMNS");
    ::unsetenv("LINES");
    ::unsetenv("TERMCAP");
    ::setenv("LOGNAME", pw->pw_name, 1);
    ::setenv("USER", pw->pw_name, 1);
    ::setenv("SHELL", shell, 1);
    ::setenv("HOME", pw->pw_dir, 1);
    // TODO figure out a way to use tic command with custom term info file
    ::setenv("TERM", "xterm-256color", 1);

    char *args[] = { shell.data(), NULL, NULL };

    ::execvp(shell.constData(), args);
}

size_t
SimpleTerminal::ttyRead()
{
    int ret, written;

    /* append read bytes to unprocessed bytes */
    ret = ::read(m_master, m_readBuf + m_readBufPos, m_readBufSize - m_readBufPos);

    switch (ret) {
    case 0:
        return 0;
    case -1:
        ttyClose();
        emit sendError("Could not read from shell.");
        return 0;
    default:
        m_readBufPos += ret;
        written = termWrite(m_readBuf, m_readBufPos, 0);
        m_readBufPos -= written;
        /* keep any incomplete UTF-8 byte sequence for the next call */
        if (m_readBufPos > 0) {
            ::memmove(m_readBuf, m_readBuf + written, m_readBufPos);
        }

        emit sendUpdateView(&term);
        return ret;
    }
}

void
SimpleTerminal::termResize(int col, int row)
{
    int i, j;
    int minRow = MIN(row, term.row);
    int minCol = MIN(col, term.col);
    int *bp;
    TCursor c;

    if (col < 1 || row < 1) {
        emit sendError("termResize: error resizing to x: " + QString::number(col) +
                       ", y: " + QString::number(row));
        return;
    }

    /*
     * slide screen to keep cursor where we expect it -
     * tscrollup would work here, but we can optimize to
     * memmove because we're freeing the earlier lines
     */
    for (i = 0; i <= term.c.y - row; i++) {
        free(term.line[i]);
        free(term.alt[i]);
    }
    /* ensure that both src and dst are not NULL */
    if (i > 0) {
        memmove(term.line, term.line + i, row * sizeof(Line));
        memmove(term.alt, term.alt + i, row * sizeof(Line));
    }
    for (i += row; i < term.row; i++) {
        free(term.line[i]);
        free(term.alt[i]);
    }

    /* resize to new height */
    term.line = (Line *)realloc(term.line, row * sizeof(Line));
    term.alt = (Line *)realloc(term.alt, row * sizeof(Line));
    term.dirty = (int *)realloc(term.dirty, row * sizeof(*term.dirty));
    term.tabs = (int *)realloc(term.tabs, col * sizeof(*term.tabs));

    if (term.line == NULL || term.alt == NULL || term.dirty == NULL || term.tabs == NULL) {
        emit sendError("Error on resize");
        return;
    }

    for (i = 0; i < HISTSIZE; i++) {
        term.hist[i] = (Line)::realloc(term.hist[i], col * sizeof(Glyph));

        if (term.hist[i] == NULL) {
            emit sendError("Error on resize");
            return;
        }

        for (j = minCol; j < col; j++) {
            term.hist[i][j] = term.c.attr;
            term.hist[i][j].u = ' ';
        }
    }

    /* resize each row to new width, zero-pad if needed */
    for (i = 0; i < minRow; i++) {
        term.line[i] = (Glyph_ *)realloc(term.line[i], col * sizeof(Glyph));
        term.alt[i] = (Glyph_ *)realloc(term.alt[i], col * sizeof(Glyph));

        if (term.line[i] == NULL || term.alt[i] == NULL) {
            emit sendError("Error on resize");
            return;
        }
    }

    /* allocate any new rows */
    for (/* i = min row */; i < row; i++) {
        term.line[i] = (Glyph_ *)malloc(col * sizeof(Glyph));
        term.alt[i] = (Glyph_ *)malloc(col * sizeof(Glyph));

        if (term.line[i] == NULL || term.alt[i] == NULL) {
            emit sendError("Error on resize");
            return;
        }
    }
    if (col > term.col) {
        bp = term.tabs + term.col;

        memset(bp, 0, sizeof(*term.tabs) * (col - term.col));
        while (--bp > term.tabs && !*bp)
            /* nothing */;
        for (bp += tabSpaces; bp < term.tabs + col; bp += tabSpaces)
            *bp = 1;
    }
    /* update terminal size */
    term.col = col;
    term.row = row;
    /* reset scrolling region */
    termSetScroll(0, row - 1);
    /* make use of the LIMIT in tmoveto */
    termMoveTo(term.c.x, term.c.y);
    /* Clearing both screens (it makes dirty all lines) */
    c = term.c;
    for (i = 0; i < 2; i++) {
        if (minCol < col && 0 < minRow) {
            termClearRegion(minCol, 0, col - 1, minRow - 1);
        }
        if (0 < col && minRow < row) {
            termClearRegion(0, minRow, col - 1, row - 1);
        }
        termSwapScreen();
        termCursor(CURSOR_LOAD);
    }
    term.c = c;
}

void
SimpleTerminal::termPutChar(Rune u)
{
    char c[UTF_SIZ];
    int control;
    int width, len;
    Glyph *gp;

    control = ISCONTROL(u);
    if (u < 127 || !IS_SET(term.mode, MODE_UTF8)) {
        c[0] = u;
        width = len = 1;
    } else {
        len = utf8_encode(u, c);
        if (!control && (width = wcwidth(u)) == -1)
            width = 1;
    }

    if (IS_SET(term.mode, MODE_PRINT)) {
        tprinter(c, len);
    }

    /*
     * STR sequence must be checked before anything else
     * because it uses all following characters until it
     * receives a ESC, a SUB, a ST or any other C1 control
     * character.
     */
    if (term.esc & ESC_STR) {
        if (u == '\a' || u == 030 || u == 032 || u == 033 || ISCONTROLC1(u)) {
            term.esc &= ~(ESC_START | ESC_STR);
            term.esc |= ESC_STR_END;
            goto check_control_code;
        }

        if (m_STREscapeSeq.len + len >= m_STREscapeSeq.siz) {
            /*
             * Here is a bug in terminals. If the user never sends
             * some code to stop the str or esc command, then st
             * will stop responding. But this is better than
             * silently failing with unknown characters. At least
             * then users will report back.
             *
             * In the case users ever get fixed, here is the code:
             */
            /*
             * term.esc = 0;
             * strhandle();
             */
            if (m_STREscapeSeq.siz > (SIZE_MAX - UTF_SIZ) / 2)
                return;
            m_STREscapeSeq.siz *= 2;
            m_STREscapeSeq.buf = (char *)realloc(m_STREscapeSeq.buf, m_STREscapeSeq.siz);

            if (m_STREscapeSeq.buf == NULL) {
                emit sendError("Could not realloc buffer.");
                return;
            }
        }

        memmove(&m_STREscapeSeq.buf[m_STREscapeSeq.len], c, len);
        m_STREscapeSeq.len += len;
        return;
    }

check_control_code:
    /*
     * Actions of control codes must be performed as soon they arrive
     * because they can be embedded inside a control sequence, and
     * they must not cause conflicts with sequences.
     */
    if (control) {
        termControlCode(u);
        /*
         * control codes are not shown ever
         */
        if (!term.esc)
            term.lastc = 0;
        return;
    } else if (term.esc & ESC_START) {
        if (term.esc & ESC_CSI) {
            m_CSIEscapeSeq.buf[m_CSIEscapeSeq.len++] = u;
            if (BETWEEN(u, 0x40, 0x7E) || m_CSIEscapeSeq.len >= sizeof(m_CSIEscapeSeq.buf) - 1) {
                term.esc = 0;
                csiParse();
                csiHandle();
            }
            return;
        } else if (term.esc & ESC_UTF8) {
            termDefineUtf8(u);
        } else if (term.esc & ESC_ALTCHARSET) {
            termDefineTranslation(u);
        } else if (term.esc & ESC_TEST) {
            termDecTest(u);
        } else {
            if (!escHandle(u))
                return;
            /* sequence already finished */
        }
        term.esc = 0;
        /*
         * All characters which form part of a sequence are not
         * printed
         */
        return;
    }
    if (selected(term.c.x, term.c.y))
        selectionClear();

    gp = &term.line[term.c.y][term.c.x];
    if (IS_SET(term.mode, MODE_WRAP) && (term.c.state & CURSOR_WRAPNEXT)) {
        gp->mode |= ATTR_WRAP;
        termNewLine(1);
        gp = &term.line[term.c.y][term.c.x];
    }

    if (IS_SET(term.mode, MODE_INSERT) && term.c.x + width < term.col)
        memmove(gp + width, gp, (term.col - term.c.x - width) * sizeof(Glyph));

    if (term.c.x + width > term.col) {
        termNewLine(1);
        gp = &term.line[term.c.y][term.c.x];
    }

    termSetChar(u, &term.c.attr, term.c.x, term.c.y);
    term.lastc = u;

    if (width == 2) {
        gp->mode |= ATTR_WIDE;
        if (term.c.x + 1 < term.col) {
            if (gp[1].mode == ATTR_WIDE && term.c.x + 2 < term.col) {
                gp[2].u = ' ';
                gp[2].mode &= ~ATTR_WDUMMY;
            }
            gp[1].u = '\0';
            gp[1].mode = ATTR_WDUMMY;
        }
    }
    if (term.c.x + width < term.col) {
        termMoveTo(term.c.x + width, term.c.y);
    } else {
        term.c.state |= CURSOR_WRAPNEXT;
    }
}

int
SimpleTerminal::termWrite(const char *buf, int size, int show_ctrl)
{
    int charsize;
    Rune u;
    int n;

    for (n = 0; n < size; n += charsize) {
        if (IS_SET(term.mode, MODE_UTF8)) {
            /* process a complete utf8 char */
            charsize = utf8_decode(buf + n, &u, size - n);
            if (charsize == 0)
                break;
        } else {
            u = buf[n] & 0xFF;
            charsize = 1;
        }
        if (show_ctrl && ISCONTROL(u)) {
            if (u & 0x80) {
                u &= 0x7f;
                termPutChar('^');
                termPutChar('[');
            } else if (u != '\n' && u != '\r' && u != '\t') {
                u ^= 0x40;
                termPutChar('^');
            }
        }
        termPutChar(u);
    }
    return n;
}

void
SimpleTerminal::ttyWrite(const char *s, size_t n, int may_echo)
{
    const char *next;

    kScrollDown(term.scr);

    if (may_echo && IS_SET(term.mode, MODE_ECHO))
        termWrite(s, n, 1);

    if (!IS_SET(term.mode, MODE_CRLF)) {
        ttyWriteRaw(s, n);
        return;
    }

    /* This is similar to how the kernel handles ONLCR for ttys */
    while (n > 0) {
        if (*s == '\r') {
            next = s + 1;
            ttyWriteRaw("\r\n", 2);
        } else {
            next = (char *)memchr(s, '\r', n);
            DEFAULT(next, s + n);
            ttyWriteRaw(s, next - s);
        }
        n -= next - s;
        s = next;
    }
}

void
SimpleTerminal::ttyWriteRaw(const char *s, size_t n)
{
    fd_set wfd, rfd;
    ssize_t r;
    size_t lim = 256;

    /*
     * Remember that we are using a pty, which might be a modem line.
     * Writing too much will clog the line. That's why we are doing this
     * dance.
     * FIXME: Migrate the world to Plan 9.
     */
    while (n > 0) {
        FD_ZERO(&wfd);
        FD_ZERO(&rfd);
        FD_SET(m_master, &wfd);
        FD_SET(m_master, &rfd);

        /* Check if we can write. */
        if (pselect(m_master + 1, &rfd, &wfd, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) {
                continue;
            }
            emit sendError("Pselect failed in ttyWriteRaw");
            return;
        }
        if (FD_ISSET(m_master, &wfd)) {
            /*
             * Only write the bytes written by ttywrite() or the
             * default of 256. This seems to be a reasonable value
             * for a serial line. Bigger values might clog the I/O.
             */
            if ((r = write(m_master, s, (n < lim) ? n : lim)) < 0) {
                emit sendError("Error on write in ttyWriteRaw.");
                return;
            }
            if (r < n) {
                /*
                 * We weren't able to write out everything.
                 * This means the buffer is getting full
                 * again. Empty it.
                 */
                if (n < lim)
                    lim = ttyRead();
                n -= r;
                s += r;
            } else {
                /* All bytes have been written. */
                break;
            }
        }
        if (FD_ISSET(m_master, &rfd))
            lim = ttyRead();
    }
    return;
}

size_t
SimpleTerminal::utf8_decode(const char *c, Rune *u, size_t clen)
{
    size_t i, j, len, type;
    Rune udecoded;

    *u = UTF_INVALID;
    if (!clen)
        return 0;
    udecoded = utf8_decodeByte(c[0], &len);
    if (!BETWEEN(len, 1, UTF_SIZ))
        return 1;
    for (i = 1, j = 1; i < clen && j < len; ++i, ++j) {
        udecoded = (udecoded << 6) | utf8_decodeByte(c[i], &type);
        if (type != 0)
            return j;
    }
    if (j < len)
        return 0;
    *u = udecoded;
    utf8_validate(u, len);

    return len;
}

Rune
SimpleTerminal::utf8_decodeByte(char c, size_t *i)
{
    for (*i = 0; *i < LEN(utfmask); ++(*i))
        if (((uchar)c & utfmask[*i]) == utfbyte[*i])
            return (uchar)c & ~utfmask[*i];

    return 0;
}

size_t
SimpleTerminal::utf8_encode(Rune u, char *c)
{
    size_t len, i;

    len = utf8_validate(&u, 0);
    if (len > UTF_SIZ)
        return 0;

    for (i = len - 1; i != 0; --i) {
        c[i] = utf8_encodeByte(u, 0);
        u >>= 6;
    }
    c[0] = utf8_encodeByte(u, len);

    return len;
}

char
SimpleTerminal::utf8_encodeByte(Rune u, size_t i)
{
    return Utf8::byte[i] | (u & ~Utf8::mask[i]);
}

size_t
SimpleTerminal::utf8_validate(Rune *u, size_t i)
{
    if (!BETWEEN(*u, Utf8::min[i], Utf8::max[i]) || BETWEEN(*u, 0xD800, 0xDFFF))
        *u = UTF_INVALID;
    for (i = 1; *u > Utf8::max[i]; ++i)
        ;

    return i;
}

void
SimpleTerminal::termControlCode(uchar ascii)
{
    switch (ascii) {
    case '\t': /* HT */
        termPutTab(1);
        return;
    case '\b': /* BS */
        termMoveTo(term.c.x - 1, term.c.y);
        return;
    case '\r': /* CR */
        termMoveTo(0, term.c.y);
        return;
    case '\f': /* LF */
    case '\v': /* VT */
    case '\n': /* LF */
        /* go to first col if the mode is set */
        termNewLine(IS_SET(term.mode, MODE_CRLF));
        return;
    case '\a': /* BEL */
        if (term.esc & ESC_STR_END) {
            /* backwards compatibility to xterm */
            strHandle();
        } else {
            bell();
        }
        break;
    case '\033': /* ESC */
        csiReset();
        term.esc &= ~(ESC_CSI | ESC_ALTCHARSET | ESC_TEST);
        term.esc |= ESC_START;
        return;
    case '\016': /* SO (LS1 -- Locking shift 1) */
    case '\017': /* SI (LS0 -- Locking shift 0) */
        term.charset = 1 - (ascii - '\016');
        return;
    case '\032': /* SUB */
        termSetChar('?', &term.c.attr, term.c.x, term.c.y);
        /* FALLTHROUGH */
    case '\030': /* CAN */
        csiReset();
        break;
    case '\005': /* ENQ (IGNORED) */
    case '\000': /* NUL (IGNORED) */
    case '\021': /* XON (IGNORED) */
    case '\023': /* XOFF (IGNORED) */
    case 0177:   /* DEL (IGNORED) */
        return;
    case 0x80: /* TODO: PAD */
    case 0x81: /* TODO: HOP */
    case 0x82: /* TODO: BPH */
    case 0x83: /* TODO: NBH */
    case 0x84: /* TODO: IND */
        break;
    case 0x85:       /* NEL -- Next line */
        termNewLine(1); /* always go to first col */
        break;
    case 0x86: /* TODO: SSA */
    case 0x87: /* TODO: ESA */
        break;
    case 0x88: /* HTS -- Horizontal tab stop */
        term.tabs[term.c.x] = 1;
        break;
    case 0x89: /* TODO: HTJ */
    case 0x8a: /* TODO: VTS */
    case 0x8b: /* TODO: PLD */
    case 0x8c: /* TODO: PLU */
    case 0x8d: /* TODO: RI */
    case 0x8e: /* TODO: SS2 */
    case 0x8f: /* TODO: SS3 */
    case 0x91: /* TODO: PU1 */
    case 0x92: /* TODO: PU2 */
    case 0x93: /* TODO: STS */
    case 0x94: /* TODO: CCH */
    case 0x95: /* TODO: MW */
    case 0x96: /* TODO: SPA */
    case 0x97: /* TODO: EPA */
    case 0x98: /* TODO: SOS */
    case 0x99: /* TODO: SGCI */
        break;
    case 0x9a: /* DECID -- Identify Terminal */
        ttyWrite(vtiden, strlen(vtiden), 0);
        break;
    case 0x9b: /* TODO: CSI */
    case 0x9c: /* TODO: ST */
        break;
    case 0x90: /* DCS -- Device Control String */
    case 0x9d: /* OSC -- Operating System Command */
    case 0x9e: /* PM -- Privacy Message */
    case 0x9f: /* APC -- Application Program Command */
        termStrSequence(ascii);
        return;
    }
    /* only CAN, SUB, \a and C1 chars interrupt a sequence */
    term.esc &= ~(ESC_STR_END | ESC_STR);
}

void
SimpleTerminal::termPutTab(int n)
{
    uint x = term.c.x;

    if (n > 0) {
        while (x < term.col && n--)
            for (++x; x < term.col && !term.tabs[x]; ++x)
                /* nothing */;
    } else if (n < 0) {
        while (x > 0 && n++)
            for (--x; x > 0 && !term.tabs[x]; --x)
                /* nothing */;
    }
    term.c.x = LIMIT(x, 0, term.col - 1);
}

void
SimpleTerminal::termMoveTo(int x, int y)
{
    int miny, maxy;

    if (term.c.state & CURSOR_ORIGIN) {
        miny = term.top;
        maxy = term.bot;
    } else {
        miny = 0;
        maxy = term.row - 1;
    }
    term.c.state &= ~CURSOR_WRAPNEXT;
    term.c.x = LIMIT(x, 0, term.col - 1);
    term.c.y = LIMIT(y, miny, maxy);
}

void
SimpleTerminal::termStrSequence(uchar c)
{
    switch (c) {
    case 0x90: /* DCS -- Device Control String */
        c = 'P';
        break;
    case 0x9f: /* APC -- Application Program Command */
        c = '_';
        break;
    case 0x9e: /* PM -- Privacy Message */
        c = '^';
        break;
    case 0x9d: /* OSC -- Operating System Command */
        c = ']';
        break;
    }
    strReset();
    m_STREscapeSeq.type = c;
    term.esc |= ESC_STR;
}

void
SimpleTerminal::strReset(void)
{
    char *buf = (char *)realloc(m_STREscapeSeq.buf, STR_BUF_SIZ);

    if (buf == NULL) {
        emit sendError("Error while realloc in strReset.");
        return;
    }

    m_STREscapeSeq = (STREscape){
        .buf = buf,
        .siz = STR_BUF_SIZ,
    };
}

void
SimpleTerminal::termNewLine(int first_col)
{
    int y = term.c.y;

    if (y == term.bot) {
        termScrollUp(term.top, 1, 1);
    } else {
        y++;
    }
    termMoveTo(first_col ? 0 : term.c.x, y);
}

void
SimpleTerminal::termClearRegion(int x1, int y1, int x2, int y2)
{
    int x, y, temp;
    Glyph *gp;

    if (x1 > x2)
        temp = x1, x1 = x2, x2 = temp;
    if (y1 > y2)
        temp = y1, y1 = y2, y2 = temp;

    LIMIT(x1, 0, term.col - 1);
    LIMIT(x2, 0, term.col - 1);
    LIMIT(y1, 0, term.row - 1);
    LIMIT(y2, 0, term.row - 1);

    for (y = y1; y <= y2; y++) {
        term.dirty[y] = 1;
        for (x = x1; x <= x2; x++) {
            gp = &term.line[y][x];
            if (selected(x, y))
                selectionClear();
            gp->fg = term.c.attr.fg;
            gp->bg = term.c.attr.bg;
            gp->mode = 0;
            gp->u = ' ';
        }
    }
}

int
SimpleTerminal::selected(int x, int y)
{
    if (selection.mode == SEL_EMPTY || selection.ob.x == -1 ||
        selection.alt != IS_SET(term.mode, MODE_ALTSCREEN))
        return 0;

    if (selection.type == SEL_RECTANGULAR)
        return BETWEEN(y, selection.nb.y, selection.ne.y) && BETWEEN(x, selection.nb.x, selection.ne.x);

    return BETWEEN(y, selection.nb.y, selection.ne.y) && (y != selection.nb.y || x >= selection.nb.x) &&
           (y != selection.ne.y || x <= selection.ne.x);
}

void
SimpleTerminal::selectionClear(void)
{
    if (selection.ob.x == -1)
        return;
    selection.mode = SEL_IDLE;
    selection.ob.x = -1;
    termSetDirt(selection.nb.y, selection.ne.y);
}

void
SimpleTerminal::selectionScroll(int orig, int n)
{
    if (selection.ob.x == -1)
        return;

    selection.ob.y += n;
    selection.oe.y += n;
    selectionNormalize();
}

void
SimpleTerminal::selectionNormalize(void)
{
    int i;

    if (selection.type == SEL_REGULAR && selection.ob.y != selection.oe.y) {
        selection.nb.x = selection.ob.y < selection.oe.y ? selection.ob.x : selection.oe.x;
        selection.ne.x = selection.ob.y < selection.oe.y ? selection.oe.x : selection.ob.x;
    } else {
        selection.nb.x = MIN(selection.ob.x, selection.oe.x);
        selection.ne.x = MAX(selection.ob.x, selection.oe.x);
    }
    selection.nb.y = MIN(selection.ob.y, selection.oe.y);
    selection.ne.y = MAX(selection.ob.y, selection.oe.y);

    selectionSnap(&selection.nb.x, &selection.nb.y, -1);
    selectionSnap(&selection.ne.x, &selection.ne.y, +1);

    /* expand selection over line breaks */
    if (selection.type == SEL_RECTANGULAR)
        return;
    i = termLineLen(selection.nb.y);
    if (i < selection.nb.x)
        selection.nb.x = i;
    if (termLineLen(selection.ne.y) <= selection.ne.x)
        selection.ne.x = term.col - 1;
}

void
SimpleTerminal::termSetDirt(int top, int bot)
{
    int i;

    LIMIT(top, 0, term.row - 1);
    LIMIT(bot, 0, term.row - 1);

    for (i = top; i <= bot; i++)
        term.dirty[i] = 1;
}

void
SimpleTerminal::strHandle(void)
{
    char *p = NULL, *dec;
    int j, narg, par;

    term.esc &= ~(ESC_STR_END | ESC_STR);
    strParse();
    par = (narg = m_STREscapeSeq.narg) ? atoi(m_STREscapeSeq.args[0]) : 0;

    switch (m_STREscapeSeq.type) {
    case ']': /* OSC -- Operating System Command */
        switch (par) {
        // Florian Plesker: removed set title since it is a widget
        case 0:
        case 1:
        case 2:
            return;
        case 52:
            if (narg > 2 && allowWindowOps) {
                dec = base64Dec(m_STREscapeSeq.args[2]);
                if (dec) {
                    xsetsel(dec);
                    xclipcopy();
                } else {
                    fprintf(stderr, "erresc: invalid base64\n");
                }
            }
            return;
        case 10:
            if (narg < 2)
                break;

            p = m_STREscapeSeq.args[1];

            if (!strcmp(p, "?"))
                oscColorResponse(default_fg, 10);
            else if (xsetcolorname(default_fg, p))
                fprintf(stderr, "erresc: invalid foreground color: %s\n", p);
            else
                redraw();
            return;
        case 11:
            if (narg < 2)
                break;

            p = m_STREscapeSeq.args[1];

            if (!strcmp(p, "?"))
                oscColorResponse(default_bg, 11);
            else if (xsetcolorname(default_bg, p))
                fprintf(stderr, "erresc: invalid background color: %s\n", p);
            else
                redraw();
            return;
        case 12:
            if (narg < 2)
                break;

            p = m_STREscapeSeq.args[1];

            if (!strcmp(p, "?"))
                oscColorResponse(default_cs, 12);
            else if (xsetcolorname(default_cs, p))
                fprintf(stderr, "erresc: invalid cursor color: %s\n", p);
            else
                redraw();
            return;
        case 4: /* color set */
            if (narg < 3)
                break;
            p = m_STREscapeSeq.args[2];
            /* FALLTHROUGH */
        case 104: /* color reset */
            j = (narg > 1) ? atoi(m_STREscapeSeq.args[1]) : -1;

            if (p && !strcmp(p, "?"))
                osc4_color_response(j);
            else if (xsetcolorname(j, p)) {
                if (par == 104 && narg <= 1)
                    return; /* color reset without parameter */
                fprintf(stderr, "erresc: invalid color j=%d, p=%s\n", j, p ? p : "(null)");
            } else {
                /*
                 * TODO if defaultbg color is changed, borders
                 * are dirty
                 */
                redraw();
            }
            return;
        }
        break;
    case 'k': /* old title set compatibility */
        return;
    case 'P': /* DCS -- Device Control String */
    case '_': /* APC -- Application Program Command */
    case '^': /* PM -- Privacy Message */
        return;
    }

    fprintf(stderr, "erresc: unknown str ");
    strDump();
}

void
SimpleTerminal::strParse(void)
{
    int c;
    char *p = m_STREscapeSeq.buf;

    m_STREscapeSeq.narg = 0;
    m_STREscapeSeq.buf[m_STREscapeSeq.len] = '\0';

    if (*p == '\0')
        return;

    while (m_STREscapeSeq.narg < STR_ARG_SIZ) {
        m_STREscapeSeq.args[m_STREscapeSeq.narg++] = p;
        while ((c = *p) != ';' && c != '\0')
            ++p;
        if (c == '\0')
            return;
        *p++ = '\0';
    }
}

void
SimpleTerminal::strDump(void)
{
    size_t i;
    uint c;

    fprintf(stderr, "ESC%c", m_STREscapeSeq.type);
    for (i = 0; i < m_STREscapeSeq.len; i++) {
        c = m_STREscapeSeq.buf[i] & 0xff;
        if (c == '\0') {
            putc('\n', stderr);
            return;
        } else if (isprint(c)) {
            putc(c, stderr);
        } else if (c == '\n') {
            fprintf(stderr, "(\\n)");
        } else if (c == '\r') {
            fprintf(stderr, "(\\r)");
        } else if (c == 0x1b) {
            fprintf(stderr, "(\\e)");
        } else {
            fprintf(stderr, "(%02x)", c);
        }
    }
    fprintf(stderr, "ESC\\\n");
}

void
SimpleTerminal::termSetChar(Rune u, const Glyph *attr, int x, int y)
{
    static const char *vt100_0[62] = {
        /* 0x41 - 0x7e */
        "↑", "↓", "→", "←", "█", "▚", "☃",      /* A - G */
        0,   0,   0,   0,   0,   0,   0,   0,   /* H - O */
        0,   0,   0,   0,   0,   0,   0,   0,   /* P - W */
        0,   0,   0,   0,   0,   0,   0,   " ", /* X - _ */
        "◆", "▒", "␉", "␌", "␍", "␊", "°", "±", /* ` - g */
        "␤", "␋", "┘", "┐", "┌", "└", "┼", "⎺", /* h - o */
        "⎻", "─", "⎼", "⎽", "├", "┤", "┴", "┬", /* p - w */
        "│", "≤", "≥", "π", "≠", "£", "·",      /* x - ~ */
    };

    /*
     * The table is proudly stolen from rxvt.
     */
    if (term.trantbl[term.charset] == CS_GRAPHIC0 && BETWEEN(u, 0x41, 0x7e) && vt100_0[u - 0x41])
        utf8_decode(vt100_0[u - 0x41], &u, UTF_SIZ);

    if (term.line[y][x].mode & ATTR_WIDE) {
        if (x + 1 < term.col) {
            term.line[y][x + 1].u = ' ';
            term.line[y][x + 1].mode &= ~ATTR_WDUMMY;
        }
    } else if (term.line[y][x].mode & ATTR_WDUMMY) {
        term.line[y][x - 1].u = ' ';
        term.line[y][x - 1].mode &= ~ATTR_WIDE;
    }

    term.dirty[y] = 1;
    term.line[y][x] = *attr;
    term.line[y][x].u = u;
}

void
SimpleTerminal::csiReset(void)
{
    memset(&m_CSIEscapeSeq, 0, sizeof(m_CSIEscapeSeq));
}

void
SimpleTerminal::csiParse(void)
{
    char *p = m_CSIEscapeSeq.buf, *np;
    long int v;

    m_CSIEscapeSeq.narg = 0;
    if (*p == '?') {
        m_CSIEscapeSeq.priv = 1;
        p++;
    }

    m_CSIEscapeSeq.buf[m_CSIEscapeSeq.len] = '\0';
    while (p < m_CSIEscapeSeq.buf + m_CSIEscapeSeq.len) {
        np = NULL;
        v = strtol(p, &np, 10);
        if (np == p)
            v = 0;
        if (v == LONG_MAX || v == LONG_MIN)
            v = -1;
        m_CSIEscapeSeq.arg[m_CSIEscapeSeq.narg++] = v;
        p = np;
        if (*p != ';' || m_CSIEscapeSeq.narg == ESC_ARG_SIZ)
            break;
        p++;
    }
    m_CSIEscapeSeq.mode[0] = *p++;
    m_CSIEscapeSeq.mode[1] = (p < m_CSIEscapeSeq.buf + m_CSIEscapeSeq.len) ? *p : '\0';
}

void
SimpleTerminal::csiHandle(void)
{
    char buf[40];
    int len;

    switch (m_CSIEscapeSeq.mode[0]) {
    default:
    unknown:
        fprintf(stderr, "erresc: unknown csi ");
        csiDump();
        /* die(""); */
        break;
    case '@': /* ICH -- Insert <n> blank char */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termInsertBlank(m_CSIEscapeSeq.arg[0]);
        break;
    case 'A': /* CUU -- Cursor <n> Up */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termMoveTo(term.c.x, term.c.y - m_CSIEscapeSeq.arg[0]);
        break;
    case 'B': /* CUD -- Cursor <n> Down */
    case 'e': /* VPR --Cursor <n> Down */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termMoveTo(term.c.x, term.c.y + m_CSIEscapeSeq.arg[0]);
        break;
    case 'i': /* MC -- Media Copy */
        switch (m_CSIEscapeSeq.arg[0]) {
        case 0:
            termDump();
            break;
        case 1:
            termDumpLine(term.c.y);
            break;
        case 2:
            termDumpSelection();
            break;
        case 4:
            term.mode &= ~MODE_PRINT;
            break;
        case 5:
            term.mode |= MODE_PRINT;
            break;
        }
        break;
    case 'c': /* DA -- Device Attributes */
        if (m_CSIEscapeSeq.arg[0] == 0)
            ttyWrite(vtiden, strlen(vtiden), 0);
        break;
    case 'b': /* REP -- if last char is printable print it <n> more times */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        if (term.lastc)
            while (m_CSIEscapeSeq.arg[0]-- > 0)
                termPutChar(term.lastc);
        break;
    case 'C': /* CUF -- Cursor <n> Forward */
    case 'a': /* HPR -- Cursor <n> Forward */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termMoveTo(term.c.x + m_CSIEscapeSeq.arg[0], term.c.y);
        break;
    case 'D': /* CUB -- Cursor <n> Backward */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termMoveTo(term.c.x - m_CSIEscapeSeq.arg[0], term.c.y);
        break;
    case 'E': /* CNL -- Cursor <n> Down and first col */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termMoveTo(0, term.c.y + m_CSIEscapeSeq.arg[0]);
        break;
    case 'F': /* CPL -- Cursor <n> Up and first col */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termMoveTo(0, term.c.y - m_CSIEscapeSeq.arg[0]);
        break;
    case 'g': /* TBC -- Tabulation clear */
        switch (m_CSIEscapeSeq.arg[0]) {
        case 0: /* clear current tab stop */
            term.tabs[term.c.x] = 0;
            break;
        case 3: /* clear all the tabs */
            memset(term.tabs, 0, term.col * sizeof(*term.tabs));
            break;
        default:
            goto unknown;
        }
        break;
    case 'G': /* CHA -- Move to <col> */
    case '`': /* HPA */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termMoveTo(m_CSIEscapeSeq.arg[0] - 1, term.c.y);
        break;
    case 'H': /* CUP -- Move to <row> <col> */
    case 'f': /* HVP */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        DEFAULT(m_CSIEscapeSeq.arg[1], 1);
        termMoveATo(m_CSIEscapeSeq.arg[1] - 1, m_CSIEscapeSeq.arg[0] - 1);
        break;
    case 'I': /* CHT -- Cursor Forward Tabulation <n> tab stops */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termPutTab(m_CSIEscapeSeq.arg[0]);
        break;
    case 'J': /* ED -- Clear screen */
        switch (m_CSIEscapeSeq.arg[0]) {
        case 0: /* below */
            termClearRegion(term.c.x, term.c.y, term.col - 1, term.c.y);
            if (term.c.y < term.row - 1) {
                termClearRegion(0, term.c.y + 1, term.col - 1, term.row - 1);
            }
            break;
        case 1: /* above */
            if (term.c.y > 1)
                termClearRegion(0, 0, term.col - 1, term.c.y - 1);
            termClearRegion(0, term.c.y, term.c.x, term.c.y);
            break;
        case 2: /* all */
            termClearRegion(0, 0, term.col - 1, term.row - 1);
            break;
        case 3: /* delete scroll back */
            term.scr = 0;
            term.histi = 0;
            break;
        default:
            goto unknown;
        }
        break;
    case 'K': /* EL -- Clear line */
        switch (m_CSIEscapeSeq.arg[0]) {
        case 0: /* right */
            termClearRegion(term.c.x, term.c.y, term.col - 1, term.c.y);
            break;
        case 1: /* left */
            termClearRegion(0, term.c.y, term.c.x, term.c.y);
            break;
        case 2: /* all */
            termClearRegion(0, term.c.y, term.col - 1, term.c.y);
            break;
        }
        break;
    case 'S': /* SU -- Scroll <n> line up */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termScrollUp(term.top, m_CSIEscapeSeq.arg[0], 0);
        break;
    case 'T': /* SD -- Scroll <n> line down */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termScrollDown(term.top, m_CSIEscapeSeq.arg[0], 0);
        break;
    case 'L': /* IL -- Insert <n> blank lines */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termInsertBlankLine(m_CSIEscapeSeq.arg[0]);
        break;
    case 'l': /* RM -- Reset Mode */
        termSetMode(m_CSIEscapeSeq.priv, 0, m_CSIEscapeSeq.arg, m_CSIEscapeSeq.narg);
        break;
    case 'M': /* DL -- Delete <n> lines */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termDeleteLine(m_CSIEscapeSeq.arg[0]);
        break;
    case 'X': /* ECH -- Erase <n> char */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termClearRegion(term.c.x, term.c.y, term.c.x + m_CSIEscapeSeq.arg[0] - 1, term.c.y);
        break;
    case 'P': /* DCH -- Delete <n> char */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termDeleteChar(m_CSIEscapeSeq.arg[0]);
        break;
    case 'Z': /* CBT -- Cursor Backward Tabulation <n> tab stops */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termPutTab(-m_CSIEscapeSeq.arg[0]);
        break;
    case 'd': /* VPA -- Move to <row> */
        DEFAULT(m_CSIEscapeSeq.arg[0], 1);
        termMoveATo(term.c.x, m_CSIEscapeSeq.arg[0] - 1);
        break;
    case 'h': /* SM -- Set terminal mode */
        termSetMode(m_CSIEscapeSeq.priv, 1, m_CSIEscapeSeq.arg, m_CSIEscapeSeq.narg);
        break;
    case 'm': /* SGR -- Terminal attribute (color) */
        termSetAttr(m_CSIEscapeSeq.arg, m_CSIEscapeSeq.narg);
        break;
    case 'n': /* DSR – Device Status Report (cursor position) */
        if (m_CSIEscapeSeq.arg[0] == 6) {
            len = snprintf(buf, sizeof(buf), "\033[%i;%iR", term.c.y + 1, term.c.x + 1);
            ttyWrite(buf, len, 0);
        }
        break;
    case 'r': /* DECSTBM -- Set Scrolling Region */
        if (m_CSIEscapeSeq.priv) {
            goto unknown;
        } else {
            DEFAULT(m_CSIEscapeSeq.arg[0], 1);
            DEFAULT(m_CSIEscapeSeq.arg[1], term.row);
            termSetScroll(m_CSIEscapeSeq.arg[0] - 1, m_CSIEscapeSeq.arg[1] - 1);
            termMoveATo(0, 0);
        }
        break;
    case 's': /* DECSC -- Save cursor position (ANSI.SYS) */
        termCursor(CURSOR_SAVE);
        break;
    case 'u': /* DECRC -- Restore cursor position (ANSI.SYS) */
        termCursor(CURSOR_LOAD);
        break;
    case ' ':
        switch (m_CSIEscapeSeq.mode[1]) {
        case 'q': /* DECSCUSR -- Set Cursor Style */
            if (xsetcursor(m_CSIEscapeSeq.arg[0]))
                goto unknown;
            break;
        default:
            goto unknown;
        }
        break;
    }
}

void
SimpleTerminal::termSetScroll(int t, int b)
{
    int temp;

    LIMIT(t, 0, term.row - 1);
    LIMIT(b, 0, term.row - 1);
    if (t > b) {
        temp = t;
        t = b;
        b = temp;
    }
    term.top = t;
    term.bot = b;
}

void
SimpleTerminal::termCursor(int mode)
{
    static TCursor c[2];
    int alt = IS_SET(term.mode, MODE_ALTSCREEN);

    if (mode == CURSOR_SAVE) {
        c[alt] = term.c;
    } else if (mode == CURSOR_LOAD) {
        term.c = c[alt];
        termMoveTo(c[alt].x, c[alt].y);
    }
}

void
SimpleTerminal::csiDump(void)
{
    size_t i;
    uint c;

    fprintf(stderr, "ESC[");
    for (i = 0; i < m_CSIEscapeSeq.len; i++) {
        c = m_CSIEscapeSeq.buf[i] & 0xff;
        if (isprint(c)) {
            putc(c, stderr);
        } else if (c == '\n') {
            fprintf(stderr, "(\\n)");
        } else if (c == '\r') {
            fprintf(stderr, "(\\r)");
        } else if (c == 0x1b) {
            fprintf(stderr, "(\\e)");
        } else {
            fprintf(stderr, "(%02x)", c);
        }
    }
    putc('\n', stderr);
}

void
SimpleTerminal::termDumpLine(int n)
{
    char buf[UTF_SIZ];
    const Glyph *bp, *end;

    bp = &term.line[n][0];
    end = &bp[MIN(termLineLen(n), term.col) - 1];
    if (bp != end || bp->u != ' ') {
        for (; bp <= end; ++bp)
            tprinter(buf, utf8_encode(bp->u, buf));
    }
    tprinter("\n", 1);
}

int
SimpleTerminal::termLineLen(int y)
{
    int i = term.col;

    if (TLINE(term, y)[i - 1].mode & ATTR_WRAP)
        return i;

    while (i > 0 && TLINE(term, y)[i - 1].u == ' ')
        --i;

    return i;
}

void
SimpleTerminal::termDump(void)
{
    int i;

    for (i = 0; i < term.row; ++i)
        termDumpLine(i);
}

void
SimpleTerminal::termInsertBlank(int n)
{
    int dst, src, size;
    Glyph *line;

    LIMIT(n, 0, term.col - term.c.x);

    dst = term.c.x + n;
    src = term.c.x;
    size = term.col - dst;
    line = term.line[term.c.y];

    memmove(&line[dst], &line[src], size * sizeof(Glyph));
    termClearRegion(src, term.c.y, dst - 1, term.c.y);
}

void
SimpleTerminal::termInsertBlankLine(int n)
{
    if (BETWEEN(term.c.y, term.top, term.bot))
        termScrollDown(term.c.y, n, 0);
}

void
SimpleTerminal::kScrollDown(int n)
{
    if (n < 0)
        n = term.row + n;

    if (n > term.scr)
        n = term.scr;

    if (term.scr > 0) {
        term.scr -= n;
        selectionScroll(0, -n);
        termFullDirt();
    }
}

void
SimpleTerminal::kScrollUp(int n)
{
    if (n < 0)
        n = term.row + n;

    if (term.scr <= HISTSIZE - n) {
        term.scr += n;
        selectionScroll(0, n);
        termFullDirt();
    }
}

void
SimpleTerminal::termScrollDown(int orig, int n, int copyHistory)
{
    int i;
    Line temp;

    LIMIT(n, 0, term.bot - orig + 1);

    if (copyHistory) {
        term.histi = (term.histi - 1 + HISTSIZE) % HISTSIZE;
        temp = term.hist[term.histi];
        term.hist[term.histi] = term.line[term.bot];
        term.line[term.bot] = temp;
    }

    termSetDirt(orig, term.bot - n);
    termClearRegion(0, term.bot - n + 1, term.col - 1, term.bot);

    for (i = term.bot; i >= orig + n; i--) {
        temp = term.line[i];
        term.line[i] = term.line[i - n];
        term.line[i - n] = temp;
    }

    if (term.scr == 0)
        selectionScroll(orig, n);
}

void
SimpleTerminal::termScrollUp(int orig, int n, int copyHistory)
{
    int i;
    Line temp;

    LIMIT(n, 0, term.bot - orig + 1);

    if (copyHistory) {
        term.histi = (term.histi + 1) % HISTSIZE;
        temp = term.hist[term.histi];
        term.hist[term.histi] = term.line[orig];
        term.line[orig] = temp;
    }

    if (term.scr > 0 && term.scr < HISTSIZE)
        term.scr = MIN(term.scr + n, HISTSIZE - 1);

    termClearRegion(0, orig, term.col - 1, orig + n - 1);
    termSetDirt(orig + n, term.bot);

    for (i = orig; i <= term.bot - n; i++) {
        temp = term.line[i];
        term.line[i] = term.line[i + n];
        term.line[i + n] = temp;
    }

    if (term.scr == 0)
        selectionScroll(orig, -n);
}

/* for absolute user moves, when decom is set */
void
SimpleTerminal::termMoveATo(int x, int y)
{
    termMoveTo(x, y + ((term.c.state & CURSOR_ORIGIN) ? term.top : 0));
}

void
SimpleTerminal::termDeleteChar(int n)
{
    int dst, src, size;
    Glyph *line;

    LIMIT(n, 0, term.col - term.c.x);

    dst = term.c.x;
    src = term.c.x + n;
    size = term.col - src;
    line = term.line[term.c.y];

    memmove(&line[dst], &line[src], size * sizeof(Glyph));
    termClearRegion(term.col - n, term.c.y, term.col - 1, term.c.y);
}

void
SimpleTerminal::termDefineTranslation(char ascii)
{
    static char cs[] = "0B";
    static int vcs[] = { CS_GRAPHIC0, CS_USA };
    char *p;

    if ((p = strchr(cs, ascii)) == NULL) {
        fprintf(stderr, "esc unhandled charset: ESC ( %c\n", ascii);
    } else {
        term.trantbl[term.icharset] = vcs[p - cs];
    }
}

void
SimpleTerminal::termDecTest(char c)
{
    int x, y;

    if (c == '8') { /* DEC screen alignment test. */
        for (x = 0; x < term.col; ++x) {
            for (y = 0; y < term.row; ++y)
                termSetChar('E', &term.c.attr, x, y);
        }
    }
}

void
SimpleTerminal::termDefineUtf8(char ascii)
{
    if (ascii == 'G')
        term.mode |= MODE_UTF8;
    else if (ascii == '@')
        term.mode &= ~MODE_UTF8;
}

void
SimpleTerminal::termDeleteLine(int n)
{
    if (BETWEEN(term.c.y, term.top, term.bot))
        termScrollUp(term.c.y, n, 0);
}

/*
 * returns 1 when the sequence is finished and it hasn't to read
 * more characters for this sequence, otherwise 0
 */
int
SimpleTerminal::escHandle(uchar ascii)
{
    switch (ascii) {
    case '[':
        term.esc |= ESC_CSI;
        return 0;
    case '#':
        term.esc |= ESC_TEST;
        return 0;
    case '%':
        term.esc |= ESC_UTF8;
        return 0;
    case 'P': /* DCS -- Device Control String */
    case '_': /* APC -- Application Program Command */
    case '^': /* PM -- Privacy Message */
    case ']': /* OSC -- Operating System Command */
    case 'k': /* old title set compatibility */
        termStrSequence(ascii);
        return 0;
    case 'n': /* LS2 -- Locking shift 2 */
    case 'o': /* LS3 -- Locking shift 3 */
        term.charset = 2 + (ascii - 'n');
        break;
    case '(': /* GZD4 -- set primary charset G0 */
    case ')': /* G1D4 -- set secondary charset G1 */
    case '*': /* G2D4 -- set tertiary charset G2 */
    case '+': /* G3D4 -- set quaternary charset G3 */
        term.icharset = ascii - '(';
        term.esc |= ESC_ALTCHARSET;
        return 0;
    case 'D': /* IND -- Linefeed */
        if (term.c.y == term.bot) {
            termScrollUp(term.top, 1, 1);
        } else {
            termMoveTo(term.c.x, term.c.y + 1);
        }
        break;
    case 'E':        /* NEL -- Next line */
        termNewLine(1); /* always go to first col */
        break;
    case 'H': /* HTS -- Horizontal tab stop */
        term.tabs[term.c.x] = 1;
        break;
    case 'M': /* RI -- Reverse index */
        if (term.c.y == term.top) {
            termScrollDown(term.top, 1, 1);
        } else {
            termMoveTo(term.c.x, term.c.y - 1);
        }
        break;
    case 'Z': /* DECID -- Identify Terminal */
        ttyWrite(vtiden, strlen(vtiden), 0);
        break;
    case 'c': /* RIS -- Reset to initial state */
        termReset();
        xloadcols();
        break;
    case '=': /* DECPAM -- Application keypad */
        // TODO xsetmode(1, MODE_APPKEYPAD);
        break;
    case '>': /* DECPNM -- Normal keypad */
        // TODO xsetmode(0, MODE_APPKEYPAD);
        break;
    case '7': /* DECSC -- Save Cursor */
        termCursor(CURSOR_SAVE);
        break;
    case '8': /* DECRC -- Restore Cursor */
        termCursor(CURSOR_LOAD);
        break;
    case '\\': /* ST -- String Terminator */
        if (term.esc & ESC_STR_END)
            strHandle();
        break;
    default:
        fprintf(stderr, "erresc: unknown sequence ESC 0x%02X '%c'\n", (uchar)ascii,
                isprint(ascii) ? ascii : '.');
        break;
    }
    return 1;
}

void
SimpleTerminal::termReset(void)
{
    uint i;

    term.c = (TCursor){ {
                            .u = default_cursor,
                            .mode = ATTR_NULL,
                            .fg = default_fg, // see define default fg
                            .bg = default_bg  // see define default bg
                        },
                        .x = 0,
                        .y = 0,
                        .state = CURSOR_DEFAULT };

    memset(term.tabs, 0, term.col * sizeof(*term.tabs));
    for (i = tabSpaces; i < term.col; i += tabSpaces)
        term.tabs[i] = 1;
    term.top = 0;
    term.bot = term.row - 1;
    term.mode = MODE_WRAP | MODE_UTF8;
    memset(term.trantbl, CS_USA, sizeof(term.trantbl));
    term.charset = 0;

    for (i = 0; i < 2; i++) {
        termMoveTo(0, 0);
        termCursor(CURSOR_SAVE);
        termClearRegion(0, 0, term.col - 1, term.row - 1);
        termSwapScreen();
    }
}

void
SimpleTerminal::termSwapScreen(void)
{
    Line *tmp = term.line;

    term.line = term.alt;
    term.alt = tmp;

    int temp = term.scr;
    term.altScr = term.scr;
    term.scr = temp;

    temp = term.histi;
    term.histi = term.altHisti;
    term.altHisti = temp;

    if (term.mode & MODE_ALTSCREEN) {
        // alt screen should not have scroll
        term.altScr = 0;
        term.altHisti = 0;
    }

    term.mode ^= MODE_ALTSCREEN;
    termFullDirt();
}

void
SimpleTerminal::termFullDirt(void)
{
    termSetDirt(0, term.row - 1);
}

void
SimpleTerminal::termDumpSelection(void)
{
    char *ptr;

    if ((ptr = getSelection())) {
        tprinter(ptr, strlen(ptr));
        free(ptr);
    }
}

char *
SimpleTerminal::getSelection(void)
{
    char *str, *ptr;
    int y, bufsize, lastx, linelen;
    const Glyph *gp, *last;

    if (selection.ob.x == -1)
        return NULL;

    bufsize = (term.col + 1) * (selection.ne.y - selection.nb.y + 1) * UTF_SIZ;
    char *buf = (char *)malloc(bufsize);

    if (buf == NULL) {
        emit sendError("Error on malloc in getSelection.");
        return NULL;
    }

    ptr = str = buf;

    /* append every set & selected glyph to the selection */
    for (y = selection.nb.y; y <= selection.ne.y; y++) {
        if ((linelen = termLineLen(y)) == 0) {
            *ptr++ = '\n';
            continue;
        }

        if (selection.type == SEL_RECTANGULAR) {
            gp = &TLINE(term, y)[selection.nb.x];
            lastx = selection.ne.x;
        } else {
            gp = &TLINE(term, y)[selection.nb.y == y ? selection.nb.x : 0];
            lastx = (selection.ne.y == y) ? selection.ne.x : term.col - 1;
        }
        last = &TLINE(term, y)[MIN(lastx, linelen - 1)];
        while (last >= gp && last->u == ' ')
            --last;

        for (; gp <= last; ++gp) {
            if (gp->mode & ATTR_WDUMMY)
                continue;

            ptr += utf8_encode(gp->u, ptr);
        }

        /*
         * Copy and pasting of line endings is inconsistent
         * in the inconsistent terminal and GUI world.
         * The best solution seems like to produce '\n' when
         * something is copied from st and convert '\n' to
         * '\r', when something to be pasted is received by
         * st.
         * FIXME: Fix the computer world.
         */
        if ((y < selection.ne.y || lastx >= linelen) &&
            (!(last->mode & ATTR_WRAP) || selection.type == SEL_RECTANGULAR))
            *ptr++ = '\n';
    }
    *ptr = 0;
    return str;
}

void
SimpleTerminal::oscColorResponse(int index, int num)
{
    int n;
    char buf[32];
    unsigned char r, g, b;

    if (xgetcolor(index, &r, &g, &b)) {
        fprintf(stderr, "erresc: failed to fetch osc color %d\n", index);
        return;
    }

    n = snprintf(buf, sizeof buf, "\033]%d;rgb:%02x%02x/%02x%02x/%02x%02x\007", num, r, r, g, g, b,
                 b);

    ttyWrite(buf, n, 1);
}

void
SimpleTerminal::redraw(void)
{
    termFullDirt();
    draw();
}

char *
SimpleTerminal::base64Dec(const char *src)
{
    size_t in_len = strlen(src);
    char *result, *dst;

    if (in_len % 4)
        in_len += 4 - (in_len % 4);

    char *buf = (char *)malloc(in_len / 4 * 3 + 1);

    if (buf == NULL) {
        emit sendError("Error on malloc in base64Dec");
        return NULL;
    }

    result = dst = buf;
    while (*src) {
        int a = base64_digits[(unsigned char)base64DecGetChar(&src)];
        int b = base64_digits[(unsigned char)base64DecGetChar(&src)];
        int c = base64_digits[(unsigned char)base64DecGetChar(&src)];
        int d = base64_digits[(unsigned char)base64DecGetChar(&src)];

        /* invalid input. 'a' can be -1, e.g. if src is "\n" (c-str) */
        if (a == -1 || b == -1)
            break;

        *dst++ = (a << 2) | ((b & 0x30) >> 4);
        if (c == -1)
            break;
        *dst++ = ((b & 0x0f) << 4) | ((c & 0x3c) >> 2);
        if (d == -1)
            break;
        *dst++ = ((c & 0x03) << 6) | d;
    }
    *dst = '\0';
    return result;
}

char
SimpleTerminal::base64DecGetChar(const char **src)
{
    while (**src && !isprint(**src))
        (*src)++;
    return **src ? *((*src)++) : '='; /* emulate padding if string ends */
}

void
SimpleTerminal::osc4_color_response(int num)
{
    int n;
    char buf[32];
    unsigned char r, g, b;

    if (xgetcolor(num, &r, &g, &b)) {
        emit sendError("erresc: failed to fetch osc4 color: " + QString::number(num));
        return;
    }

    n = snprintf(buf, sizeof buf, "\033]4;%d;rgb:%02x%02x/%02x%02x/%02x%02x\007", num, r, r, g, g,
                 b, b);

    ttyWrite(buf, n, 1);
}

void
SimpleTerminal::selectionSnap(int *x, int *y, int direction)
{
    int newx, newy, xt, yt;
    int delim, prevdelim;
    const Glyph *gp, *prevgp;

    switch (selection.snap) {
    case SNAP_WORD:
        /*
         * Snap around if the word wraps around at the end or
         * beginning of a line.
         */
        prevgp = &TLINE(term, *y)[*x];
        prevdelim = ISDELIM(prevgp->u);
        for (;;) {
            newx = *x + direction;
            newy = *y;
            if (!BETWEEN(newx, 0, term.col - 1)) {
                newy += direction;
                newx = (newx + term.col) % term.col;
                if (!BETWEEN(newy, 0, term.row - 1))
                    break;

                if (direction > 0)
                    yt = *y, xt = *x;
                else
                    yt = newy, xt = newx;
                if (!(TLINE(term, yt)[xt].mode & ATTR_WRAP))
                    break;
            }

            if (newx >= termLineLen(newy))
                break;

            gp = &TLINE(term, newy)[newx];
            delim = ISDELIM(gp->u);
            if (!(gp->mode & ATTR_WDUMMY) && (delim != prevdelim || (delim && gp->u != prevgp->u)))
                break;

            *x = newx;
            *y = newy;
            prevgp = gp;
            prevdelim = delim;
        }
        break;
    case SNAP_LINE:
        /*
         * Snap around if the the previous line or the current one
         * has set ATTR_WRAP at its end. Then the whole next or
         * previous line will be selected.
         */
        *x = (direction < 0) ? 0 : term.col - 1;
        if (direction < 0) {
            for (; *y > 0; *y += direction) {
                if (!(TLINE(term, *y - 1)[term.col - 1].mode & ATTR_WRAP)) {
                    break;
                }
            }
        } else if (direction > 0) {
            for (; *y < term.row - 1; *y += direction) {
                if (!(TLINE(term, *y)[term.col - 1].mode & ATTR_WRAP)) {
                    break;
                }
            }
        }
        break;
    }
}

void
SimpleTerminal::termSetAttr(const int *attr, int l)
{
    int i;
    int32_t idx;

    for (i = 0; i < l; i++) {
        switch (attr[i]) {
        case 0:
            term.c.attr.mode &= ~(ATTR_BOLD | ATTR_FAINT | ATTR_ITALIC | ATTR_UNDERLINE |
                                  ATTR_BLINK | ATTR_REVERSE | ATTR_INVISIBLE | ATTR_STRUCK);
            term.c.attr.fg = default_fg;
            term.c.attr.bg = default_bg;
            break;
        case 1:
            term.c.attr.mode |= ATTR_BOLD;
            break;
        case 2:
            term.c.attr.mode |= ATTR_FAINT;
            break;
        case 3:
            term.c.attr.mode |= ATTR_ITALIC;
            break;
        case 4:
            term.c.attr.mode |= ATTR_UNDERLINE;
            break;
        case 5: /* slow blink */
            /* FALLTHROUGH */
        case 6: /* rapid blink */
            term.c.attr.mode |= ATTR_BLINK;
            break;
        case 7:
            term.c.attr.mode |= ATTR_REVERSE;
            break;
        case 8:
            term.c.attr.mode |= ATTR_INVISIBLE;
            break;
        case 9:
            term.c.attr.mode |= ATTR_STRUCK;
            break;
        case 22:
            term.c.attr.mode &= ~(ATTR_BOLD | ATTR_FAINT);
            break;
        case 23:
            term.c.attr.mode &= ~ATTR_ITALIC;
            break;
        case 24:
            term.c.attr.mode &= ~ATTR_UNDERLINE;
            break;
        case 25:
            term.c.attr.mode &= ~ATTR_BLINK;
            break;
        case 27:
            term.c.attr.mode &= ~ATTR_REVERSE;
            break;
        case 28:
            term.c.attr.mode &= ~ATTR_INVISIBLE;
            break;
        case 29:
            term.c.attr.mode &= ~ATTR_STRUCK;
            break;
        case 38:
            if ((idx = termDefineColor(attr, &i, l)) >= 0)
                term.c.attr.fg = idx;
            break;
        case 39:
            term.c.attr.fg = default_fg;
            break;
        case 48:
            if ((idx = termDefineColor(attr, &i, l)) >= 0)
                term.c.attr.bg = idx;
            break;
        case 49:
            term.c.attr.bg = default_bg;
            break;
        default:
            if (BETWEEN(attr[i], 30, 37)) {
                term.c.attr.fg = attr[i] - 30;
            } else if (BETWEEN(attr[i], 40, 47)) {
                term.c.attr.bg = attr[i] - 40;
            } else if (BETWEEN(attr[i], 90, 97)) {
                term.c.attr.fg = attr[i] - 90 + 8;
            } else if (BETWEEN(attr[i], 100, 107)) {
                term.c.attr.bg = attr[i] - 100 + 8;
            } else {
                emit sendError("erresc(default): gfx attr " + QString::number(attr[i]) +
                               " unknown.");
                csiDump();
            }
            break;
        }
    }
}

int32_t
SimpleTerminal::termDefineColor(const int *attr, int *npar, int l)
{
    int32_t idx = -1;
    uint r, g, b;

    switch (attr[*npar + 1]) {
    case 2: /* direct color in RGB space */
        if (*npar + 4 >= l) {
            emit sendError("erresc(38): Incorrect number of parameters " + QString::number(*npar));
            break;
        }
        r = attr[*npar + 2];
        g = attr[*npar + 3];
        b = attr[*npar + 4];
        *npar += 4;
        if (!BETWEEN(r, 0, 255) || !BETWEEN(g, 0, 255) || !BETWEEN(b, 0, 255)) {
            emit sendError("erresc: bad rgb color " + QString::number(r) + "," +
                           QString::number(g) + "," + QString::number(b));
        } else {

            idx = TRUECOLOR(r, g, b);
        }
        break;
    case 5: /* indexed color */
        if (*npar + 2 >= l) {
            emit sendError("erresc(38): Incorrect number of parameters " + QString::number(*npar));
            break;
        }
        *npar += 2;
        if (!BETWEEN(attr[*npar], 0, 255))
            emit sendError("erresc: bad fgcolor " + QString::number(attr[*npar]));
        else
            idx = attr[*npar];
        break;
    case 0: /* implemented defined (only foreground) */
    case 1: /* transparent */
    case 3: /* direct color in CMY space */
    case 4: /* direct color in CMYK space */
    default:
        emit sendError("erresc(38): gfx attr " + QString::number(attr[*npar]) + " unknown\n");
        break;
    }

    return idx;
}

void
SimpleTerminal::termSetMode(int priv, int set, const int *args, int narg)
{

    int alt;
    const int *lim;

    for (lim = args + narg; args < lim; ++args) {
        if (priv) {
            switch (*args) {
            case 1: /* DECCKM -- Cursor key */
                xSetMode(set, MODE_APPCURSOR);
                break;
            case 5: /* DECSCNM -- Reverse video */
                xSetMode(set, MODE_REVERSE);
                break;
            case 6: /* DECOM -- Origin */
                MODBIT(term.c.state, set, CURSOR_ORIGIN);
                termMoveATo(0, 0);
                break;
            case 7: /* DECAWM -- Auto wrap */
                MODBIT(term.mode, set, MODE_WRAP);
                break;
            case 0:  /* Error (IGNORED) */
            case 2:  /* DECANM -- ANSI/VT52 (IGNORED) */
            case 3:  /* DECCOLM -- Column  (IGNORED) */
            case 4:  /* DECSCLM -- Scroll (IGNORED) */
            case 8:  /* DECARM -- Auto repeat (IGNORED) */
            case 18: /* DECPFF -- Printer feed (IGNORED) */
            case 19: /* DECPEX -- Printer extent (IGNORED) */
            case 42: /* DECNRCM -- National characters (IGNORED) */
            case 12: /* att610 -- Start blinking cursor (IGNORED) */
                break;
            case 25: /* DECTCEM -- Text Cursor Enable Mode */
                xSetMode(!set, MODE_HIDE);
                break;
            case 9: /* X10 mouse compatibility mode */
                // xsetpointermotion(0);
                xSetMode(0, MODE_MOUSE);
                xSetMode(set, MODE_MOUSEX10);
                break;
            case 1000: /* 1000: report button press */
                // xsetpointermotion(0);
                xSetMode(0, MODE_MOUSE);
                xSetMode(set, MODE_MOUSEBTN);
                break;
            case 1002: /* 1002: report motion on button press */
                // xsetpointermotion(0);
                xSetMode(0, MODE_MOUSE);
                xSetMode(set, MODE_MOUSEMOTION);
                break;
            case 1003: /* 1003: enable all mouse motions */
                // xsetpointermotion(set);
                xSetMode(0, MODE_MOUSE);
                xSetMode(set, MODE_MOUSEMANY);
                break;
            case 1004: /* 1004: send focus events to tty */
                xSetMode(set, MODE_FOCUS);
                break;
            case 1006: /* 1006: extended reporting mode */
                xSetMode(set, MODE_MOUSESGR);
                break;
            case 1034:
                xSetMode(set, MODE_8BIT);
                break;
            case 1049: /* swap screen & set/restore cursor as xterm */
                termCursor((set) ? CURSOR_SAVE : CURSOR_LOAD);
                /* FALLTHROUGH */
            case 47: /* swap screen */
            case 1047:
                alt = IS_SET(term.mode, MODE_ALTSCREEN);
                if (alt) {
                    termClearRegion(0, 0, term.col - 1, term.row - 1);
                }
                if (set ^ alt) /* set is always 1 or 0 */
                    termSwapScreen();
                if (*args != 1049)
                    break;
                /* FALLTHROUGH */
            case 1048:
                termCursor((set) ? CURSOR_SAVE : CURSOR_LOAD);
                break;
            case 2004: /* 2004: bracketed paste mode */
                xSetMode(set, MODE_BRCKTPASTE);
                break;
                /* Not implemented mouse modes. See comments there. */
            case 1001: /* mouse highlight mode; can hang the
                  terminal by design when implemented. */
            case 1005: /* UTF-8 mouse mode; will confuse
                  applications not supporting UTF-8
                  and luit. */
            case 1015: /* urxvt mangled mouse mode; incompatible
                  and can be mistaken for other control
                  codes. */
                break;
            default:
                emit sendError("erresc: unknown private set/reset mode " + QString::number(*args));
                break;
            }
        } else {
            switch (*args) {
            case 0: /* Error (IGNORED) */
                break;
            case 2:
                xSetMode(set, MODE_KBDLOCK);
                break;
            case 4: /* IRM -- Insertion-replacement */
                MODBIT(term.mode, set, MODE_INSERT);
                break;
            case 12: /* SRM -- Send/Receive */
                MODBIT(term.mode, !set, MODE_ECHO);
                break;
            case 20: /* LNM -- Linefeed/new line */
                MODBIT(term.mode, set, MODE_CRLF);
                break;
            default:
                emit sendError("erresc: unknown set/reset mode " + QString::number(*args));
                break;
            }
        }
    }
}

void
SimpleTerminal::xSetMode(int set, unsigned int flags)
{
    int mode = m_win.mode;
    MODBIT(m_win.mode, set, flags);
    if ((m_win.mode & MODE_REVERSE) != (mode & MODE_REVERSE))
        redraw();
}

void
SimpleTerminal::bell()
{
    QApplication::beep();
}

void
SimpleTerminal::ttyResize(int tw, int th)
{
    m_win.tw = tw;
    m_win.th = th;

    m_winSize.ws_row = term.row;
    m_winSize.ws_col = term.col;
    m_winSize.ws_xpixel = tw;
    m_winSize.ws_ypixel = th;

    if (ioctl(m_master, TIOCSWINSZ, &m_winSize) < 0) {
        emit sendError("Couldn't set window size: " + QString(strerror(errno)));
    }
}

void
SimpleTerminal::selectionStart(int col, int row, int snap)
{
    selectionClear();
    selection.mode = SEL_EMPTY;
    selection.type = SEL_REGULAR;
    selection.alt = IS_SET(term.mode, MODE_ALTSCREEN);
    selection.snap = snap;
    selection.oe.x = selection.ob.x = col;
    selection.oe.y = selection.ob.y = row;
    selectionNormalize();

    if (selection.snap != 0)
        selection.mode = SEL_READY;
    termSetDirt(selection.nb.y, selection.ne.y);
}

void
SimpleTerminal::selectionExtEnd(int col, int row, int type, int done)
{
    int oldey, oldex, oldsby, oldsey, oldtype;

    if (selection.mode == SEL_IDLE)
        return;
    if (done && selection.mode == SEL_EMPTY) {
        selectionClear();
        return;
    }

    oldey = selection.oe.y;
    oldex = selection.oe.x;
    oldsby = selection.nb.y;
    oldsey = selection.ne.y;
    oldtype = selection.type;

    selection.oe.x = col;
    selection.oe.y = row;
    selectionNormalize();
    selection.type = type;

    if (oldey != selection.oe.y || oldex != selection.oe.x || oldtype != selection.type ||
        selection.mode == SEL_EMPTY)
        termSetDirt(MIN(selection.nb.y, oldsby), MAX(selection.ne.y, oldsey));

    selection.mode = done ? SEL_IDLE : SEL_READY;
}

// ------------------------------ TODO ------------------------
void
SimpleTerminal::xsetsel(char *strvtiden)
{
    // TODO
    // qDebug() << "xsetsel";
}

void
SimpleTerminal::xclipcopy(void)
{
    // TODO
    // qDebug() << "xclipcopy";
}

void
SimpleTerminal::draw(void)
{
    // TODO
    // qDebug() << "Draw";
}

int
SimpleTerminal::xsetcolorname(int x, const char *name)
{
    // TODO
    // qDebug() << "xsetcolorname";
    return 0;
}

int
SimpleTerminal::xgetcolor(int x, unsigned char *r, unsigned char *g, unsigned char *b)
{
    // TODO
    // qDebug() << "xgetcolor";
    return 0;
}

void
SimpleTerminal::xloadcols()
{
    // TODO
    // qDebug() << "xloadcols";
}

int
SimpleTerminal::xsetcursor(int cursor)
{
    // TODO
    // qDebug() << "xsetcursor";
}

void
SimpleTerminal::tprinter(char *s, size_t len)
{
    // TODO
    // qDebug() << "tprinter";
}
