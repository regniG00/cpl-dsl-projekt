// ============================================================================
// File: src/ir/IRBuilder.cpp
// ============================================================================
#include "ir/IRBuilder.h"

#include <any>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <utility>

using Parser = AufgabenerstellungsgrammatikParser;
using std::any;
using std::any_cast;

int IRBuilder::parseIntStrict(const std::string& s) {
    try {
        size_t idx = 0;
        int v = std::stoi(s, &idx, 10);
        if (idx != s.size()) throw std::runtime_error("non-int tail");
        return v;
    } catch (...) {
        throw std::runtime_error("Could not parse int from: '" + s + "'");
    }
}

bool IRBuilder::isWordishToken(int tokenType) {
    return tokenType == AufgabenerstellungsgrammatikLexer::LETTERS ||
           tokenType == AufgabenerstellungsgrammatikLexer::NUMBER;
}

bool IRBuilder::isPunctToken(int tokenType) {
    return tokenType == AufgabenerstellungsgrammatikLexer::PUNCTUATION;
}

bool IRBuilder::isNoSpaceLeftToken(int tokenType) {
    // no space BEFORE these tokens
    return isPunctToken(tokenType) ||
           tokenType == AufgabenerstellungsgrammatikLexer::CONNECTION || // ',' only
           tokenType == AufgabenerstellungsgrammatikLexer::T__1 || // ')'
           tokenType == AufgabenerstellungsgrammatikLexer::T__6 || // ']'
           tokenType == AufgabenerstellungsgrammatikLexer::T__3 || // ';'
           tokenType == AufgabenerstellungsgrammatikLexer::T__5 || // '/'
           tokenType == AufgabenerstellungsgrammatikLexer::T__7;   // ',' literal in grammar likely ends up CONNECTION; keep safe
}

bool IRBuilder::isNoSpaceRightToken(int tokenType) {
    // no space AFTER these tokens
    return tokenType == AufgabenerstellungsgrammatikLexer::T__0 || // '('
           tokenType == AufgabenerstellungsgrammatikLexer::T__4 || // '-'
           tokenType == AufgabenerstellungsgrammatikLexer::T__2 || // ':'
           tokenType == AufgabenerstellungsgrammatikLexer::T__5 || // '/'
           tokenType == AufgabenerstellungsgrammatikLexer::T__4 || // '-'
           tokenType == AufgabenerstellungsgrammatikLexer::T__5 || // '/'
           tokenType == AufgabenerstellungsgrammatikLexer::T__6;   // '['
}

std::string IRBuilder::textJoin(antlr4::ParserRuleContext* ctx, bool keepNewlines) const {
    if (!ctx || !tokens || !ctx->getStart() || !ctx->getStop()) return "";

    int a = ctx->getStart()->getTokenIndex();
    int b = ctx->getStop()->getTokenIndex();
    if (a < 0 || b < a) return "";

    std::ostringstream out;
    antlr4::Token* prev = nullptr;

    for (int i = a; i <= b; ++i) {
        auto* t = tokens->get(i);
        if (!t) continue;

        int tt = t->getType();

        if (tt == AufgabenerstellungsgrammatikLexer::NEWLINE) {
            if (keepNewlines) out << "\n";
            prev = nullptr;
            continue;
        }

        const std::string cur = t->getText();

        if (prev) {
            int pt = prev->getType();

            // join digits: NUMBER followed by NUMBER => no space ("1""6" -> "16")
            if (pt == AufgabenerstellungsgrammatikLexer::NUMBER && tt == AufgabenerstellungsgrammatikLexer::NUMBER) {
                // no space
            }
            // default word spacing
            else if (isWordishToken(pt) && isWordishToken(tt)) {
                out << ' ';
            }
            // space between word and '('? (want "ist (" not "ist(")
            else if (isWordishToken(pt) && tt == AufgabenerstellungsgrammatikLexer::T__0) { // '('
                out << ' ';
            }
            // space between ')' and word? (want ") zu")
            else if (pt == AufgabenerstellungsgrammatikLexer::T__1 && isWordishToken(tt)) { // ')'
                out << ' ';
            }
            // prevent spaces around punctuation/brackets
            else if (isNoSpaceLeftToken(tt) || isNoSpaceRightToken(pt)) {
                // no space
            }
            // conservative: space between wordish and wordish handled; else keep none
        }

        out << cur;
        prev = t;
    }

    return out.str();
}

