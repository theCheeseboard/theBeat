#include "playlistlistwidget.h"

PlaylistListWidget::PlaylistListWidget(QWidget *parent) : QListView(parent)
{

}

QSize PlaylistListWidget::sizeHint() const {
    QSize oldHint = QListView::sizeHint();
    oldHint.setHeight(0);
    return oldHint;
}
