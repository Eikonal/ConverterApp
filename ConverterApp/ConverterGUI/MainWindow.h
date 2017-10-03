#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
namespace Ui { class MainWindow; }

class MainWindow : public QWidget
{
    Q_OBJECT

    Ui::MainWindow *ui;
    QString m_inputFileName, m_outputFileName;

    void initFunctionsList();

    // vector I/O
    void saveVector(const std::vector<int>& output) const;
    void loadVector(std::vector<int>& input) const;

public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

private slots:
    void selectInputFile();
    void selectOutputFile();
    void runCommand();
};

#endif // MAINWINDOW_H
