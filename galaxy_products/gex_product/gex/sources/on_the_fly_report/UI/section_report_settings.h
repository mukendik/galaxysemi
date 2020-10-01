#ifndef SECTION_REPORT_SETTINGS_H
#define SECTION_REPORT_SETTINGS_H

#include <QWidget>
#include <section_element.h>

namespace Ui {
class section_settings;
}

class section_report_settings : public QWidget
{
    Q_OBJECT

public:
    explicit section_report_settings(QWidget *parent = 0);
    ~section_report_settings();


    SectionElement*     getSectionElement      () const { return mSectionElement;}
    void                clear                  ();
    void                loadSectionElement     (SectionElement* sectionElement);

    void keyPressEvent(QKeyEvent* event);

signals:
    void    sectionNameChanged  (const QString& name);
    void    testsSet            (Component*, bool);
    void    changesHasBeenMade  ();

public slots:
    void    updateSectionSettings ();
    void    commentChanged          ();
    void    nameChanged             (const QString& name);
    void    onPickTest              ();
    void    onPickGroups            ();
    void    informUser              ();
    void    updateTestFilterType    ();
    void    updateGroupFilterType   ();
    void    updateTopNValue         (int value);
    void    updateSplitRule         (bool value);

private:
    void    fillListTests       ();
    void    fillListGroups      ();

    Ui::section_settings        *mUi;
    SectionElement              *mSectionElement;
    bool                        mDislayMsgOfNoChildTied;

};

#endif // SECTION_SETTINGS_H
