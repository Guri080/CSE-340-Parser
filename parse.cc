#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"
#include "lexer.h"
#include <iostream>
#include <map>

using namespace std;

map<string, int> locationDict;
LexicalAnalyzer lexer;

int location(string str, bool isVar)
{
    // make the variables are unique inside mem
    if (locationDict.count(str) <= 0)
    {
        locationDict[str] = next_available;

        if (isVar)
        {
            mem[next_available] = 0;
        }
        else
        {
            mem[next_available] = stoi(str);
        }
        next_available++;
    }
    return next_available;
}

struct InstructionNode *insertNode(InstructionNode *head, InstructionNode *&last, InstructionNode *newNode)
{

    if (head == nullptr)
    {
        head = newNode;
        last = newNode;
    }
    else
    {
        last->next = newNode;
        last = newNode;
    }

    newNode->next = nullptr;
    return head;
}

struct InstructionNode *insertList(InstructionNode *head1, InstructionNode *&last, InstructionNode *head2)
{
    if (head1 == nullptr)
    {
        head1 = head2;
        last = head2;
        while (last != nullptr && last->next != nullptr)
        {
            last = last->next;
        }
    }
    else
    {
        last->next = head2;

        if (head2 != nullptr)
        {
            last = head2;
            while (last->next != nullptr)
            {
                last = last->next;
            }
        }
    }

    return head1;
}

struct InstructionNode *parse_generate_intermediate_representation()
{
    Token token;
    InstructionNode *head = nullptr;
    InstructionNode *last = nullptr;

    // get the var_section
    do
    {
        token = lexer.GetToken();
        // if the token type is LBRACE then that means
        // this method was called from a recurssive method
        // in WHILE, IF, FOR or SWITCH. At that point we already have the
        // var body so break
        if (token.token_type == LBRACE)
        {
            break;
        }
        if (token.token_type == ID)
        {
            location(token.lexeme, true);
        }

    } while (token.token_type != SEMICOLON);

    // comsume the semicolon
    if (lexer.peek(1).token_type == SEMICOLON)
    {
        lexer.GetToken();
    }

