#include <QApplication>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QStatusBar>
#include <QStringList>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QWidget>

class DevForgeWindow final : public QMainWindow {
public:
    DevForgeWindow()
        : process_(this) {
        setWindowTitle("DevForge");
        resize(1040, 700);
        setMinimumSize(760, 520);

        auto *central = new QWidget(this);
        auto *root = new QVBoxLayout(central);
        root->setContentsMargins(20, 18, 20, 18);
        root->setSpacing(14);

        auto *header = new QHBoxLayout();
        auto *brandColumn = new QVBoxLayout();
        auto *title = new QLabel("DevForge");
        title->setObjectName("appTitle");
        auto *subtitle = new QLabel("Linux Developer Toolbox");
        subtitle->setObjectName("muted");
        brandColumn->addWidget(title);
        brandColumn->addWidget(subtitle);
        header->addLayout(brandColumn);
        header->addStretch();

        projectLabel_ = new QLabel("No project selected");
        projectLabel_->setObjectName("projectPath");
        projectLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
        header->addWidget(projectLabel_, 1);
        auto *chooseButton = new QPushButton("Choose Project");
        chooseButton->setObjectName("primaryButton");
        header->addWidget(chooseButton);
        root->addLayout(header);

        auto *separator = new QFrame();
        separator->setFrameShape(QFrame::HLine);
        separator->setObjectName("separator");
        root->addWidget(separator);

        auto *actions = new QHBoxLayout();
        auto *buildButton = new QPushButton("Configure + Build");
        buildButton->setObjectName("primaryButton");
        auto *gitButton = new QPushButton("Check Git Status");
        auto *clearButton = new QPushButton("Clear Output");
        actions->addWidget(buildButton);
        actions->addWidget(gitButton);
        actions->addStretch();
        actions->addWidget(clearButton);
        root->addLayout(actions);

        auto *outputHeader = new QHBoxLayout();
        auto *outputTitle = new QLabel("OUTPUT");
        outputTitle->setObjectName("sectionTitle");
        outputHeader->addWidget(outputTitle);
        outputHeader->addStretch();
        statusLabel_ = new QLabel("Ready");
        statusLabel_->setObjectName("muted");
        outputHeader->addWidget(statusLabel_);
        root->addLayout(outputHeader);

        output_ = new QPlainTextEdit();
        output_->setReadOnly(true);
        output_->setPlaceholderText("Build logs and Git status will appear here.");
        output_->setLineWrapMode(QPlainTextEdit::NoWrap);
        QFont mono("monospace");
        mono.setStyleHint(QFont::Monospace);
        mono.setPointSize(10);
        output_->setFont(mono);
        root->addWidget(output_, 1);

        setCentralWidget(central);
        statusBar()->showMessage("DevForge 0.1.0");

        setStyleSheet(R"(
            QMainWindow, QWidget {
                background: #111318;
                color: #e7e9ef;
                font-family: sans-serif;
                font-size: 13px;
            }
            QLabel#appTitle {
                color: #ffffff;
                font-size: 28px;
                font-weight: 700;
            }
            QLabel#muted {
                color: #9ba3b4;
            }
            QLabel#projectPath {
                color: #c7ccda;
                padding: 8px;
                background: #191d25;
                border: 1px solid #303645;
                border-radius: 6px;
            }
            QLabel#sectionTitle {
                color: #9ba3b4;
                font-size: 11px;
                font-weight: 700;
                letter-spacing: 1px;
            }
            QFrame#separator {
                color: #303645;
            }
            QPushButton {
                color: #e7e9ef;
                background: #252a35;
                border: 1px solid #3a4151;
                border-radius: 7px;
                padding: 9px 14px;
            }
            QPushButton:hover {
                background: #303747;
            }
            QPushButton:disabled {
                color: #737b8b;
                background: #1b1e26;
            }
            QPushButton#primaryButton {
                color: #ffffff;
                background: #536dfe;
                border: 1px solid #6d82ff;
                font-weight: 600;
            }
            QPushButton#primaryButton:hover {
                background: #647bff;
            }
            QPlainTextEdit {
                background: #0b0d11;
                color: #d4d9e5;
                border: 1px solid #303645;
                border-radius: 8px;
                padding: 12px;
                selection-background-color: #536dfe;
            }
            QStatusBar {
                color: #9ba3b4;
                background: #0d0f13;
                border-top: 1px solid #252a35;
            }
        )");

        connect(chooseButton, &QPushButton::clicked, this, [this]() {
            chooseProject();
        });
        connect(buildButton, &QPushButton::clicked, this, [this]() {
            startBuild();
        });
        connect(gitButton, &QPushButton::clicked, this, [this]() {
            checkGitStatus();
        });
        connect(clearButton, &QPushButton::clicked, this, [this]() {
            output_->clear();
        });

        connect(&process_, &QProcess::readyReadStandardOutput, this, [this]() {
            const QString text = QString::fromLocal8Bit(process_.readAllStandardOutput());
            processOutput_ += text;
            appendOutput(text);
        });
        connect(&process_, &QProcess::readyReadStandardError, this, [this]() {
            const QString text = QString::fromLocal8Bit(process_.readAllStandardError());
            processOutput_ += text;
            appendOutput(text);
        });
        connect(&process_,
                qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
                this,
                [this](int exitCode, QProcess::ExitStatus exitStatus) {
                    handleProcessFinished(exitCode, exitStatus);
                });
        connect(&process_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart) {
                appendOutput("\nERROR: Could not start the process. Is the required program installed?\n");
                statusLabel_->setText("Failed to start");
                setBusy(false);
                processMode_ = ProcessMode::None;
            }
        });
    }

