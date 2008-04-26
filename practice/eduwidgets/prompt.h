//
// C++ Interface: prompt
//
// Description: The prompt displays the question portion or original translation of an entry
///\file prompt.h
//
// Author: David Capel <wot.narg@gmail.com>, (C) 2008
//

 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef PROMPT_H
#define PROMPT_H

#include <eduwidget.h>

/**
* @class Prompt
* @author David Capel <wot.narg@gmail.com>
* @brief This shows the prompt (the question, original translation, image, etc) for each question.
* Depending on the mode, this will take a variety of forms.
* This class gets each question from the database and gives the answer
* to the @class Input object.
*/
class Prompt : public EduWidget
{

    Q_OBJECT

    public slots:
        /// Sets the question entry.
        virtual void slotSetEntry(KEduVocExpression* entry);
        /// Called to show a new prompt and perform other tasks to create a new question.
        virtual void slotNewPrompt();

    protected:        
        KEduVocDocument   *m_doc;
        KEduVocExpression *m_entry;
        
    signals:
        /// Emitted when the question is changed.
        virtual void signalQuestionChanged();
        /// Emitted when the answer has changed.
        /// This is used to provide the answer to the @class Input object.
        virtual void signalAnswerChanged(QString& answer);
};



#endif
