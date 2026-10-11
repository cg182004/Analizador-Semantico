# Analizador semántico gráfico en C

## Objetivo
Aplicación de escritorio para Windows hecha en **C (Win32 API)**. Permite ingresar código de un **subconjunto didáctico de C**, analizar declaraciones, asignaciones, tipos y expresiones aritméticas, y consultar la tabla de símbolos.

> La consigna menciona tanto «semántico» como «sintáctico». Este proyecto implementa principalmente **análisis semántico**, con comprobaciones sintácticas sencillas (por ejemplo, `;` y paréntesis). No pretende ser un compilador C completo.

## Requisitos
- Windows 10/11.
- Code::Blocks con compilador GCC/MinGW instalado, o MinGW GCC accesible desde CMD.
- No necesita bibliotecas externas: se utiliza la API gráfica Win32.

## Cómo compilar
**Opción A (CMD):** en la carpeta del proyecto ejecutar:

```bat
gcc main.c analizador.c -o AnalizadorSemantico.exe -mwindows -Wall -Wextra -std=c11 -lgdi32 -luser32
```

También puedes hacer doble clic en `compilar.bat` si `gcc` se encuentra en el PATH.

**Opción B (Code::Blocks):** crear proyecto **Win32 GUI project**, elegir **C**, agregar `main.c`, `analizador.c` y `analizador.h`, y verificar que el enlace sea a aplicación **GUI** (opción `-mwindows`). Si se crea por error un proyecto de consola, configurar `-mwindows` en las opciones de enlazador. El punto de entrada es `WinMain`.

## Uso
1. Abre `AnalizadorSemantico.exe`.
2. Escribe el código en el editor izquierdo/superior.
3. Pulsa **Analizar**.
4. Consulta el diagnóstico y la tabla de símbolos.
5. **Cargar ejemplo** inserta código con errores intencionales; **Limpiar** vacía los paneles.

## Lenguaje aceptado para las pruebas
Se aceptan **una declaración o asignación por línea**. Son válidas:

```
int edad = 20;
float precio = 13.5;
char inicial = 'A';
string nombre = "Ana";
int cantidad;
cantidad = 10;
float total = (precio + cantidad) * 2;
```

- Tipos: `int`, `float`, `char`, `string` (`string` es una extensión didáctica; **no existe como tipo nativo en C**).
- Identificadores: letras o `_` al inicio; después letras, números o `_`; máximo 63 caracteres.
- Literales: enteros, decimales, caracteres entre comillas simples y cadenas entre comillas dobles.
- Expresiones: `+`, `-`, `*`, `/`, paréntesis, signos unarios `+`/`-`.
- Reglas: declarar antes de usar; no redeclarar una variable; `float` admite `int`; `int` no admite `float` sin conversión; `char`/`string` solo aceptan su mismo tipo; operaciones aritméticas solo con valores numéricos. Por simplicidad, la división `/` produce tipo `float`.
- Se permiten líneas vacías y comentarios de línea `//` al final o al principio. Las directivas `#` completas se ignoran.

**No soporta:** múltiples sentencias en una misma línea, bloques `{}`, funciones, `if`, `while`, arreglos, punteros, tipos `double`, conversiones explícitas ni comentarios multilínea. El análisis usa una única tabla de símbolos global, sin ámbitos anidados. No ejecuta el código ni calcula resultados.

## Casos de prueba
| Prueba | Entrada | Resultado esperado |
|---|---|---|
| Correcta | `int edad = 20;` | Línea OK, variable registrada |
| Variable duplicada | `int a = 1;` y `int a = 2;` | Error de redeclaración |
| No declarada | `x = 5;` | Error: no declarada |
| Tipos incompatibles | `int x = 2.5;` | Error: `float` a `int` |
| Operación inválida | `int x = 'A' + 3;` | Error: no numérica |
| Error de puntuación | `int x = 5` | Error: falta `;` |
| Promoción válida | `float x = 5;` | Línea OK |

Incluye `pruebas/correcto.txt` y `pruebas/errores.txt` para copiar y pegar.

## Explicación para exposición
1. **Introducción:** el compilador incluye análisis léxico, sintáctico y semántico.
2. **Análisis sintáctico:** revisa la estructura gramatical; aquí solo se comprueban elementos básicos de las sentencias.
3. **Análisis semántico:** revisa el significado de las sentencias: variables declaradas, duplicadas y tipos compatibles.
4. **Tabla de símbolos:** almacena el identificador, el tipo y la línea de declaración.
5. **Demo:** ejecutar un ejemplo correcto, luego un ejemplo incorrecto y mostrar mensajes por línea.
6. **Conclusión:** identificar errores antes de ejecutar ayuda a mejorar la confiabilidad del programa.

## Archivos
- `main.c`: ventana, botones, editor y paneles Win32.
- `analizador.c`: motor del análisis.
- `analizador.h`: interfaz del motor.
- `compilar.bat`: comando de compilación en Windows.
- `pruebas/`: entradas de muestra.

## Publicación en GitHub
El repositorio puede incluir archivos `.c`, `.h`, `.md`, `.bat`, `pruebas/` y, si el docente pide ejecutable, el `.exe` compilado. La subida a GitHub queda a cargo del estudiante.
