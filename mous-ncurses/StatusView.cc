#include "StatusView.h"

StatusView::StatusView()
{
}

StatusView::~StatusView()
{
}

bool StatusView::InjectKey(int key)
{
    switch (key) {
        case ' ':
            break;

        case '>':
            break;

        case '<':
            break;

        case 'n':
            break;

        case 'p':
            break;

        case '+':
            break;

        case '-':
            break;

        default:
            return false;
    }
    return true;
}
