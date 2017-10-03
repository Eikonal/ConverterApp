#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <dlfcn.h>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    ui = new Ui::MainWindow;
    ui->setupUi(this);

    QObject::connect(ui->inputPB, &QAbstractButton::clicked, this, &MainWindow::selectInputFile);
    QObject::connect(ui->runPB, &QAbstractButton::clicked, this, &MainWindow::runCommand);
    QObject::connect(ui->outputPB, &QAbstractButton::clicked, this, &MainWindow::selectOutputFile);

    initFunctionsList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::selectInputFile()
{
    const QString fileName = QFileDialog::getOpenFileName(this, "Select input file");
    if (fileName.isEmpty()) return;
    m_inputFileName = fileName;

    ui->inputL->setText(QFileInfo(m_inputFileName).fileName());
}

void MainWindow::selectOutputFile()
{
    const QString fileName = QFileDialog::getSaveFileName(this, "Select output file");
    if (fileName.isEmpty()) return;
    m_outputFileName = fileName;
    ui->outputL->setText(QFileInfo(m_outputFileName).fileName());
}

void MainWindow::saveVector(const std::vector<int>& output) const
{
    QFile outputFile(m_outputFileName);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Failed to open output file" << m_outputFileName << ":" << outputFile.errorString();
        return;
    }

    QDataStream os(&outputFile);
    os << QVector<int>::fromStdVector(output);
}

void MainWindow::loadVector(std::vector<int>& input) const
{
    QFile inputFile(m_inputFileName);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open input file" << m_inputFileName << ":" << inputFile.errorString();
        return;
    }

    QVector<int> inputVector;
    QDataStream is(&inputFile);
    is >> inputVector;

    input = inputVector.toStdVector();
}

void MainWindow::runCommand()
{
#if 0
    // use synthetic input vector
    std::vector<int> inputData = {
        1, 5, 68, 2, 33, 11, 239, 3333,
        5, 68, 2, 3445, 11, 239, 3333, 87,
        1, 5, 68, 2, 33, 11, 239, 3333,
        1, 5, 68, 2, 33, 11, 239, 3333,
        1, 5, 68, 2, 33, 11, 239, 3333,
        1, 5, 68, 2, 33, 11, 239, 3333,
        1, 5, 68, 2, 33, 11, 239, 3333,
        1, 5, 68, 2, 33, 11, 239, 3333
    };
#else
    // load input vector
    std::vector<int> inputData;
    loadVector(inputData);
#endif

    // check input file path
    while (!QFileInfo(m_inputFileName).isReadable()) {
        int status = QMessageBox::critical(
                    this,
                    "Invalid input file",
                    "Input file\n\"" + m_inputFileName + "\"\nnot readable. Please select a different file.",
                    QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
        if (status != QMessageBox::Ok) return;
        selectInputFile();
    }

    // check output file path
    while (!QFileInfo(m_outputFileName).isWritable()) {
        int status = QMessageBox::critical(
                    this,
                    "Invalid output file",
                    "Output file\n\"" + m_outputFileName + "\"\nnot writeable. Please select a different file.",
                    QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
        if (status != QMessageBox::Ok) return;
        selectOutputFile();
    }

    /* =========================================== ===== */
    const QString pluginFile = ui->functionCB->currentData().toString();
    if (pluginFile.isEmpty()) return;

    // open shared object
    qDebug() << "Opening shared object" << pluginFile;
    void* libHandle = dlopen(pluginFile.toLocal8Bit().constData(), RTLD_LAZY);
    if (!libHandle) {
        qWarning() << "Failed to open shared object" << pluginFile << ":" << dlerror();
        dlclose(libHandle);
        return;
    }

    // search transform function
    dlerror(); // clear current error (if any)
    void (*transformFunction)(const std::vector<int>& input, std::vector<int>& output);
    *(void**)(&transformFunction) = dlsym(libHandle, "applyConversion");
    if (!transformFunction || dlerror()) {
        qWarning() << "Could not find function";
        dlclose(libHandle);
        return;
    }

    // apply transform
    std::vector<int> outputData(inputData.size());
    (*transformFunction)(inputData, outputData);

    qDebug() << "Input data:" << QVector<int>::fromStdVector(inputData);
    qDebug() << "Output data:" << QVector<int>::fromStdVector(outputData);

    // close shared object
    dlclose(libHandle);

    // save output vector
    saveVector(outputData);
}

void MainWindow::initFunctionsList()
{
    // specify plugin path
    static const QDir pluginPath("../plugins");

    // list all plugins from plugin path
    const QFileInfoList pluginFiles = pluginPath.entryInfoList(QStringList("*.so"), QDir::Files);
    for (const QFileInfo pluginFile : pluginFiles) {
        // open shared object
        const QString fileName = pluginFile.absoluteFilePath();
        qDebug() << "Opening shared object" << fileName;
        void* libHandle = dlopen(fileName.toLocal8Bit().constData(), RTLD_LAZY);
        if (!libHandle) {
            qWarning() << "Failed to open shared object" << fileName << ":" << dlerror();
            dlclose(libHandle);
            continue;
        }

        // search info function
        dlerror(); // clear current error (if any)
        const char* (*pluginName)(void);
        *(void**)(&pluginName) = dlsym(libHandle, "pluginName");
        if (!pluginName || dlerror()) {
            qWarning() << "Could not find function";
            dlclose(libHandle);
            continue;
        }

        // add plugin to list
        ui->functionCB->addItem((*pluginName)(), pluginFile.absoluteFilePath());

        // close shared object
        dlclose(libHandle);
    }
}
