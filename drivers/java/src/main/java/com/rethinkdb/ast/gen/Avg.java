// Autogenerated by metajava.py.
// Do not edit this file directly.
// The template for this file is located at:
// ../../../../../../../../templates/AstSubclass.java
package com.rethinkdb.ast.gen;

import java.util.Optional;
import com.rethinkdb.ast.ReqlAst;
import com.rethinkdb.ast.helper.Arguments;
import com.rethinkdb.ast.helper.OptArgs;
import com.rethinkdb.proto.TermType;



public class Avg extends ReqlQuery {


    public Avg(java.lang.Object arg) {
        this(new Arguments(arg), null);
    }
    public Avg(Arguments args, OptArgs optargs) {
        this(null, args, optargs);
    }
    public Avg(ReqlAst prev, Arguments args, OptArgs optargs) {
        this(prev, TermType.AVG, args, optargs);
    }
    protected Avg(ReqlAst previous, TermType termType, Arguments args, OptArgs optargs){
        super(previous, termType, args, optargs);
    }


    /* Static factories */
    public static Avg fromArgs(Object... args){
        return new Avg(new Arguments(args), null);
    }


}