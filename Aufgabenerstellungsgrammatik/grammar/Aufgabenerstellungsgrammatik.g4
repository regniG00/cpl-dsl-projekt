grammar Aufgabenerstellungsgrammatik;
/** Overarching Logic */
prog:   tasks NEWLINE? EOF;
tasks:  (task_definition NEWLINE)* task_definition ;
task_definition:   endless_words task;

/** Basic Non-Terminal */
sentence: endless_words PUNCTUATION;
endless_words:  (word)+ (CONNECTION? (word)+)*;


task:   '(' RIGHT_OR_FALSE ')' ':' NEWLINE? true_false_task (NEWLINE true_false_task)* ';'
        | '(' SORTING ')' ':' NEWLINE? sorting_task (NEWLINE sorting_task)* ';'
        | '(' MATCHING ')' ':' NEWLINE? matching_task (NEWLINE matching_task)* ';'
        | '(' MARKING ')' ':' NEWLINE? marking_task ';'
        | '(' CLOZE_TEXT')' ':' NEWLINE? cloze_task ';'
        | '('CORRECTION_TEXT')' ':' NEWLINE? correction_task ';'
        | '('CHOICE_TEXT')' ':' NEWLINE? choice_task (NEWLINE choice_task)* ';';


/** Tasks */
true_false_task:        question_or_statement true_false_answer;

/**
Sortieraufgabe - Die Richtige Reihenfolge wird angegeben.
Wenn eine Punktzahl angegeben wird, gibt es dies Punkte nur bei der korrekten Reihenfolge.
Wenn keine Punkte angegeben sind, gibt es anteilig für jeden korrekte Antwort einen Punkt.

Also bei der Reihenfolge 1,2,4,3 gibt es 2 von 4 Punkten
*/
sorting_task:           question_or_statement ('('positive_task_point')')? item+;

/**
Zuordnungsaufgabe - Die richtigen Paare werden angegeben.
Punktevergabe hat das gleiche Prinzip wie bei Sortierungsaufgabe.
Wenn Punktzahl angegeben dann gibt es nur Punkte wenn alle korrekt Zugeordnet wurden.
Ansonsten 1 Punkt für jedes richtige Paar.
 */
matching_task:          matching_question_or_statement ('('positive_task_point')')? matching_item+;

/** Markiere alle Autoren - Nur markieren */
/** Markiere alle Rechtschreibfehler - Nur markieren, aber mit Korrektur der Wörter */
marking_task:           question_or_statement NEWLINE? marking_text; 

/** Markiere alle Rechtschreibfehler und korrigiere diese - Markieren und selber korrigieren */
correction_task:        question_or_statement NEWLINE? correction_text;

/** Lückentext - Cloze */
cloze_task:             question_or_statement NEWLINE? cloze_text;

/** Single oder Multiple Choice */
choice_task:            question_or_statement correct_choice+ false_choices;


/** Task items */
question_or_statement: sentence;
matching_question_or_statement: endless_words '(' word ')' endless_words '(' word')' PUNCTUATION;
true_false_answer:   ANSWER_TRUE
        | ANSWER_FALSE ARROW reason;
reason: sentence
        | {notifyErrorListeners("Begründung muss mit einem Satzzeichen enden (. ! ?).");} endless_words;
item:   ('-'word);
matching_item: '-'word'/'word;

correct_choice: '-'endless_words('('positive_task_point')');
false_choices: ('-'endless_words('('negative_task_point')'))+
                |('-'endless_words)+;

marking_text:  ((sentence | marked_sentence)+ NEWLINE?)*;
marked_sentence: (endless_words? (marked_word endless_words?)+ PUNCTUATION);
marked_word: '(' endless_words ')' marked_word_point;
marked_word_point: ('[' endless_words ','positive_task_point']') | ('['positive_task_point']');

correction_text:  ((sentence | correction_sentence)+ NEWLINE?)*;
correction_sentence: (endless_words? (correction_word endless_words?)+ PUNCTUATION);
correction_word: '(' word ')' ('[' word ','positive_task_point']');

cloze_text:     ((sentence | cloze_sentence)+ NEWLINE?)*;
cloze_sentence: (endless_words? (cloze_word endless_words?)+ PUNCTUATION);
cloze_word:     ('(' word ','positive_task_point')');

word : (LETTERS | NUMBER);
positive_task_point: NUMBER;
negative_task_point: ('-'NUMBER);


/** Terminal (Needs to be extended) */
/** Task Types */
RIGHT_OR_FALSE  :'RoF';
SORTING :'Umordnung';
MATCHING:'Zuordnung';
MARKING:'Markierung';
CLOZE_TEXT:'Lückentext';
CORRECTION_TEXT:'Textkorrektur';
CHOICE_TEXT:'Auswahl';

ANSWER_TRUE: '-Richtig';
ANSWER_FALSE: '-Falsch';

/** Terminal */
PUNCTUATION: ('.'|'?'|'!');
//TRUE    : ('Richtig');
//FALSE   : ('Falsch');
LETTERS : [\p{L}]+;
NUMBER: [0-9]+;
//WORD    : ('a'..'z' | 'A'..'Z' | [0-9] | [öÖäÄüÜß])+;
CONNECTION: (',' | ':' | '-' | ';');
ARROW: '->';

/** Skip unnecessary whitespaces */
WS: [ \t]+ -> skip ;
NEWLINE: '\r'? '\n';

/** Gobble literals unless specified character is encountered */
/**TEXT : ~[;\-\[\]\(\)\->\n]+ ;*/