std::string IRBuilder::readEndlessWords(Parser::Endless_wordsContext* ew) const {
    if (!ew) return "";
    // endless_words has no punctuation; keepNewlines=false
    return textJoin(ew, false);
}

SentenceIR IRBuilder::readSentence(Parser::SentenceContext* s) const {
    SentenceIR outS;
    if (!s) return outS;

    outS.text = readEndlessWords(s->endless_words());

    if (auto* p = s->PUNCTUATION()) {
        const std::string t = p->getText();
        outS.punctuation = t.empty() ? '.' : t[0];
    } else {
        outS.punctuation = '.';
    }
    return outS;
}

// prog: tasks NEWLINE? EOF;
any IRBuilder::visitProg(Parser::ProgContext* ctx) {
    ProgramIR prog;

    auto* tasksCtx = ctx->tasks();
    if (!tasksCtx) return prog;

    for (auto* td : tasksCtx->task_definition()) {
        TaskIR t = any_cast<TaskIR>(visitTask_definition(td));
        prog.tasks.push_back(std::move(t));
    }

    return prog;
}

// task_definition: endless_words task;
any IRBuilder::visitTask_definition(Parser::Task_definitionContext* ctx) {
    TaskIR task;

    task.header = readEndlessWords(ctx->endless_words());

    auto* tctx = ctx->task();
    if (!tctx) {
        task.type = "Unknown";
        return task;
    }

    // ----------------------------
    // RoF
    // ----------------------------
    if (tctx->RIGHT_OR_FALSE()) {
        task.type = "RoF";

        for (auto* tf : tctx->true_false_task()) {
            TrueFalseTaskIR line;
            line.question = readSentence(tf->question_or_statement()->sentence());

            auto* ans = tf->true_false_answer();
            if (ans->ANSWER_TRUE()) {
                line.answer.isTrue = true;
                line.answer.reason.reset();
            } else {
                line.answer.isTrue = false;
                if (ans->reason()) {
                    // reason: sentence | endless_words (error branch)
                    if (ans->reason()->sentence()) {
                        line.answer.reason = readSentence(ans->reason()->sentence());
                    } else if (ans->reason()->endless_words()) {
                        SentenceIR rs;
                        rs.text = readEndlessWords(ans->reason()->endless_words());
                        rs.punctuation = '.'; // unknown (error branch), default
                        line.answer.reason = rs;
                    }
                }
            }

            task.rof.push_back(std::move(line));
        }

        return task;
    }

    // ----------------------------
    // Sorting
    // ----------------------------
    if (tctx->SORTING()) {
        task.type = "Umordnung";

        for (auto* s : tctx->sorting_task()) {
            SortingLineIR line;
            line.question = readSentence(s->question_or_statement()->sentence());

            if (s->positive_task_point()) {
                line.points.pointsIfAllCorrect = parseIntStrict(s->positive_task_point()->getText());
                line.points.scoringMode = ScoringModeIR::AllOrNothing;
            } else {
                line.points.pointsIfAllCorrect.reset();
                line.points.scoringMode = ScoringModeIR::PartialPerCorrect;
            }

            for (auto* it : s->item()) {
                if (auto* w = it->word()) line.items.push_back(w->getText());
            }

            task.sorting.push_back(std::move(line));
        }

        return task;
    }

    // ----------------------------
    // Matching
    // ----------------------------
    if (tctx->MATCHING()) {
        task.type = "Zuordnung";

        for (auto* m : tctx->matching_task()) {
            MatchingLineIR line;

            // matching_question_or_statement:
            // endless_words '(' word ')' endless_words '(' word')' PUNCTUATION;
            auto* mq = m->matching_question_or_statement();
            line.question.prefix = readEndlessWords(mq->endless_words(0));
            line.question.slotA  = mq->word(0)->getText();
            line.question.middle = readEndlessWords(mq->endless_words(1));
            line.question.slotB  = mq->word(1)->getText();
            line.question.punctuation = mq->PUNCTUATION()->getText()[0];

            if (m->positive_task_point()) {
                line.points.pointsIfAllCorrect = parseIntStrict(m->positive_task_point()->getText());
                line.points.scoringMode = ScoringModeIR::AllOrNothing;
            } else {
                line.points.pointsIfAllCorrect.reset();
                line.points.scoringMode = ScoringModeIR::PartialPerCorrect;
            }

            for (auto* mi : m->matching_item()) {
                MatchingItemIR p;
                auto ws = mi->word();
                p.left  = ws.size() >= 1 ? ws[0]->getText() : "";
                p.right = ws.size() >= 2 ? ws[1]->getText() : "";
                line.pairs.push_back(std::move(p));
            }

            task.matching.push_back(std::move(line));
        }

        return task;
    }

    // ----------------------------
    // Marking
    // ----------------------------
    if (tctx->MARKING()) {
        task.type = "Markierung";

        MarkingTaskIR out;
        auto* mt = tctx->marking_task();
        out.question = readSentence(mt->question_or_statement()->sentence());

        // marking_text: ((sentence | marked_sentence)+ NEWLINE?)*;
        if (auto* txt = mt->marking_text()) {
            for (auto* ms : txt->marked_sentence()) {
                MarkingSentenceIR s;
                s.punctuation = ms->PUNCTUATION()->getText()[0];

                // We build parts by walking children tokens in this marked_sentence context.
                // Plain text comes from endless_words chunks, marks from marked_word.
                //
                // marked_sentence: (endless_words? (marked_word endless_words?)+ PUNCTUATION);
                // marked_word: '(' endless_words ')' marked_word_point;
                //
                // Strategy: iterate through marked_word nodes and optional endless_words around them.

                // prefix endless_words?
                size_t ewIdx = 0;
                if (ms->endless_words().size() > 0) {
                    // first endless_words is prefix if it appears before first marked_word
                    // In this rule, endless_words? appears before (marked_word ...)+
                    // It will be ms->endless_words(0)
                    MarkingPartIR p;
                    p.text = readEndlessWords(ms->endless_words(ewIdx));
                    p.mark.reset();
                    s.parts.push_back(std::move(p));
                    ewIdx++;
                }

                for (size_t i = 0; i < ms->marked_word().size(); ++i) {
                    auto* mw = ms->marked_word(i);

                    MarkedSpanIR mark;
                    mark.markedText = readEndlessWords(mw->endless_words());

                    // marked_word_point: ('[' endless_words ',' positive_task_point ']') | ('[' positive_task_point ']');
                    auto* mp = mw->marked_word_point();
                    if (mp->endless_words()) {
                        mark.correction = readEndlessWords(mp->endless_words());
                    } else {
                        mark.correction.reset();
                    }
                    mark.points = parseIntStrict(mp->positive_task_point()->getText());

                    MarkingPartIR pm;
                    pm.text.clear();
                    pm.mark = mark;
                    s.parts.push_back(std::move(pm));

                    // trailing endless_words? after this mark (if present)
                    if (ewIdx < ms->endless_words().size()) {
                        MarkingPartIR pt;
                        pt.text = readEndlessWords(ms->endless_words(ewIdx));
                        pt.mark.reset();
                        s.parts.push_back(std::move(pt));
                        ewIdx++;
                    }
                }

                out.sentences.push_back(std::move(s));
            }

            // also include plain sentences (no marks) inside marking_text
            for (auto* plain : txt->sentence()) {
                MarkingSentenceIR s;
                s.punctuation = plain->PUNCTUATION()->getText()[0];
                MarkingPartIR p;
                p.text = readEndlessWords(plain->endless_words());
                s.parts.push_back(std::move(p));
                out.sentences.push_back(std::move(s));
            }
        }

        task.marking = std::move(out);
        return task;
    }

    // ----------------------------
    // Cloze
    // ----------------------------
    if (tctx->CLOZE_TEXT()) {
        task.type = "Lückentext";

        ClozeTaskIR out;
        auto* ct = tctx->cloze_task();
        out.question = readSentence(ct->question_or_statement()->sentence());

        // cloze_text: ((sentence | cloze_sentence)+ NEWLINE?)*;
        if (auto* txt = ct->cloze_text()) {
            for (auto* cs : txt->cloze_sentence()) {
                ClozeSentenceIR s;
                s.punctuation = cs->PUNCTUATION()->getText()[0];

                // cloze_sentence: (endless_words? (cloze_word endless_words?)+ PUNCTUATION);
                size_t ewIdx = 0;

                if (cs->endless_words().size() > 0) {
                    ClozePartIR p;
                    p.text = readEndlessWords(cs->endless_words(ewIdx));
                    p.blank.reset();
                    s.parts.push_back(std::move(p));
                    ewIdx++;
                }

                for (size_t i = 0; i < cs->cloze_word().size(); ++i) {
                    auto* cw = cs->cloze_word(i);

                    // cloze_word: '(' word ',' positive_task_point ')'
                    ClozeBlankIR b;
                    b.solution = cw->word()->getText();
                    b.points = parseIntStrict(cw->positive_task_point()->getText());

                    ClozePartIR pb;
                    pb.text.clear();
                    pb.blank = b;
                    s.parts.push_back(std::move(pb));

                    if (ewIdx < cs->endless_words().size()) {
                        ClozePartIR pt;
                        pt.text = readEndlessWords(cs->endless_words(ewIdx));
                        pt.blank.reset();
                        s.parts.push_back(std::move(pt));
                        ewIdx++;
                    }
                }

                out.sentences.push_back(std::move(s));
            }

            // include plain sentence lines too
            for (auto* plain : txt->sentence()) {
                ClozeSentenceIR s;
                s.punctuation = plain->PUNCTUATION()->getText()[0];
                ClozePartIR p;
                p.text = readEndlessWords(plain->endless_words());
                s.parts.push_back(std::move(p));
                out.sentences.push_back(std::move(s));
            }
        }

        task.cloze = std::move(out);
        return task;
    }

    // ----------------------------
    // Correction
    // ----------------------------
    if (tctx->CORRECTION_TEXT()) {
        task.type = "Textkorrektur";

        CorrectionTaskIR out;
        auto* ct = tctx->correction_task();
        out.question = readSentence(ct->question_or_statement()->sentence());

        // correction_text: ((sentence | correction_sentence)+ NEWLINE?)*;
        if (auto* txt = ct->correction_text()) {
            for (auto* cs : txt->correction_sentence()) {
                CorrectionSentenceIR s;
                s.punctuation = cs->PUNCTUATION()->getText()[0];

                // correction_sentence: (endless_words? (correction_word endless_words?)+ PUNCTUATION);
                size_t ewIdx = 0;

                if (cs->endless_words().size() > 0) {
                    CorrectionPartIR p;
                    p.text = readEndlessWords(cs->endless_words(ewIdx));
                    p.corr.reset();
                    s.parts.push_back(std::move(p));
                    ewIdx++;
                }

                for (size_t i = 0; i < cs->correction_word().size(); ++i) {
                    auto* cw = cs->correction_word(i);

                    // correction_word: '(' word ')' ('[' word ',' positive_task_point ']');
                    CorrectionSpanIR c;
                    c.wrong = cw->word(0)->getText();
                    c.correct = cw->word(1)->getText();
                    c.points = parseIntStrict(cw->positive_task_point()->getText());

                    CorrectionPartIR pc;
                    pc.text.clear();
                    pc.corr = c;
                    s.parts.push_back(std::move(pc));

                    if (ewIdx < cs->endless_words().size()) {
                        CorrectionPartIR pt;
                        pt.text = readEndlessWords(cs->endless_words(ewIdx));
                        pt.corr.reset();
                        s.parts.push_back(std::move(pt));
                        ewIdx++;
                    }
                }

                out.sentences.push_back(std::move(s));
            }

            for (auto* plain : txt->sentence()) {
                CorrectionSentenceIR s;
                s.punctuation = plain->PUNCTUATION()->getText()[0];
                CorrectionPartIR p;
                p.text = readEndlessWords(plain->endless_words());
                s.parts.push_back(std::move(p));
                out.sentences.push_back(std::move(s));
            }
        }

        task.correction = std::move(out);
        return task;
    }

    // ----------------------------
    // Choice
    // ----------------------------
    if (tctx->CHOICE_TEXT()) {
        task.type = "Auswahl";

        for (auto* ch : tctx->choice_task()) {
            ChoiceLineIR line;
            line.question = readSentence(ch->question_or_statement()->sentence());

            // correct_choice+ (each has points)
            for (auto* cc : ch->correct_choice()) {
                ChoiceOptionIR opt;
                opt.isCorrect = true;
                opt.text = readEndlessWords(cc->endless_words());
                opt.points = parseIntStrict(cc->positive_task_point()->getText());
                line.options.push_back(std::move(opt));
            }

            // false_choices
            if (auto* fc = ch->false_choices()) {
                auto ews = fc->endless_words();
                auto nps = fc->negative_task_point(); // may be empty if alt without points

                for (size_t i = 0; i < ews.size(); ++i) {
                    ChoiceOptionIR opt;
                    opt.isCorrect = false;
                    opt.text = readEndlessWords(ews[i]);
                    if (i < nps.size()) opt.points = parseIntStrict(nps[i]->getText());
                    else opt.points = 0;
                    line.options.push_back(std::move(opt));
                }
            }

            task.choice.push_back(std::move(line));
        }

        return task;
    }

    task.type = "Unknown";
    return task;
}
