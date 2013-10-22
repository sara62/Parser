from __future__ import print_function
import re
from math import fabs,sin,cos,tan,asin,acos,atan,atan2,exp,log,log10,sqrt
from random import uniform
from sys import exit
from time import sleep
try: useSerial = True; from serial import Serial; ser = Serial("COM5", 9600)
except: useSerial = False

running = True
prog    = []
stack   = []
cstack  = []
cline   = 0
token_pat = re.compile("\s*(?:([A-Z]{2,})|([A-Z]|[-]?[\d\.\E]+)|([^ \n]+))")

class value(object): value = 0; isvar = False
class literal(value):
    def __init__(self, value): self.value = value
    def __repr__(self): return "(literal %f)" % self.value
class variable(value):
    isvar = True
    def __init__(self, value, tag): self.value = value; self.tag = tag
    def __repr__(self): return "(variable %s %f)" % (self.tag, self.value)

class control(object): line = -1; type = -1
class if_c(control):
    type = 1
    def __init__(self, branch): self.branch = branch
class gosub_c(control):
    type = 2
    def __init__(self, goline): global cline; self.line = cline; cline = goline
class while_c(control):
    type = 3
    def __init__(self, line, var):
        if var.isvar: self.line = line; self.var = var.tag
        else: raise SyntaxError("While not initialized with a variable")

class operator:
    def __init__(self, argnum, func): self.args = argnum; self.do = func
    def numArgs(self): return self.args

def assign_():
    right,left = stack.pop(), stack.pop()
    if right.isvar: right.value = left.value
    else: raise SyntaxError("Assignment target not a variable")
def add_():     stack[-2] = literal(stack[-2].value+stack[-1].value); stack.pop()
def sub_():     stack[-2] = literal(stack[-2].value-stack[-1].value); stack.pop()
def mul_():     stack[-2] = literal(stack[-2].value*stack[-1].value); stack.pop()
def div_():     stack[-2] = literal(stack[-2].value/stack[-1].value); stack.pop()
def pow_():     stack[-2] = literal(stack[-2].value**stack[-1].value); stack.pop()
def mod_():     stack[-2] = literal(stack[-2].value%stack[-1].value); stack.pop()
def grt_():     stack[-2] = literal(1 if stack[-2].value>stack[-1].value else 0); stack.pop()
def lst_():     stack[-2] = literal(1 if stack[-2].value<stack[-1].value else 0); stack.pop()
def geq_():     stack[-2] = literal(1 if stack[-2].value>=stack[-1].value else 0); stack.pop()
def leq_():     stack[-2] = literal(1 if stack[-2].value<=stack[-1].value else 0); stack.pop()
def eql_():     stack[-2] = literal(1 if stack[-2].value==stack[-1].value else 0); stack.pop()
def neq_():     stack[-2] = literal(1 if stack[-2].value!=stack[-1].value else 0); stack.pop()
def abs_():     stack[-1] = literal(fabs(stack[-1].value))
def sin_():     stack[-1] = literal(sin(stack[-1].value))
def cos_():     stack[-1] = literal(cos(stack[-1].value))
def tan_():     stack[-1] = literal(tan(stack[-1].value))
def asin_():    stack[-1] = literal(asin(stack[-1].value))
def acos_():    stack[-1] = literal(acos(stack[-1].value))
def atan_():    stack[-1] = literal(atan(stack[-1].value))
def atnt_():    stack[-2] = literal(atan2(stack[-2].value,stack[-1].value)); stack.pop()
def exp_():     stack[-1] = literal(exp(stack[-1].value))
def log_():     stack[-1] = literal(log(stack[-1].value))
def logt_():    stack[-1] = literal(log10(stack[-1].value))
def sqrt_():    stack[-1] = literal(sqrt(stack[-1].value))
def goto_():    global cline; cline = int(stack.pop().value-2)
def gosub_():   cstack.append(gosub_c(int(stack.pop().value-2)))
def ret_():
    global cline;
    if len(cstack)>0 and cstack[-1].type == 2: cline = cstack[-1].line; del cstack[-1]
    else: raise SyntaxError("Return statement with no prior gosub")
def if_():      cstack.append(if_c(True if stack.pop().value>0 else False))
def else_():
    if len(cstack)>0 and cstack[-1].type == 1: cstack[-1].branch = not cstack[-1].branch
    else: raise SyntaxError("Else statement with no prior if")
def endif_():
    if len(cstack)>0 and cstack[-1].type == 1: del cstack[-1]
    else: raise SyntaxError("Endif statement with no prior if")
def while_():   cstack.append(while_c(int(cline), stack.pop()))
def loop_():
    global cline
    if len(cstack)>0 and cstack[-1].type == 3:
        if variables[cstack[-1].var].value>0: cline = cstack[-1].line
        else: del cstack[-1]
