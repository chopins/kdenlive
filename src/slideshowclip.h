/***************************************************************************
 *   Copyright (C) 2008 by Jean-Baptiste Mardelle (jb@kdenlive.org)        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/


#ifndef SLIDESHOWCLIP_H
#define SLIDESHOWCLIP_H


#include "definitions.h"
#include "docclipbase.h"
#include "timecode.h"
#include "ui_slideshowclip_ui.h"

#include <QFuture>
#include <QFutureWatcher>

class SlideshowClip : public QDialog
{
    Q_OBJECT

public:
    SlideshowClip(Timecode tc, QWidget * parent = 0);
    /** return selected path for slideshow in MLT format */
    QString selectedPath() const;
    QString clipName() const;
    QString clipDuration() const;
    QString lumaDuration() const;
    int imageCount() const;
    bool loop() const;
    bool fade() const;
    QString lumaFile() const;
    int softness() const;

private slots:
    void parseFolder();
    void slotEnableLuma(int state);
    void slotEnableThumbs(int state);
    void slotEnableLumaFile(int state);
    void doGetThumbs();
    void slotUpdateDurationFormat(int ix);
    void slotGenerateThumbs();
    void slotCheckGenerateThumbs();

private:
    Ui::SlideshowClip_UI m_view;
    int m_count;
    Timecode m_timecode;
    QFuture<void> m_future;
    QFutureWatcher<void> m_watcher;
};


#endif

