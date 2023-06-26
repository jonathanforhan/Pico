#include "MainWindow.hpp"

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent),
      m_window(new QWidget(this)),
      m_layout(new QHBoxLayout(m_window)),
      m_model(new QFileSystemModel(m_window)),
      m_tree(new QTreeView(m_window)),
      m_textEdit(new QTextEdit(m_window))
{
    setCentralWidget(m_window);

    m_model->setRootPath(QDir::currentPath());
    m_tree->setModel(m_model);
    m_tree->setRootIndex(m_model->index(QDir::currentPath()));
    m_tree->setMaximumWidth(500);

    m_layout->addWidget(m_tree);

    m_layout->addWidget(m_textEdit);
    m_textEdit->setFont(QFont("JetBrains Mono  NF", 12));

    setupSignals();
}

MainWindow::~MainWindow()
{}

bool
MainWindow::setTextBufferFromFile(const QString &filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    m_textEdit->clear();

    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine();
        m_textEdit->append(line);
    }

    auto cursor = m_textEdit->textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    m_textEdit->setTextCursor(cursor);

    m_textEdit->setFocus();

    return true;
}

QSize
MainWindow::sizeHint(void) const
{
    constexpr QSize size(1920 / 1.5, 1080 / 1.5);
    return size;
}

void
MainWindow::setupSignals()
{
    connect(m_tree, &QTreeView::clicked, [=]() {
        this->setTextBufferFromFile(m_model->filePath(m_tree->currentIndex()));
    });
}
