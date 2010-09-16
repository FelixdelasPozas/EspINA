#include <QApplication>
#include "espina.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Espina espina;
    espina.show();
    return app.exec();
};
