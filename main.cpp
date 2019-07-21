//
// Created by hengfeng zhuo on 2019-07-20.
//
#include "main.h"
#include "dialog.h"

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;
    w.show();

    return a.exec();
}
