# SPL64
A system programming language compiler for the Twin-64 System. The idea is to have a higher level way to write low level code instead of pure assembler language. The compiler will produce an assembler file for the assembler to process.


## Scalar types

```
<typ>               	   ->  <scalar-typ> | <structured-typ>

<scalar-typ>        ->  "BYTE" | "SHORT" | "WORD" | "INT"

<structured-typ>  -> <array-typ> | <record-typ> | <string-type>

<array-typ>         ->  "ARRAY" <const-expr> "OF" <typ>
                        		"END"

<record-typ>        ->  "RECORD" [ "(" <ident> ")" ]
                        		[ <field-list> ]
                        		"END"    

<field-list>            ->  { <ident> ":" <typ> ";" }

<pointer-typ>       ->  "POINTER" "TO" <typ"> ";"

<string-typ>        ->  "STRING" "[“ <const-expr> "]" { ":=" <str-val> } ";"
```

## Declarations

```
<const-decl>        ->  "CONST" <ident> "=" <expr> ";"

<type-decl>         ->  "TYPE"  <ident> "=" <typ"> ";"

<var-decl>          ->  "VAR"   <ident> [ "=" <value> ] ";"

<func-decl>         ->  "FUNC" <ident> 
                        		"(" <parm-list> ")"
                        		[ ":" <typ> ] ";"
                        		<option-list>
                        		<stmt-seq>
                        		"END"

<parm-list>         -> [ <ident> ":" <type> ] { "," <ident> ":" <type> }

<option-list>       -> { "OPTION" <ident> "=" <val> ";" }

<module-decl>       	->  "MODULE"
                        		     [ <import-decl> ] 
                              	     [ <export-decl> ]
                                       [ <var-decl> ]
                                       [ <const-decl> ]
                                       [ <type-decl> ]
                                       [ <func-dec> ]
                        		     "BEGIN"
                        		    <stmt-seq>
                        		   "END"

<import-decl>       	->  "IMPORT <ident> { "," <ident> } ";"

<export-decl>       	->  "EXPORT <ident> { "," <ident> } ";"

<var-decl>  		->  "VAR" <ident> ":" <type> [ ":=" <val> ] ";"

<const-decl>        	->  "CONST" <ident> [  ":" <type> ] ":=" <val> ";"

<func-decl>       	->  "FUNC" <ident> "(" <parm-list> ")" [ ":" <type> ] ";"
                        			<var-decl>
                        			<const-decl>
                        			"BEGIN"
                        			<stmt-seq>
                        			"END"

## Expressions

rework expressions, they are much simpler....

```
<expr>              		->  <simple-expr> <rel-op> <simple-expr>

<rel-op>            	->  ( "==" | "!=" | "<" | ">" | "<=" | ">=" )

<simple-expr>       	->  [ "+" | "-" ] <term> { <termOp> <term> }

<termOp>            	->  "+" | “-“ | “OR“ | "XOR"

<term>              	->  <factor> { <facOp> <factor> }

<facOp>             	->  "*" | “/“ |  "\%“ | "AND"

<factor>            		->  <val> |
                        		    <ident> |
                                     "NOT" "(" <factor> ")"  |
                                     "(" <expr> ")" |
                                     <func-call>

<val>               ->  "<numeric> | 
                        "L%" <val> | 
                        "R%" <val>

<adr-ref>           ->  "@" <ident>

<func-call>         ->  <ident> [ arg-list] ";"

<arg-list>         ->  "(" [ <expr> ] { "," <expr> } ")"


( have ".[pos:len] for bit selector ? )
( have ".field" for records )
( have "[ index ] for arrays )

```

## Statements

```
<stmt-seq>          ->  <stmt> { ";" <stmt> } 

<stmt>              ->  <assign-stmt<> |
                        <if-stmt> | 
                        <loop-stmt> |
                        <with-stmt> |

<assign-stmt>       ->  <ident> ":=" <expr> ";"

<if-stmt>           ->  "IF" <expr> "THEN" <stmt-seq>
                        { "ELSEIF" <expr> "THEN" <stmt-seq> }
                        [ "ELSE" <stmt-seq> ]
                        "END"

<while-stmt>        ->  "WHILE" <expr> DO
                        <statement-seq>
                        { "ELSE" <statement-seq> }
                        "END"

<loop-stmt>         ->  "LOOP" 
                        <loop-stmt-seq> 
                        "END

<loop-stmt-seq>     ->  <stmt-seq> | "CONTINUE" ";" | "BREAK" ";"

<with-stmt>         ->  "WITH" <adr> "DO" 
                        <stmt-seq> 
                        "END"

<repeat-stmt>       ->  "REPEAT" 
                        <stmt-seq>
                        "UNTIL" <expr> "END"

<case-stmt>         ->  "CASE" <expr> "OF" 
                        <case-list>
                        { "ELSE" <stmt-seq> }
                        END

<case-list>         ->  { "WHEN" <const-val> ":" <stmt-seq> ";" }



```

## Example

```
MODULE example 

    IMPORT aModule;
    EXPORTS ;

    CONST maxItem = 10;

    TYPE listEntry = RECORD

        field1 : WORD;
        field2 : BYTE;
        field3 : POINTER TO int;
    END

    TYPE payLoadEntry = STRUCT ( listEntry )

        field4 : INT;
    END

    FUNC add2 ( in1 : WORD; in2: WORD ) : WORD;

        add2 = in1 + in2;
    END
END

```

Alternative: simplify if we mimic an IR type language. We could have predefined types
such as words, bytes, etc. We could explicitly have statements like ENTER or LEAVE in 
a function, and so on.

Many assembler instructions could be be just predefined functions: e.g.
    R1 :-= EXTR( R2, pos, len );
instead of 
    R1 := R2.[pos:len];