/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 */
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cstdarg>
#include <cctype>
#include <cstring>
#include <string>
#include "compiler.h"
#include "lexer.h"
#include <map>

using namespace std;

#define DEBUG 1 // 1 => Turn ON debugging, 0 => Turn OFF debugging

int mem[1000];
int next_available = 0;

vector<int> inputs;
int next_input = 0;

void debug(const char *format, ...)
{
    va_list args;
    if (DEBUG)
    {
        va_start(args, format);
        vfprintf(stdout, format, args);
        va_end(args);
    }
}

void execute_program(struct InstructionNode *program)
{
    struct InstructionNode *pc = program;
    int op1, op2, result;

    while (pc != NULL)
    {
        switch (pc->type)
        {
        case NOOP:
            pc = pc->next;
            break;
        case IN:
            mem[pc->input_inst.var_index] = inputs[next_input];
            next_input++;
            pc = pc->next;
            break;
        case OUT:
            printf("%d ", mem[pc->output_inst.var_index]);
            pc = pc->next;
            break;
        case ASSIGN:
            switch (pc->assign_inst.op)
            {
            case OPERATOR_PLUS:
                op1 = mem[pc->assign_inst.operand1_index];
                op2 = mem[pc->assign_inst.operand2_index];
                result = op1 + op2;
                break;
            case OPERATOR_MINUS:
                op1 = mem[pc->assign_inst.operand1_index];
                op2 = mem[pc->assign_inst.operand2_index];
                result = op1 - op2;
                break;
            case OPERATOR_MULT:
                op1 = mem[pc->assign_inst.operand1_index];
                op2 = mem[pc->assign_inst.operand2_index];
                result = op1 * op2;
                break;
            case OPERATOR_DIV:
                op1 = mem[pc->assign_inst.operand1_index];
                op2 = mem[pc->assign_inst.operand2_index];
                result = op1 / op2;
                break;
            case OPERATOR_NONE:
                op1 = mem[pc->assign_inst.operand1_index];
                result = op1;
                break;
            }
            mem[pc->assign_inst.left_hand_side_index] = result;
            pc = pc->next;
            break;
        case CJMP:
            if (pc->cjmp_inst.target == NULL)
            {
                debug("Error: pc->cjmp_inst->target is null.\n");
                exit(1);
            }
            op1 = mem[pc->cjmp_inst.operand1_index];
            op2 = mem[pc->cjmp_inst.operand2_index];
            switch (pc->cjmp_inst.condition_op)
            {
            case CONDITION_GREATER:
                if (op1 > op2)
                    pc = pc->next;
                else
                    pc = pc->cjmp_inst.target;
                break;
            case CONDITION_LESS:
                if (op1 < op2)
                    pc = pc->next;
                else
                    pc = pc->cjmp_inst.target;
                break;
            case CONDITION_NOTEQUAL:
                if (op1 != op2)
                    pc = pc->next;
                else
                    pc = pc->cjmp_inst.target;
                break;
            }
            break;
        case JMP:

            if (pc->jmp_inst.target == NULL)
            {
                debug("Error: pc->jmp_inst->target is null.\n");
                exit(1);
            }
            pc = pc->jmp_inst.target;
            break;
        default:
            debug("Error: invalid value for pc->type (%d).\n", pc->type);
            exit(1);
            break;
        }
    }
}

// int location(string str, bool isVar)
// {
//     // make the variables are unique inside mem
//     if (locationDict.count(str) <= 0)
//     {
//         locationDict[str] = next_available;

//         if (isVar)
//         {
//             mem[next_available] = 0;
//         }
//         else
//         {
//             mem[next_available] = stoi(str);
//         }
//         next_available++;
//     }
//     return next_available;
// }

// struct InstructionNode *insertNode(InstructionNode *head, InstructionNode *&last, InstructionNode *newNode)
// {

//     if (head == nullptr)
//     {
//         head = newNode;
//         last = newNode;
//     }
//     else
//     {
//         last->next = newNode;
//         last = newNode;
//     }

//     newNode->next = nullptr;
//     return head;
// }

// struct InstructionNode *insertList(InstructionNode *head1, InstructionNode *&last, InstructionNode *head2)
// {
//     if (head1 == nullptr)
//     {
//         head1 = head2;
//         last = head2;
//         while (last != nullptr && last->next != nullptr)
//         {
//             last = last->next;
//         }
//     }
//     else
//     {
//         last->next = head2;

//         if (head2 != nullptr)
//         {
//             last = head2;
//             while (last->next != nullptr)
//             {
//                 last = last->next;
//             }
//         }
//     }

