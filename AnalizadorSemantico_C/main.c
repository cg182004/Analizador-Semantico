#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "analizador.h"

#define ID_EDITOR 101
#define ID_RESULTADOS 102
#define ID_TABLA 103
#define ID_ANALIZAR 104
#define ID_LIMPIAR 106

static HWND editor, resultados, tabla, botonAnalizar, botonLimpiar;
static HWND tituloEditor, tituloResultados, tituloTabla;
static HFONT fuente, fuenteMono;
static HWND crear(HWND parent,const char *clase,const char *texto,DWORD estilo,int id){
    HWND h=CreateWindowExA((id==ID_EDITOR||id==ID_RESULTADOS||id==ID_TABLA)?WS_EX_CLIENTEDGE:0,
        clase,texto,WS_CHILD|WS_VISIBLE|estilo,0,0,100,24,parent,(HMENU)(INT_PTR)id,GetModuleHandleA(NULL),NULL);
    SendMessageA(h,WM_SETFONT,(WPARAM)((id==ID_EDITOR||id==ID_RESULTADOS||id==ID_TABLA)?fuenteMono:fuente),TRUE);
    return h;
}
static void redimensionar(int w,int alto){
    int m=16, gap=10, b=36, leyenda=20, medio, ancho;
    if(w<600)w=600;
    if(alto<350)alto=350;
    medio=(alto-2*m-b-2*gap-leyenda)/2;
    ancho=w-2*m;
    MoveWindow(tituloEditor,m,m,ancho,leyenda,TRUE);
    MoveWindow(editor,m,m+leyenda,ancho,medio,TRUE);
    MoveWindow(botonAnalizar,m,m+leyenda+medio+gap,140,b,TRUE);
    MoveWindow(botonLimpiar,m+150,m+leyenda+medio+gap,130,b,TRUE);
    MoveWindow(tituloResultados,m,m+leyenda+medio+gap+b+gap,ancho/2-gap,leyenda,TRUE);
    MoveWindow(tituloTabla,m+ancho/2,m+leyenda+medio+gap+b+gap,ancho/2,leyenda,TRUE);
    MoveWindow(resultados,m,m+2*leyenda+medio+gap+b+gap,ancho/2-gap,alto-(m+2*leyenda+medio+gap+b+gap)-m,TRUE);
    MoveWindow(tabla,m+ancho/2,m+2*leyenda+medio+gap+b+gap,ancho/2,alto-(m+2*leyenda+medio+gap+b+gap)-m,TRUE);
}
static LRESULT CALLBACK ventana(HWND h,UINT msg,WPARAM w,LPARAM l){
    switch(msg){
        case WM_CREATE:
            fuente=CreateFontA(-18,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,"Segoe UI");
            fuenteMono=CreateFontA(-17,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,FIXED_PITCH,"Consolas");
            tituloEditor=crear(h,"STATIC","CODIGO FUENTE (C simplificado)",0,0);
            tituloResultados=crear(h,"STATIC","DIAGNOSTICO SEMANTICO",0,0);
            tituloTabla=crear(h,"STATIC","TABLA DE SIMBOLOS",0,0);
            editor=crear(h,"EDIT","",ES_MULTILINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL|WS_VSCROLL|WS_HSCROLL|ES_WANTRETURN|WS_TABSTOP,ID_EDITOR);
            resultados=crear(h,"EDIT","",ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL,ID_RESULTADOS);
            tabla=crear(h,"EDIT","",ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL|WS_HSCROLL|ES_AUTOHSCROLL,ID_TABLA);
            botonAnalizar=crear(h,"BUTTON","Analizar",BS_PUSHBUTTON,ID_ANALIZAR);
            botonLimpiar=crear(h,"BUTTON","Limpiar",BS_PUSHBUTTON,ID_LIMPIAR);
            return 0;
        case WM_SIZE:redimensionar(LOWORD(l),HIWORD(l));return 0;
        case WM_COMMAND:
            switch(LOWORD(w)){
                case ID_ANALIZAR:{
                    int longitud=GetWindowTextLengthA(editor);
                    char *texto=(char*)malloc((size_t)longitud+1);
                    char *salida=(char*)calloc(1,131072), *simbolos=(char*)calloc(1,32768);
                    if(!texto||!salida||!simbolos){MessageBoxA(h,"No hay memoria suficiente.","Error",MB_ICONERROR);free(texto);free(salida);free(simbolos);return 0;}
                    GetWindowTextA(editor,texto,longitud+1);
                    analizar_codigo(texto,salida,131072,simbolos,32768);
                    SetWindowTextA(resultados,salida);SetWindowTextA(tabla,simbolos);
                    free(texto);free(salida);free(simbolos);return 0;
                }
                case ID_LIMPIAR:SetWindowTextA(editor,"");SetWindowTextA(resultados,"");SetWindowTextA(tabla,"");return 0;
            }
            break;
        case WM_DESTROY:DeleteObject(fuente);DeleteObject(fuenteMono);PostQuitMessage(0);return 0;
    }
    return DefWindowProcA(h,msg,w,l);
}
int WINAPI WinMain(HINSTANCE inst,HINSTANCE previo,LPSTR cmd,int show){
    WNDCLASSA wc={0};HWND h;MSG msg;(void)previo;(void)cmd;
    wc.lpfnWndProc=ventana;wc.hInstance=inst;wc.lpszClassName="AnalizadorSemanticoGUI";
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);wc.hIcon=LoadIcon(NULL,IDI_APPLICATION);
    wc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
    if(!RegisterClassA(&wc))return 1;
    h=CreateWindowA(wc.lpszClassName,"Analizador semantico - Proyecto en C",WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                    CW_USEDEFAULT,CW_USEDEFAULT,1040,730,NULL,NULL,inst,NULL);
    if(!h)return 1;
    ShowWindow(h,show);UpdateWindow(h);
    while(GetMessageA(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageA(&msg);}return (int)msg.wParam;
}
