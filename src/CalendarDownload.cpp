/*
 * Copyright (c) 2010 Mark Liversedge (liversedge@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "CalendarDownload.h"
#ifdef GC_HAVE_ICAL
#include "ICalendar.h"
#include <libical/ical.h>
#endif

CalendarDownload::CalendarDownload(MainWindow *main) : main(main)
{
    nam = new QNetworkAccessManager(this);
    connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
}

bool
CalendarDownload::download()
{
    QString request = appsettings->cvalue(main->cyclist, GC_WEBCAL_URL, "").toString();
    if (request == "") return false;
    else {
        // change webcal to http, since it is basically the same port
        QRegExp webcal("^webcal:");
        request.replace(webcal, QString("http:"));
    }

    QNetworkReply *reply = nam->get(QNetworkRequest(QUrl(request)));

    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::warning(main, tr("Calendar Data Download"), reply->errorString());
        return false;
    }
    return true;
}

#if 0
static QString propertyToString(icalproperty *p)
{
    if (p) {
        icalvalue *v = icalproperty_get_value(p);
        QString converted(icalvalue_as_ical_string(v));

        // some special characters are escaped in the text
        converted.replace("\\n", "\n");
        converted.replace("\\;", ";");

        return converted;
    } else {
        return QString("");
    }
}

static QDate propertyToDate(icalproperty *p)
{
    if (p) {
        icalvalue *v = icalproperty_get_value(p);
        struct icaltimetype date = icalvalue_get_datetime(v);
        QDate when(date.year, date.month, date.day);
        return when;
    } else {
        return QDate();
    }
}
#endif

void
CalendarDownload::downloadFinished(QNetworkReply *reply)
{
    QString fulltext = reply->readAll();
    QStringList errors;

#ifdef GC_HAVE_ICAL
    QString remoteCache = main->home.absolutePath()+"/remote.ics";
    QFile remoteCacheFile(remoteCache);

    if (fulltext != "") {

        // update remote cache - write to it!
        remoteCacheFile.open(QFile::ReadWrite | QFile::Text);
        QTextStream out(&remoteCacheFile);
        out << fulltext;
        remoteCacheFile.close();

    } else {

        if (remoteCacheFile.exists()) {
            QMessageBox msgBox;
            msgBox.setText("Remote Calendar not available, reverting to cached workouts.");
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();

            // read cache
            // read in the whole thing
            remoteCacheFile.open(QFile::ReadOnly | QFile::Text);
            QTextStream in(&remoteCacheFile);
            fulltext = in.readAll();
            remoteCacheFile.close();
        }
    }

    if (fulltext != "") main->rideCalendar->refreshRemote(fulltext);
#endif
}