//     return head1;
// }

// struct InstructionNode *parse_generate_intermediate_representation()
// {
//     Token token;
//     InstructionNode *head = nullptr;
//     InstructionNode *last = nullptr;

//     // get the var_section
//     do
//     {
//         token = lexer.GetToken();
//         // if the token type is LBRACE then that means
//         // this method was called from a recurssive method
//         // in WHILE or IF. At that point we already have the
//         // var body so break
//         if (token.token_type == LBRACE)
//         {
//             break;
//         }
//         if (token.token_type == ID)
//         {
//             location(token.lexeme, true);
//         }

//     } while (token.token_type != SEMICOLON);

//     // comsume the semicolon
//     if (lexer.peek(1).token_type == SEMICOLON)
//     {
//         lexer.GetToken();
//     }

//     // get the body section
//     do
//     {
//         // if the next token is semicolon then consume the token
//         if (lexer.peek(1).token_type == SEMICOLON)
//         {
//             token = lexer.GetToken();
//         }
//         token = lexer.GetToken();

//         // if the token type is INPUT
//         if (token.token_type == INPUT)
//         {
//             token = lexer.GetToken();
//             InstructionNode *newNode = new InstructionNode;
//             newNode->type = IN;
//             newNode->input_inst.var_index = locationDict[token.lexeme];
//             // insert the newNode
//             head = insertNode(head, last, newNode);
//         }
//         else if (token.token_type == ID)
//         {
//             // use the token
//             InstructionNode *newNode = new InstructionNode;
//             newNode->type = ASSIGN; // c = 10
//             newNode->assign_inst.left_hand_side_index = locationDict[token.lexeme];
//             newNode->assign_inst.op = OPERATOR_NONE;

//             // to to see if the next opertor is an =
//             // if it then consume that token and then
//             // get the next token
//             if (lexer.peek(1).token_type == EQUAL)
//             {
//                 token = lexer.GetToken();
//             }

//             token = lexer.GetToken();

//             // if the token is a variable like a,b,c...
//             if (token.token_type == NUM)
//             {
//                 string num = token.lexeme;
//                 // insert to mem if num is unique.
//                 // 'location' method has a check for
//                 // uniqueness
//                 location(num, false);
//                 newNode->assign_inst.operand1_index = locationDict[num];
//             }
//             else if (token.token_type == ID)
//             {
//                 newNode->assign_inst.operand1_index = locationDict[token.lexeme];
//             }

//             token = lexer.GetToken();

//             if (token.token_type == PLUS)
//                 newNode->assign_inst.op = OPERATOR_PLUS;
//             else if (token.token_type == MINUS)
//                 newNode->assign_inst.op = OPERATOR_MINUS;
//             else if (token.token_type == MULT)
//                 newNode->assign_inst.op = OPERATOR_MULT;
//             else if (token.token_type == DIV)
//                 newNode->assign_inst.op = OPERATOR_DIV;

//             // if the token is a semicolon do not get it
//             if (token.token_type != SEMICOLON)
//             {
//                 token = lexer.GetToken();
//             }

//             if (newNode->assign_inst.op != OPERATOR_NONE)
//             {
//                 if (token.token_type == NUM)
//                 {
//                     string num = token.lexeme;
//                     // insert to mem if num is unique.
//                     // 'location' method has a check for
//                     // uniqueness
//                     location(num, false);
//                     newNode->assign_inst.operand2_index = locationDict[num];
//                 }
//                 else if (token.token_type == ID)
//                 {
//                     newNode->assign_inst.operand2_index = locationDict[token.lexeme];
//                 }
//             }

//             // insert the new node
//             head = insertNode(head, last, newNode);
//         }
//         else if (token.token_type == OUTPUT)
//         {
//             token = lexer.GetToken();
//             InstructionNode *newNode = new InstructionNode;
//             newNode->type = OUT;
//             newNode->output_inst.var_index = locationDict[token.lexeme];
//             head = insertNode(head, last, newNode);
//         }
//         else if (token.token_type == IF)
//         {
//             //  i17->type = CJMP; // if d > 1
//             //  i17->cjmp_inst.condition_op = CONDITION_GREATER;
//             //  i17->cjmp_inst.operand1_index = address_d;
//             //  i17->cjmp_inst.operand2_index = address_one;
//             //  i17->cjmp_inst.target = i19; // if not (d > 1) skip body of IF
//             //  i17->next = i18;

//             InstructionNode *ifNode = new InstructionNode;
//             ifNode->type = CJMP;