def break_():
    global cline; type = -1
    for x in reversed(xrange(len(cstack))):
        if cstack[x].type == while_c.type: type = 1; del cstack[x]
    if len(cstack)>0 and type != -1:
        for i in xrange(cline,len(prog)):
            if "LOOP" in prog[i]: cline = i; return
    raise SyntaxError("Break with no prior control statement")
def input_():
    while True:
        try: temp = float(raw_input("Float input -> "))
        except: continue
        else: stack.append(literal(temp)); break
def kill_():    running = False

operators = {
    "=":    operator(2, assign_),
    "+":    operator(2, add_),
    "-":    operator(2, sub_),
    "*":    operator(2, mul_),
    "/":    operator(2, div_),
    "**":   operator(2, pow_),
    "%":    operator(2, mod_),
    ">":    operator(2, grt_),
    "<":    operator(2, lst_),
    ">=":   operator(2, geq_),
    "<=":   operator(2, leq_),
    "==":   operator(2, eql_),
    "<>":   operator(2, neq_),
    "ABS":  operator(1, abs_),
    "SIN":  operator(1, sin_),
    "COS":  operator(1, cos_),
    "TAN":  operator(1, tan_),
    "ASIN": operator(1, asin_),
    "ACOS": operator(1, acos_),
    "ATAN": operator(1, atan_),
    "ATNT": operator(2, atnt_),
    "EXP":  operator(1, exp_),
    "LOG":  operator(1, log_),
    "LOGT": operator(1, logt_),
    "SQRT": operator(1, sqrt_),
    "RAND": operator(0, lambda: stack.append(literal(uniform(0,1)))),
    "GOTO": operator(1, goto_),
    "GOSUB":operator(1, gosub_),
    "RET":  operator(0, ret_),
    "IF":   operator(1, if_),
    "ELSE": operator(0, else_),
    "ENDIF":operator(0, endif_),
    "WHILE":operator(1, while_),
    "LOOP": operator(0, loop_),
    "BREAK":operator(0, break_),
    "INPUT":operator(0, input_),
    "SER":  operator(1, (lambda: ser.write(chr(int(stack.pop().value)))) if useSerial else (lambda: print("Line:",cline+1,stack.pop()))),
    "DUPE": operator(1, lambda: stack.append(stack[-1])),
    "POP":  operator(1, lambda: print("Line:",cline+1,stack.pop())),
    "PEEK": operator(1, lambda: print("Line:",cline+1,stack[-1])),
    "PRNT": operator(0, lambda: print(stack,"\n",cstack)),
    "LINE": operator(0, lambda: print("Line:",cline+1)),
    "WAIT": operator(1, lambda: sleep(stack.pop().value)),
    "QUIT": operator(0, kill_) }

variables = { }
for tag in range(ord('A'), ord('Z')+1): variables[chr(tag)] = variable(0.0, chr(tag))

def parseRPN(program):
    for function, number, symbol in token_pat.findall(program):
        oper = function if function != "" else (symbol if symbol != "" else ""); i = -1
        for x in reversed(xrange(len(cstack))):
            if cstack[x].type == if_c.type: i = x
        if i == -1 or oper in ["ELSE", "ENDIF"] or cstack[i].branch == True:
            if number:
                if len(number) == 1 and 64 < ord(number) < 91: stack.append(variables[number])
                else: stack.append(literal(float(number)))
            elif oper in operators:
                if len(stack) >= operators[oper].numArgs():
                    operators[oper].do()
                else: raise SyntaxError("Not enough pushed values for computation!")
            elif oper == "REM": return
            elif oper == "PRINT": print(program[program.index("PRINT")+6:]); return
            else: raise SyntaxError("Unknown operator %s" % oper)
        else: continue
    return

def runProg():
    global cline, running; cline = 0
    while True:
        try: parseRPN(prog[cline])
        except SyntaxError as e: print("Line:", cline+1,e); break
        except IndexError as e: print("Program terminated normally."); break

        if running == False: running = True; break
        else: cline += 1

def reset():
    global running
    del stack[:]; del prog[:]; del cstack[:]; running = True
    for var in variables: variables[var].value = 0

while True:
    file = raw_input("Program? -> ")
    if file.lower() == "exit": exit()
    elif file.lower()[:5] == "load ":
        reset(); print("Loading...")
        try:
            for line in open(file[5:], "r"): prog.append(line)
        except IOError as e: print(e); continue
    elif file.lower() == "run": runProg()
    elif file.lower() == "clear": reset(); print("Program cleared")
    elif file.lower() == "list":
        for line in prog: print(line)
    elif file.lower() == "help": print("Commands: EXIT, LOAD, RUN, CLEAR, LIST, HELP")