    // get the body section
    do
    {
        // if the next token is semicolon then consume the token
        if (lexer.peek(1).token_type == SEMICOLON)
        {
            token = lexer.GetToken();
        }
        token = lexer.GetToken();

        // if the token type is INPUT
        if (token.token_type == INPUT)
        {
            token = lexer.GetToken();
            InstructionNode *newNode = new InstructionNode;
            newNode->type = IN;
            newNode->input_inst.var_index = locationDict[token.lexeme];
            // insert the newNode
            head = insertNode(head, last, newNode);
        }
        else if (token.token_type == ID)
        {
            // use the token
            InstructionNode *newNode = new InstructionNode;
            newNode->type = ASSIGN; // c = 10
            newNode->assign_inst.left_hand_side_index = locationDict[token.lexeme];
            newNode->assign_inst.op = OPERATOR_NONE;

            // to to see if the next opertor is an =
            // if it then consume that token and then
            // get the next token
            if (lexer.peek(1).token_type == EQUAL)
            {
                token = lexer.GetToken();
            }

            token = lexer.GetToken();

            // if the token is a variable like a,b,c...
            if (token.token_type == NUM)
            {
                string num = token.lexeme;
                // insert to mem if num is unique.
                // 'location' method has a check for
                // uniqueness
                location(num, false);
                newNode->assign_inst.operand1_index = locationDict[num];
            }
            else if (token.token_type == ID)
            {
                newNode->assign_inst.operand1_index = locationDict[token.lexeme];
            }

            token = lexer.GetToken();

            if (token.token_type == PLUS)
                newNode->assign_inst.op = OPERATOR_PLUS;
            else if (token.token_type == MINUS)
                newNode->assign_inst.op = OPERATOR_MINUS;
            else if (token.token_type == MULT)
                newNode->assign_inst.op = OPERATOR_MULT;
            else if (token.token_type == DIV)
                newNode->assign_inst.op = OPERATOR_DIV;

            // if the token is a semicolon do not get it
            if (token.token_type != SEMICOLON)
            {
                token = lexer.GetToken();
            }

            if (newNode->assign_inst.op != OPERATOR_NONE)
            {
                if (token.token_type == NUM)
                {
                    string num = token.lexeme;
                    // insert to mem if num is unique.
                    // 'location' method has a check for
                    // uniqueness
                    location(num, false);
                    newNode->assign_inst.operand2_index = locationDict[num];
                }
                else if (token.token_type == ID)
                {
                    newNode->assign_inst.operand2_index = locationDict[token.lexeme];
                }
            }

            // insert the new node
            head = insertNode(head, last, newNode);
        }
        else if (token.token_type == OUTPUT)
        {
            token = lexer.GetToken();
            InstructionNode *newNode = new InstructionNode;
            newNode->type = OUT;
            newNode->output_inst.var_index = locationDict[token.lexeme];
            head = insertNode(head, last, newNode);
        }
        else if (token.token_type == IF)
        {

            InstructionNode *ifNode = new InstructionNode;
            ifNode->type = CJMP;

            // get the while statment. Eg d > 1 then operand1 = d operand2 = 1
            token = lexer.GetToken();
            ifNode->cjmp_inst.operand1_index = locationDict[token.lexeme];

            // get the condition
            token = lexer.GetToken();
            if (token.token_type == GREATER)
                ifNode->cjmp_inst.condition_op = CONDITION_GREATER;
            else if (token.token_type == LESS)
                ifNode->cjmp_inst.condition_op = CONDITION_LESS;
            else if (token.token_type == NOTEQUAL)
                ifNode->cjmp_inst.condition_op = CONDITION_NOTEQUAL;

            // get the 2nd operand
            token = lexer.GetToken();
            if (token.token_type == NUM)
            {
                string num = token.lexeme;
                // insert to mem if num is unique.'location' method has a check to
                // maintain uniqueness
                location(num, false);
                ifNode->assign_inst.operand2_index = locationDict[num];
            }
            else if (token.token_type == ID)
            {
                ifNode->assign_inst.operand2_index = locationDict[token.lexeme];
            }

            // insert the ifNode to the instruction list
            head = insertNode(head, last, ifNode);

            // consume a semicolon;
            if (lexer.peek(1).token_type == SEMICOLON)
            {
                lexer.GetToken();
            }

            // get the rest of the if body instructions and then insert them to
            // main head
            InstructionNode *tempHead = parse_generate_intermediate_representation();
            head = insertList(head, last, tempHead);

            //
            InstructionNode *noopNode = new InstructionNode;
            noopNode->type = NOOP;
            head = insertNode(head, last, noopNode);

            //
            ifNode->cjmp_inst.target = noopNode;
        }
        else if (token.token_type == WHILE)
        {
            InstructionNode *whileNode = new InstructionNode;
            whileNode->type = CJMP;

            // get the while statment. Eg d > 1 then operand1 = d operand2 = 1
            token = lexer.GetToken();
            whileNode->cjmp_inst.operand1_index = locationDict[token.lexeme];

            // get the condition
            token = lexer.GetToken();
            if (token.token_type == GREATER)
                whileNode->cjmp_inst.condition_op = CONDITION_GREATER;
            else if (token.token_type == LESS)
                whileNode->cjmp_inst.condition_op = CONDITION_LESS;
            else if (token.token_type == NOTEQUAL)
                whileNode->cjmp_inst.condition_op = CONDITION_NOTEQUAL;

            // get the 2nd operand
            token = lexer.GetToken();
            if (token.token_type == NUM)
            {
                string num = token.lexeme;
                // insert to mem if num is unique.
                // 'location' method has a check for
                // uniqueness
                location(num, false);
                whileNode->assign_inst.operand2_index = locationDict[num];
            }
            else if (token.token_type == ID)
            {
                whileNode->assign_inst.operand2_index = locationDict[token.lexeme];
            }

            // insert the whileNode to the instruction list
            head = insertNode(head, last, whileNode);

            // consume a semicolon;
            if (lexer.peek(1).token_type == SEMICOLON)
            {
                lexer.GetToken();
            }
            // get the rest of the while body instructions and then insert them to
            // main head
            InstructionNode *tempHead = parse_generate_intermediate_representation();
            head = insertList(head, last, tempHead);

            InstructionNode *jmpToWhileNode = new InstructionNode;
            jmpToWhileNode->type = JMP;
            jmpToWhileNode->jmp_inst.target = whileNode;
            head = insertNode(head, last, jmpToWhileNode);

            InstructionNode *noopNode = new InstructionNode;
            noopNode->type = NOOP;
            head = insertNode(head, last, noopNode);

            // When the while loop fails it jumps to this NOOP statement
            whileNode->cjmp_inst.target = noopNode;
        }
        else if (token.token_type == FOR)
        {
            // consume an FOR and LPAREN token
            token = lexer.GetToken();
            token = lexer.GetToken();

            //------------------------------------------------
            // get the first condition. eg a = 0
            InstructionNode *initVar = new InstructionNode;
            initVar->type = ASSIGN;
            initVar->assign_inst.left_hand_side_index = locationDict[token.lexeme];
            initVar->assign_inst.op = OPERATOR_NONE;

            // to to see if the next opertor is an =
            // if it then consume that token and then
            // get the next token
            if (lexer.peek(1).token_type == EQUAL)
            {
                token = lexer.GetToken();
            }

            token = lexer.GetToken();

            // if the token is a variable like a,b,c...
            if (token.token_type == NUM)
            {
                string num = token.lexeme;
                // insert to mem if num is unique.
                // 'location' method has a check for
                // uniqueness
                location(num, false);
                initVar->assign_inst.operand1_index = locationDict[num];
            }
            else if (token.token_type == ID)
            {
                initVar->assign_inst.operand1_index = locationDict[token.lexeme];
            }

            // consume a semicolon
            if (lexer.peek(1).token_type == SEMICOLON)
            {
                lexer.GetToken();
            }

            // insert the condition to the list
            head = insertNode(head, last, initVar);

            //------------------------------------------------
            // get the next condtion. eg a < 10
            InstructionNode *forNode = new InstructionNode;
            forNode->type = CJMP;

            // get the while statment. Eg d > 1 then operand1 = d operand2 = 1
            token = lexer.GetToken();
            forNode->cjmp_inst.operand1_index = locationDict[token.lexeme];

            // get the condition
            token = lexer.GetToken();
            if (token.token_type == GREATER)
                forNode->cjmp_inst.condition_op = CONDITION_GREATER;
            else if (token.token_type == LESS)
                forNode->cjmp_inst.condition_op = CONDITION_LESS;
            else if (token.token_type == NOTEQUAL)
                exit(1); // for loop cannot have a notequal condition

            // get the 2nd operand
            token = lexer.GetToken();
            if (token.token_type == NUM)
            {
                string num = token.lexeme;
                // insert to mem if num is unique.
                // 'location' method has a check for
                // uniqueness
                location(num, false);
                forNode->assign_inst.operand2_index = locationDict[num];
            }
            else if (token.token_type == ID)
            {
                forNode->assign_inst.operand2_index = locationDict[token.lexeme];
            }

            // insert instruction
            head = insertNode(head, last, forNode);
            // consume a semicolon
            if (lexer.peek(1).token_type == SEMICOLON)
            {
                lexer.GetToken();
            }
            token = lexer.GetToken();
            //------------------------------------------------
            // get the incrementing/decrementing condition. eg a = a + 1
            // use the token
            InstructionNode *varChange = new InstructionNode;
            varChange->type = ASSIGN;
            varChange->assign_inst.left_hand_side_index = locationDict[token.lexeme];
            varChange->assign_inst.op = OPERATOR_NONE;

            // to to see if the next opertor is an =
            // if it then consume that token and then
            // get the next token
            if (lexer.peek(1).token_type == EQUAL)
            {
                token = lexer.GetToken();
            }

            token = lexer.GetToken();

            // if the token is a variable like a,b,c...
            if (token.token_type == NUM)
            {
                string num = token.lexeme;
                // insert to mem if num is unique.
                // 'location' method has a check for
                // uniqueness
                location(num, false);
                varChange->assign_inst.operand1_index = locationDict[num];
            }
            else if (token.token_type == ID)
            {
                varChange->assign_inst.operand1_index = locationDict[token.lexeme];
            }

            token = lexer.GetToken();

            if (token.token_type == PLUS)
                varChange->assign_inst.op = OPERATOR_PLUS;
            else if (token.token_type == MINUS)
                varChange->assign_inst.op = OPERATOR_MINUS;
            else if (token.token_type == MULT)
                varChange->assign_inst.op = OPERATOR_MULT;
            else if (token.token_type == DIV)
                varChange->assign_inst.op = OPERATOR_DIV;

            // if the token is a semicolon do not get it
            if (token.token_type != SEMICOLON)
            {
                token = lexer.GetToken();
            }

            if (varChange->assign_inst.op != OPERATOR_NONE)
            {
                if (token.token_type == NUM)
                {
                    string num = token.lexeme;
                    // insert to mem if num is unique.
                    // 'location' method has a check for
                    // uniqueness
                    location(num, false);
                    varChange->assign_inst.operand2_index = locationDict[num];
                }
                else if (token.token_type == ID)
                {
                    varChange->assign_inst.operand2_index = locationDict[token.lexeme];
                }
            }

            // get a semicolon
            if (lexer.peek(1).token_type == SEMICOLON)
            {
                token = lexer.GetToken();
            }

            // get the right paran
            if (lexer.peek(1).token_type == RPAREN)
            {
                token = lexer.GetToken();
            }

            // recurssive get the for loop body and append the instructions
            // to the instruction list
            InstructionNode *tempHead = parse_generate_intermediate_representation();
            head = insertList(head, last, tempHead);

            // insert the increment/decrement node here
            head = insertNode(head, last, varChange);

            // the jump node after all the instructions
            InstructionNode *jmpToforNode = new InstructionNode;
            jmpToforNode->type = JMP;
            jmpToforNode->jmp_inst.target = forNode;
            head = insertNode(head, last, jmpToforNode);

            InstructionNode *noopNode = new InstructionNode;
            noopNode->type = NOOP;
            head = insertNode(head, last, noopNode);

            // When the while loop fails it jumps to this NOOP statement
            forNode->cjmp_inst.target = noopNode;
        }
        else if (token.token_type == SWITCH)
        {
            cout << "token1: " << token.token_type << endl;
            Token condVar = lexer.GetToken();
            bool caseSuccess = false;
            cout << "token2: " << token.token_type << endl;
            if (token.token_type == LBRACE)
            {
                token = lexer.GetToken();
            }
            cout << "token3: " << token.token_type << endl;

            do
            {
                // get the token
                token = lexer.GetToken();
                // if our token is of type CASE we know the next token is the second variable we need
                // to compare our tokens
                if (token.token_type == CASE && lexer.peek(1).lexeme == condVar.lexeme)
                {
                    // get the var
                    token = lexer.GetToken();
                    // get the colon
                    token = lexer.GetToken();
                    // get the body
                    InstructionNode *tempHead = parse_generate_intermediate_representation();
                    head = insertList(head, last, tempHead);
                    caseSuccess = true;
                }

            } while (token.token_type != DEFAULT);

            // if this is not true then we run our default case

            if (!caseSuccess)
            {
            }
        }
    } while (token.token_type != RBRACE);

    // This is the base case for recurssion
    if (lexer.peek(1).token_type != NUM)
    {
        return head;
    }

    // input section
    do
    {
        token = lexer.GetToken();
        if (token.token_type == NUM)
        {
            inputs.push_back(stoi(token.lexeme));
        }
    } while (token.token_type != END_OF_FILE);

    return head;
}