private:
    enum class ProcessMode {
        None,
        Configure,
        Build,
        GitStatus
    };

    QProcess process_;
    QPlainTextEdit *output_ = nullptr;
    QLabel *projectLabel_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QString projectPath_;
    QString processOutput_;
    ProcessMode processMode_ = ProcessMode::None;

    void appendOutput(const QString &text) {
        output_->moveCursor(QTextCursor::End);
        output_->insertPlainText(text);
        output_->moveCursor(QTextCursor::End);
    }

    void setBusy(bool busy) {
        statusLabel_->setText(busy ? "Working..." : "Ready");
    }

    void chooseProject() {
        const QString selected = QFileDialog::getExistingDirectory(
            this, "Choose project folder", projectPath_.isEmpty() ? QString() : projectPath_);

        if (selected.isEmpty()) {
            return;
        }

        projectPath_ = selected;
        projectLabel_->setText(projectPath_);
        projectLabel_->setToolTip(projectPath_);
        statusBar()->showMessage("Project selected: " + projectPath_, 5000);
        statusLabel_->setText("Project selected");
        appendOutput("\n> Project selected: " + projectPath_ + "\n");
    }

    bool ensureProjectSelected() {
        if (!projectPath_.isEmpty()) {
            return true;
        }

        appendOutput("ERROR: Choose a project folder first.\n");
        statusLabel_->setText("No project selected");
        return false;
    }

    void runCommand(const QString &program,
                    const QStringList &arguments,
                    ProcessMode mode,
                    const QString &description) {
        if (process_.state() != QProcess::NotRunning) {
            appendOutput("A command is already running. Please wait.\n");
            return;
        }

        processOutput_.clear();
        processMode_ = mode;
        setBusy(true);
        appendOutput("\n> " + description + "\n");
        appendOutput("$ " + program + " " + arguments.join(' ') + "\n");

        process_.setWorkingDirectory(projectPath_);
        process_.setProcessChannelMode(QProcess::SeparateChannels);
        process_.start(program, arguments);
    }

    void startBuild() {
        if (!ensureProjectSelected()) {
            return;
        }

        if (!QFileInfo::exists(projectPath_ + "/CMakeLists.txt")) {
            appendOutput("ERROR: CMakeLists.txt was not found in the selected folder.\n");
            statusLabel_->setText("Not a CMake project");
            return;
        }

        const QString buildDirectory = projectPath_ + "/build";
        runCommand("cmake",
                   {"-S", projectPath_, "-B", buildDirectory, "-G", "Ninja",
                    "-DCMAKE_BUILD_TYPE=Release"},
                   ProcessMode::Configure,
                   "Configuring CMake project");
    }

    void checkGitStatus() {
        if (!ensureProjectSelected()) {
            return;
        }

        runCommand("git", {"status", "--short", "--branch"},
                   ProcessMode::GitStatus, "Checking Git status");
    }

    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
        const ProcessMode finishedMode = processMode_;
        processMode_ = ProcessMode::None;

        if (exitStatus != QProcess::NormalExit || exitCode != 0) {
            appendOutput("\nCommand failed (exit code " + QString::number(exitCode) + ").\n");
            statusLabel_->setText("Command failed");
            setBusy(false);
            return;
        }

        if (finishedMode == ProcessMode::Configure) {
            appendOutput("\nCMake configure completed successfully.\n");
            runCommand("cmake",
                       {"--build", projectPath_ + "/build", "--parallel", "2"},
                       ProcessMode::Build,
                       "Building project");
            return;
        }

        if (finishedMode == ProcessMode::Build) {
            appendOutput("\nBuild completed successfully.\n");
            statusLabel_->setText("Build succeeded");
            statusBar()->showMessage("Build completed successfully", 5000);
            setBusy(false);
            return;
        }

        if (finishedMode == ProcessMode::GitStatus) {
            appendOutput("\nGit status check completed.\n");
            if (processOutput_.trimmed().isEmpty()) {
                appendOutput("Git returned no status text.\n");
            }
            statusLabel_->setText("Git status checked");
            setBusy(false);
            return;
        }

        setBusy(false);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("DevForge");
    QApplication::setApplicationVersion("0.1.0");

    DevForgeWindow window;
    window.show();

    return app.exec();
}