//             // get the while statment. Eg d > 1 then operand1 = d operand2 = 1
//             token = lexer.GetToken();
//             ifNode->cjmp_inst.operand1_index = locationDict[token.lexeme];

//             // get the condition
//             token = lexer.GetToken();
//             if (token.token_type == GREATER)
//                 ifNode->cjmp_inst.condition_op = CONDITION_GREATER;
//             else if (token.token_type == LESS)
//                 ifNode->cjmp_inst.condition_op = CONDITION_LESS;
//             else if (token.token_type == NOTEQUAL)
//                 ifNode->cjmp_inst.condition_op = CONDITION_NOTEQUAL;

//             // get the 2nd operand
//             token = lexer.GetToken();
//             if (token.token_type == NUM)
//             {
//                 string num = token.lexeme;
//                 // insert to mem if num is unique.'location' method has a check to
//                 // maintain uniqueness
//                 location(num, false);
//                 ifNode->assign_inst.operand2_index = locationDict[num];
//             }
//             else if (token.token_type == ID)
//             {
//                 ifNode->assign_inst.operand2_index = locationDict[token.lexeme];
//             }

//             // insert the ifNode to the instruction list
//             head = insertNode(head, last, ifNode);

//             // consume a semicolon;
//             if (lexer.peek(1).token_type == SEMICOLON)
//             {
//                 lexer.GetToken();
//             }

//             // get the rest of the if body instructions and then insert them to
//             // main head
//             InstructionNode *tempHead = parse_generate_intermediate_representation();
//             head = insertList(head, last, tempHead);

//             //
//             InstructionNode *noopNode = new InstructionNode;
//             noopNode->type = NOOP;
//             head = insertNode(head, last, noopNode);

//             //
//             ifNode->cjmp_inst.target = noopNode;
//         }
//         else if (token.token_type == WHILE)
//         {
//             InstructionNode *whileNode = new InstructionNode;
//             whileNode->type = CJMP;

//             // get the while statment. Eg d > 1 then operand1 = d operand2 = 1
//             token = lexer.GetToken();
//             whileNode->cjmp_inst.operand1_index = locationDict[token.lexeme];

//             // get the condition
//             token = lexer.GetToken();
//             if (token.token_type == GREATER)
//                 whileNode->cjmp_inst.condition_op = CONDITION_GREATER;
//             else if (token.token_type == LESS)
//                 whileNode->cjmp_inst.condition_op = CONDITION_LESS;
//             else if (token.token_type == NOTEQUAL)
//                 whileNode->cjmp_inst.condition_op = CONDITION_NOTEQUAL;

//             // get the 2nd operand
//             token = lexer.GetToken();
//             if (token.token_type == NUM)
//             {
//                 string num = token.lexeme;
//                 // insert to mem if num is unique.
//                 // 'location' method has a check for
//                 // uniqueness
//                 location(num, false);
//                 whileNode->assign_inst.operand2_index = locationDict[num];
//             }
//             else if (token.token_type == ID)
//             {
//                 whileNode->assign_inst.operand2_index = locationDict[token.lexeme];
//             }

//             // insert the whileNode to the instruction list
//             head = insertNode(head, last, whileNode);

//             // consume a semicolon;
//             if (lexer.peek(1).token_type == SEMICOLON)
//             {
//                 lexer.GetToken();
//             }
//             // get the rest of the while body instructions and then insert them to
//             // main head
//             InstructionNode *tempHead = parse_generate_intermediate_representation();
//             head = insertList(head, last, tempHead);

//             InstructionNode *jmpToWhileNode = new InstructionNode;
//             jmpToWhileNode->type = JMP;
//             jmpToWhileNode->jmp_inst.target = whileNode;
//             head = insertNode(head, last, jmpToWhileNode);

//             InstructionNode *noopNode = new InstructionNode;
//             noopNode->type = NOOP;
//             head = insertNode(head, last, noopNode);

//             // When the while loop fails it jumps to this NOOP statement
//             whileNode->cjmp_inst.target = noopNode;
//         }
//     } while (token.token_type != RBRACE);

//     // This is the base case for recurssion
//     if (lexer.peek(1).token_type != NUM)
//     {
//         return head;
//     }

//     // input section
//     do
//     {
//         token = lexer.GetToken();
//         if (token.token_type == NUM)
//         {
//             inputs.push_back(stoi(token.lexeme));
//         }

//     } while (token.token_type != END_OF_FILE);

//     return head;
// }

int main()
{
    struct InstructionNode *program;
    program = parse_generate_intermediate_representation();
    execute_program(program);
    return 0;
}
