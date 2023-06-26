#pragma once

#include <QFile>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QTextDocument>
#include <QTextEdit>
#include <QTreeView>

class MainWindow : public QMainWindow
{
public:
    MainWindow(QMainWindow *parent = nullptr);
    ~MainWindow() override;

private slots:
    bool
    setTextBufferFromFile(const QString &filePath);

private:
    QWidget *m_window;
    QHBoxLayout *m_layout;
    QFileSystemModel *m_model;
    QTreeView *m_tree;
    QTextEdit *m_textEdit;

    QSize
    sizeHint(void) const override;

    void
    setupSignals();
};
