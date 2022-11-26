#ifndef NUMERISEUR_H
#define NUMERISEUR_H

#include <png.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "grille.h"


grille * read_png_to_grille (const char * png_file);

#endif