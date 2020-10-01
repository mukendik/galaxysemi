#ifndef XML_MANAGER_H
#define XML_MANAGER_H

#include <QDomDocument>

namespace XML
{

    /**
     * @brief The Observer class. Used to update the progress bar
     */
    class Observer
    {
        public:
            Observer(){}
            virtual ~Observer(){}

            virtual void Notify() = 0;
    };


    class XMLParser
    {
        public :

            XMLParser();
            ~XMLParser();

            bool        LoadXMLFile             (const char* fileToLoad);

            QString     GetNameSpaceURI         (const QString &tagName) const;
            QString     GetAttribute            (const QString &tagName, const QString &attributeName) const;
            QString     GetText                 (const QString &tagName) const;

            /**
             * @brief return the number of node with the same tag tagName
             * @param tagName
             * @return the number of node with the same tag
             */
            int         GetNbIdenticalNodes     (const QString &tagName) const { return  mQDomDocument.elementsByTagName(tagName).size(); }
            bool        HasNode                 (const QString &tagName) const { return  (mQDomDocument.elementsByTagName(tagName).size() > 0); }

            /**
             * @brief Set the tag with multiple instance that will be looped
             * @param tagName
             */
            void        SetRepeatedNodes        (const QString &tagName);
            /**
             * @brief go to the first or next node set by the function SetRepeatedNodes
             * @return
             */
            bool        NextRepeatedNode        ()  ;
            QString     GetRepeatedNodeAttribut (const QString &attributeName) const;
            QString     GetRepeatedNodeText     () const;

            void        AttachObserver          (Observer* observer) {mListObservers.push_back(observer);}
            int         GetProgressStep         () const { return mProgressStep; }
            int         GetTotalNodes           () const { return mTotalNodes; }


    private:
            QList<Observer*>    mListObservers;
            QDomDocument        mQDomDocument;
            QDomElement         mCurrentElement;
            QDomElement         mNullElement;
            QDomNodeList        mScannedElements;
            mutable int         mScannedIndex ;
            mutable int         mProgressStep;
            int                 mTotalNodes;

            void            UpdateProgressStep  () const;
            void            CalculNbTotalNodes  (QDomNode node);
            QDomElement     GetDomElement       (QDomElement element, const QString& tagName) const;
            QString         GetAttribute        (QDomElement element, const QString& tagName, const QString& attributeName) const;
            QString         GetNameSpaceURI     (QDomElement element, const QString& tagName) const;
            QString         GetText             (QDomElement element, const QString &tagName) const;
    };

}

#endif

