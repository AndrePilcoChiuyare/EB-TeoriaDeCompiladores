grammar expr;

prog
    : stat+;

stat
    : expr NEWLINE                                      # printExpr
    | ID '=' expr NEWLINE                               # assign
    | NEWLINE                                           # blank
    ;

expr
    : expr op=('*'|'/') expr                            # MulDiv
    | expr op=('+'|'-') expr                            # AddSub
    | expr op=('<'|'>'|'>='|'<='| '=='|'!=') expr       # Cond
    | NUMBER                                            # Number
    | ID                                                # Id
    | IF expr THEN expr ELSE expr                       # IfElse
    | IF expr THEN expr                                 # If
    ;

MUL     : '*' ;
DIV     : '/' ;
ADD     : '+' ;
SUB     : '-' ;
GR      : '>' ;
LS      : '<' ;
GRE     : '>=';
LSE     : '<=';
EQ      : '==';
NEQ     : '!=';

IF      : 'if' ;
THEN    : 'then' ;
ELSE    : 'else' ;
ID      : [a-zA-Z]+ ;
NUMBER  : [+-]?([0-9]*[.])?[0-9]+ ;
NEWLINE : '\r'? '\n' ;
WS      : [ \t]+ -> skip ;