#include <QApplication>
#include <QPushButton>
#include <QMessageBox>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QPushButton button("Click me!");
    button.resize(200, 60);

    QObject::connect(&button, &QPushButton::clicked, [&]() {
        QMessageBox::information(&button, "Hello", "Hello from Qt!");
    });

    button.show();
    return app.exec();
}