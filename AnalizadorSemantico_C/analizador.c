#include "analizador.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#define MAX_SIMBOLOS 200
#define MAX_NOMBRE 64
#define MAX_LINEA 1024

typedef enum { T_INVALIDO=-1, T_INT, T_FLOAT, T_CHAR, T_STRING } Tipo;
typedef struct {char nombre[MAX_NOMBRE]; Tipo tipo; int linea;} Simbolo;
typedef struct {
    Simbolo simbolos[MAX_SIMBOLOS]; int ns, errores, lineas, linea;
    const char *cursor; char *salida, *tabla; size_t salida_cap, tabla_cap;
} Contexto;

static const char *nombre_tipo(Tipo t) {
    switch(t) {case T_INT:return "int"; case T_FLOAT:return "float"; case T_CHAR:return "char"; case T_STRING:return "string"; default:return "invalido";}
}
static void anexar(char *dst, size_t cap, const char *fmt, ...) {
    size_t n = strlen(dst); va_list ap;
    if(n>=cap) return;
    va_start(ap,fmt); vsnprintf(dst+n,cap-n,fmt,ap); va_end(ap);
}
static void error(Contexto *c, const char *fmt, ...) {
    char mensaje[320]; va_list ap; va_start(ap,fmt);
    vsnprintf(mensaje,sizeof(mensaje),fmt,ap); va_end(ap);
    anexar(c->salida,c->salida_cap,"Linea %d: ERROR: %s\r\n",c->linea,mensaje); c->errores++;
}
static void espacios(Contexto *c){while(*c->cursor==' '||*c->cursor=='\t'||*c->cursor=='\r') c->cursor++;}
static int identificador(Contexto *c, char *s){int n=0; espacios(c);
    if(!isalpha((unsigned char)*c->cursor)&&*c->cursor!='_')return 0;
    while(isalnum((unsigned char)*c->cursor)||*c->cursor=='_'){
        if(n<MAX_NOMBRE-1)s[n++]=*c->cursor;
        c->cursor++;
    }
    s[n]=0;return 1;
}
static int buscar(Contexto *c,const char *nombre){int i;for(i=0;i<c->ns;i++)if(!strcmp(c->simbolos[i].nombre,nombre))return i;return -1;}
static int numerico(Tipo t){return t==T_INT||t==T_FLOAT;}
static Tipo expresion(Contexto *c);
static Tipo primario(Contexto *c){char nombre[MAX_NOMBRE]; int i; Tipo t; espacios(c);
    if(*c->cursor=='('){c->cursor++;t=expresion(c);espacios(c);if(*c->cursor==')')c->cursor++;else error(c,"falta ')' en expresion");return t;}
    if(*c->cursor=='-'||*c->cursor=='+'){c->cursor++;t=primario(c);if(t!=T_INVALIDO&&!numerico(t)){error(c,"signo + o - aplicado a tipo no numerico");return T_INVALIDO;}return t;}
    if(*c->cursor=='\''){
        c->cursor++;if(!*c->cursor){error(c,"literal char incompleto");return T_INVALIDO;}
        if(*c->cursor=='\\'&&c->cursor[1])c->cursor+=2;else c->cursor++;
        if(*c->cursor!='\''){error(c,"literal char invalido");return T_INVALIDO;}
        c->cursor++;return T_CHAR;
    }
    if(*c->cursor=='"'){
        c->cursor++;while(*c->cursor&&*c->cursor!='"'){if(*c->cursor=='\\'&&c->cursor[1])c->cursor++;c->cursor++;}
        if(*c->cursor!='"'){error(c,"cadena sin cerrar");return T_INVALIDO;}c->cursor++;return T_STRING;
    }
    if(isdigit((unsigned char)*c->cursor)||(*c->cursor=='.'&&isdigit((unsigned char)c->cursor[1]))){int puntos=0;
        while(isdigit((unsigned char)*c->cursor)||*c->cursor=='.'){if(*c->cursor=='.')puntos++;c->cursor++;}
        if(puntos>1){error(c,"numero mal formado");return T_INVALIDO;}
        return puntos?T_FLOAT:T_INT;
    }
    if(identificador(c,nombre)){
        i=buscar(c,nombre);if(i<0){error(c,"variable '%s' no declarada",nombre);return T_INVALIDO;}
        return c->simbolos[i].tipo;
    }
    error(c,"se esperaba un valor, variable o expresion");return T_INVALIDO;
}
static Tipo termino(Contexto *c){Tipo a=primario(c),b;char op;while(1){espacios(c);op=*c->cursor;if(op!='*'&&op!='/')break;c->cursor++;b=primario(c);
    if(a==T_INVALIDO||b==T_INVALIDO){a=T_INVALIDO;continue;}
    if(!numerico(a)||!numerico(b)){error(c,"operacion aritmetica entre tipos no numericos");a=T_INVALIDO;}
    else a=(a==T_FLOAT||b==T_FLOAT||op=='/')?T_FLOAT:T_INT;
}return a;}
static Tipo expresion(Contexto *c){Tipo a=termino(c),b;char op;while(1){espacios(c);op=*c->cursor;if(op!='+'&&op!='-')break;c->cursor++;b=termino(c);
    if(a==T_INVALIDO||b==T_INVALIDO){a=T_INVALIDO;continue;}
    if(!numerico(a)||!numerico(b)){error(c,"operacion aritmetica entre tipos no numericos");a=T_INVALIDO;}
    else a=(a==T_FLOAT||b==T_FLOAT)?T_FLOAT:T_INT;
}return a;}
static void analizar_linea(Contexto *c, const char *linea){char nombre[MAX_NOMBRE];Tipo tipo=T_INVALIDO, valor;int decl=0, i, inicio_errores=c->errores;
    c->cursor=linea;espacios(c);if(!*c->cursor||*c->cursor=='#'||(c->cursor[0]=='/'&&c->cursor[1]=='/'))return;
    if(!strncmp(c->cursor,"int",3)&&isspace((unsigned char)c->cursor[3])){tipo=T_INT;c->cursor+=3;decl=1;}
    else if(!strncmp(c->cursor,"float",5)&&isspace((unsigned char)c->cursor[5])){tipo=T_FLOAT;c->cursor+=5;decl=1;}
    else if(!strncmp(c->cursor,"char",4)&&isspace((unsigned char)c->cursor[4])){tipo=T_CHAR;c->cursor+=4;decl=1;}
    else if(!strncmp(c->cursor,"string",6)&&isspace((unsigned char)c->cursor[6])){tipo=T_STRING;c->cursor+=6;decl=1;}
    if(!identificador(c,nombre)){error(c,"se esperaba nombre de variable");return;}
    i=buscar(c,nombre);
    if(decl){if(i>=0){error(c,"variable '%s' ya declarada (linea %d)",nombre,c->simbolos[i].linea);return;}
        if(c->ns>=MAX_SIMBOLOS){error(c,"tabla de simbolos llena");return;}
        i=c->ns++;strcpy(c->simbolos[i].nombre,nombre);c->simbolos[i].tipo=tipo;c->simbolos[i].linea=c->linea;
    }else if(i<0){error(c,"variable '%s' no declarada",nombre);return;}
    espacios(c);
    if(*c->cursor=='='){
        c->cursor++;valor=expresion(c);
        if(valor!=T_INVALIDO&&valor!=c->simbolos[i].tipo&&!(c->simbolos[i].tipo==T_FLOAT&&valor==T_INT))
            error(c,"tipos incompatibles: '%s' es %s, expresion es %s",nombre,nombre_tipo(c->simbolos[i].tipo),nombre_tipo(valor));
    }else if(*c->cursor!=';'){error(c,"se esperaba '=' o ';'");return;}
    espacios(c);if(*c->cursor!=';'){error(c,"falta ';' al final de la instruccion");return;}
    c->cursor++;espacios(c);
    if(*c->cursor&&strncmp(c->cursor,"//",2))error(c,"texto inesperado despues de ';'");
    if(inicio_errores==c->errores)anexar(c->salida,c->salida_cap,"Linea %d: OK\r\n",c->linea);
}
void analizar_codigo(const char *codigo,char *salida,size_t cap_salida,char *tabla,size_t cap_tabla){
    Contexto c; char linea[MAX_LINEA];const char *p=codigo;int i,n;
    memset(&c,0,sizeof(c));c.salida=salida;c.tabla=tabla;c.salida_cap=cap_salida;c.tabla_cap=cap_tabla;
    if(!cap_salida||!cap_tabla)return;
    salida[0]=tabla[0]=0;
    while(*p){n=0;c.linea++;c.lineas++;
        while(*p&&*p!='\n'){if(n<MAX_LINEA-1)linea[n++]=*p;p++;}
        linea[n]=0;if(*p=='\n')p++;
        if(n==MAX_LINEA-1){error(&c,"linea demasiado larga");continue;}
        analizar_linea(&c,linea);
    }
    if(!c.lineas)anexar(salida,cap_salida,"No se ingreso codigo.\r\n");
    anexar(salida,cap_salida,"\r\nRESULTADO: %d error(es) detectado(s).\r\n",c.errores);
    if(!c.errores&&c.lineas)anexar(salida,cap_salida,"Analisis completado correctamente.\r\n");
    anexar(tabla,cap_tabla,"VARIABLE                 TIPO       LINEA\r\n");
    anexar(tabla,cap_tabla,"------------------------------------------------\r\n");
    for(i=0;i<c.ns;i++)anexar(tabla,cap_tabla,"%-24s %-10s %d\r\n",c.simbolos[i].nombre,nombre_tipo(c.simbolos[i].tipo),c.simbolos[i].linea);
    if(!c.ns)anexar(tabla,cap_tabla,"(sin variables)\r\n");
}
